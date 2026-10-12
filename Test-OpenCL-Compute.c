/* Full-pipeline OpenCL compute test (Affinity-Wine-Builder, wine-11.18).
 *
 * Proves real GPU compute through the bridge on the host driver: build a
 * program from source, run a vector-add kernel, read back and verify, then
 * exercise the 0062/0063 path (callback on a real kernel event + Flush,
 * wait with no further OpenCL calls).
 *
 * Build: x86_64-w64-mingw32-gcc -O2 -Wall -o ocl-compute.exe ocl-compute.c
 * Exit: 0=PASS(all verified) 1=FAIL 2=SKIP(no capable platform/device)
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
typedef struct _cl_sampler *cl_sampler;
typedef struct _cl_event *cl_event;
typedef int32_t cl_int;
typedef uint32_t cl_uint;
typedef intptr_t cl_context_properties;
typedef uint64_t cl_device_type;
typedef size_t cl_mem_flags;
#define CL_API_CALL __stdcall
#define CL_CALLBACK __stdcall
#define CL_SUCCESS 0
#define CL_COMPLETE 0x0
#define CL_TRUE 1
#define CL_FALSE 0
#define CL_DEVICE_TYPE_GPU (1ull << 1)
#define CL_DEVICE_TYPE_ALL 0xFFFFFFFFull
#define CL_MEM_READ_ONLY (1 << 0)
#define CL_MEM_WRITE_ONLY (1 << 1)
#define CL_PROGRAM_BUILD_LOG 0x1183
#define CL_PROGRAM_BUILD_STATUS 0x1181
#define CL_BUILD_SUCCESS 0

#define FN(ret, name, ...) typedef ret (CL_API_CALL *PFN_##name)(__VA_ARGS__)
FN(cl_int, GetPlatformIDs, cl_uint, cl_platform_id *, cl_uint *);
FN(cl_int, GetDeviceIDs, cl_platform_id, cl_device_type, cl_uint, cl_device_id *, cl_uint *);
FN(cl_context, CreateContext, const cl_context_properties *, cl_uint, const cl_device_id *,
   void (CL_CALLBACK *)(const char *, const void *, size_t, void *), void *, cl_int *);
FN(cl_command_queue, CreateCommandQueue, cl_context, cl_device_id, cl_uint, cl_int *);
FN(cl_program, CreateProgramWithSource, cl_context, cl_uint, const char **, const size_t *, cl_int *);
FN(cl_int, BuildProgram, cl_program, cl_uint, const cl_device_id *, const char *,
   void (CL_CALLBACK *)(cl_program, void *), void *);
FN(cl_int, GetProgramBuildInfo, cl_program, cl_device_id, cl_uint, size_t, void *, size_t *);
FN(cl_kernel, CreateKernel, cl_program, const char *, cl_int *);
FN(cl_mem, CreateBuffer, cl_context, cl_mem_flags, size_t, void *, cl_int *);
FN(cl_int, SetKernelArg, cl_kernel, cl_uint, size_t, const void *);
FN(cl_int, EnqueueWriteBuffer, cl_command_queue, cl_mem, cl_uint, size_t, size_t, const void *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueNDRangeKernel, cl_command_queue, cl_kernel, cl_uint, const size_t *, const size_t *, const size_t *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueReadBuffer, cl_command_queue, cl_mem, cl_uint, size_t, size_t, void *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, Finish, cl_command_queue);
FN(cl_int, Flush, cl_command_queue);
FN(cl_int, SetEventCallback, cl_event, cl_int, void (CL_CALLBACK *)(cl_event, cl_int, void *), void *);
FN(cl_int, EnqueueNativeKernel, cl_command_queue, void (CL_CALLBACK *)(void *), void *, size_t, cl_uint, const cl_mem *, const void **, cl_uint, const cl_event *, cl_event *);
FN(cl_int, WaitForEvents, cl_uint, const cl_event *);
FN(cl_int, SetMemObjectDestructorCallback, cl_mem, void (CL_CALLBACK *)(cl_mem, void *), void *);
FN(cl_int, RetainMemObject, cl_mem);
FN(cl_command_queue, CreateCommandQueueWithProperties, cl_context, cl_device_id, const intptr_t *, cl_int *);
FN(cl_sampler, CreateSamplerWithProperties, cl_context, const intptr_t *, cl_int *);
FN(cl_kernel, CloneKernel, cl_kernel, cl_int *);
FN(cl_int, GetKernelSubGroupInfo, cl_kernel, cl_device_id, uint32_t, size_t, const void *, size_t, void *, size_t *);
FN(cl_int, GetDeviceAndHostTimer, cl_device_id, uint64_t *, uint64_t *);
FN(cl_int, GetHostTimer, cl_device_id, uint64_t *);
FN(cl_program, CreateProgramWithIL, cl_context, const void *, size_t, cl_int *);
FN(cl_mem, CreatePipe, cl_context, uint64_t, uint32_t, uint32_t, const intptr_t *, cl_int *);
FN(cl_int, GetPipeInfo, cl_mem, uint32_t, size_t, void *, size_t *);
FN(cl_int, SetDefaultDeviceCommandQueue, cl_context, cl_device_id, cl_command_queue);
FN(void *, SVMAlloc, cl_context, uint64_t, size_t, uint32_t);
FN(void, SVMFree, cl_context, void *);
FN(cl_int, SetKernelArgSVMPointer, cl_kernel, uint32_t, const void *);
FN(cl_int, EnqueueSVMMemcpy, cl_command_queue, uint32_t, void *, const void *, size_t, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueSVMMemFill, cl_command_queue, void *, const void *, size_t, size_t, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueSVMMap, cl_command_queue, uint32_t, uint64_t, void *, size_t, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueSVMUnmap, cl_command_queue, void *, cl_uint, const cl_event *, cl_event *);
FN(cl_int, EnqueueSVMMigrateMem, cl_command_queue, uint32_t, const void **, const size_t *, uint64_t, cl_uint, const cl_event *, cl_event *);
FN(cl_int, GetDeviceInfo, cl_device_id, uint32_t, size_t, void *, size_t *);
FN(cl_int, ReleaseKernel, cl_kernel);
FN(cl_int, ReleaseProgram, cl_program);
FN(cl_int, ReleaseCommandQueue, cl_command_queue);
FN(cl_int, ReleaseSampler, cl_sampler);
FN(cl_int, ReleaseMemObject, cl_mem);
FN(cl_int, ReleaseContext, cl_context);
FN(cl_int, ReleaseEvent, cl_event);

static volatile LONG cb_fired;
static volatile LONG build_notified;
static volatile LONG nk_ran;
static volatile LONG dtor_count;
static void CL_CALLBACK on_dtor(cl_mem m, void *d)
{
    (void)m; (void)d;
    InterlockedIncrement(&dtor_count);
}
static int nk_magic;
static void CL_CALLBACK nk_main(void *arg)
{
    *(int *)arg = 0x4E4B;
    InterlockedExchange(&nk_ran, 1);
}
static void CL_CALLBACK on_build_done(cl_program p, void *d)
{
    (void)p; (void)d;
    InterlockedExchange(&build_notified, 1);
}
static void CL_CALLBACK on_kernel_done(cl_event e, cl_int s, void *d)
{
    (void)e; (void)s; (void)d;
    InterlockedExchange(&cb_fired, 1);
}

#define LOAD(n) p##n = (PFN_##n)GetProcAddress(mod, "cl" #n); if (!p##n) { printf("SKIP: no cl" #n "\n"); return 2; }

static const char src[] =
    "__kernel void vadd(__global const float *a, __global const float *b, __global float *c)\n"
    "{\n    int i = get_global_id(0);\n    c[i] = a[i] + b[i];\n}\n"
    "__kernel void svm_add(__global const float *a, __global float *b)\n"
    "{\n    int i = get_global_id(0);\n    b[i] = a[i] * 2.0f;\n}\n";

#define N 1024
static float ha[N], hb[N], hc[N];

int main(void)
{
    HMODULE mod = LoadLibraryA("opencl.dll");
    PFN_GetPlatformIDs pGetPlatformIDs; PFN_GetDeviceIDs pGetDeviceIDs;
    PFN_CreateContext pCreateContext; PFN_CreateCommandQueue pCreateCommandQueue;
    PFN_CreateProgramWithSource pCreateProgramWithSource; PFN_BuildProgram pBuildProgram;
    PFN_GetProgramBuildInfo pGetProgramBuildInfo; PFN_CreateKernel pCreateKernel;
    PFN_CreateBuffer pCreateBuffer; PFN_SetKernelArg pSetKernelArg;
    PFN_EnqueueWriteBuffer pEnqueueWriteBuffer; PFN_EnqueueNDRangeKernel pEnqueueNDRangeKernel;
    PFN_EnqueueReadBuffer pEnqueueReadBuffer; PFN_Finish pFinish; PFN_Flush pFlush;
    PFN_SetEventCallback pSetEventCallback; PFN_ReleaseMemObject pReleaseMemObject;
    PFN_ReleaseKernel pReleaseKernel; PFN_ReleaseProgram pReleaseProgram;
    PFN_ReleaseCommandQueue pReleaseCommandQueue; PFN_ReleaseContext pReleaseContext;
    PFN_ReleaseEvent pReleaseEvent; PFN_EnqueueNativeKernel pEnqueueNativeKernel;
    PFN_WaitForEvents pWaitForEvents; PFN_SetMemObjectDestructorCallback pSetMemObjectDestructorCallback;
    PFN_RetainMemObject pRetainMemObject; PFN_CreateCommandQueueWithProperties pCreateCommandQueueWithProperties;
    PFN_CreateSamplerWithProperties pCreateSamplerWithProperties; PFN_CloneKernel pCloneKernel;
    PFN_GetKernelSubGroupInfo pGetKernelSubGroupInfo; PFN_GetDeviceAndHostTimer pGetDeviceAndHostTimer;
    PFN_GetHostTimer pGetHostTimer; PFN_CreateProgramWithIL pCreateProgramWithIL;
    PFN_CreatePipe pCreatePipe; PFN_GetPipeInfo pGetPipeInfo;
    PFN_SetDefaultDeviceCommandQueue pSetDefaultDeviceCommandQueue; PFN_ReleaseSampler pReleaseSampler;
    PFN_SVMAlloc pSVMAlloc; PFN_SVMFree pSVMFree; PFN_SetKernelArgSVMPointer pSetKernelArgSVMPointer;
    PFN_EnqueueSVMMemcpy pEnqueueSVMMemcpy; PFN_EnqueueSVMMemFill pEnqueueSVMMemFill;
    PFN_EnqueueSVMMap pEnqueueSVMMap; PFN_EnqueueSVMUnmap pEnqueueSVMUnmap;
    PFN_EnqueueSVMMigrateMem pEnqueueSVMMigrateMem; PFN_GetDeviceInfo pGetDeviceInfo;
    cl_platform_id plat; cl_device_id dev; cl_context ctx; cl_command_queue q;
    cl_program prog; cl_kernel kern; cl_mem ba, bb, bc; cl_event ev = NULL;
    cl_int err; size_t gws = N; int i, waited, pass = 0;
    const char *strings[1] = { src };
    char log[4096]; size_t logsz = 0;

    if (!mod) { printf("SKIP: no opencl.dll\n"); return 2; }
    LOAD(GetPlatformIDs); LOAD(GetDeviceIDs); LOAD(CreateContext); LOAD(CreateCommandQueue);
    LOAD(CreateProgramWithSource); LOAD(BuildProgram); LOAD(GetProgramBuildInfo); LOAD(CreateKernel);
    LOAD(CreateBuffer); LOAD(SetKernelArg); LOAD(EnqueueWriteBuffer); LOAD(EnqueueNDRangeKernel);
    LOAD(EnqueueReadBuffer); LOAD(Finish); LOAD(Flush); LOAD(SetEventCallback);
    LOAD(ReleaseMemObject); LOAD(ReleaseKernel); LOAD(ReleaseProgram);
    LOAD(ReleaseCommandQueue); LOAD(ReleaseContext); LOAD(ReleaseEvent);
    LOAD(EnqueueNativeKernel); LOAD(WaitForEvents); LOAD(SetMemObjectDestructorCallback);
    LOAD(RetainMemObject); LOAD(CreateCommandQueueWithProperties); LOAD(CreateSamplerWithProperties);
    LOAD(CloneKernel); LOAD(GetKernelSubGroupInfo); LOAD(GetDeviceAndHostTimer); LOAD(GetHostTimer);
    LOAD(CreateProgramWithIL); LOAD(CreatePipe); LOAD(GetPipeInfo); LOAD(SetDefaultDeviceCommandQueue);
    LOAD(ReleaseSampler); LOAD(SVMAlloc); LOAD(SVMFree); LOAD(SetKernelArgSVMPointer);
    LOAD(EnqueueSVMMemcpy); LOAD(EnqueueSVMMemFill); LOAD(EnqueueSVMMap); LOAD(EnqueueSVMUnmap);
    LOAD(EnqueueSVMMigrateMem); LOAD(GetDeviceInfo);

    if (pGetPlatformIDs(1, &plat, NULL) != CL_SUCCESS) { printf("SKIP: no platform\n"); return 2; }
    if (pGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, 1, &dev, NULL) != CL_SUCCESS &&
        pGetDeviceIDs(plat, CL_DEVICE_TYPE_ALL, 1, &dev, NULL) != CL_SUCCESS) { printf("SKIP: no device\n"); return 2; }
    ctx = pCreateContext(NULL, 1, &dev, NULL, NULL, &err);
    if (!ctx || err) { printf("SKIP: context %d\n", (int)err); return 2; }
    q = pCreateCommandQueue(ctx, dev, 0, &err);
    if (!q || err) { printf("FAIL: queue %d\n", (int)err); return 1; }
    prog = pCreateProgramWithSource(ctx, 1, strings, NULL, &err);
    if (!prog || err) { printf("FAIL: program %d\n", (int)err); return 1; }
    err = pBuildProgram(prog, 1, &dev, "", NULL, NULL);
    if (err) {
        pGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_LOG, sizeof(log) - 1, log, &logsz);
        log[sizeof(log) - 1] = 0;
        printf("FAIL: build %d log: %s\n", (int)err, log);
        return 1;
    }
    printf("program built from source OK\n");
    /* Async-notify path (0064): same source, completion callback must fire. */
    {
        cl_program prog2 = pCreateProgramWithSource(ctx, 1, strings, NULL, &err);
        if (!prog2 || err) { printf("FAIL: program2 %d\n", (int)err); return 1; }
        err = pBuildProgram(prog2, 1, &dev, "", on_build_done, NULL);
        if (err) { printf("FAIL: async build %d\n", (int)err); return 1; }
        if (!build_notified) { printf("FAIL: build notify never fired\n"); return 1; }
        printf("program async-notify OK\n");
        pReleaseProgram(prog2);
    }
    kern = pCreateKernel(prog, "vadd", &err);
    if (!kern || err) { printf("FAIL: kernel %d\n", (int)err); return 1; }
    for (i = 0; i < N; i++) { ha[i] = (float)i; hb[i] = (float)(2 * i); hc[i] = 0; }
    ba = pCreateBuffer(ctx, CL_MEM_READ_ONLY, sizeof(ha), NULL, &err);
    bb = pCreateBuffer(ctx, CL_MEM_READ_ONLY, sizeof(hb), NULL, &err);
    bc = pCreateBuffer(ctx, CL_MEM_WRITE_ONLY, sizeof(hc), NULL, &err);
    if (!ba || !bb || !bc) { printf("FAIL: buffers\n"); return 1; }
    if (pEnqueueWriteBuffer(q, ba, CL_TRUE, 0, sizeof(ha), ha, 0, NULL, NULL)) { printf("FAIL: write a\n"); return 1; }
    if (pEnqueueWriteBuffer(q, bb, CL_TRUE, 0, sizeof(hb), hb, 0, NULL, NULL)) { printf("FAIL: write b\n"); return 1; }
    if (pSetKernelArg(kern, 0, sizeof(ba), &ba) || pSetKernelArg(kern, 1, sizeof(bb), &bb) ||
        pSetKernelArg(kern, 2, sizeof(bc), &bc)) { printf("FAIL: args\n"); return 1; }
    if (pEnqueueNDRangeKernel(q, kern, 1, NULL, &gws, NULL, 0, NULL, &ev) || !ev) { printf("FAIL: ndrange\n"); return 1; }
    if (pSetEventCallback(ev, CL_COMPLETE, on_kernel_done, NULL)) { printf("FAIL: setcallback\n"); return 1; }
    if (pFlush(q)) { printf("FAIL: flush\n"); return 1; }
    for (waited = 0; waited < 10000; waited += 10) { if (cb_fired) break; Sleep(10); }
    if (!cb_fired) { printf("FAIL: kernel callback never fired\n"); return 1; }
    printf("kernel event callback OK\n");
    if (pFinish(q)) { printf("FAIL: finish\n"); return 1; }
    if (pEnqueueReadBuffer(q, bc, CL_TRUE, 0, sizeof(hc), hc, 0, NULL, NULL)) { printf("FAIL: read\n"); return 1; }
    for (i = 0; i < N; i++) if (hc[i] != ha[i] + hb[i]) { printf("FAIL: verify [%d] %f != %f\n", i, hc[i], ha[i] + hb[i]); return 1; }
    printf("PASS: %d-element vector add verified on device\n", N);
    pass = 1;
    /* Destructor callbacks (0066): tracked create/retain/release fires once. */
    {
        cl_mem bd = pCreateBuffer(ctx, CL_MEM_READ_ONLY, 64, NULL, &err);
        if (!bd || err) { printf("FAIL: dtor buffer %d\n", (int)err); return 1; }
        if (pSetMemObjectDestructorCallback(bd, on_dtor, NULL)) { printf("FAIL: dtor register\n"); return 1; }
        if (pRetainMemObject(bd)) { printf("FAIL: dtor retain\n"); return 1; }
        if (pReleaseMemObject(bd)) { printf("FAIL: dtor release1\n"); return 1; }
        if (dtor_count) { printf("FAIL: dtor fired early (%d)\n", (int)dtor_count); return 1; }
        if (pReleaseMemObject(bd)) { printf("FAIL: dtor release2\n"); return 1; }
        if (dtor_count != 1) { printf("FAIL: dtor count=%d\n", (int)dtor_count); return 1; }
        printf("destructor callback OK\n");
    }
    /* Native-kernel emulation (0065): runs PE user_func after dependencies. */
    {
        cl_event nkev = NULL;
        nk_magic = 0;
        if (pEnqueueNativeKernel(q, nk_main, &nk_magic, sizeof(nk_magic), 0, NULL, NULL, 0, NULL, &nkev) || !nkev) { printf("FAIL: native enqueue\n"); return 1; }
        if (pWaitForEvents(1, &nkev)) { printf("FAIL: native wait\n"); return 1; }
        if (!nk_ran || nk_magic != 0x4E4B) { printf("FAIL: native ran=%d magic=%x\n", (int)nk_ran, nk_magic); return 1; }
        printf("native kernel OK\n");
        pReleaseEvent(nkev);
        /* NULL-event variant: fire and poll. */
        nk_ran = 0; nk_magic = 0;
        if (pEnqueueNativeKernel(q, nk_main, &nk_magic, sizeof(nk_magic), 0, NULL, NULL, 0, NULL, NULL)) { printf("FAIL: native null-ev\n"); return 1; }
        for (waited = 0; waited < 5000; waited += 10) { if (nk_ran) break; Sleep(10); }
        if (!nk_ran || nk_magic != 0x4E4B) { printf("FAIL: native null-ev ran=%d\n", (int)nk_ran); return 1; }
        printf("native kernel NULL-event OK\n");
    }
    /* Missing-API coverage (0067): must at least bridge through. */
    {
        cl_command_queue q2 = pCreateCommandQueueWithProperties(ctx, dev, NULL, &err);
        cl_sampler smp;
        cl_kernel kc;
        uint64_t ts1 = 0, ts2 = 0;
        if (!q2 || err) { printf("FAIL: cq-with-props %d\n", (int)err); return 1; }
        printf("cq-with-properties OK\n");
        pReleaseCommandQueue(q2);
        smp = pCreateSamplerWithProperties(ctx, NULL, &err);
        if (!smp || err) { printf("FAIL: sampler-with-props %d\n", (int)err); return 1; }
        printf("sampler-with-properties OK\n");
        pReleaseSampler(smp);
        kc = pCloneKernel(kern, &err);
        if (!kc || err) { printf("FAIL: clone %d\n", (int)err); return 1; }
        printf("clone-kernel OK\n");
        pReleaseKernel(kc);
        err = pGetHostTimer(dev, &ts1);
        printf("host-timer -> %d ts=%llu\n", (int)err, (unsigned long long)ts1);
        err = pGetDeviceAndHostTimer(dev, &ts1, &ts2);
        printf("device-and-host-timer -> %d\n", (int)err);
        {
            size_t sz = 0;
            err = pGetKernelSubGroupInfo(kern, dev, 0x11B8, 0, NULL, 0, NULL, &sz);
            printf("subgroup-info-size -> %d\n", (int)err);
        }
        {
            cl_program ilp = pCreateProgramWithIL(ctx, "", 0, &err);
            printf("program-with-IL(empty) -> prog=%p err=%d\n", ilp, (int)err);
            if (ilp) pReleaseProgram(ilp);
        }
        {
            cl_mem pipe = pCreatePipe(ctx, 1, 64, 16, NULL, &err);
            printf("create-pipe -> %p err=%d\n", pipe, (int)err);
            if (pipe) {
                uint32_t pk = 0; size_t rs = 0;
                err = pGetPipeInfo(pipe, 0x1120, sizeof(pk), &pk, &rs);
                printf("pipe-info -> %d packet=%u\n", (int)err, pk);
                pReleaseMemObject(pipe);
            }
        }
        err = pSetDefaultDeviceCommandQueue(ctx, dev, q);
        printf("set-default-queue -> %d\n", (int)err);
    }
    /* SVM coarse-grain roundtrip (0068), skipped when unsupported. */
    {
        uint64_t caps = 0;
        pGetDeviceInfo(dev, 0x105D, sizeof(caps), &caps, NULL);
        printf("svm caps query=0x%llx (informational only)\n", (unsigned long long)caps);
        {
            float *sa, *sb, *sc; cl_kernel sk; int ok = 1, j;
            sa = pSVMAlloc(ctx, 1, N * sizeof(float), 0);
            sb = pSVMAlloc(ctx, 1, N * sizeof(float), 0);
            sc = pSVMAlloc(ctx, 1, N * sizeof(float), 0);
            if (!sa || !sb || !sc) {
                printf("svm: alloc unsupported, skipped\n");
            } else {
            if (pEnqueueSVMMap(q, 1, 3, sa, N * sizeof(float), 0, NULL, NULL)) { printf("FAIL: svm map\n"); return 1; }
            for (j = 0; j < N; j++) sa[j] = (float)j;
            if (pEnqueueSVMUnmap(q, sa, 0, NULL, NULL)) { printf("FAIL: svm unmap\n"); return 1; }
            sk = pCreateKernel(prog, "svm_add", &err);
            if (!sk || err) { printf("FAIL: svm kernel %d\n", (int)err); return 1; }
            if (pSetKernelArgSVMPointer(sk, 0, sa) || pSetKernelArgSVMPointer(sk, 1, sb)) { printf("FAIL: svm args\n"); return 1; }
            if (pEnqueueNDRangeKernel(q, sk, 1, NULL, &gws, NULL, 0, NULL, NULL)) { printf("FAIL: svm ndrange\n"); return 1; }
            if (pFinish(q)) { printf("FAIL: svm finish\n"); return 1; }
            if (pEnqueueSVMMemcpy(q, 1, sc, sb, N * sizeof(float), 0, NULL, NULL)) { printf("FAIL: svm memcpy\n"); return 1; }
            if (pEnqueueSVMMap(q, 1, 1, sc, N * sizeof(float), 0, NULL, NULL)) { printf("FAIL: svm map c\n"); return 1; }
            for (j = 0; j < N; j++) if (sc[j] != (float)j * 2.0f) { ok = 0; break; }
            if (pEnqueueSVMUnmap(q, sc, 0, NULL, NULL)) { printf("FAIL: svm unmap c\n"); return 1; }
            {
                const void *ptrs[1] = { sa }; size_t sizes[1] = { N * sizeof(float) };
                err = pEnqueueSVMMigrateMem(q, 1, ptrs, sizes, 0, 0, NULL, NULL);
                printf("svm-migrate -> %d\n", (int)err);
            }
            {
                float pat = 1.0f;
                if (pEnqueueSVMMemFill(q, sc, &pat, sizeof(pat), N * sizeof(float), 0, NULL, NULL)) { printf("FAIL: svm fill\n"); return 1; }
                if (pFinish(q)) { printf("FAIL: svm fill finish\n"); return 1; }
                if (pEnqueueSVMMap(q, 1, 1, sc, N * sizeof(float), 0, NULL, NULL)) { printf("FAIL: svm map fill\n"); return 1; }
                if (sc[0] != 1.0f || sc[N-1] != 1.0f) ok = 0;
                if (pEnqueueSVMUnmap(q, sc, 0, NULL, NULL)) { printf("FAIL: svm unmap fill\n"); return 1; }
            }
            pSVMFree(ctx, sa); pSVMFree(ctx, sb); pSVMFree(ctx, sc);
            pReleaseKernel(sk);
            if (!ok) { printf("FAIL: svm verify\n"); return 1; }
            printf("svm coarse-grain OK\n");
            }
        }
    }
    pReleaseEvent(ev);
    pReleaseMemObject(ba); pReleaseMemObject(bb); pReleaseMemObject(bc);
    pReleaseKernel(kern); pReleaseProgram(prog); pReleaseCommandQueue(q); pReleaseContext(ctx);
    return pass ? 0 : 1;
}
