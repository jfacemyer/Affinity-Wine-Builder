/* OpenCL event-callback hang repro (Affinity-Wine-Builder, wine-11.18).
 *
 * Affinity's startup pattern: register a completion callback, then wait for
 * it WITHOUT making further OpenCL calls. The 11.18 bridge only fires
 * callbacks as a side effect of later OpenCL calls, so the wait hangs and
 * OpenCL has to stay disabled.
 *
 * This test reproduces that pattern deterministically with a user event
 * (no GPU kernel needed): main thread registers CL_COMPLETE callback and
 * polls a flag with Sleep() only; a setter thread completes the event.
 * Old bridge -> TIMEOUT/FAIL. Pumper-thread bridge -> PASS.
 *
 * Build (Windows/Wine): x86_64-w64-mingw32-gcc -O2 -o ocl-repro.exe ocl-repro.c
 */
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

/* Minimal OpenCL 1.2 types (runtime-linked; no CL headers needed). */
typedef struct _cl_platform_id *cl_platform_id;
typedef struct _cl_device_id *cl_device_id;
typedef struct _cl_context *cl_context;
typedef struct _cl_event *cl_event;
typedef int32_t cl_int;
typedef uint32_t cl_uint;
typedef intptr_t cl_context_properties;
typedef uint64_t cl_device_type;
#define CL_API_CALL __stdcall
#define CL_CALLBACK __stdcall
#define CL_SUCCESS 0
#define CL_COMPLETE 0x0
#define CL_DEVICE_TYPE_ALL 0xFFFFFFFFull

#define CHECK(fn, what) do { if (!(fn)) { printf("SKIP: cannot load " what "\n"); return 2; } } while (0)

typedef cl_int (CL_API_CALL *PFN_GetPlatformIDs)(cl_uint, cl_platform_id *, cl_uint *);
typedef cl_int (CL_API_CALL *PFN_GetDeviceIDs)(cl_platform_id, cl_device_type, cl_uint, cl_device_id *, cl_uint *);
typedef cl_context (CL_API_CALL *PFN_CreateContext)(const cl_context_properties *, cl_uint, const cl_device_id *,
        void (CL_CALLBACK *)(const char *, const void *, size_t, void *), void *, cl_int *);
typedef cl_event (CL_API_CALL *PFN_CreateUserEvent)(cl_context, cl_int *);
typedef cl_int (CL_API_CALL *PFN_SetEventCallback)(cl_event, cl_int,
        void (CL_CALLBACK *)(cl_event, cl_int, void *), void *);
typedef cl_int (CL_API_CALL *PFN_SetUserEventStatus)(cl_event, cl_int);
typedef cl_int (CL_API_CALL *PFN_ReleaseEvent)(cl_event);
typedef cl_int (CL_API_CALL *PFN_ReleaseContext)(cl_context);

static volatile LONG fired;
static PFN_SetUserEventStatus pSetUserEventStatus;
static cl_event g_event;

static void CL_CALLBACK on_complete(cl_event e, cl_int status, void *data)
{
    (void)e; (void)data;
    printf("callback fired status=%d\n", (int)status);
    fflush(stdout);
    InterlockedExchange(&fired, 1);
}

static DWORD WINAPI setter_thread(void *arg)
{
    (void)arg;
    Sleep(300);
    printf("setter: completing event\n");
    fflush(stdout);
    pSetUserEventStatus(g_event, CL_COMPLETE);
    return 0;
}

int main(void)
{
    HMODULE mod = LoadLibraryA("opencl.dll");
    PFN_GetPlatformIDs pGetPlatformIDs;
    PFN_GetDeviceIDs pGetDeviceIDs;
    PFN_CreateContext pCreateContext;
    PFN_CreateUserEvent pCreateUserEvent;
    PFN_SetEventCallback pSetEventCallback;
    PFN_ReleaseEvent pReleaseEvent;
    PFN_ReleaseContext pReleaseContext;
    cl_platform_id plat;
    cl_device_id dev;
    cl_context ctx;
    cl_int err;
    HANDLE th;
    int waited, pass;

    if (!mod) { printf("SKIP: no opencl.dll\n"); return 2; }
    pGetPlatformIDs = (PFN_GetPlatformIDs)GetProcAddress(mod, "clGetPlatformIDs");
    pGetDeviceIDs = (PFN_GetDeviceIDs)GetProcAddress(mod, "clGetDeviceIDs");
    pCreateContext = (PFN_CreateContext)GetProcAddress(mod, "clCreateContext");
    pCreateUserEvent = (PFN_CreateUserEvent)GetProcAddress(mod, "clCreateUserEvent");
    pSetEventCallback = (PFN_SetEventCallback)GetProcAddress(mod, "clSetEventCallback");
    pSetUserEventStatus = (PFN_SetUserEventStatus)GetProcAddress(mod, "clSetUserEventStatus");
    pReleaseEvent = (PFN_ReleaseEvent)GetProcAddress(mod, "clReleaseEvent");
    pReleaseContext = (PFN_ReleaseContext)GetProcAddress(mod, "clReleaseContext");
    CHECK(pGetPlatformIDs, "clGetPlatformIDs"); CHECK(pGetDeviceIDs, "clGetDeviceIDs");
    CHECK(pCreateContext, "clCreateContext"); CHECK(pCreateUserEvent, "clCreateUserEvent");
    CHECK(pSetEventCallback, "clSetEventCallback"); CHECK(pSetUserEventStatus, "clSetUserEventStatus");
    CHECK(pReleaseEvent, "clReleaseEvent"); CHECK(pReleaseContext, "clReleaseContext");

    if (pGetPlatformIDs(1, &plat, NULL) != CL_SUCCESS) { printf("SKIP: no CL platform\n"); return 2; }
    if (pGetDeviceIDs(plat, CL_DEVICE_TYPE_ALL, 1, &dev, NULL) != CL_SUCCESS) { printf("SKIP: no CL device\n"); return 2; }
    ctx = pCreateContext(NULL, 1, &dev, NULL, NULL, &err);
    if (!ctx || err != CL_SUCCESS) { printf("SKIP: context failed %d\n", (int)err); return 2; }
    g_event = pCreateUserEvent(ctx, &err);
    if (!g_event || err != CL_SUCCESS) { printf("SKIP: user event failed %d\n", (int)err); return 2; }

    if (pSetEventCallback(g_event, CL_COMPLETE, on_complete, NULL) != CL_SUCCESS) { printf("FAIL: SetEventCallback rejected\n"); return 1; }

    th = CreateThread(NULL, 0, setter_thread, NULL, 0, NULL);
    if (!th) { printf("SKIP: no thread\n"); return 2; }

    /* The Affinity pattern: wait with NO further OpenCL calls. */
    for (waited = 0; waited < 5000; waited += 10)
    {
        if (fired) break;
        Sleep(10);
    }
    WaitForSingleObject(th, INFINITE);
    CloseHandle(th);

    if (fired) printf("PASS: callback delivered while waiting\n");
    else printf("FAIL: callback never fired (hang)\n");
    pass = (fired != 0);
    pReleaseEvent(g_event);
    pReleaseContext(ctx);
    return pass ? 0 : 1;
}
