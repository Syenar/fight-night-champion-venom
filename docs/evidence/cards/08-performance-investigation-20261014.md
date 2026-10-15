# Card 08 performance investigation — 2026-10-14

Status: **PARTIAL, FPS and dominant bottleneck UNKNOWN.** Read-only investigation by the performance subagent. No game launched, closed, driven or attached; no SDK, title source, executable, DLL, graphics setting or FPSCR changed. Probe67 was left alone. Completed probe62/native61 and probe65/native64 logs and sources are the measurement inputs. They are different scenarios and cannot establish a performance delta between builds.

## Retained measurements

Private reproducible analysis: `research/performance-20261014/measure_retained.py`; run from workspace with `python -X utf8 research/performance-20261014/measure_retained.py`. Raw per-file hashes, lifecycle, counts, rates and limitations: `research/performance-20261014/retained-metrics.json`. Source/probe-manifest hashes: `source-evidence-hashes.json` in that directory. This report contains no proprietary code or image data.

| Completed probe | 62 | 65 |
|---|---:|---:|
| Retained lifecycle, seconds | 241.38944 | 241.404137 |
| Runtime log bytes | 12,300,948 | 32,316,566 |
| Runtime log lines | 102,682 | 176,225 |
| Lines per lifecycle second | 425.38 | 730.00 |
| Peak lines in one wall-clock second | 526 | 950 |
| XMA unknown-register debug messages | 89,694 | 89,792 |
| Invalid texture-fetch warning messages | 12,449 | 84,180 |
| All debug messages | 90,061 | 90,175 |
| All warning messages | 12,478 | 84,213 |
| Renderer PPM screenshots | 100 | 110 |
| Screenshot bytes | 276,481,600 | 304,129,760 |
| Capture completion interval min / median / max, seconds | 1.024 / 1.030 / 1.040 | 2.027 / 2.034 / 2.090 |
| FPS / frame-time distribution | UNKNOWN | UNKNOWN |

The runtime logs retain startup through termination (62: local17:07:04.511–17:11:05.446; 65:19:00:01.066–19:04:01.950). Logs include eight nonstandard/nonparsed lines per probe. Every rotated runtime log was counted. Both lifecycle records report exit0/windowclose without watchdog; that does not qualify general shutdown. Rate denominators include startup/movie/menu/pause, so these are whole-probe rates, not exclusively fighting rates. Neither screenshot cadence nor compile success measures rendering FPS.

## Findings and hypotheses

### Logging: demonstrated large repeated work, time cost unmeasured

`toolchain/source-patched/src/core/logging.cpp:134` creates synchronous `spdlog::logger`, not async logger. Every accepted message visits its sinks in the caller. Windows keeps the MSVC debug sink; the file sink is mutex-protected and rotates at5MiB. `include/rex/logging/types.h:84` sets flush level to **info**, and `BuildLogConfig` leaves it unchanged. Thus every warning/info/error also calls the sink flush path. File flushing does not necessarily mean a physical disk flush, but it is extra synchronous I/O work. `src/ui/rex_app.cpp:177` adds a2048-entry ring-buffer sink to all categories; `include/rex/logging/sink.h:79` formats payloads, constructs strings and takes additional locks.

The GPU thread emitted84,180 invalid-fetch warnings in probe65, each with six hex arguments and a long repeated suggestion. The audio register debug stream emitted89,792 messages. Source origins: `src/audio/xma_decoder.cpp:348` and `src/graphics/pipeline/texture/cache.cpp:917`. These are real measured message counts, not proof audio or GPU computation is slow. The invalid-fetch branch returns without accepting the invalid texture; removing or rate-limiting its diagnostic must preserve this return and all rendering behavior.

**Largest measured avoidable event population:** these two streams account for173,972/176,225 lines in probe65 (98.72%). This makes diagnostic overhead the first narrow optimization experiment. It is not evidence of the largest wall-time bottleneck. Restoring `--log_level=info` removes debug output (~51.17% of lines in65) but leaves the warning flood. First compare debug against info without changing graphics, captures, code or cache. If warning logging remains hot in a CPU/I/O trace, test a bounded duplicate-warning diagnostic (first full message, count/summarize identical fetch keys) preserving the exact invalid-fetch return. Do not enable `gpu_allow_invalid_fetch_constants` or suppress a guest draw to make logs disappear.

### Screenshot observer effect: concrete readback wait and small-write loop

`src/ui/d3d12/d3d12_presenter.cpp:205` creates a committed readback buffer, command allocator/list, submits a copy and explicitly awaits its fence at303 on each screenshot. It maps/copies the data afterward. The capture worker uses its own thread, but shares the D3D12 direct queue; observer cost can therefore involve both the worker and queue ordering.

`repair15-canonical-tu2/src/venom_app.h:83` writes three RGB bytes once **per pixel**. Each1280×720 shot makes921,600 `ofstream::write` calls (buffered library calls, not necessarily OS writes). Probe65's110shots make101,376,000 calls and write304.13MB;62 makes92,160,000calls. Scheduling uses sleep then capture/write then next sleep, so median34ms above the intended2-second interval in65 includes readback, serialization, flush and sleep scheduling. It does not isolate GPU stall time or prove a34ms render-frame stall. Probe62's analogous excess is30ms.

Narrow candidate if tracing attributes significant CPU to this loop: preserve identical RGB output and capture semantics, assemble one RGB row and write once per row; compare exact output hashes/layout and retain observer overhead. This changes diagnostic serialization only. No candidate was applied here. Use sparse/manual milestone captures for an FPS experiment only when the control fixture does not depend on those images; the recovery vision watcher does depend on them. Changing capture count during a watcher-driven scenario also changes its measurement/input path and would invalidate a simple A/B comparison.

### CPU/generated core: actual software FMA branch, hotness unknown

Native61's generated compile flags include `-O2 -DNDEBUG`, `-mssse3`, `-msse4.1`, C++23 and async exception handling, with debug/line tables. It is **optimized RelWithDebInfo**, not an unoptimized Debug title. No `-mfma`, `-march=native`, LTO or fast-math appears in the inspected generated compile/link flags. `include/rex/ppc/vector_fma.h:10` selects four scalar `std::fma` calls when native FMA is unavailable. Native64 imports `fma` and `fmaf` from the CRT math DLL (`research/performance-20261014/native64-imports.txt`). Static15,090 repaired generated expressions from the accepted FMA evidence do not establish invocation frequency or runtime cost.

This is a plausible CPU candidate only after sampled stacks show the FMA/CRT path consuming material guest-thread time. A hardware-FMA candidate must use guarded feature dispatch or a separately identified CPU-target build, preserve operand aliasing, single rounding, signed-zero, NaN/FTZ/DAZ/FPSCR contracts and rerun the actual arithmetic qualification. A blanket fast-math flag or removal of FMA is not a valid optimization. No hardware throughput, CPU model or CPU hot-function percentages were measured here.

### Xenos/cache/shader/driver: concrete paths, no measured dominance

D3D12 submission completion waits in `src/graphics/d3d12/command_processor.cpp:3154,3173` increment command-buffer-stall counters before `WaitForSingleObject`; counts alone will not measure duration. Delayed readback can fall back to synchronous waits on a cache miss. Texture and pipeline hit/miss counters exist at `d3d12/texture_cache.cpp:2048` and `d3d12/pipeline_cache.cpp:1003`. Pipeline storage load/compile has explicit worker/event waits (`pipeline_cache.cpp:663,683`), and new runtime pipeline logs numbered104 in62 and143 in65. Pipeline creation count is not compile time or shader churn rate. Isolated fresh caches contain2files/89,492bytes in62 and2files/145,640bytes in65, so cold/warm cache state must be controlled. RX7900XTX adapter identity is logged; driver version/utilization/queue latency are not retained.

Source defaults: vsync true, async shader compilation true, occlusion queries true, clear memory page state true, resolve readback none, memexport readback true with fast double buffering/fallback; D3D12-specific readback overrides false, bindless true, pipeline creation threads auto. Default values do not prove every effective setting or executed path; manifests admit no overrides. Keep these defaults for the first performance comparison. Disabling coherency, occlusion or correctness readbacks may alter fighters/physics and is not admitted as a speed fix.

## Profiling gap and exact next commands

Native61 CMake enables perf counters and Tracy; definitions include `TRACY_ON_DEMAND`, `TRACY_MANUAL_LIFETIME`, `TRACY_DELAYED_INIT`. At guest XE_SWAP, `src/graphics/command_processor.cpp:929–946` already computes host-tick delta (`frame_time_us`), integer FPS and calls `Profiler::Flip()`. These are **guest swap intervals**, not necessarily displayed-frame intervals or simulation tick intervals.

However, `perf_log_csv` is only declared in `src/core/perf/counter.cpp:22`. `SetCsvLogPath` has **no caller** in the inspected SDK source/include. `Profiler::Startup()` has no caller either; Tracy FrameMark is guarded by `IsProfilerStarted`. Private `profiler-wiring-search.txt` retains the search. Adding `--perf_log_csv` alone must not be claimed to measure FPS. No ETL/Tracy/PresentMon/per-frame CSV was supplied by these completed probes; FPS remains UNKNOWN.

First create a separate host-only diagnostic overlay that calls `rex::perf::Init()` and `SetCsvLogPath` after cvar initialization, starts Tracy only when opted in, and preserves guest core/runtime/GPU/settings. Verify the CSV header/file exists and nonzero guest swap intervals are produced. Retain exact overlay/pair hashes. CSV writes are buffered, flushed every60swaps; account for its observer cost. Use raw microseconds, omit first zero row, report mean/median/p95/p99/max and `1e6/mean(frame_time_us)` rather than averaging truncated integer FPS. Correlate elapsed sum with scene transitions, captures, fixture events and bout clock. Use ETW present events to distinguish swap cadence from actual displayed cadence.

WPR is available (`C:\Windows\system32\wpr.exe` version10.0.26100); `wpr -profiles` verifies CPU, GPU and FileIO profiles. No WPR collection was started. For a **new admitted profiling run**, after checking no unrelated recorder owns a session and after the parent coordinates the sole game/supervisor, these are the exact tracing commands:

```powershell
# Workspace cwd; do not run against the uncoordinated current probe.
wpr -status
wpr -start CPU -start GPU -start FileIO -filemode
# Run the separately admitted hash-checked scenario under its retained supervisor.
# Stop after the agreed bounded fight span, retaining the actual game lifecycle separately.
wpr -stop "research\performance-20261014\next-admitted-fight.etl" "Venom bounded performance baseline"
```

WPR can require elevated Windows access; this is a future measurement, not collected evidence. Analyze CPU sampled/precise wait stacks by exact PID/module; distinguish generated guest execution, CRT FMA, logging formatting/flush/ring locks, XMA worker, D3D12 command processor, driver and capture thread. Join GPU queue/present/fence stalls and disk/file events with screenshot times. Do not infer a driver bottleneck from `WaitForSingleObject` without the corresponding queue/fence timeline. Retain symbol paths and exact loaded binaries.

Tracy capture source exists at `toolchain/source-patched/thirdparty/tracy/capture`, but no runnable capture executable was established. Source usage is `capture -o output.tracy [-a address] [-p port] [-s seconds]`. Once a pinned separately built capture tool and opt-in Startup wiring are verified, use `capture.exe -a 127.0.0.1 -o <new-private-path>.tracy -s 60`; retain tool hash/version. Do not present this as a ready command against the existing build.

## Minimal controlled experiment

1. Fix diagnostic CSV wiring in an isolated host overlay; use one frozen correct native title/runtime/GPU pair and a proven repeatable ordinary controller scenario. Preserve baseline user/profile hashes and choose identical warm-cache copies for both arms, recording their hashes. Retain same capture cadence, settings, physical-device policy and bounded fight window. Collect swap CSV and one bounded WPR trace.
2. Baseline: original debug logging. Candidate: only `--log_level=info`. Predict lower audio log event count and reduced logging CPU/lock/I/O time. Require measured same-scene frame-time improvement beyond run-to-run noise, not just fewer lines. Keep warnings/errors and normal controller/guest semantics. Run baseline/candidate repetitions after the first comparison discriminates; record exceptions and incomplete scenarios.
3. If warnings dominate trace time, a separate diagnostic rate-limit candidate preserving invalid-fetch behavior; if capture serialization dominates, the row-write candidate; if FMA dominates guest execution, qualify feature-guarded hardware FMA. One candidate at a time. If GPU is busy rather than waiting, inspect measured shader/draw/texture path before selecting an optimization.
4. Accept a candidate only with both fighters sustained through movement/punches, same real bout progression, timing and recovery behavior, unchanged physics/FPSCR contracts, audio/video/no new glitches, actual saves/profile behavior and clean admitted lifecycle evidence. Performance improvements do not close native full-game, Rust, true SBS, browser or release gates.

No largest wall-time optimization can be established from existing evidence. The first measurable and reversible optimization experiment is diagnostic logging because its repeated population is counted and its synchronous work is source-established; its FPS benefit remains a falsifiable hypothesis.
