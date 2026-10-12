# Wine 11.18 patch set

`0001`–`0031` are the 11.16 set's ElementalWarrior base, carried over
**unchanged**, numbers included. On 11.18 two report *"Reversed (or previously
applied)"* and are skipped by `Build-Wine-11.18.sh`:

- `0001-downgrade-dxcore-to-10_8` -- as on 11.16. Its fuzzed attempt leaves
  10.8's dxcore sources behind, which `0002`/`0006` then patch; Wine's dxcore is
  identical in 11.16 and 11.18, so the result is the same dxcore r3 ships.
- `0004-vkd3d-shader-zero-pad-output-signature` -- upstream: 11.18's bundled
  vkd3d already has all seven hunks.

`0032`–`0049` are the Affinity patch set, in order. Fifteen are byte-identical
to the 11.16 set; three were ported:

| | |
|---|---|
| `0033` widl | Was "three fixes". The methodimpl table fix is upstream as 8b963e98; the other two remain |
| `0035` widl | Upstream 09eba117 moved the assembly name into `struct winmd`; same fix there |
| `0036` widl | The same commit moved the import list into `struct winmd`; same walk-up there |

`0032` must not ship without `0035`: with the assembly name still set to the
output path, implementing `RoResolveNamespace` makes the CLR fail later and
harder than leaving it stubbed.

Verified against a pristine `wine-11.18.tar.xz` (2026-10-01): `0032`–`0049`
apply in order with no fuzz.

`0050`–`0061` are 11.18-only fixes, in order (see `0059`/`0054` notes
below). `0062`–`0063` are new OpenCL-forward-progress work so GPU support
can stay **on** instead of disabled:

| | |
|---|---|
| `0062` opencl | `clFlush` submits the queue for real (+ pumps due callbacks). `0030` had made it `return CL_SUCCESS`; with nothing submitted a startup wait never ends |
| `0063` opencl | Callback pumper thread: one helper per process pumps queued event callbacks every 1 ms while pending (50 ms idle sleep). Fixes the register-callback-then-wait-without-further-calls startup hang; callbacks still run serially through `check_completed_callbacks()` |

Repro: `ocl-repro.c` pattern (user event + `CL_COMPLETE` callback, setter
thread completes it, main thread waits 5 s with no further CL calls) hangs
on `0061` and passes with `0062`–`0063`. Module `dlls/opencl` builds
warning-free (only pre-existing `0021` `invalidate_kernel_args` warning).

`0064`–`0068` complete the bridge toward 100% OpenCL coverage, in order:

| | |
|---|---|
| `0064` opencl | Program build/compile/link notification callbacks fire synchronously in PE (host can't invoke them; NULL path unchanged) |
| `0065` opencl | `clEnqueueNativeKernel` emulated on a helper thread (wait-list, run `user_func`, signal user event) instead of `CL_INVALID_OPERATION` |
| `0066` opencl | Memory-object destructor callbacks via shadow refcounts (tracked creates fire exactly once at zero; foreign handles keep old never-fire behavior) |
| `0067` opencl | Ten missing 2.x APIs bridged as passthroughs: CloneKernel, CQ/SamplerWithProperties, ProgramWithIL, Pipe/PipeInfo, SubGroupInfo, DeviceAndHostTimer, HostTimer, SetDefaultDeviceCommandQueue |
| `0068` opencl | Full SVM family bridged: SVMAlloc/Free, SetKernelArgSVMPointer, SVMMemcpy/MemFill/Map/Unmap/MigrateMem (verified coarse-grain on NVIDIA) |

Full-pipeline proof: `Test-OpenCL-Compute.sh` builds from source, runs a
1024-element kernel with bit-exact readback, then exercises async-notify,
destructor (exactly-once), native kernels (event + NULL-event), all ten
`0067` APIs and a coarse-grain SVM roundtrip. Still stubbed by design:
context-creation error callbacks (no host error text to synthesize) and
untracked-foreign destructor handles (never-fire beats use-after-free).

## Numbering notes

- **`0024`/`0025` are deliberate gaps, not dropped patches.** The alt 11.11 set
  carries `0024-d2d1-stub-Widen-with-empty-geometry-to-prevent-caller-freeze`,
  which is the same fix as this set's `0013-d2d1-stub-Widen-with-empty-geometry`.
  It was superseded before the set reached 11.18 and the number was never
  reused. Nothing was removed from this set.
- **`0059` was formerly a second `0052`.** `0052-d2d1-cache-geometry-upload-buffers`
  and `0052-winspool-map-printer-connections-to-local` shared a number and only
  applied in the right order because `d2d1` sorts before `winspool`. The
  winspool one is renumbered to `0059`; it touches only `dlls/winspool.drv/info.c`,
  which no patch in `0053`–`0058` touches, so the apply order and the patch
  bytes are both unchanged.
- `0054` ships disabled (`.patchOFF`).
