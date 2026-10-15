
### 2026-10-06 — Public loader pivot / cycle 33

- Builder: downloaded Xenon Recomp `2aa583907d6c8962bc11a971f6b9033a19e229ee` and ReXGlue SDK `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518` under `research/third_party/`.
- Builder: Xenon Recomp configured and built 180 targets in `research/third_party/xenon-recomp/out/build/ninja-min`, including `xenon-prepare`, `recomp-driver`, `module-inspector`, and `xenon_runtime_host`.
- Critic: base extracted game recognized as title `45410915`, media `2DC50868`, effective hash `2f140ffd2abfbae2b74df61d1ccb802102f70109`; TU2 `.xexp` failed with malformed LZX tree; STFS payload failed as non-XEX. Evidence is in `research/xenon-prepare-fnc/status-base.json` and `research/third_party/LOADER_SELECTION.md`.
- Gate: `BLOCKED` at TU2 loader qualification. Next action: test the correct base-XEX/update pairing, preserve the exact input hashes, and reproduce the verified TU2 effective image before generating a game module.

## Cycle 34 — loader qualification / native boot probe

Builder: cloned public ReXGlue and Xenon-Recomp revisions, repaired Windows third-party symlink checkout and SSSE3 build flags, generated ReXGlue C++ from the immutable base XEX, added explicit USB-camera stubs and one observed internal dispatch mapping, and linked `research/rexglue-fnc-base/out/build/ninja-min/Release/fnc_tu2_probe.exe`.

Critic: verified exact source/tool commits, generated partition count, three unresolved branches, executable/runtime hashes, and a fresh launch. The probe created a responsive visible native window and survived the prior `0x822C6C68` fatal for at least 28 seconds, but logs still report missing `cache:`/`D:\data\xenon\fe\main.big` content and no title/gameplay capture. The base probe is therefore evidence of a working loader/runtime bootstrap only; it does not satisfy the TU2 or full-game gate.

Next action: obtain a valid exact TU2 XEX/XEXP pairing or a reproducible update application path, then regenerate the ReXGlue project from that image; separately implement the missing content/cache VFS and remaining three control-flow targets before claiming native gameplay.
### Cycle 35 — 2026-10-06 — repeatable TU2 pair gate and external identity check

- Builder: added `research/third_party/Verify-FncTu2Pair.ps1`. It hashes the exact base XEX and TU2 XEXP, runs the pinned Xenon loader in a clean cache, writes `research/xenon-prepare-fnc/TU2_PAIR_VERIFY.json`, and never modifies the owned inputs.
- Builder evidence: base XEX SHA-256 `90A698C71AA714E383CE092750CE767E07145F1CA38D8A9C9B3BD8005EA29FF6`; TU2 XEXP SHA-256 `007A53A0A81C28F5F511FE0B8CC6AB5B6C65B6C81CC84EB5D1D394A4AEA961D9`; Xenon exits `2` with `XEXP header-delta LZX patch failed ... LZX tree transmission is malformed`.
- Fresh critic: the failure reproduces from a clean cache with the same bytes. The public Xenia game-patches catalog independently identifies Fight Night Champion TU2 as title `45410915`, default XEX hash `7BF0B50F84DF7641`, and media `2DC50868`, matching the Xenia runtime identity. This confirms the target update identity but does not make the supplied XEXP patchable by Xenon.
- Gate result: the exact TU2 executable gate remains BLOCKED; base native generated code remains quarantined and cannot be promoted as the final recompile. Next action is to acquire or reconstruct a valid effective XEX image matching the public TU2 identity, then rerun generation from that exact image.

## Cycle 36 — 2026-10-06 — effective TU2 header capture and synthetic-loader qualification

- Builder: rebuilt the isolated research Xenia after adding a read-only capture of the post-`ApplyTitleUpdate` XEX header. A fresh run produced `research/pivot/capture-header-run2/research/patched_guest/45410915_tu2_effective_header.bin` (16,384 bytes) and the exact guest image (20,316,160 bytes). The guest image SHA-256 remains `EC2C955DB9E55A6207A391A2AB80F60332DC41C3FE4C07968C7F07D5A9621CA4`; the header security record declares image size `0x01360000`, load address `0x82000000`, and entry point `0x82297338`.
- Builder: created a research-only synthetic uncompressed XEX by combining that captured effective header with the captured guest image, changing only the copied header's file-format compression/encryption fields and recomputing its first-page integrity digest. It is at `research/pivot/capture-header-run2/research/patched_guest/45410915_tu2_effective_uncompressed_digest.xex`, SHA-256 `1B8E87A5D9FFBAC50B720EF014E1EA0C8EE8CC065AFDD213F93D3A24E888022C`. The source XEX/TU2 hashes were rechecked unchanged (`90A698...29FF6`, `007A53...961D9`).
- Fresh critic: Xenon-Recomp's loader accepted the synthetic image far enough to identify title `45410915`, media `2DC50868`, effective image hash `b4deae876e2e04f42cf0582af1b6563b625bdb0b`, and 28,458 function candidates. The full analysis then failed at graph-cache promotion with Windows `Access is denied`; a second clean cache reproduced the same promotion defect. `module-inspector` also exits `0xC0000409` after candidate collection. This qualifies the exact header/body handoff but does not qualify code generation or runtime.
- Gate result: the exact TU2 image is now consumable by the loader as a research synthetic XEX, but the native recompile remains `BLOCKED` at analysis/cache promotion and runtime bootstrap. Next repair is to make graph-cache publication robust on Windows (without weakening verification), then rerun analysis and fresh critic checks before generating or promoting any native module.

### Cycle 37 — 2026-10-07 — exact-TU2 critical-section stall trace

- Builder: disproved the current oversized zero-fill hypothesis with the run10/run11 evidence already recorded. Added bounded per-thread `KeDelayExecutionThread` diagnostics, then ran the D3D12 exact-TU2 host. Run13 loaded TU2 and kept executing, but MainThread plateaued at `0x827AF6B8`; thread 3 and thread 11 delay calls returned success with a relative 1 ms interval. Run14 added bounded main-thread critical-section observations and again loaded TU2, then plateaued at `0x827AF6B8`. Its logs show one thread-11 critical-section wait/wake on `0xB582EF00`; no first frame or present was captured.
- Critic: Ghidra maps `0x827AF6B8` to the guest call to `xboxkrnl!RtlEnterCriticalSection` ordinal 293. The existing trace cap filled on earlier initialization locks, so the state/pointer at the suspected callsite is not established. No synchronization semantics were changed. A narrowed trace build succeeded, but runs15/16 failed before XEX boot at `Failed to mount content graph`, so those runs do not validate the narrowed probe.
- Regression: focused CTest result is 3/4 (thread creation, kernel lifecycle, and RTL critical-section suites pass; `xenon_session_tests` reproducibly fails the directory content mount assertion at line 376). The mount failure's underlying filesystem error is not yet captured. Do not claim a code defect or promote a new runtime conclusion until this is diagnosed.
- Gate: Card 07 remains IN_PROGRESS. Full native recompilation, first frame, gameplay, SBS, and browser execution are all OPEN. Evidence and hashes are recorded in `docs/evidence/cards/07-guest-loop-inspection-20261007.json`.
- Next: find the content-device registration error and restore the session test; then rerun the exact TU2 trace with a bounded callsite-specific capture of the critical-section address, lock count, recursion, owner, and wait result. Continue only from fresh evidence; no first frame means no native-playable acceptance.

### Cycle 38 — 2026-10-07 — critical-section callsite disproved as permanent deadlock

- Builder: rebuilt the D3D12 runtime with per-guest-thread, per-callsite diagnostics for guest PC `0x827AF6B8`. Elevated Run17 mounted and loaded the exact TU2 image; MainThread advanced to 929,485 blocks. Elevated Run18 sampled the lock pointer and guest-visible state for the suspected callsite across guest threads 3, 8, 11, and 12. Acquisitions generally began with `lockCount=-1, owner=0` and returned with `lockCount=0, recursion=1`; a thread-11 contention on `0xB582EF00` registered, waited, and woke successfully. Run18's main thread advanced past 3.1 million blocks. No first frame/present was captured.
- Critic: the earlier stable PC `0x827AF6B8` is not enough to call a thread stalled; exact callsite traces show successful lock acquisitions and completed wakeup. Runs15/16 failed only under restricted tool execution: `HostPathDevice` returned `access_denied` while canonicalizing the content root. Elevation allowed the same local content to mount in runs17/18. The focused CTest suite passes 4/4 under elevation. No lock, wait, or delay semantics were modified.
- Gate: native runtime still does not have a verified first frame, playable game, SBS output, or browser build. Card 07 remains IN_PROGRESS.
- Next: instrument the exact-TU2 guest GPU path with counters/traces for command submission, render-target writes, swap/present, and output capture. Locate the first missing or invalid graphics transition; keep all game-level acceptance open until a real frame is verified.
- Evidence: `docs/evidence/cards/07-guest-loop-inspection-20261007.json`; Run17 log SHA-256 `4D30EED1FC6EDF76C8BA2BD8777665EB6837C3FC56935351B0EA845BB425716C`; Run18 log SHA-256 `18C0AF45C0B0644D3478FF5F33AD6D0F6868E269409761A2E86A4FB83C4C3494`; Run18 host SHA-256 `A7BA1CC6B3F130DA2596EB5FE09CB12887FDC7C3B37D05BBD131A7CC0CE23DA6`.

## Cycle 39 — 2026-10-07 — bounded GPU-pump trace and startup handoff triage

- Builder: added bounded diagnostic sampling to `research/third_party/xenon-recomp/src/core/session.cpp` for ring configuration/indexes, pending IR, front-buffer base/size/format/generation, and present status. Rebuilt `xenon_runtime_host` from the D3D12 build tree and launched exact TU2 in `research/native-host-probe-20261007-gpu-pump-run19/`.
- Runtime evidence: effective TU2 image SHA-256 `EBDCBE683DB2F1C29AA43AC97E79492926C099B2EE253DB502CABCBEC133D52F`; runtime host SHA-256 `00068F2EC4E61602EDC60964B79A0B818EBB65C76D7387592DF48184AFD855C7`; generated module SHA-256 `85C29E095B39BC6A1922890B7ED85D2E5CFDDD6AE51C85BBC804B529831AEFBC`; run19 log SHA-256 `E2C91F73A97060C3065BDF7A6F62F85786BEE674F9D85F9DA66C7288EF158296`; adaptive observations SHA-256 `2DE043C0AF73A1C8E18A63B2114DE3923B940D82EC4FA8B4BF4D58E867C460E6`.
- Critic: verified status says stopped by the documented `stop.signal`, TU2 is loaded, window is created, no guest trap occurred, and GPU pump saw 1,380 ticks with ring unconfigured, no pending IR, no front buffer, and zero present attempts. The diagnostic isolates the immediate missing state; it does not explain why guest startup has not reached graphics initialization.
- Regression: `xenon_session_tests`, `xenon_gpu_pump_tests`, and `xenon_video_export_tests` pass 3/3 with elevated path access. A normal-context session test fails only at Windows path canonicalization; the elevated rerun passes.
- Next: identify the guest functions and live inputs behind repeated adaptive handoffs at `0x82290ED4` and `0x82C75DA0`; correlate function ownership, caller LR, thread, instruction counts, and mapped memory in the saved TU2 Ghidra analysis and actual generated-module manifest. Change runtime semantics only if this proves a contract mismatch. After startup reaches Vd ring setup/front-buffer publication, resume command/render-target/present tracing. Full-game first-frame, offline-mode, SBS, and browser acceptance remain OPEN.

## Cycle 40 — 2026-10-07 — wait-path correction and Vd service trace

- Builder: inspected Run24 periodic PCs and the exact generated PPC around `0x82C71598`. Run24 reached sample 884,736 and its `0x82C715E8` `KeDelayExecutionThread` calls returned success for relative 1 ms and 3 ms intervals; sampled `NtWaitForSingleObjectEx` calls on other threads returned expected `STATUS_TIMEOUT`. This removes the stale permanent-delay/critical-section diagnosis without claiming a boot.
- Builder: added research-only bounded `Vd*` export argument/result tracing (first 64 calls, then every 4096th) and targeted register probes around `0x822A3648`; rebuilt D3D12 `xenon_runtime_host`. Source SHA-256: `B3CF0839F60B1658CC3C2566031B23B4D352215CE519370CCE9529E224DF7C2D`; header SHA-256: `87C91ECF45E2ADFB8A2A9D4499E229631EA9CDA05A995BF5F9F8492C551E1F5E`; host SHA-256: `4B0A1696CD3804B7DB8EBB9291AB238859B4221E92EAFAC861AB540C9B0A5642`.
- Critic: build succeeds and targeted GPU-pump/video-export tests pass. `xenon_session_tests` fails its directory content mount assertion under the current restricted process token. Run25 also failed before loading the XEX at HostPathDevice content graph mount; `Start-Process -Verb RunAs` returned Access denied. Therefore the new export trace has no runtime observation yet; no frame, gameplay, SBS, or browser gate advances.
- Next: invoke `research/native-host-probe-20261007-pc-reg-run25-elevated/Run-AdminProbe.ps1` from an elevated PowerShell window; it verifies the built host hash, runs the exact-TU2 probe for five minutes, signals stop and prints artifact hashes/status/log. Inspect Vd service events and graphics state, then repair only a demonstrated runtime contract mismatch. Evidence: `docs/evidence/cards/07-guest-loop-inspection-20261007.json`.

## Cycle 41 — 2026-10-07 — elevated launch wrapper diagnostics (historical; superseded by Runs 26/33/34)

- Builder: inspected both runs the user launched. The child host stderr was being captured but not printed; it reported `Trailing content after JSON value`. Compact UTF-8 launch config hash: `F3FD79AC6B14004C4D23C9A1236C411C7722D1A348CA811E528E7778C26838ED`. The helper now quotes its config argument, enforces one active probe per run directory with an exclusive lock, and prints result/stdout/stderr. PowerShell syntax check passes.
- Runtime: direct host runs with compact and pretty configs parsed successfully and initialized XenonSession, then failed at `HostPathDevice: Failed to mount content graph` under this restricted token. Exact guest code did not execute. The first elevated wrapper attempt stopped at config parsing; no conclusion about elevated content mounting.
- Critic: user launching the helper twice can cause shared status/log collisions; don't infer anything from a silent console. Latest wrapper avoids concurrent run corruption and exposes captured errors. The compact config and direct-host run outputs are recorded in `docs/evidence/cards/07-run25-launch-diagnostics-20261007.json`.
- Historical next step, superseded: the elevated helper was later run as Run26. See Cycle 44 and `docs/evidence/cards/07-run26-run33-run34-20261008.json`; no user action is pending.
## Cycle 42 — 2026-10-07 — elevated TU2 guest execution; all-thread trace prepared (historical; Run26 executed)

- Builder: reviewed the user's elevated Run25 output and split the final session from the three-session shared log. Verified exact title ID/media ID, TU2 application, effective-image hash, generated-module bind, presentation-window creation, and five minutes of guest execution. Added instrumentation to sample every guest thread at the existing bounded cadence, record each successful export callsite once, and log bounded `Vd*` calls through the structured observations path even when ordinary export logging is capped. Built the D3D12 runtime host (`9D2F1A05F4E574D0A885C28DCD70DA72673A4215E9CC80E1EA46B3B986F04C19`). Prepared a fresh isolated Run26 config and elevated helper, both syntax/JSON validated.
- Runtime: Run25 had 18,849 guest dispatch returns and 19 main-thread periodic samples; GPU probe reached 1,380 ticks while the command ring/front buffer remained zero and presents were skipped. No structured Vd export trace was recorded. The wrapper signaled stop after five minutes; guest did not stop cooperatively within the five-second grace and was terminated. This is not a title/first-frame boot.
- Critic: TU2/content-mount issue is resolved for the elevated launch path. Do not overstate absence of Vd calls based solely on a 128-call console cap. No runtime semantic fix is justified yet; the concrete remaining diagnostic gap is thread ownership/PC and unbounded Vd presence. The D3D12 host rebuild succeeds; GPU-pump/video-export focused tests pass. A non-elevated session test still fails directory content mount, matching the platform-token restriction and prior elevated 4/4 test evidence.
- Gate: Card 07 remains IN_PROGRESS. Native game, first frame, full offline acceptance, SBS, browser, and release remain open.
- Historical next step, completed by Run26: inspect actual per-thread samples, structured Vd events, GPU state, and shutdown. See Cycle 44 for the corrected result.
- Evidence: `docs/evidence/cards/07-run25-launch-diagnostics-20261007.json`; Run25 final-session log SHA-256 `CB8BB9BA0462BE4E35289CC53A881BE71BC8BA0759873837A2897EC9670A8B43`; effective TU2 SHA-256 `EBDCBE683DB2F1C29AA43AC97E79492926C099B2EE253DB502CABCBEC133D52F`; Run26 host SHA-256 `9D2F1A05F4E574D0A885C28DCD70DA72673A4215E9CC80E1EA46B3B986F04C19`.
## Cycle 43 — 2026-10-07 — export-callsite coverage and elevation audit (superseded by actual Run26 result)

- Builder: expanded the Run26-only observations to record the first successful callsite for each imported export, in addition to every-thread periodic PC/register samples and bounded structured Vd traces. Removed unconditional per-call Vd console logging so a frame-rate path cannot flood the five-minute probe log. Rebuilt the D3D12 host and focused test binaries. Host SHA-256: `9D2F1A05F4E574D0A885C28DCD70DA72673A4215E9CC80E1EA46B3B986F04C19`; source SHA-256 `55010F4D5103D45E4A21EF9022583384322279E3A91EE0D089BFC2B68268CC2F`; header SHA-256 `CFCB8E6172972F597111D9E0C0FEFD12449E54BEA032137856A914FDB3D10E37`. Run26 helper/config remain isolated, validated, and hash-pinned.
- Tests: GPU-pump PASS; video-export PASS. Session test fails at the pre-existing content-mount assertion under the non-elevated token (exit 3, `session_tests.cpp:376`). Focused elevated suite had passed earlier; do not count the restricted-token failure as a graphics regression.
- Critic: attempted actual run26 execution through the elevated-command route; the host did not start because this Codex process token marks `BUILTIN\Administrators` deny-only. The helper correctly rejected it. No inference about run26 runtime behavior is available yet. Run25 remains the last exact TU2 evidence and still has no title/first frame or initialized GPU ring/front buffer.
- Gate: Card 07 remains IN_PROGRESS; native game/first frame/full offline game/SBS/browser/release remain open.
- Historical next step, completed by Run26: the user ran the pinned helper. Its actual result is documented in Cycle 44; all subsequent probes are run by Codex.
- Evidence: `docs/evidence/cards/07-run25-launch-diagnostics-20261007.json`; run26 host `research/third_party/xenon-recomp/out/build/d3d12-probe/runtime_host/Debug/xenon_runtime_host.exe`.


## Cycle 44 — 2026-10-08 — self-run TU2 mounts and graphics-startup trace

- Builder: corrected the prior Run26 narrative using the user's pasted elevated result: Run26 had actually started and completed the five-minute watchdog interval. It loaded TU2 and emitted the requested traces, but did not set GPU state or produce a Vd call; shutdown required termination. No more user-run probe is needed for this diagnosis.
- Builder: fixed `HostPathDevice` content-root canonicalization for the restricted Codex token with a lexical-normal fallback guarded by component existence checks and reparse-point rejection. Focused filesystem/session tests pass when temp paths are placed inside writable `research/`.
- Runtime: Release Run33 mounted the content graph, applied the exact TU2 image, bound the native module, and began guest execution with no guest-I/O errors. Self-run Release Run34 used host SHA-256 `83897930132025086FC3C98BE5BD578D5D7449C726FA89736E9C91C233EECC51`, effective-image SHA-256 `EBDCBE683DB2F1C29AA43AC97E79492926C099B2EE253DB502CABCBEC133D52F`, and ran from `2026-10-08T02:53:25Z` to `02:58:34Z`. Stop was cooperative. Its 4,378 structured records include 297 periodic samples across guest threads 3–7 and 213 successful import callsites (205 xboxkrnl, 8 XAM). No Vd calls were observed. GPU probe reached 1,380 ticks with no ring/front-buffer setup; no first frame.
- Critic: the fallback repaired only the host filesystem mount gate; it does not explain the remaining startup/display failure. Run34’s successful exports were mostly thread, critical-section, loader, and network/bootstrap routines. The trace proves broad guest initialization and progressing code, not a title boot. Avoid concluding that a particular missing Vd import caused the stall because no Vd call path was observed. The GuestThread main instance terminated in the final status snapshot, while several worker guest threads remained marked running as stop began; investigate lifecycle/entry-state semantics alongside call-path tracing before changing behavior.
- Gate: Card 07 remains IN_PROGRESS; title/first-frame, full offline game, SBS, browser, and release remain OPEN.
- Next: decode the bounded startup and periodic PCs for each thread from the exact TU2 Ghidra/analysis output; map successful import ordinals and callsites; capture one detailed startup window around the last main-thread transition using low-volume structured events. Add a test only if a runtime-contract defect is proven. Preserve Run26/33/34 artifacts.
- Evidence: `docs/evidence/cards/07-run26-run33-run34-20261008.json`; Run34 observations SHA-256 `8EDF51689FD55240A66B64A7579F5DE9A71C374CE4967CD0384E2781EB13EDEF`; log SHA-256 `3979ECC451DAE4E7F494EA4182A28AD08DBB37CEB8A884FE2323FBD9C284BE2D`.

## Cycle 45 — 2026-10-08 — self-run guest file path trace and content-layout diagnosis

- Builder: self-started three exact-TU2 guest-I/O probes. Run35 and Run36 were 90-second environment/trace checks; Run37 explicitly passed `XENON_TRACE_GUEST_IO=1` to the host and ran the five-minute startup window. No user launch/probe was required.
- Runtime: Run37 surfaced six `STATUS_OBJECT_NAME_NOT_FOUND` events for `D:\data\xenon\fe\main.big`, `D:\data\common\ai\`, `cache:\`, and `\Device\Harddisk0\partition0`. It still had no Vd calls, command ring, front buffer, title, or frame and required forced termination at watchdog stop.
- Critic: inspected the actual extraction list/tree and archive headers. The first two guest paths are absent from loose `data`; actual `fe.big`/`be_gp.big` are separate EB-signature archives under `databigs\xenon`. This proves the current VFS exposes the loose tree but not the archive-backed logical data expected by the title. Which archive entry maps to each missing logical path is still unknown, so do not alias by filename guess. `cache:` and HDD partition0 need separate writable-device handling.
- Gate: Card 07 remains IN_PROGRESS. The data/VFS contract is a concrete new gap; it is not yet proven to be the only startup/graphics blocker. All game, first-frame, SBS, browser and release gates remain OPEN.
- Next: parse both EB archive indexes, validate logical paths against the guest requests, implement read-only archive-backed VFS routing and writable cache/HDD mounts with focused tests, then rerun exact TU2 and require the missing paths to resolve before returning to Vd/GPU startup tracing.
- Evidence: `docs/evidence/cards/07-run35-run36-run37-guest-io-20261008.json`; Run37 host SHA-256 `83897930132025086FC3C98BE5BD578D5D7449C726FA89736E9C91C233EECC51`; log `51D39231B506A6298E1F427F6BC47408A8177A33A59EE6796300A8CA489E88A5`; observations `5BF2C103927286F5E4383237E211BCBB47CFD7B3A23A9C5016CDB4DFE8107720`; effective image `EBDCBE683DB2F1C29AA43AC97E79492926C099B2EE253DB502CABCBEC133D52F`.

## 2026-10-11 — Startup semantic correction and REA audit

Fresh critic review retracted Run201/202 as invalid because standalone-`blr` shortcuts hid a zero-LR control-flow failure. The shortcuts were removed. Exact TU2 `0x822A5B1C` was corrected from an incorrect 32-bit read to `lbzx` byte semantics in both generated and manual dispatch paths. Rebuilt module/runtime and ran Run204; startup advanced beyond the masked boundary and now fails at guest `0x82A43880` on an unmapped write target `0x80` with `ctr=0x822A5B3C`, after 139,688 main-thread blocks. Card 07 remains IN_PROGRESS. REA was audited against its public workflow; the initial sandbox package lookup returned `ENOTFOUND`, so that attempt changed no registration. This was superseded by the elevated cached-package setup recorded below. See `docs/REA_INTEGRATION.md` and `research/repair/startup-readiness/attempt-04/run204/`.

## 2026-10-10 — REA setup applied

The user's REA dry-run was plan-only. The cached REA 6.3.0 package was applied with elevation after the public registry lookup failed in the sandbox. Codex registration is aligned, the pre-change config backup is preserved, and skill v34 (139 tools) is installed. Elevated Ghidra verification passes for Ghidra 12.1.2, Temurin 21.0.11, the headless/native decompiler, and native-authority controls. Evidence: `research/rea/setup-20261010/`. Optional Hopper/IDA checks remain informational. Restart/reconnect Codex before using the new MCP server. Native startup remains at the Run204 guest memory-contract fault; REA setup does not count as a game boot or recompile milestone.


## 2026-10-10 — REA live target comparison

A controlled comparison tested the new MCP against the existing workflow. REA opened the x86-64 native probe through Ghidra, but exhaustive overview timed out after 300 seconds. The exact TU2 `effective-tu2.xex` was rejected as unsupported PE architecture, confirming that this Windows REA/Ghidra route cannot directly analyze the Xbox PPC/XEX image. The target-comparison evidence is `research/rea/setup-20261010/target-comparison.json`; the native-probe session was closed cleanly. REA is operational and useful for supported native helper binaries and evidence orchestration, while the existing PPC/XEX Ghidra/Xenon analysis remains required for the game image.

## 2026-10-10 — bounded REA runtime-control pass

REA successfully analyzed the smaller x86-64 runtime contract test: 8 segments, 888 procedures, 204 strings, deterministic searches, and an `invoke_main` function dossier. The session was closed with a retained evidence snapshot. This confirms the updated workflow is useful for supported native runtime artifacts while the PPC/XEX guest remains on the Xenon-aware path. Evidence: `research/rea/setup-20261010/runtime-contract-comparison.json`.

## 2026-10-12 — loop cycle: trap correction and upstream-boundary isolation

- Builder: rebuilt the Debug host with the pre-existing `twi` 26/31 branch policy, proving Run273 passes `0x8229BA78` and reaches the real D470 null-object fault. Added generated-only tracing in Run275 and captured valid versus failing D470 inputs. Restored the diagnostic-only dynamic fallback source, rebuilt the clean host, and ran Run276.
- Critic: Run276 reproduces the same `0x8249D480` write fault with exact TU2 hash and unchanged canonical module hash. The failing `r3/r4/r31=0` is upstream of D470; no protection or pointer fabrication was used. Native first-frame and all downstream gates remain blocked.
- Next: trace the E040/E06C producer/continuation and compare the branch/return state across valid and failing calls, then apply one evidence-backed causal fix and rerun fresh.
- Verification: after rebuilding the three focused test binaries, xenon_cpu_tests and xenon_video_export_tests pass. xenon_gpu_pump_tests fails at its harness precondition (create_guest_process() returns false because the synthetic harness has no loaded XEX); this is a test-fixture precondition failure, not evidence of a guest-game fix, and no unrelated test code was changed.
- Run277 critic: separate diagnostic module hash and run artifacts are preserved. The run is reproducible but inconclusive because generated stderr/stdout markers were not captured; no code path was promoted. Next pass will instrument the registry/compiled lookup selection itself.
- Registry trace attempt (Run278): prepared an isolated registry.cpp that logs the selected v2 entries and state to a run-local file. Direct compilation of the 265k-line generated registry exceeded the available command window without producing an object, so no diagnostic module was linked and no source outside the isolated run folder changed. This is an infrastructure limitation of whole-registry recompilation, not a runtime conclusion.
- Next: use a smaller generated wrapper or registry object-level hook so lookup selection can be observed without recompiling the entire registry translation unit.

## 2026-10-12 — Runs279-282 lookup/restore/caller probes
- Builder: built isolated lookup wrapper (Run279), D470 exit probe (Run280), restore-slot probe (Run281), and E06C-entry probe (Run282); ran each against exact TU2 image without changing canonical module or user-owned files.
- Fresh critic: Run279 shows registry hits are valid and the zero state is upstream of D470 body; Run281 shows first restore slot is correct; Runs280/282 are explicitly inconclusive because their hooks did not fire. No probe is counted as a fix.
- Result: fault remains `cia=0x8249D480`, write request `0x2C`, no first frame. Next action is exact guest-instruction/control-flow comparison around the caller transition into the second D470 call, followed by a new isolated diagnostic build.
- Evidence: `research/native-host-probe-20261012-e06c-run282/runs279-282-evidence.json`.
## 2026-10-12 — revised workflow Card 01 ReXGlue qualification

- Builder: created isolated campaign `research/rexglue-qualification-20261012/`, captured exact synthetic-TU2/base-XEX hashes, current and historical ReXGlue hashes, and CLI help. Ran six distinct code-generation attempts: synthetic digest input, uncompressed synthetic input, retail control, preserved historical executable, `--force`, init-shaped manifest, and relative-manifest invocation.
- Result: every attempt exited Windows `0xC0000409` immediately after regenerating `generated/rexglue.cmake`; no analysis partition, generated title source, or runtime candidate was emitted. The current executable hash differs from the stale toolchain manifest record.
- Fresh critic: verified campaign input hashes, original-input preservation, identical retail-control failure, no canonical module promotion, and no first-frame claim. Card 01 is BLOCKED at tool identity/codegen, not at the game runtime.
- Next: rebuild or instrument ReXGlue from the pinned commit in a clean isolated build, refresh the manifest with actual executable/submodule/compiler hashes, then rerun base control before exact TU2.

## 2026-10-12 — ReXGlue codegen repair cycle

- Builder: isolated pinned ReXGlue source, patched only research copy to avoid throwing on Windows reparse paths, rebuilt with Clang 22.1.8 and `-mssse3`, and ran base-control codegen.
- Evidence: CDB captured `canonical`/`relative` filesystem exceptions; repaired executable hash `D3995950807BB27CA06F6EFF61AAA3F7ECE67E3FC6575AD20EDADC375448D795` (later rebuild copied to campaign output hash `D3995950807BB27CA06F6EFF61AAA3F7ECE67E3FC6575AD20EDADC375448D795`); base generation wrote 431 files with zero unresolved analysis errors.
- Distinct repair: added entrypoint gap seeds for `0x82BD8108`, `0x82362218`, and `0x8230EA68`; the fresh rerun reported 431 unchanged files.
- Critic: inspected generated output, logs, hashes, and rerun; marked **codegen qualified / runtime unqualified**. One max-file-size warning remains. Exact TU2 generation, runtime link, first frame, and full acceptance remain open.
- Next action: run the patched qualified tool against the exact TU2 effective campaign image with equivalent gap seeds, then inspect imports and runtime setup before any promotion.

## 2026-10-12 — exact TU2 codegen pass

- Builder: ran the patched isolated ReXGlue tool on the effective TU2 research image. Four new branch gaps were seeded from the first pass; a clean output then generated 433 files with no unresolved branch/write lines.
- Critic: fresh rerun produced 433 unchanged files. Remaining evidence is explicit: 13 unresolved XAM/xboxkrnl ordinals and many `sc` stubs. Codegen is qualified; runtime is not.
- Next: map imports and bind syscall/runtime services, then link and capture a first frame.

## 2026-10-13 — retained build and input/boundary audit

- Builder: repaired local host ImGui include path; retained supervised full generated compile reached final link and failed `sub_0`. Status and logs persisted. No executable promoted.
- Critic (self-review): rejected previous codegen qualification. Compared wrapper body hash and all 617 import-record words with canonical raw image; old wrapper has 617 invalid types, canonical has zero. Inspected `gap_8230EA6C`: compare followed by implicit host return, losing guest flow.
- Repair: switched repair15 to hash-matching pre-import-resolution TU2 wrapper and removed all fabricated size=4 entries. First clean pass returns four unresolved tail targets; next pass uses name-only entries to discover full bodies.
- Remaining: verify generated CFG/import coverage, link corrected output, supervised native first frame/full offline acceptance, Rust, normal/true SBS, standalone browser.

## Current native milestone — 2026-10-13

Corrected canonical TU2 code now compiles and links against the isolated ReXGlue runtime. The runtime wrapper is SHA-256 `6BF4C44B9EBD2D7CA27929E0B6D7DC3308CC420D932511F0CDAD9C48671ED91C`, containing one exact canonical payload; the older `1E4361...` wrapper contains a duplicate body and must not be used at runtime. CDB proved the old body overwrite caused an ASCII-valued invalid host function pointer during `sub_82297B50`. A single-payload launch proceeds beyond it.

Observed indirect entries `0x822C6FA8` and `0x82EB2A10` were added with full reachable-body discovery. Subsequent launches advance to `SetInterruptCallback(82CE6410, ...)` and fail at the next unregistered target `0x826169A8`. That entry is being generated/built. Five private EB archives were extracted to campaign assets with per-member hash verification (12,813 files, zero collisions); the `main.big` missing-path failure disappears, while `alphaoverlay.fxo` and cache alias/service contracts remain unresolved. The existing SDK USB-camera export source was enabled to fix seven final-link errors; webcam functionality remains stubbed/unverified.

**Next action:** finish the build with `0x826169A8`, run a supervised probe on the single-payload XEX and isolated extracted content, and inspect the next exact failure and game window. Runtime/first-frame/full offline, Rust, normal/true SBS and standalone browser acceptance remain open. Evidence: `research/rexglue-qualification-20261012/progress-evidence-20261013.json`, `single-image-repair18.json`, `canonical-codegen-audit-20261013.json`, `archive-extraction-manifest.json`, `probe15` through `probe24` folders. Review is self-review, not independent acceptance.


2026-10-13 observed-entry loop 2: 0x828B8970 fully discovered and linked; next target 0x822CDD78; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-2/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 3: 0x822CDD78 fully discovered and linked; next target 0x827282B8; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-3/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 4: 0x827282B8 fully discovered and linked; next target 0x82CFF778; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-4/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 5: 0x82CFF778 fully discovered and linked; next target 0x827CD9D8; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-5/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 6: 0x827CD9D8 fully discovered and linked; next target 0x82616948; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-6/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 7: 0x82616948 fully discovered and linked; next target None; probe exit 0. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-7/result.json. No first-frame or gameplay acceptance.

## Latest verified runtime result — 2026-10-13

The recompiled ReXGlue candidate loads canonical TU2, initializes graphics/audio, and survives a 60-second native probe. That probe received a window-close request and exited 0; the runtime log says it hard-exits after title termination, so this does not close graceful teardown acceptance. The longer visual probe31 ran about 130 seconds, then hit a new unregistered indirect target `0x82B00028` (exit `0xC0000409`). Invalid texture-fetch and XMA-register warnings also remain.

**Next action:** inspect and generate the complete reachable entry at `0x82B00028`, rebuild and rerun the same scenario; separately diagnose rendering warnings. Do not repeat old callback fixes or treat the 60-second survival as a complete boot. First frame/title/menu/gameplay remain UNVERIFIED. The attempted Computer Use screenshot showed another application and was rejected; the user stopped further Computer Use with Escape. Do not resume app input without a new user request. All full-game, Rust, SBS and standalone browser gates remain open. Evidence: `research/rexglue-qualification-20261012/probe-entry-loop-7/`, `probe31-visual-verification/`, and `entry-loop-cycle-7/result.json`.


## User-confirmed native output — 2026-10-13

The user supplied three screenshots of the recompiled Venom window showing recognizable Fight Night Champion/EA Sports startup artwork and copyright text. **Game output is observed; correct rendering FAILS** because all captures contain severe green/black corruption. The user reports that it cannot proceed past the start screen. This replaces the previous uncertainty about whether any game pixels were produced; it does not pass correct-first-frame, menus or gameplay acceptance.

The exact interactive session exited `0xC0000409`, and its terminal log confirms unregistered indirect target `0x82B00028`. Repeated invalid texture-fetch constants are also recorded. **Next:** repair that exact generated entry and trace texture-constant producer/transfer/interpretation byte order against pinned Xenia, with a causal regression and repeat screenshots. The graphics cause is still a hypothesis; no global byte-swap or invalid-fetch bypass is approved as a proven fix.

Evidence: [native startup screenshot index](evidence/cards/08-native-start-screen-corrupt-20261013.json); original screenshots copied unchanged and hashed under `research/rexglue-qualification-20261012/user-start-screen-evidence-20261013/`. Native full-game, Rust, SBS, browser and release gates remain incomplete.


2026-10-13 observed-entry loop 8: 0x82B00028 fully discovered and linked; next target 0x8235EB40; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-8/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 9: 0x8235EB40 fully discovered and linked; next target None; probe exit 0. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-9/result.json. No first-frame or gameplay acceptance.

2026-10-13 vector-pack repair: two observed startup entries regenerated; exact emitted-pack regression 2000 checks PASS and original-bug negative control FAIL as expected; renderer capture39 clean intro frame, captures37/38 green. Full title/menu/gameplay/audio/SBS/browser acceptance remains OPEN. Evidence docs/evidence/cards/08-vector-pack-repair-20261013.json.

2026-10-13 observed-entry loop 10: 0x82893328 fully discovered and linked; next target 0x82E1CC78; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-10/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 11: 0x82E1CC78 fully discovered and linked; next target None; probe exit 0. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-11/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 12: 0x82DB45C0 fully discovered and linked; next target 0x828B8950; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-12/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 13: 0x828B8950 fully discovered and linked; next target 0x828B8930; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-13/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 14: 0x828B8930 fully discovered and linked; next target 0x828A7A00; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-14/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 15: 0x828A7A00 fully discovered and linked; next target None; probe exit 0. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-15/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 16: 0x82E1A030 fully discovered and linked; next target 0x824DFF78; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-16/result.json. No first-frame or gameplay acceptance.

2026-10-13 Start-to-menu: 27 constructor/table-verified callback entries generated and registered; optional Start replay confirmed; readable autosave dialog captured. Main menu NOT PASSED. Probe42 later hit 0x82E1A030; use latest replay-loop result. Evidence docs/evidence/cards/08-start-to-autosave-20261013.json.

2026-10-13 observed-entry loop 17: 0x824DFF78 fully discovered and linked; next target 0x827CC418; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-17/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 18: 0x827CC418 fully discovered and linked; next target 0x82B53AC0; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-18/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 19: 0x82B53AC0 fully discovered and linked; next target 0x82B98D48; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-19/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 20: 0x82B98D48 fully discovered and linked; next target 0x8239BA60; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-20/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 21: 0x8239BA60 fully discovered and linked; next target 0x823A15A8; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-21/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 22: 0x823A15A8 fully discovered and linked; next target 0x82417D70; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-22/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 23: 0x82417D70 fully discovered and linked; next target 0x82417D40; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-23/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 24: 0x82417D40 fully discovered and linked; next target 0x82396EC0; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-24/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 25: 0x82396EC0 fully discovered and linked; next target 0x82396F60; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-25/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 26: 0x82396F60 fully discovered and linked; next target 0x8239BA88; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-26/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 27: 0x8239BA88 fully discovered and linked; next target 0x82394B28; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-27/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 28: 0x82394B28 fully discovered and linked; next target 0x82417D80; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-28/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 29: 0x82417D80 fully discovered and linked; next target 0x8244C508; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-29/result.json. No first-frame or gameplay acceptance.

2026-10-13 reviewed leaf batch: 368 new/5 preserved, 373 generated semantics independently PASS; cycle29 runtime evidence retained, main-menu/full-game acceptance remains open. Evidence docs/evidence/cards/08-leaf-generated-20261013.json.

2026-10-13 observed-entry loop 30: 0x8244C508 fully discovered and linked; next target 0x8244C538; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-30/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 31: 0x8244C538 fully discovered and linked; next target 0x8244C580; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-31/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 32: 0x8244C580 fully discovered and linked; next target 0x8244C5B0; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-32/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 33: 0x8244C5B0 fully discovered and linked; next target 0x82B8AC50; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-33/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 34: 0x82B8AC50 fully discovered and linked; next target 0x82384430; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-34/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 35: 0x82384430 fully discovered and linked; next target 0x82373698; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-35/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 36: 0x82373698 fully discovered and linked; next target 0x823DB9A0; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-36/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 37: 0x823DB9A0 fully discovered and linked; next target 0x82DAFFA0; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-37/result.json. No first-frame or gameplay acceptance.

2026-10-13 post-autosave progression: EA Canada production frame independently verified; 373+9+75 scoped generated callback audits PASS and complete old-body/mapping preservation PASS. Main menu/full offline/SBS/browser gates remain OPEN. Evidence docs/evidence/cards/08-post-autosave-film-20261013.json.

2026-10-13 observed-entry loop 38: 0x82DAFFA0 fully discovered and linked; next target 0x823DB438; probe exit -1073740791. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-38/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 39: 0x823DB438 fully discovered and linked; next target None; probe exit 0. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-39/result.json. No first-frame or gameplay acceptance.

2026-10-13 native tutorial: prison fight/HUD captured and user confirms punch input; cycle39 survived180seconds then supervisor closed it. Tutorial/menu completion, saves and entire offline/SBS/browser matrix remain open. Evidence docs/evidence/cards/08-native-tutorial-20261013.json.

2026-10-13 sustained tutorial rendering FAIL: fighters disappear before a later82DAFF88 callback crash. CauseUNKNOWN; single-variable synchronous-memexport comparison inprogress. Visible tutorial/user-confirmed punches do not close visual or stability acceptance. Evidence docs/evidence/cards/08-fighter-disappearance-20261013.json.

2026-10-13 observed-entry loop 40: 0x82DAFF88 fully discovered and linked; next target None; probe exit 0. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-40/result.json. No first-frame or gameplay acceptance.

2026-10-13 observed-entry loop 41: 0x82DAFEB0 fully discovered and linked; next target None; probe exit 0. Evidence research/rexglue-qualification-20261012/entry-loop-cycle-41/result.json. No first-frame or gameplay acceptance.

2026-10-14 recovery measurement: independent exp2 scoped PASS; dense capture distinguishes recovery/gameover from sustained fight. Portable FMA candidate tested, not adopted. Continue actual-control measurement; no gate promotion.


2026-10-14 FMA integration: compiled portable single-rounding candidate; exact frozen-body/archive checks and prior numeric regressions PASS. Probe55 fails recovery; prompt detector false-negative under repair. No gameplay or final-goal gate promotion. Evidence docs/evidence/cards/08-fma-integration-20261014.json.


2026-10-14 recovery58: normal lateral input centers view; missing RS-up detector state identified from actual renderer frames. No get-up/gameplay gate passes. ABI copied-class negative control identifies separate64-bit value truncation; not adopted. Evidence docs/evidence/cards/08-recovery58-20261014.json.


2026-10-14 manual60: visible responding user-session native15 game opened, automaticinput/timeoutclose disabled. Separate native61ABIbuild preserves manual15pair; detector59 criticPASS but no recovery runtimepass. Window existence is not gameplay acceptance. Evidence docs/evidence/cards/08-manual60-20261014.json.


2026-10-14 handoff: native61 reviewed build/pair PASS, never launched. Manual native15 pause frame narrowPASS; subsequent exit0xC0000409 without automaticstop is unresolved, no long-session acceptance. Detector59 reviewed, not runtimepassed. Next admitted61probe/crashstack/save-content evidence. See docs/HANDOFF_2026-10-14.md.
