/* OpenCL API coverage suite (Affinity-Wine-Builder, wine-11.18).
 *
 * Calls every function the 11.18 opencl.dll exports and validates sane
 * behavior: strict checks (data verified, sane values) fail the run;
 * driver-optional queries only log their code (must not crash or hang).
 * New 0064-0068 surface is deep-tested in Test-OpenCL-Compute; here it
 * gets presence + basic checks so one binary proves the whole surface.
 *
 * Build: x86_64-w64-mingw32-gcc -O2 -Wall -o ocl-coverage.exe ocl-coverage.c
 * Exit: 0=all strict checks pass 1=FAIL 2=SKIP(no platform/device)
 */
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct _cl_platform_id *cl_platform_id;
typedef struct _cl_device_id *cl_device_id;
typedef struct _cl_context *cl_context;
typedef struct _cl_command_queue *cl_command_queue;
typedef struct _cl_mem *cl_mem;
typedef struct _cl_program *cl_program;
typedef struct _cl_kernel *cl_kernel;
typedef struct _cl_event *cl_event;
typedef struct _cl_sampler *cl_sampler;
typedef int32_t cl_int;
typedef uint32_t cl_uint;
typedef intptr_t cl_context_properties;
typedef uint64_t cl_device_type;
typedef uint64_t cl_bitfield;
typedef uint64_t cl_ulong;
#define CL_API_CALL __stdcall
#define CL_CALLBACK __stdcall
#define CL_SUCCESS 0
#define CL_COMPLETE 0x0
#define CL_TRUE 1
#define CL_DEVICE_TYPE_ALL 0xFFFFFFFFull
#define CL_MEM_READ_WRITE 1
#define CL_MEM_READ_ONLY 4
#define CL_MAP_READ 1
#define CL_MAP_WRITE 2
#define CL_QUEUE_PROFILING_ENABLE 2

typedef struct { uint32_t order, type; } imgfmt_t;
typedef struct { uint32_t type; size_t w, h, depth, array, row, slice; uint32_t mips, samples; cl_mem buf, obj; } imgdesc_t;

#define FN(ret, name, ...) typedef ret (CL_API_CALL *PFN_##name)(__VA_ARGS__)
FN(cl_int, GetPlatformIDs, cl_uint, cl_platform_id *, cl_uint *);
FN(cl_int, GetPlatformInfo, cl_platform_id, uint32_t, size_t, void *, size_t *);
FN(cl_int, GetDeviceIDs, cl_platform_id, cl_device_type, cl_uint, cl_device_id *, cl_uint *);
FN(cl_int, GetDeviceInfo, cl_device_id, uint32_t, size_t, void *, size_t *);
FN(cl_context, CreateContext, const cl_context_properties *, cl_uint, const cl_device_id *, void *, void *, cl_int *);
FN(cl_int, GetContextInfo, cl_context, uint32_t, size_t, void *, size_t *);
FN(cl_int, RetainContext, cl_context);
FN(cl_int, ReleaseContext, cl_context);
FN(cl_command_queue, CreateCommandQueue, cl_context, cl_device_id, uint64_t, cl_int *);
FN(cl_int, GetCommandQueueInfo, cl_command_queue, uint32_t, size_t, void *, size_t *);
FN(cl_int, RetainCommandQueue, cl_command_queue);
FN(cl_int, ReleaseCommandQueue, cl_command_queue);
FN(cl_int, SetCommandQueueProperty, cl_command_queue, uint64_t, uint32_t, uint64_t *);
FN(cl_mem, CreateBuffer, cl_context, uint64_t, size_t, void *, cl_int *);
FN(cl_int, GetMemObjectInfo, cl_mem, uint32_t, size_t, void *, size_t *);
FN(cl_int, RetainMemObject, cl_mem);
FN(cl_int, ReleaseMemObject, cl_mem);
FN(cl_int, SetMemObjectDestructorCallback, cl_mem, void (CL_CALLBACK *)(cl_mem, void *), void *);
FN(cl_int, EnqueueWriteBuffer, cl_command_queue, cl_mem, uint32_t, size_t, size_t, const void *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueReadBuffer, cl_command_queue, cl_mem, uint32_t, size_t, size_t, void *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueCopyBuffer, cl_command_queue, cl_mem, cl_mem, size_t, size_t, size_t, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueFillBuffer, cl_command_queue, cl_mem, const void *, size_t, size_t, size_t, cl_uint, const cl_event *, cl_event *);
FN(void *, EnqueueMapBuffer, cl_command_queue, cl_mem, uint32_t, uint64_t, size_t, size_t, cl_uint, const cl_event *, cl_event *, cl_int *);
FN(cl_int, EnqueueUnmapMemObject, cl_command_queue, cl_mem, void *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueWriteBufferRect, cl_command_queue, cl_mem, uint32_t, const size_t *, const size_t *, const size_t *, size_t, size_t, size_t, size_t, const void *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueReadBufferRect, cl_command_queue, cl_mem, uint32_t, const size_t *, const size_t *, const size_t *, size_t, size_t, size_t, size_t, void *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueCopyBufferRect, cl_command_queue, cl_mem, cl_mem, const size_t *, const size_t *, const size_t *, size_t, size_t, size_t, size_t, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueMigrateMemObjects, cl_command_queue, cl_uint, const cl_mem *, uint64_t, cl_uint, const cl_event *, cl_event *);
FN(cl_mem, CreateImage2D, cl_context, uint64_t, const imgfmt_t *, size_t, size_t, size_t, void *, cl_int *);
FN(cl_mem, CreateImage, cl_context, uint64_t, const imgfmt_t *, const imgdesc_t *, void *, cl_int *);
FN(cl_int, GetImageInfo, cl_mem, uint32_t, size_t, void *, size_t *);
FN(cl_int, EnqueueWriteImage, cl_command_queue, cl_mem, uint32_t, const size_t *, const size_t *, size_t, size_t, const void *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueReadImage, cl_command_queue, cl_mem, uint32_t, const size_t *, const size_t *, size_t, size_t, void *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueCopyImage, cl_command_queue, cl_mem, cl_mem, const size_t *, const size_t *, const size_t *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueFillImage, cl_command_queue, cl_mem, const void *, const size_t *, const size_t *, cl_uint, const cl_event *, cl_event *);
FN(void *, EnqueueMapImage, cl_command_queue, cl_mem, uint32_t, uint64_t, const size_t *, const size_t *, size_t *, size_t *, cl_uint, const cl_event *, cl_event *, cl_int *);
FN(cl_int, GetSupportedImageFormats, cl_context, uint64_t, uint32_t, cl_uint, imgfmt_t *, cl_uint *);
FN(cl_sampler, CreateSampler, cl_context, uint32_t, uint32_t, uint32_t, cl_int *);
FN(cl_int, GetSamplerInfo, cl_sampler, uint32_t, size_t, void *, size_t *);
FN(cl_int, ReleaseSampler, cl_sampler);
FN(cl_program, CreateProgramWithSource, cl_context, cl_uint, const char **, const size_t *, cl_int *);
FN(cl_int, BuildProgram, cl_program, cl_uint, const cl_device_id *, const char *, void *, void *);
FN(cl_int, GetProgramInfo, cl_program, uint32_t, size_t, void *, size_t *);
FN(cl_int, GetProgramBuildInfo, cl_program, cl_device_id, uint32_t, size_t, void *, size_t *);
FN(cl_program, CreateProgramWithBinary, cl_context, cl_uint, const cl_device_id *, const size_t *, const unsigned char **, cl_int *, cl_int *);
FN(cl_int, CreateKernelsInProgram, cl_program, cl_uint, cl_kernel *, cl_uint *);
FN(cl_int, ReleaseProgram, cl_program);
FN(cl_kernel, CreateKernel, cl_program, const char *, cl_int *);
FN(cl_int, GetKernelInfo, cl_kernel, uint32_t, size_t, void *, size_t *);
FN(cl_int, GetKernelArgInfo, cl_kernel, cl_uint, uint32_t, size_t, void *, size_t *);
FN(cl_int, GetKernelWorkGroupInfo, cl_kernel, cl_device_id, uint32_t, size_t, void *, size_t *);
FN(cl_int, SetKernelArg, cl_kernel, cl_uint, size_t, const void *);
FN(cl_int, RetainKernel, cl_kernel);
FN(cl_int, ReleaseKernel, cl_kernel);
FN(cl_int, EnqueueNDRangeKernel, cl_command_queue, cl_kernel, cl_uint, const size_t *, const size_t *, const size_t *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueTask, cl_command_queue, cl_kernel, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueMarker, cl_command_queue, cl_event *);
FN(cl_int, EnqueueMarkerWithWaitList, cl_command_queue, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueBarrier, cl_command_queue);
FN(cl_int, EnqueueBarrierWithWaitList, cl_command_queue, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueWaitForEvents, cl_command_queue, cl_uint, const cl_event *);
FN(cl_int, WaitForEvents, cl_uint, const cl_event *);
FN(cl_event, CreateUserEvent, cl_context, cl_int *);
FN(cl_int, SetUserEventStatus, cl_event, cl_int);
FN(cl_int, SetEventCallback, cl_event, cl_int, void (CL_CALLBACK *)(cl_event, cl_int, void *), void *);
FN(cl_int, GetEventInfo, cl_event, uint32_t, size_t, void *, size_t *);
FN(cl_int, GetEventProfilingInfo, cl_event, uint32_t, size_t, void *, size_t *);
FN(cl_int, RetainEvent, cl_event);
FN(cl_int, ReleaseEvent, cl_event);
FN(cl_int, Finish, cl_command_queue);
FN(cl_int, Flush, cl_command_queue);
FN(cl_int, CreateSubDevices, cl_device_id, const intptr_t *, cl_uint, cl_device_id *, cl_uint *);
FN(cl_int, ReleaseDevice, cl_device_id);
FN(void *, GetExtensionFunctionAddress, const char *);
FN(void *, GetExtensionFunctionAddressForPlatform, cl_platform_id, const char *);
FN(cl_int, UnloadCompiler, void);
FN(cl_int, UnloadPlatformCompiler, cl_platform_id);
FN(cl_mem, CreateFromGLBuffer, cl_context *, uint64_t, uint32_t, int *);
FN(cl_int, EnqueueAcquireGLObjects, cl_command_queue, cl_uint, const cl_mem *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueReleaseGLObjects, cl_command_queue, cl_uint, const cl_mem *, cl_uint, const cl_event *, cl_event *);
FN(cl_command_queue, CreateCommandQueueWithProperties, cl_context, cl_device_id, const intptr_t *, cl_int *);
FN(cl_kernel, CloneKernel, cl_kernel, cl_int *);
FN(void *, SVMAlloc, cl_context, uint64_t, size_t, uint32_t);
FN(void, SVMFree, cl_context, void *);

#define LOAD(n) p##n = (PFN_##n)GetProcAddress(mod, "cl" #n); if (!p##n) { printf("SKIP: no cl" #n "\n"); return 2; }

static int strict_total = 0, strict_ok = 0;
#define REQUIRE(cond, ...) do { strict_total++; if (cond) { strict_ok++; } else { printf("FAIL: " __VA_ARGS__); return 1; } } while (0)
#define TOL(call, name) do { cl_int _e = (call); printf("info: %s -> %d\n", name, (int)_e); } while (0)

static const char ksrc[] =
    "__kernel void vadd(__global const float *a, __global const float *b, __global float *c)\n"
    "{ int i = get_global_id(0); c[i] = a[i] + b[i]; }\n"
    "__kernel void one(__global float *c)\n"
    "{ c[0] = 42.0f; }\n";

#define N 256
static float ha[N], hb[N], hc[N];

int main(void)
{
    HMODULE mod = LoadLibraryA("opencl.dll");
    PFN_GetPlatformIDs pGetPlatformIDs; PFN_GetPlatformInfo pGetPlatformInfo;
    PFN_GetDeviceIDs pGetDeviceIDs; PFN_GetDeviceInfo pGetDeviceInfo;
    PFN_CreateContext pCreateContext; PFN_GetContextInfo pGetContextInfo;
    PFN_RetainContext pRetainContext; PFN_ReleaseContext pReleaseContext;
    PFN_CreateCommandQueue pCreateCommandQueue; PFN_GetCommandQueueInfo pGetCommandQueueInfo;
    PFN_RetainCommandQueue pRetainCommandQueue; PFN_ReleaseCommandQueue pReleaseCommandQueue;
    PFN_SetCommandQueueProperty pSetCommandQueueProperty;
    PFN_CreateBuffer pCreateBuffer; PFN_GetMemObjectInfo pGetMemObjectInfo;
    PFN_RetainMemObject pRetainMemObject; PFN_ReleaseMemObject pReleaseMemObject;
    PFN_SetMemObjectDestructorCallback pSetMemObjectDestructorCallback;
    PFN_EnqueueWriteBuffer pEnqueueWriteBuffer; PFN_EnqueueReadBuffer pEnqueueReadBuffer;
    PFN_EnqueueCopyBuffer pEnqueueCopyBuffer; PFN_EnqueueFillBuffer pEnqueueFillBuffer;
    PFN_EnqueueMapBuffer pEnqueueMapBuffer; PFN_EnqueueUnmapMemObject pEnqueueUnmapMemObject;
    PFN_EnqueueWriteBufferRect pEnqueueWriteBufferRect; PFN_EnqueueReadBufferRect pEnqueueReadBufferRect;
    PFN_EnqueueCopyBufferRect pEnqueueCopyBufferRect; PFN_EnqueueMigrateMemObjects pEnqueueMigrateMemObjects;
    PFN_CreateImage2D pCreateImage2D; PFN_CreateImage pCreateImage; PFN_GetImageInfo pGetImageInfo;
    PFN_EnqueueWriteImage pEnqueueWriteImage; PFN_EnqueueReadImage pEnqueueReadImage;
    PFN_EnqueueCopyImage pEnqueueCopyImage; PFN_EnqueueFillImage pEnqueueFillImage;
    PFN_EnqueueMapImage pEnqueueMapImage; PFN_GetSupportedImageFormats pGetSupportedImageFormats;
    PFN_CreateSampler pCreateSampler; PFN_GetSamplerInfo pGetSamplerInfo; PFN_ReleaseSampler pReleaseSampler;
    PFN_CreateProgramWithSource pCreateProgramWithSource; PFN_BuildProgram pBuildProgram;
    PFN_GetProgramInfo pGetProgramInfo; PFN_GetProgramBuildInfo pGetProgramBuildInfo;
    PFN_CreateProgramWithBinary pCreateProgramWithBinary; PFN_CreateKernelsInProgram pCreateKernelsInProgram;
    PFN_ReleaseProgram pReleaseProgram;
    PFN_CreateKernel pCreateKernel; PFN_GetKernelInfo pGetKernelInfo; PFN_GetKernelArgInfo pGetKernelArgInfo;
    PFN_GetKernelWorkGroupInfo pGetKernelWorkGroupInfo; PFN_SetKernelArg pSetKernelArg;
    PFN_RetainKernel pRetainKernel; PFN_ReleaseKernel pReleaseKernel;
    PFN_EnqueueNDRangeKernel pEnqueueNDRangeKernel; PFN_EnqueueTask pEnqueueTask;
    PFN_EnqueueMarker pEnqueueMarker; PFN_EnqueueMarkerWithWaitList pEnqueueMarkerWithWaitList;
    PFN_EnqueueBarrier pEnqueueBarrier; PFN_EnqueueBarrierWithWaitList pEnqueueBarrierWithWaitList;
    PFN_EnqueueWaitForEvents pEnqueueWaitForEvents; PFN_WaitForEvents pWaitForEvents;
    PFN_CreateUserEvent pCreateUserEvent; PFN_SetUserEventStatus pSetUserEventStatus;
    PFN_SetEventCallback pSetEventCallback; PFN_GetEventInfo pGetEventInfo;
    PFN_GetEventProfilingInfo pGetEventProfilingInfo; PFN_RetainEvent pRetainEvent; PFN_ReleaseEvent pReleaseEvent;
    PFN_Finish pFinish; PFN_Flush pFlush;
    PFN_CreateSubDevices pCreateSubDevices; PFN_ReleaseDevice pReleaseDevice;
    PFN_GetExtensionFunctionAddress pGetExtensionFunctionAddress;
    PFN_GetExtensionFunctionAddressForPlatform pGetExtensionFunctionAddressForPlatform;
    PFN_UnloadCompiler pUnloadCompiler; PFN_UnloadPlatformCompiler pUnloadPlatformCompiler;
    PFN_CreateFromGLBuffer pCreateFromGLBuffer;
    PFN_EnqueueAcquireGLObjects pEnqueueAcquireGLObjects; PFN_EnqueueReleaseGLObjects pEnqueueReleaseGLObjects;
    PFN_CreateCommandQueueWithProperties pCreateCommandQueueWithProperties;
    PFN_CloneKernel pCloneKernel; PFN_SVMAlloc pSVMAlloc; PFN_SVMFree pSVMFree;
    cl_platform_id plat; cl_device_id dev; cl_context ctx; cl_command_queue q, pq;
    cl_int err; cl_uint n = 0; int i; size_t gws = 64;
    char buf[256]; cl_uint u32 = 0; size_t sz = 0;
    cl_mem ba, bb, bc, scratch; cl_program prog, bprog; cl_kernel kern, kc;
    cl_event ev = NULL, ev2 = NULL; cl_sampler smp;

    if (!mod) { printf("SKIP: no opencl.dll\n"); return 2; }
    LOAD(GetPlatformIDs); LOAD(GetPlatformInfo); LOAD(GetDeviceIDs); LOAD(GetDeviceInfo);
    LOAD(CreateContext); LOAD(GetContextInfo); LOAD(RetainContext); LOAD(ReleaseContext);
    LOAD(CreateCommandQueue); LOAD(GetCommandQueueInfo); LOAD(RetainCommandQueue); LOAD(ReleaseCommandQueue);
    LOAD(SetCommandQueueProperty);
    LOAD(CreateBuffer); LOAD(GetMemObjectInfo); LOAD(RetainMemObject); LOAD(ReleaseMemObject);
    LOAD(SetMemObjectDestructorCallback);
    LOAD(EnqueueWriteBuffer); LOAD(EnqueueReadBuffer); LOAD(EnqueueCopyBuffer); LOAD(EnqueueFillBuffer);
    LOAD(EnqueueMapBuffer); LOAD(EnqueueUnmapMemObject);
    LOAD(EnqueueWriteBufferRect); LOAD(EnqueueReadBufferRect); LOAD(EnqueueCopyBufferRect);
    LOAD(EnqueueMigrateMemObjects);
    LOAD(CreateImage2D); LOAD(CreateImage); LOAD(GetImageInfo);
    LOAD(EnqueueWriteImage); LOAD(EnqueueReadImage); LOAD(EnqueueCopyImage); LOAD(EnqueueFillImage);
    LOAD(EnqueueMapImage); LOAD(GetSupportedImageFormats);
    LOAD(CreateSampler); LOAD(GetSamplerInfo); LOAD(ReleaseSampler);
    LOAD(CreateProgramWithSource); LOAD(BuildProgram); LOAD(GetProgramInfo); LOAD(GetProgramBuildInfo);
    LOAD(CreateProgramWithBinary); LOAD(CreateKernelsInProgram); LOAD(ReleaseProgram);
    LOAD(CreateKernel); LOAD(GetKernelInfo); LOAD(GetKernelArgInfo); LOAD(GetKernelWorkGroupInfo);
    LOAD(SetKernelArg); LOAD(RetainKernel); LOAD(ReleaseKernel);
    LOAD(EnqueueNDRangeKernel); LOAD(EnqueueTask);
    LOAD(EnqueueMarker); LOAD(EnqueueMarkerWithWaitList);
    LOAD(EnqueueBarrier); LOAD(EnqueueBarrierWithWaitList);
    LOAD(EnqueueWaitForEvents); LOAD(WaitForEvents);
    LOAD(CreateUserEvent); LOAD(SetUserEventStatus); LOAD(SetEventCallback);
    LOAD(GetEventInfo); LOAD(GetEventProfilingInfo); LOAD(RetainEvent); LOAD(ReleaseEvent);
    LOAD(Finish); LOAD(Flush);
    LOAD(CreateSubDevices); LOAD(ReleaseDevice);
    LOAD(GetExtensionFunctionAddress); LOAD(GetExtensionFunctionAddressForPlatform);
    LOAD(UnloadCompiler); LOAD(UnloadPlatformCompiler);
    LOAD(CreateFromGLBuffer); LOAD(EnqueueAcquireGLObjects); LOAD(EnqueueReleaseGLObjects);
    LOAD(CreateCommandQueueWithProperties); LOAD(CloneKernel); LOAD(SVMAlloc); LOAD(SVMFree);
    printf("imports: all present\n");

    /* platform / device / context / queue */
    REQUIRE(pGetPlatformIDs(1, &plat, &n) == CL_SUCCESS && n >= 1, "platforms\n");
    REQUIRE(pGetPlatformInfo(plat, 0x0902, sizeof(buf), buf, NULL) == CL_SUCCESS && buf[0], "plat name\n");
    REQUIRE(pGetDeviceIDs(plat, CL_DEVICE_TYPE_ALL, 1, &dev, &n) == CL_SUCCESS && n >= 1, "devices\n");
    REQUIRE(pGetDeviceInfo(dev, 0x102B, sizeof(buf), buf, NULL) == CL_SUCCESS && buf[0], "dev name %s\n", buf);
    REQUIRE(pGetDeviceInfo(dev, 0x1002, sizeof(u32), &u32, NULL) == CL_SUCCESS && u32 >= 1, "compute units %u\n", u32);
    REQUIRE(pGetDeviceInfo(dev, 0x1004, sizeof(sz), &sz, NULL) == CL_SUCCESS && sz >= 64, "max wg %llu\n", (unsigned long long)sz);
    ctx = pCreateContext(NULL, 1, &dev, NULL, NULL, &err);
    REQUIRE(ctx && err == CL_SUCCESS, "context %d\n", (int)err);
    REQUIRE(pGetContextInfo(ctx, 0x1083, sizeof(u32), &u32, NULL) == CL_SUCCESS && u32 == 1, "ctx ndev\n");
    { cl_device_id dd = NULL; REQUIRE(pGetContextInfo(ctx, 0x1081, sizeof(dd), &dd, NULL) == CL_SUCCESS && dd == dev, "ctx dev\n"); }
    REQUIRE(pRetainContext(ctx) == CL_SUCCESS, "retain ctx\n");
    q = pCreateCommandQueue(ctx, dev, 0, &err);
    REQUIRE(q && err == CL_SUCCESS, "queue %d\n", (int)err);
    { cl_context qc = NULL; REQUIRE(pGetCommandQueueInfo(q, 0x1090, sizeof(qc), &qc, NULL) == CL_SUCCESS && qc == ctx, "queue ctx\n"); }
    { uint64_t qp = 99; REQUIRE(pGetCommandQueueInfo(q, 0x1093, sizeof(qp), &qp, NULL) == CL_SUCCESS && qp == 0, "queue props\n"); }
    REQUIRE(pRetainCommandQueue(q) == CL_SUCCESS, "retain q\n");
    REQUIRE(pReleaseCommandQueue(q) == CL_SUCCESS, "release q\n");
    pq = pCreateCommandQueue(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
    REQUIRE(pq && err == CL_SUCCESS, "profiling queue %d\n", (int)err);
    TOL(pSetCommandQueueProperty(q, 0, 0, NULL), "set-queue-prop");
    printf("phase 1 info/context/queue ok\n");

    /* buffers */
    for (i = 0; i < N; i++) { ha[i] = (float)i; hb[i] = (float)(2 * i); hc[i] = 0; }
    ba = pCreateBuffer(ctx, CL_MEM_READ_ONLY, sizeof(ha), NULL, &err);
    REQUIRE(ba && err == CL_SUCCESS, "create a\n");
    bb = pCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof(hb), NULL, &err);
    REQUIRE(bb && err == CL_SUCCESS, "create b\n");
    bc = pCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof(hc), NULL, &err);
    REQUIRE(bc && err == CL_SUCCESS, "create c\n");
    REQUIRE(pGetMemObjectInfo(ba, 0x1102, sizeof(sz), &sz, NULL) == CL_SUCCESS && sz == sizeof(ha), "mem size\n");
    REQUIRE(pGetMemObjectInfo(ba, 0x1100, sizeof(u32), &u32, NULL) == CL_SUCCESS && u32 == 0x10F0, "mem type %x\n", u32);
    REQUIRE(pGetMemObjectInfo(ba, 0x1105, sizeof(u32), &u32, NULL) == CL_SUCCESS && u32 == 1, "mem refcount\n");
    REQUIRE(pRetainMemObject(ba) == CL_SUCCESS, "retain\n");
    REQUIRE(pReleaseMemObject(ba) == CL_SUCCESS, "release\n");
    REQUIRE(pEnqueueWriteBuffer(q, ba, CL_TRUE, 0, sizeof(ha), ha, 0, NULL, NULL) == CL_SUCCESS, "write a\n");
    REQUIRE(pEnqueueWriteBuffer(q, bb, CL_TRUE, 0, sizeof(hb), hb, 0, NULL, NULL) == CL_SUCCESS, "write b\n");
    REQUIRE(pEnqueueCopyBuffer(q, ba, bc, 0, 0, sizeof(ha), 0, NULL, NULL) == CL_SUCCESS, "copy\n");
    REQUIRE(pEnqueueReadBuffer(q, bc, CL_TRUE, 0, sizeof(hc), hc, 0, NULL, NULL) == CL_SUCCESS, "read\n");
    for (i = 0; i < N; i++) REQUIRE(hc[i] == ha[i], "copy verify [%d]\n", i);
    { float pat = 7.0f; REQUIRE(pEnqueueFillBuffer(q, bc, &pat, sizeof(pat), 0, sizeof(hc), 0, NULL, NULL) == CL_SUCCESS, "fill\n"); }
    REQUIRE(pFinish(q) == CL_SUCCESS, "finish\n");
    REQUIRE(pEnqueueReadBuffer(q, bc, CL_TRUE, 0, sizeof(hc), hc, 0, NULL, NULL) == CL_SUCCESS, "read fill\n");
    for (i = 0; i < N; i++) REQUIRE(hc[i] == 7.0f, "fill verify [%d]\n", i);
    { cl_int merr = 0; float *mp = pEnqueueMapBuffer(q, ba, CL_TRUE, CL_MAP_READ, 0, sizeof(ha), 0, NULL, NULL, &merr);
      REQUIRE(mp && merr == CL_SUCCESS, "map %d\n", (int)merr);
      for (i = 0; i < N; i++) REQUIRE(mp[i] == ha[i], "map verify [%d]\n", i);
      REQUIRE(pEnqueueUnmapMemObject(q, ba, mp, 0, NULL, NULL) == CL_SUCCESS, "unmap\n"); }
    { size_t bo[3] = {0,0,0}, ho[3] = {0,0,0}, rg[3] = {4 * sizeof(float),4,1};
      cl_mem bd = pCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof(hc), NULL, &err);
      float tile[16]; REQUIRE(bd && err == CL_SUCCESS, "rect scratch\n");
      REQUIRE(pEnqueueWriteBufferRect(q, bd, CL_TRUE, bo, ho, rg, 16 * sizeof(float), 0, 4 * sizeof(float), 0, ha, 0, NULL, NULL) == CL_SUCCESS, "write rect\n");
      REQUIRE(pEnqueueReadBufferRect(q, bd, CL_TRUE, bo, ho, rg, 16 * sizeof(float), 0, 4 * sizeof(float), 0, tile, 0, NULL, NULL) == CL_SUCCESS, "read rect\n");
      for (i = 0; i < 16; i++) REQUIRE(tile[i] == ha[i], "rect verify [%d]\n", i);
      REQUIRE(pEnqueueCopyBufferRect(q, bd, bc, bo, bo, rg, 16 * sizeof(float), 0, 16 * sizeof(float), 0, 0, NULL, NULL) == CL_SUCCESS, "copy rect\n");
      REQUIRE(pEnqueueMigrateMemObjects(q, 1, &bd, 1, 0, NULL, NULL) == CL_SUCCESS, "migrate\n");
      REQUIRE(pReleaseMemObject(bd) == CL_SUCCESS, "release rect scratch\n"); }
    printf("phase 2 buffers ok\n");

    /* images + sampler */
    { imgfmt_t fmt = { 0x10B5, 0x10DE }; imgdesc_t dsc; cl_mem img, img2;
      float px[16 * 16 * 4], out[16 * 16 * 4]; size_t org[3] = {0,0,0}, reg[3] = {16,16,1};
      cl_uint nf = 0; int j;
      for (j = 0; j < 16 * 16 * 4; j++) px[j] = (float)j;
      REQUIRE(pGetSupportedImageFormats(ctx, CL_MEM_READ_WRITE, 0x10F1, 0, NULL, &nf) == CL_SUCCESS && nf > 0, "img formats %u\n", nf);
      img = pCreateImage2D(ctx, CL_MEM_READ_WRITE, &fmt, 16, 16, 0, NULL, &err);
      REQUIRE(img && err == CL_SUCCESS, "image2d %d\n", (int)err);
      REQUIRE(pGetImageInfo(img, 0x1114, sizeof(sz), &sz, NULL) == CL_SUCCESS && sz == 16, "img w\n");
      REQUIRE(pGetImageInfo(img, 0x1115, sizeof(sz), &sz, NULL) == CL_SUCCESS && sz == 16, "img h\n");
      REQUIRE(pGetImageInfo(img, 0x1111, sizeof(sz), &sz, NULL) == CL_SUCCESS && sz == 16, "img elem\n");
      REQUIRE(pEnqueueWriteImage(q, img, CL_TRUE, org, reg, 0, 0, px, 0, NULL, NULL) == CL_SUCCESS, "write img\n");
      REQUIRE(pEnqueueReadImage(q, img, CL_TRUE, org, reg, 0, 0, out, 0, NULL, NULL) == CL_SUCCESS, "read img\n");
      for (j = 0; j < 16 * 16 * 4; j++) REQUIRE(out[j] == px[j], "img verify [%d]\n", j);
      img2 = pCreateImage2D(ctx, CL_MEM_READ_WRITE, &fmt, 16, 16, 0, NULL, &err);
      REQUIRE(img2 && err == CL_SUCCESS, "image2d b\n");
      REQUIRE(pEnqueueCopyImage(q, img, img2, org, org, reg, 0, NULL, NULL) == CL_SUCCESS, "copy img\n");
      { float fillc[4] = {1,2,3,4}; REQUIRE(pEnqueueFillImage(q, img2, fillc, org, reg, 0, NULL, NULL) == CL_SUCCESS, "fill img\n"); }
      REQUIRE(pFinish(q) == CL_SUCCESS, "finish img\n");
      REQUIRE(pEnqueueReadImage(q, img2, CL_TRUE, org, reg, 0, 0, out, 0, NULL, NULL) == CL_SUCCESS, "read fill\n");
      REQUIRE(out[0] == 1.0f && out[1] == 2.0f && out[3] == 4.0f, "fill verify\n");
      { size_t rp = 0, sp = 0; cl_int merr = 0;
        void *mp = pEnqueueMapImage(q, img, CL_TRUE, CL_MAP_READ, org, reg, &rp, &sp, 0, NULL, NULL, &merr);
        REQUIRE(mp && merr == CL_SUCCESS && rp == 16 * 4 * sizeof(float), "map img %d rp=%llu\n", (int)merr, (unsigned long long)rp);
        REQUIRE(pEnqueueUnmapMemObject(q, img, mp, 0, NULL, NULL) == CL_SUCCESS, "unmap img\n"); }
      memset(&dsc, 0, sizeof(dsc)); dsc.type = 0x10F1; dsc.w = 8; dsc.h = 8;
      { cl_mem img3 = pCreateImage(ctx, CL_MEM_READ_ONLY, &fmt, &dsc, NULL, &err);
        REQUIRE(img3 && err == CL_SUCCESS, "image-desc %d\n", (int)err);
        REQUIRE(pReleaseMemObject(img3) == CL_SUCCESS, "release img3\n"); }
      REQUIRE(pReleaseMemObject(img) == CL_SUCCESS, "release img\n");
      REQUIRE(pReleaseMemObject(img2) == CL_SUCCESS, "release img2\n");
      smp = pCreateSampler(ctx, CL_TRUE, 0x1131, 0x1141, &err);
      REQUIRE(smp && err == CL_SUCCESS, "sampler %d\n", (int)err);
      REQUIRE(pGetSamplerInfo(smp, 0x1152, sizeof(u32), &u32, NULL) == CL_SUCCESS && u32 == CL_TRUE, "smp norm\n");
      REQUIRE(pGetSamplerInfo(smp, 0x1153, sizeof(u32), &u32, NULL) == CL_SUCCESS && u32 == 0x1131, "smp addr\n");
      REQUIRE(pGetSamplerInfo(smp, 0x1154, sizeof(u32), &u32, NULL) == CL_SUCCESS && u32 == 0x1141, "smp filt\n");
      REQUIRE(pReleaseSampler(smp) == CL_SUCCESS, "release smp\n"); }
    printf("phase 3 images+sampler ok\n");

    /* programs + kernels */
    { const char *ss[1] = { ksrc }; size_t nk = 0; cl_uint nku = 0; char names[256] = {0};      prog = pCreateProgramWithSource(ctx, 1, ss, NULL, &err);
      REQUIRE(prog && err == CL_SUCCESS, "program src\n");
      REQUIRE(pBuildProgram(prog, 1, &dev, "-cl-kernel-arg-info", NULL, NULL) == CL_SUCCESS, "build\n");
      REQUIRE(pGetProgramInfo(prog, 0x1167, sizeof(nk), &nk, NULL) == CL_SUCCESS && nk == 2, "num kernels %llu\n", (unsigned long long)nk);
      REQUIRE(pGetProgramInfo(prog, 0x1168, sizeof(names), names, NULL) == CL_SUCCESS && strstr(names, "vadd") && strstr(names, "one"), "names %s\n", names);
      REQUIRE(pCreateKernelsInProgram(prog, 0, NULL, &nku) == CL_SUCCESS && nku == 2, "kernels-in-prog\n");
      { size_t bsz = 0; unsigned char *bin = NULL; const unsigned char *bins[1]; cl_int bst = 0;
        REQUIRE(pGetProgramInfo(prog, 0x1165, sizeof(bsz), &bsz, NULL) == CL_SUCCESS && bsz > 0, "bin size\n");
        bin = HeapAlloc(GetProcessHeap(), 0, bsz); REQUIRE(bin, "bin alloc\n");
        bins[0] = bin;
        REQUIRE(pGetProgramInfo(prog, 0x1166, sizeof(bins), bins, NULL) == CL_SUCCESS, "binaries\n");
        bprog = pCreateProgramWithBinary(ctx, 1, &dev, &bsz, bins, &bst, &err);
        REQUIRE(bprog && err == CL_SUCCESS && bst == CL_SUCCESS, "program bin %d/%d\n", (int)err, (int)bst);
        HeapFree(GetProcessHeap(), 0, bin);
        REQUIRE(pBuildProgram(bprog, 1, &dev, "", NULL, NULL) == CL_SUCCESS, "build bin\n");
        { cl_kernel bk = pCreateKernel(bprog, "vadd", &err);
          REQUIRE(bk && err == CL_SUCCESS, "kernel from bin\n");
          REQUIRE(pReleaseKernel(bk) == CL_SUCCESS, "release bk\n"); }
        REQUIRE(pReleaseProgram(bprog) == CL_SUCCESS, "release bprog\n"); }
      kern = pCreateKernel(prog, "vadd", &err);
      REQUIRE(kern && err == CL_SUCCESS, "kernel\n");
      { char fn[64] = {0}; REQUIRE(pGetKernelInfo(kern, 0x1190, sizeof(fn), fn, NULL) == CL_SUCCESS && !strcmp(fn, "vadd"), "kname %s\n", fn); }
      REQUIRE(pGetKernelInfo(kern, 0x1191, sizeof(u32), &u32, NULL) == CL_SUCCESS && u32 == 3, "nargs\n");
      REQUIRE(pGetKernelWorkGroupInfo(kern, dev, 0x11B0, sizeof(sz), &sz, NULL) == CL_SUCCESS && sz >= 64, "wg size\n");
      REQUIRE(pGetKernelWorkGroupInfo(kern, dev, 0x11B4, sizeof(sz), &sz, NULL) == CL_SUCCESS, "wg mult call\n");
      printf("info: wg multiple=%llu\n", (unsigned long long)sz);
      REQUIRE(pGetKernelArgInfo(kern, 0, 0x1196, sizeof(u32), &u32, NULL) == CL_SUCCESS && u32 == 0x119B, "arg addr %x\n", u32);
      REQUIRE(pRetainKernel(kern) == CL_SUCCESS, "retain k\n");
      REQUIRE(pReleaseKernel(kern) == CL_SUCCESS, "release k\n");
      REQUIRE(pSetKernelArg(kern, 0, sizeof(ba), &ba) == CL_SUCCESS, "arg0\n");
      REQUIRE(pSetKernelArg(kern, 1, sizeof(bb), &bb) == CL_SUCCESS, "arg1\n");
      REQUIRE(pSetKernelArg(kern, 2, sizeof(bc), &bc) == CL_SUCCESS, "arg2\n"); }
    printf("phase 4 programs+kernels ok\n");

    /* execution + events + profiling */
    REQUIRE(pEnqueueNDRangeKernel(q, kern, 1, NULL, &gws, NULL, 0, NULL, &ev) == CL_SUCCESS && ev, "ndrange\n");
    REQUIRE(pFlush(q) == CL_SUCCESS, "flush\n");
    REQUIRE(pWaitForEvents(1, (const cl_event *)&ev) == CL_SUCCESS, "wait\n");
    { uint32_t ty = 0; REQUIRE(pGetEventInfo(ev, 0x11D1, sizeof(ty), &ty, NULL) == CL_SUCCESS && ty == 0x11F0, "ev type %x\n", ty); }
    { cl_int st = 0; REQUIRE(pGetEventInfo(ev, 0x11D3, sizeof(st), &st, NULL) == CL_SUCCESS && st == CL_COMPLETE, "ev status\n"); }
    REQUIRE(pRetainEvent(ev) == CL_SUCCESS, "retain ev\n");
    REQUIRE(pReleaseEvent(ev) == CL_SUCCESS, "release ev\n");
    REQUIRE(pEnqueueReadBuffer(q, bc, CL_TRUE, 0, sizeof(hc), hc, 0, NULL, NULL) == CL_SUCCESS, "read\n");
    for (i = 0; i < 64; i++) REQUIRE(hc[i] == ha[i] + hb[i], "ndrange verify [%d]\n", i);
    REQUIRE(pReleaseEvent(ev) == CL_SUCCESS, "release ev2\n"); ev = NULL;
    { cl_kernel ok = pCreateKernel(prog, "one", &err);
      REQUIRE(ok && err == CL_SUCCESS, "task kernel\n");
      REQUIRE(pSetKernelArg(ok, 0, sizeof(bc), &bc) == CL_SUCCESS, "task arg\n");
      REQUIRE(pEnqueueTask(q, ok, 0, NULL, &ev) == CL_SUCCESS && ev, "task\n");
      REQUIRE(pFinish(q) == CL_SUCCESS, "finish task\n");
      REQUIRE(pEnqueueReadBuffer(q, bc, CL_TRUE, 0, sizeof(float), hc, 0, NULL, NULL) == CL_SUCCESS, "read task\n");
      REQUIRE(hc[0] == 42.0f, "task verify %f\n", hc[0]);
      REQUIRE(pReleaseEvent(ev) == CL_SUCCESS, "release task ev\n"); ev = NULL;
      REQUIRE(pReleaseKernel(ok) == CL_SUCCESS, "release task k\n"); }
    REQUIRE(pEnqueueMarker(q, &ev) == CL_SUCCESS && ev, "marker\n");
    REQUIRE(pEnqueueMarkerWithWaitList(q, 1, (const cl_event *)&ev, &ev2) == CL_SUCCESS && ev2, "marker wl\n");
    REQUIRE(pEnqueueBarrier(q) == CL_SUCCESS, "barrier\n");
    REQUIRE(pEnqueueBarrierWithWaitList(q, 1, (const cl_event *)&ev, NULL) == CL_SUCCESS, "barrier wl\n");
    REQUIRE(pEnqueueWaitForEvents(q, 1, (const cl_event *)&ev) == CL_SUCCESS, "queue wait\n");
    REQUIRE(pWaitForEvents(2, (const cl_event[]){ ev, ev2 }) == CL_SUCCESS, "wait2\n");
    REQUIRE(pReleaseEvent(ev) == CL_SUCCESS, "rel ev\n");
    REQUIRE(pReleaseEvent(ev2) == CL_SUCCESS, "rel ev2\n"); ev = ev2 = NULL;
    { cl_event pev = NULL;
      REQUIRE(pEnqueueNDRangeKernel(pq, kern, 1, NULL, &gws, NULL, 0, NULL, &pev) == CL_SUCCESS && pev, "pev ndrange\n");
      REQUIRE(pFinish(pq) == CL_SUCCESS, "pev finish\n");
      { cl_ulong qd = 0, sd = 0, st = 0, en = 0;
        REQUIRE(pGetEventProfilingInfo(pev, 0x1280, sizeof(qd), &qd, NULL) == CL_SUCCESS, "prof q\n");
        REQUIRE(pGetEventProfilingInfo(pev, 0x1281, sizeof(sd), &sd, NULL) == CL_SUCCESS, "prof s\n");
        REQUIRE(pGetEventProfilingInfo(pev, 0x1282, sizeof(st), &st, NULL) == CL_SUCCESS && st > 0, "prof start\n");
        REQUIRE(pGetEventProfilingInfo(pev, 0x1283, sizeof(en), &en, NULL) == CL_SUCCESS && en >= st, "prof end\n");
        REQUIRE(qd <= sd && sd <= st, "prof order\n"); }
      REQUIRE(pReleaseEvent(pev) == CL_SUCCESS, "rel pev\n"); }
    { cl_event ue = pCreateUserEvent(ctx, &err);
      REQUIRE(ue && err == CL_SUCCESS, "user ev\n");
      REQUIRE(pSetUserEventStatus(ue, CL_COMPLETE) == CL_SUCCESS, "set status\n");
      REQUIRE(pWaitForEvents(1, (const cl_event *)&ue) == CL_SUCCESS, "wait ue\n");
      REQUIRE(pReleaseEvent(ue) == CL_SUCCESS, "rel ue\n"); }
    printf("phase 5 execution+events ok\n");

    /* new surface presence + error paths */
    { cl_command_queue q2 = pCreateCommandQueueWithProperties(ctx, dev, NULL, &err);
      REQUIRE(q2 && err == CL_SUCCESS, "cq props\n");
      REQUIRE(pReleaseCommandQueue(q2) == CL_SUCCESS, "rel q2\n");
      kc = pCloneKernel(kern, &err);
      REQUIRE(kc && err == CL_SUCCESS, "clone\n");
      REQUIRE(pReleaseKernel(kc) == CL_SUCCESS, "rel clone\n");
      scratch = pCreateBuffer(ctx, CL_MEM_READ_ONLY, 64, NULL, &err);
      REQUIRE(scratch && err == CL_SUCCESS, "scratch\n");
      { void *sv = pSVMAlloc(ctx, 1, 4096, 0);
        REQUIRE(sv, "svm alloc\n");
        pSVMFree(ctx, sv); }
      REQUIRE(pReleaseMemObject(scratch) == CL_SUCCESS, "rel scratch\n"); }
    TOL(pCreateSubDevices(dev, NULL, 0, NULL, NULL), "subdevices");
    { cl_mem gl = pCreateFromGLBuffer(&ctx, 0, 1, &err);
      printf("info: gl-buffer -> %p err=%d\n", gl, (int)err); }
    TOL(pEnqueueAcquireGLObjects(q, 0, NULL, 0, NULL, NULL), "acquire-gl");
    TOL(pEnqueueReleaseGLObjects(q, 0, NULL, 0, NULL, NULL), "release-gl");
    TOL(pUnloadCompiler(), "unload-compiler");
    TOL(pUnloadPlatformCompiler(plat), "unload-plat-compiler");
    REQUIRE(pGetExtensionFunctionAddress("clNoSuchFunctionXYZ") == NULL, "extaddr null\n");
    REQUIRE(pGetExtensionFunctionAddressForPlatform(plat, "clNoSuchFunctionXYZ") == NULL, "extaddr plat null\n");
    REQUIRE(pReleaseDevice(dev) == CL_SUCCESS, "release dev\n");
    printf("phase 6 new-api+errors ok\n");

    REQUIRE(pReleaseKernel(kern) == CL_SUCCESS, "rel kern\n");
    REQUIRE(pReleaseProgram(prog) == CL_SUCCESS, "rel prog\n");
    REQUIRE(pReleaseMemObject(ba) == CL_SUCCESS, "rel ba\n");
    REQUIRE(pReleaseMemObject(bb) == CL_SUCCESS, "rel bb\n");
    REQUIRE(pReleaseMemObject(bc) == CL_SUCCESS, "rel bc\n");
    REQUIRE(pReleaseCommandQueue(pq) == CL_SUCCESS, "rel pq\n");
    REQUIRE(pReleaseCommandQueue(q) == CL_SUCCESS, "rel q\n");
    REQUIRE(pReleaseContext(ctx) == CL_SUCCESS, "rel ctx\n");
    printf("COVERAGE: strict %d/%d passed\n", strict_ok, strict_total);
    return 0;
}
