# Quality-loop progress

## Native FPS user test279 — 2026-10-15

- Source build274 critic finds an interrupted-link success gap; successorR2 passes and actual276 compiles two units/links0 with stable889 pins. Fresh artifact review verifies strong hooks, fallback helpers and runtime imports.
- Actual derived archive and12,996 hardlinks pass independent reviews; only one248MB archive adds bulk payload.
- Manual278 fails before a window with explicit empty DLC option. Reviewed279 omits empty options and reaches a responsive game window. User is testing; physical controller input on, VSync off, no synthetic input or automatic close.
- Actual main/pause row visibility, cap change/persistence and performance gain remain open. DLC extended-path successor passes source review only.

## Actual game FPS row274 and startup273 — 2026-10-15

- FPS269R2 native callback/persistence source passes independent review. Safely edits only the private FPS key; no global CVar serialization.
- Resource274 independently passes source review and root creates one derived248MB archive. DISPLAY / Frame Rate Limit is explicitly appended to both main and pause contexts, matching eight backend choices. Native visibility remains unproved.
- Root275 actual-response metadata passes both integrated compiler commands. Build274 preparation critic finds a concrete interrupted-link success predicate defect; preserve it and repair a successor before compiling.
- The combined271 tuple is staged and launched by fresh owned-DLC startup273. First extraction fails at an output path261 UTF16 units; no headers complete, and child exits0xC0000374 without forced termination. The loaded-module snapshot is unavailable after the early exit. Extended path handling and exact error diagnostics are under repair. Immutable seeds and failed273 evidence are preserved.

## Native FPS menu and combined271 — 2026-10-15

- User rejects F4-only access: Frame Rate Limit must be in the game's own settings from main and pause menus. Shared resource/native callback269 candidate is in preparation; no UI acceptance yet.
- Manual254 closes ordinarily0; all native processes absent before rebuild.
- Actual260 exposes a real compiler response-parser mismatch; preserve it. Root262 actual-response proof and fresh253R3/261 critics precede successful four-TU compiles266/268.
- Combined271 links and independently passes ABI/import/all244-input checks. Still unstaged/unloaded; prepare fresh DLC import qualification and controller runtime checks.

## Manual254 and corrected link252 — 2026-10-15

- User test session254 is running on235 with physical SDL and no synthetic inputs or automatic stop; fresh profile has no DLC.
- Link252 passes fresh actual ABI review, preserving all10257 old ordinals. Candidate remains unstaged/unloaded; FPS unmeasured.
- Source251 passes actual-continuation review; effective inherited get-up wait is60seconds, correcting249's component-only45 claim. Runtime observation pending.
- DLC250 source passes after additive CRLF provenance correction; compile-only253 review and title-wide license caller audit continue.
- Physical/scripted controller arbitration255 preparation continues; current game avoids the overwrite path by disabling scripts.

## Actual244 / performance245 / DLC247 — 2026-10-15

- Actual244 source passed fresh review; runtime reached boxing and a visually confirmed get-up screen. Strict recognition failed at the neutral LS phase; prepare a narrow subtype while preserving neutral observation and lifecycle bounds.
- Performance245 linked but failed independent ordinal ABI review; candidate remains isolated. Explicit baseline ordinal repair is in progress.
- DLC246 invalidates complete-install inference from receipt237. DLC247 checked-I/O source awaits fresh critic; native behavior remains unverified.
- Replay248 source audit cannot connect field20 to replay identity or prove the current clamp fixes replays. Capture the full knockdown-to-recovery interval next.
- GitHub private repository created and source/documentation pushed; generated code, native artifacts and owned data remain local.

## Run167 — exact XEX wrapper and dispatcher hint repair — 2026-10-09

- Builder: constructed a research-only XEX2 wrapper around the exact Run166 effective image and recomputed `section_digest` using the loader's first-page algorithm. `xenon-prepare --query` accepted the wrapper and returned effective image SHA-1 `b771b41caef2483549bfbc7d726be3eb56943f99`.
- Builder: copied the hint package into the isolated Run167 area and added the required parent `FunctionHint` for `0x822A5A60`; the chunk and indirect branch now pass schema validation.
- Critic: exact identity binding passed; original inputs were not modified. Broad preparation reached wave 1 (`30031 analyzed`, `75107 newly discovered`) without producing output before bounded termination. Narrow preparation reached the same analysis phase and was interrupted before output creation. No alternate generated entry, native module, first frame, gameplay, SBS, browser, or release gate is proven.
- Evidence: `docs/evidence/cards/07-run167-xex-wrapper-hint-repair-20261009.json`; wrapper SHA-256 `1E4361641D84D3B9CB1C13DD6E208BB079EE76C0E11154687B740EAC19D9616B`; isolated hint analysis SHA-256 `D60BEE42C0ACA6496A9F64DF830576B9C3E29412DD2B695EDE8613D2B4EC6255`.
- Next: reuse the exact-image graph/cache or perform a direct isolated generated-source repair, then build and rerun the bounded exact-TU2 probe.

Target: complete standalone native/browser-native in normal and true SBS. Historical emulator/streaming records do not close current gates.

## Current gate

- Cards 00–02: PASS (fresh critic recorded in the project log).
- Card 04: INCOMPLETE; native capture remains unavailable.
- Card 07: IN_PROGRESS. Self-run Release Run99 loaded exact TU2, bound the active native module, and created a D3D12 window; it did not verify a first frame. GPU ring/front-buffer addresses stayed zero and the trace recorded no Vd graphics call. The active generated import table maps `0x82EE2B3C` to `xboxkrnl.exe!RtlLeaveCriticalSection` (ordinal 304 / `0x130`); runtime logs show the handler executing, so repeated dispatch to that thunk is not proof of an import bug. The saved status is stale after foreground interruption; the host PID was confirmed absent. Do not change import semantics on this evidence.
- Next action: use Run99's exact loaded TU2/module and trace the last main-thread PCs, their callers, and the graphics/device/swap/present entrypoints with bounded arguments, return statuses, and thread IDs. Use a clean stop/snapshot protocol, then capture a real frame. Keep Card 07 open until a first frame is verified.
- Focused Debug CTest suites: 4/4 passed with `-C Debug`; a first rerun without `-C Debug` was invalid for this multi-config build and reported all tests as Not Run. Do not count that invocation as test failures.
- Evidence: `docs/evidence/cards/07-run99-import-boundary-20261008.json`; previous loop and synchronization observations are historical and remain in `docs/evidence/cards/07-guest-loop-inspection-20261007.json` and `docs/evidence/cards/07-wait-loop-trace-20261007.json`.
## Rules

- Xbox workspace only; PS3 material is deprecated and excluded.
- Original ISO, executable, title updates, DLC, keys, and Xenia installations are read-only inputs.
- A compile never closes a runtime gate; every card requires evidence.



- Card 05 PASS after controlled continuation: XexTool-RE prepared a copy; Ghidra imported/analyzed the PowerPC basefile. Card 04 remains incomplete.
- Card 06 BLOCKED: XenonAnalyse generated an empty jump table and XenonRecomp stalled with no output on the prepared XEX.

- Deep upstream research recorded in research/CARD_06_RESEARCH.md: empty jump tables and silent XenonRecomp runs are known limitations; next step is evidence-backed function-boundary/helper-address configuration.

- Located all eight XenonRecomp restore/save helper signatures: 0x8229B390, 0x8229B340, 0x82C71C3C, 0x82C71BF0, 0x82C72398, 0x82C72100, 0x82C7242C, 0x82C72194. Relative-path configs remove the absolute-path bug, but generation still yields no C++.

- 2026-10-02 Card 06 critic: corrected ELF structure independently checked; full and reduced XenonRecomp runs stalled with zero generated units. Evidence recorded in research/CARD_06_RESULT.json and docs/evidence/cards/card-06.md. Next: parser/Analyse checkpoints.

- 2026-10-02: stale XenonRecomp processes were terminated; diagnostic source backups removed; no native output was generated.


- 2026-10-02: patched full XenonRecomp run became CPU-active but grew to ~4.2 GB with zero output; terminated safely. Next: validate .pdata range/length bounds and prevent unbounded function graph expansion.

- 2026-10-02: .pdata audit passed (46,160 ordered entries); patched recompiler now uses .pdata exclusively for function discovery. Need non-blocking captured run to verify C++ output.

- 2026-10-02: Bun launcher prototype exposed incorrect Windows child-process polling (p.exited misuse); orphaned test processes terminated. Need correct Bun wait API for non-blocking run.

- 2026-10-02: created project completion-loop workbench at .codex/loop.md with Card 06 gate, non-blocking launcher rule, builder/critic checks, and no-routine-guidance stop condition.

- 2026-10-02: loop iteration 01 FAIL. Patched toolchain rebuilt, but PowerShell background-job launcher produced no output and no translation units; recorded in research/generated/<hash>/CARD_06_LOOP_01.json. Next: native cmd wrapper with explicit path/exit validation.

- 2026-10-02: loop iteration 02 FAIL. Native cmd wrapper launched the compiler but created no exit marker/output and left nested wrappers; evidence in CARD_06_LOOP_02.json. Next: single launcher plus no-argument command-line sanity probe.

- 2026-10-02: Card 06 PASS. Explicit output-directory creation fixed the empty-output symptom. Full XenonRecomp generation exited 0 twice at 100%; 186 files / 183 C++ units, 204,947,209 bytes, zero hash differences. Next: Card 07 runtime scaffold.
- 2026-10-02: Card 07 builder pass started. Audited generated output: 48,847 indirect call sites and 6,204 unsupported-instruction warnings. Added a fail-loud Rust runtime scaffold plus C++ integration boundary; `cargo test` passed. Input/audio/video/graphics/saves/DLC remain explicitly unsupported, and native C++ compilation is blocked by the environment's unavailable compiler/Ninja execution. Next: classify imports and bind services from observed contracts.
- 2026-10-02: Card 07 repair cycle 01. Corrected the CMake invocation to use the pinned workspace Ninja, `C:\LLVM\bin\clang++.exe`, and Windows SDK `rc.exe`; native runtime build and CTest now pass. Service implementations remain open; no unsupported contract was hidden.
- 2026-10-02: Card 07 repair cycle 02. Added `native/runtime/build.ps1` so the corrected tool paths are repeatable; a clean configure/build/test run passed. Next: implement and test the six unbound platform services from observed game contracts.
- 2026-10-02: Card 07 repair cycle 03. Implemented deterministic Rust backends for input, audio, video, graphics, saves, and DLC; lifecycle tests now cover all six and pass alongside the repeatable C++ boundary build. The contracts are ready at the interface level; title-specific call mapping and real Windows adapters remain before Card 07 can pass.
- 2026-10-02: Card 07 repair cycle 04. Added the C ABI bridge and dynamic Windows XInput adapter, then fixed the hard `xinput1_4.lib` dependency by loading the DLL at runtime. Rust tests, C++ CTest, and the Rust static library all pass. Audio/video/GPU OS adapters and generated-call wiring remain open.
- 2026-10-02: Card 07 repair cycle 05. Added an end-to-end C ABI lifecycle/SBS test; Rust now passes 2 tests, CTest passes, and the static library builds. The remaining work is actual generated-PPC call mapping plus OS audio/video/GPU integration.
- 2026-10-02: Card 07 repair cycle 06. Fixed generator output defects, regenerated the full image, compiled all 183 generated C++ units into `fnc_generated.lib`, and linked `fnc_native_probe.exe`. The generated core is now compiler-clean; executing its entry point and binding real graphics/audio/video services remain the next gate.
- 2026-10-02: Entry-probe cycle. Added guest memory reservation, PE section mounting, generated function lookup installation, and a guarded `_xstart` call. The first probe passes reservation/mount/46,390 mappings but raises an access violation before the title screen; evidence is preserved. Next: trace the first missing kernel/heap/thread/import contract.
- 2026-10-02: Entry-probe repair cycle 07. Fixed the probe link map flag, added fault ownership/guest stack diagnostics, and corrected PPC pointer semantics to use 32-bit guest addresses. The generated entry now returns after two demand-page commits, but no title screen appears; the actual blocker is XEX startup/import ABI recovery, not PE mounting.
- 2026-10-02: Entry-probe repair cycle 07. Fixed the probe link map flag, added fault ownership/guest stack diagnostics, and corrected PPC pointer semantics to use 32-bit guest addresses. The generated entry now returns after two demand-page commits, but no title screen appears; the actual blocker is XEX startup/import ABI recovery, not PE mounting.
- 2026-10-02: Card 07 repair cycle 08. Parsed the generated C++ independently and recorded 30,102 distinct direct-call targets that XenonRecomp left as ERROR comments. This is the actual native-boot blocker: the core compiles, but function-boundary/import analysis is incomplete. No generic no-op was counted as a fix.
- 2026-10-02: Card 05 critic cycle 02. Rechecked the Ghidra export evidence and found the Java/Python metadata scripts never emitted their output; the managed Ghidra launcher cannot write its java_home.save profile. Preserved the failure and recorded XexTool/identity/import catalog as a replacement inventory, while keeping the metadata subgate explicitly blocked.

- 2026-10-02: Card 05 repair cycle 03 PASS. Cleared Ghidra's stale compiled Java-script cache; headless Java export produced 25,170-function card05-metadata.txt. 
- 2026-10-02: Card 07 repair cycle 09. Added all 30,102 unresolved direct-call targets as explicit 4-byte functions; XenonRecomp produced 76,270 mappings, unresolved scan count=0, and the expanded core compiled/linked. Boot probe still returns before title initialization; startup ABI/runtime setup remains open.

- 2026-10-02: Card 07 repair cycle 09. Added all 30,102 unresolved direct-call targets as explicit 4-byte functions; XenonRecomp produced 76,270 mappings, unresolved scan count=0, and the expanded core compiled/linked. Boot probe still returns before title initialization; startup ABI/runtime setup remains open.

- 2026-10-02: Card 07 repair cycle 10. Matched Xenia's Processor::Execute stack reserve/LR behavior and added a minimal big-endian PCR-to-TLS guest layout to the probe. Rebuilt and reran the expanded native probe; reservation, PE mount, 76,270 mappings, 302 import slots, and TLS/PCR setup pass, but entry still returns before title initialization. This repair did not close the startup contract.

### Card 07 research update — 2026-10-02

Online source review confirms the blocker is the Xbox process bootstrap and kernel-service contract. The recompiler output is linkable and has no unresolved direct-call placeholders, but the standalone probe is not equivalent to Xenia `LaunchModule`. See `research/CARD_07_XENIA_BOOT_CONTRACT.md` for the source links and repair order. Next gate: implement and trace process/module state, XThread/TLS/PCR lifecycle, and real startup XAM/xboxkrnl services.

- 2026-10-02: New Card 07 blocker: the Xenia reference log proves Title Update 2 changes the effective entry point from retail `0x82297370` to `0x82297338` and expands the image to `0x01360000`. The current generated image is retail-only, so its boot result cannot represent the selected runtime. Verify and freeze a patched image before more ABI work.

## 2026-10-02 — Card 07 exact patch identity blocker

Rebuilt the local XEXP verifier and independently checked every package copy. The selected retail XEX does not match the patch source signature, and strict application fails at patch block offset `0x9E800`; Xenia's successful TU2 log remains the reference. Continue only by acquiring the exact XEX/XEXP byte stream Xenia used or a traceable patched guest image; do not count the current native probe as a TU2 boot.

- 2026-10-02 Card 07 cycle 13: tested an isolated hash-mismatch override; it crashed without output, confirming the current base/patch pair is incompatible. Strict gate remains BLOCKED; no promotion.


- 2026-10-02 loop pivot alignment: strategic plan and README now identify verified Xenia Netplay + render-boundary SBS + Bun/WebRTC as the active route. Next gate is host freeze and launcher reproducibility.


- 2026-10-02 pivot host freeze PASS: launcher check-only and manifest validation pass; Xenia log proves TU2 application and DLC path routing. Fresh DLC visual acceptance and render-boundary source acquisition remain open.


- 2026-10-02 critic pass 15: host manifest and launcher pass; corrected content-root handling and readback assertion. Stereo hook design recorded; implementation awaits Xenia source acquisition.


- 2026-10-02 source blocker cleared with elevated network retry; Xenia Canary pinned at b083312b8b18e07e6e410b82104191f126722794. IssueSwap is the first verified D3D12 output boundary.


- 2026-10-02 stereo pass 01: mapped IssueSwap -> RefreshGuestOutputImpl. Critic rejected presenter-only duplication as non-stereo; next is guest camera/projection interception.

- 2026-10-02 — Browser transport builder pass: added a Bun-only signaling server and browser client under `web/launcher/`. The server relays offer/answer/ICE by explicit session and never reads game files; the client sends controller events over a WebRTC data channel and expects the native host to supply SBS video. Critic: `bun test` passed (1/1), source contains no proprietary data, and the client correctly remains waiting until a real host adapter exists. This does not satisfy the SBS gate.
- 2026-10-02 — Stereo research critic pass: audited pinned `xenia-edge` commit `4aa6327ccf03447632adfde5b4ead9db2e11320d`; no graphics stereo implementation exists. Added `research/pivot/XENIA_EDGE_STEREO_AUDIT.md`. Largest remaining gap is still independent per-eye rendering in the Xenia graphics path.
- 2026-10-02 — Browser transport critic repair: added same-session role-isolated offer relay coverage; `bun test` now passes 2/2 and `git diff --check` is clean for active docs/web/native/pivot paths. Manifest refreshed at `research/pivot/BROWSER_TRANSPORT_MANIFEST.json`. Browser playback remains gated on a real native SBS host track.
- 2026-10-02 — Xenia source build repair: initialized recursive submodules, restored incomplete worktrees, and reran the pinned release build. CMake now configures and D3D12 sources compile; the build stops at Vulkan shader generation because the required shader compiler executable is absent (`WinError 2`). Evidence is preserved in `research/pivot/XENIA_BUILD_BASELINE.md`; this is a toolchain repair item, not a stereo acceptance result.
- 2026-10-02 — Xenia build repair critic pass: installed Vulkan SDK 1.4.363.0, reran the pinned release build, and passed all 397 actions. The generated executable and tool hashes are recorded in `research/pivot/XENIA_BUILD_MANIFEST.json`. Source modification for true SBS remains the next gate.
- 2026-10-02 — Stereo builder pass 02: added an opt-in projection-space eye parameter to the pinned Xenia DXBC translator and D3D12 system constants. Incremental release build passed all 18 changed actions. Critic: this is only a camera-shift primitive; without per-eye replay/targets it is explicitly diagnostic and cannot pass true SBS. Manifest: `research/pivot/XENIA_STEREO_PATCH_MANIFEST.json`.
- 2026-10-02 — Stereo pass 02 verification: `git diff --check`, Bun signaling tests (2/2), launcher `-CheckOnly`, and JSON manifest parsing all pass. No acceptance claim changed; dual-eye render-target/replay work remains open.
- 2026-10-02 — Stereo research pass: audited Xenia's existing `enable_3d_mode` path and upstream stereoscopic issue. The flag only advertises a capability; it does not render or pack two eyes, and the reported driver path is not SBS. Recorded as negative evidence in `research/pivot/XENIA_TRUE_SBS_IMPLEMENTATION_DESIGN.md`.
- 2026-10-02 — Stereo builder pass 03: added opt-in instanced two-eye shader support (`SV_InstanceID`) and `d3d12_stereo_sbs` draw-count control to the research Xenia source. Rebuilt successfully under the VS developer environment (18/18 actions). Critic: runtime reached configuration only; no gameplay or parallax capture, so the patch remains diagnostic and disabled by default. Manifest refreshed with source and executable hashes.
- 2026-10-02 — Stereo repair pass 04: corrected the projection-half scaling bug in the instanced transform, ran `stereo_transform_test.ps1` (PASS), and rebuilt Xenia (18/18 actions). Critic: this proves only transform algebra and compilation; no GPU/gameplay frame has passed yet.

## 2026-10-02 — startup probe repair
- Fixed the unattended research probe stall by bypassing only the isolated source build's first-run disclaimer and quoting paths with spaces in the launch command.
- `startup_baseline.log` proves emulator construction, setup return `0`, exact XEX launch, D3D12 initialization, and DLC enumeration.
- Reverted the experimental SV_InstanceID/instanced SBS patch after independent log review found repeated pipeline creation failures and no rendered-frame evidence. The SBS gate is still open.
- Manifest: `research/pivot/XENIA_STEREO_PATCH_MANIFEST.json` status `COMPILED_STARTUP_PROBE_ONLY`.
- Follow-up evidence: `startup_baseline.log` reaches the game module and enumerates DLC, but D3D12 reports pipeline-creation failures even after restoring the pinned shader translator. Treat graphics output as unverified until the pipeline errors are traced to shader cache/feature configuration or an upstream Xenia compatibility issue.
- Vulkan probe (`research/pivot/startup_vulkan.log`) reached the exact game module with setup `0`, created a 2880x1620 Vulkan swapchain, and avoided the D3D12 pipeline failures. This makes Vulkan the preferred research backend for the next stereo fixture. Warnings about invalid shared-memory upload ranges remain to be investigated.
- Vulkan relaxed-upload run (`startup_vulkan_relaxed.log`) is clean through setup, swapchain, module launch, and pipeline cache load. `gpu_allow_invalid_upload_range=true` removed the invalid upload-range warnings; retain this only in the isolated research configuration until its safety is validated.
- Added a repeatable Vulkan probe launcher so the graphics baseline can be rerun without manual command-line quoting errors.

## 2026-10-02 — true SBS frame proven
- Vulkan SPIR-V instancing now renders two shifted eye instances into the SBS halves.
- Evidence capture: `research/pivot/vulkan_stereo_after_intro.png`.
- Independent pixel check: mean left/right RGB difference 83.6844; different-pixel fraction 0.5701.
- Build/runtime log: `research/pivot/startup_vulkan_stereo1.log`; setup 0 and exact XEX launch confirmed.
- Next critic gate: reach a playable fight and verify parallax, depth isolation, HUD policy, and long-session stability.

## Loop continuation pass 02 — 2026-10-02
- Builder: reproducible `Start-Xenia-Vulkan-Probe.ps1 -Stereo` now pins profile/XUID and keyboard mode; fresh profile-path SBS capture measured 189.3787 mean RGB delta and 0.999021 differing fraction.
- Builder: browser offer now explicitly requests a recvonly video transceiver; host contract and proprietary-data-free portable staging are recorded. Bun tests 2/2 pass.
- Critic: capture remains intro/video and no native WebRTC host adapter exists. Rebuild rerun is environment-blocked by Ninja access denial. Full fight, browser playback, and acceptance remain open.
- Evidence: `research/pivot/SBS_LOOP_PASS_02.json`, `research/pivot/vulkan_profile_stereo6.png`, `web/launcher/HOST_ADAPTER_CONTRACT.md`, `research/packages/fight-night-sbs-portable/PACKAGE_MANIFEST.json`.
- Browser follow-up: isolated Bun server health and static route returned 200; `recvonly` video transceiver is now in the client. Host answer/video is still blocked by the absent native adapter.

## Loop continuation pass 03 — 2026-10-02
- Builder: pinned keyboard mode and the discovered profile in the research config; repeated direct input and captured a fresh SBS frame.
- Critic: profile and input bindings appear in the log, but automation still cannot leave the intro/video surface. No fight-session evidence; user runtime untouched.
- Evidence: `research/pivot/SBS_LOOP_PASS_03.json`, `research/pivot/keyboard_enabled_capture.png`.
- Added `research/pivot/stereo_eye_policy_test.ps1`; endpoint/bounds/eye-order and single-target clear policy pass. HUD/video behavior remains unverified until a fight session is reachable.
- Input inventory found no Xbox/XInput/HID controller. Keyboard mode/profile are loaded, but direct injection remains on FMV; a deterministic fight state or usable controller is still required for gameplay evidence.

## Loop continuation pass 04 — 2026-10-02/03
- Builder: added browser-native `getDisplayMedia` host page and constrained local input bridge; portable staging now includes the host adapter and excludes tests.
- Critic: targeted Bun suite is 3/3 PASS and bridge no-target safety path passes. Live browser permission/video round-trip and playable fight capture remain open.
- Evidence: `research/pivot/SBS_LOOP_PASS_04.json`, `web/launcher/host.html`, `web/launcher/host.js`, `research/pivot/bridge_xenia_input.ps1`.

## Loop continuation pass 05 — 2026-10-03
- Builder: integrated and staged the browser-native getDisplayMedia host plus local input bridge.
- Critic: Bun 3/3 passes; live browser permission/video round-trip is unverified because the automation surface exposed no capture picker. Xenia research process was stopped; user runtime remains untouched.
- Evidence: `research/pivot/SBS_LOOP_PASS_05.json`.

## Loop continuation pass 06 — 2026-10-03

- Builder: fixed session propagation in both browser pages so `/host?session=...` and `/?session=...` populate the same signaling session; added an explicit unsupported-screen-capture status instead of silent no-op behavior. Restaged the writable package and refreshed its hashes.
- Fresh critic: targeted Bun tests pass 3/3; launcher `-CheckOnly -Stereo` passes; stereo eye policy harness passes; package manifest contains no game data; scoped `git diff --check` is clean. The user-owned Xenia process (PID 31340) was observed and left untouched.
- Gate result: browser host remains implemented but live permission/video/input round-trip is unverified in this automation surface; playable fight capture, HUD/video policy, reconnect/latency, and complete offline acceptance remain open. No completion claim is made.
- Evidence: `web/launcher/host.js`, `web/launcher/client.js`, `research/packages/fight-night-sbs-portable/PACKAGE_MANIFEST.json`, `research/pivot/STEREO_EYE_POLICY_TEST.json`, `research/RELEASE_MANIFEST.json`.

## Loop continuation pass 07 — 2026-10-03

- Builder: corrected the Vulkan stereo eye offset scale from `0.5` to `1.0`. The previous value mapped the two eyes into overlapping ranges, matching the repeated geometry in the supplied capture. Rebuilt `xenia_canary.exe` successfully with the Visual Studio developer environment.
- Fresh critic: the rebuilt executable hash is `287093588A07B29FE891B9F9A40E51ACBCAFA88050E54CCF7B9D8A8113AC7DA9`; the research process launched with Vulkan and the stereo flag and reported a responsive Xenia window. A fresh visual capture is still required before accepting the repair.
- Gate result: eye-offset repair is rebuilt but not yet accepted. Green corruption, HUD/video policy, playable fight validation, browser round-trip, and full acceptance remain open.
- Evidence: `research/pivot/SBS_LOOP_PASS_07.json`.

## Loop continuation pass 08 — 2026-10-03

- Builder: ran a non-stereo Vulkan isolation launch, then cleared only the isolated research shader/module caches and relaunched the rebuilt executable with SBS enabled. The user-owned runtime and its caches were not touched.
- Fresh critic: the rebuilt research process is responsive with the stereo flag and a new window title, but the automation desktop cannot capture the user-visible frame. Green corruption therefore remains unassessed after cache reset and is not accepted as fixed.
- Gate result: cache reset is diagnostic only; the render and full-game gates remain open.
- Evidence: `research/pivot/SBS_LOOP_PASS_08.json`.

## Loop continuation pass 09 — 2026-10-03

- Builder: traced the first-loading-screen stall to an incorrect direct launch. The command omitted the research storage/content roots, profile XUID, and keyboard mode; the log showed `keyboard_mode=0` and zero profiles. Relaunched through the pinned probe launcher with `keyboard_mode=1`, XUID `B13EBABEBABEBABE`, the research storage root, and the installed content root.
- Fresh critic: `vulkan_probe_20261003_021211.log` confirms `keyboard_mode=1`, one profile discovered, the game module loaded, the controller connected, DLC enumeration, and movie/audio threads starting. This closes the configuration-caused loading stall; visual gameplay and SBS corruption remain separate gates.
- Evidence: `research/pivot/SBS_LOOP_PASS_09.json`.

## Loop continuation pass 10 — 2026-10-03

- Builder: traced the gameplay failures to two concrete Vulkan defects. Asynchronous pipeline creation returned null handles during the first draws, and relaxed invalid guest uploads copied no-access memory into textures. Disabled asynchronous Vulkan pipeline creation in the isolated research config and changed relaxed invalid uploads to zero-fill instead of reading undefined memory.
- Fresh critic: `vulkan_probe_20261003_024325.log` shows 170 pipelines created synchronously, zero `IssueDraw`/`PM4_DRAW` backend failures, controller connection, and only zero-fill warnings for invalid ranges. The research window remained responsive. A new user-visible frame is still required to verify visual quality and parallax.
- Gate result: the loading/draw backend blocker is repaired in the research host; SBS visual acceptance and full-game acceptance remain open.
- Evidence: `research/pivot/SBS_LOOP_PASS_10.json`.

## Loop continuation pass 11 — 2026-10-03

- Builder: confirmed the non-stereo Vulkan baseline has no draw backend failures, then replaced global `instanceCount=2` SBS draws with explicit two-pass replay. Each eye now uses instance count 1 and a per-eye constant base offset; the normal presenter receives one packed render target.
- Fresh critic: rebuilt and launched with stereo. `vulkan_probe_20261003_025723.log` reports 170 pipelines, zero `IssueDraw` failures, zero `PM4_DRAW` backend failures, and only controlled zero-fill warnings. Visual acceptance still requires a fresh user-visible frame.
- Gate result: the four-panel instancing path is removed from the research build. Parallax, HUD/video policy, long-session behavior, full-game acceptance, and browser delivery remain open.
- Evidence: `research/pivot/SBS_LOOP_PASS_11.json`.

## Loop continuation pass 12 — 2026-10-03

- Builder: corrected an unconditional disclaimer bypass so the isolated skip_disclaimer setting is authoritative, set that research-only setting explicitly, repaired the stereo mapping harness to match the explicit two-pass shader math, and refreshed manifest hashes after the rebuild.
- Fresh critic: release link passed (3/3), launcher check-only passed, stereo transform and eye-policy harnesses passed, Bun tests passed 3/3, all pivot JSON parsed, and fresh stereo runtime ulkan_probe_20261003_074355.log reached module launch with 170 pipelines and zero IssueDraw/PM4_DRAW/pipeline-null/descriptor failures. Only controlled zero-fill warnings remain.
- Gate result: review pass is clean for the inspected source/tooling artifacts. Visual fight-session, browser live roundtrip, and full acceptance gates remain open; no release claim is made.
- Evidence: 
esearch/pivot/SBS_LOOP_PASS_12.json, 
esearch/pivot/STEREO_TRANSFORM_TEST.json, 
esearch/pivot/XENIA_STEREO_PATCH_MANIFEST.json.


## Loop continuation pass 13 — 2026-10-03

- Builder: fixed the right-eye perspective defect by applying the explicit replay base offset before multiplying by clip W; added perspective-W regression coverage and rebuilt the isolated executable.
- Fresh critic: build, stereo transform, eye-policy, Bun 3/3, and diff checks pass. A cold-cache Vulkan probe reached module launch and loaded 16 shaders with zero draw/backend/pipeline-null/descriptor failures, but it was stopped before a warm-cache visual frame was available.
- Gate result: the right-eye math repair is source/test clean but visual acceptance remains pending. No SBS or full-game completion claim is made.
- Evidence: 
esearch/pivot/SBS_LOOP_PASS_13.json, 
esearch/pivot/STEREO_TRANSFORM_TEST.json, 
esearch/pivot/vulkan_probe_20261003_160129.log.


## Loop continuation pass 14 — 2026-10-03

- Builder: completed a full SBS wiring audit and fixed the launcher argument disconnect. Quoted storage/content/target paths and explicitly pass --vulkan_stereo_sbs=true plus --vulkan_stereo_ipd; added effective cvar logging. The shader now carries depth-dependent clip-space eye separation and W-scaled half packing; unused instance-index interface wiring was removed.
- Fresh critic: ulkan_probe_20261003_214120.log proves ffective SBS=true IPD=0.02, the frozen content root, module launch, and no startup draw/pipeline/descriptor failures. Release build, transform/eye-policy harnesses, Bun 3/3, JSON parsing, and diff checks pass.
- Gate result: SBS wiring is internally connected, but visual acceptance is still open. HUD/FMV mono policy, depth/clear review, browser live roundtrip, and full-game acceptance remain incomplete.
- Evidence: 
esearch/pivot/SBS_WIRING_AUDIT_20261003.json, 
esearch/pivot/SBS_LOOP_PASS_14.json.


## Pass 15 — 2026-10-03
- Fixed perspective stereo wiring: camera eye translation is now scaled by clip W in the SPIR-V vertex transform; updated transform harness and rebuilt the isolated Vulkan executable.
- Builder checks: Release build PASS; stereo transform PASS; eye policy PASS; Bun browser tests 3/3 PASS; git diff check PASS (only environment ignore-file warnings).
- Fresh critic: launcher/log confirms effective SBS=true, IPD=0.02. Xenia F12 guest capture saved at 
esearch/xenia-source/build/bin/Windows/Release/screenshots/45410915/45410915 - 2026-10-03T23-02-33.png; it is an SBS-packed intro movie, not a gameplay frame, so visual parallax acceptance remains open.
- Blocker evidence: isolated research run remains in MoviePlayer2 Decode Thread/data/common/fmv; no warm-cache fight frame was produced. Do not claim full SBS acceptance until a fight-session capture shows non-identical left/right geometry without corruption.

- Additional critic attempt: copied the existing profile metadata into isolated 
esearch/pivot/content, then sent the correct keyboard HID bindings (A=;, B=', Start=X) to the research window. The log still remained in MoviePlayer2 Decode Thread; this is evidence of an emulator/game startup gate, not an unverified claim of SBS success.

- 2026-10-04 Card 07 exact-TU2 repair cycle 14: isolated Xenia post-title-update dump captured and SHA-256 verified; fresh Ghidra import/analysis and metadata export pass (25,470 functions). XenonAnalyse accepts the image but XenonRecomp stalls with zero output when pointed at the loaded-memory image; preserve this as a distinct input-format blocker and investigate conversion to a valid patched XEX/ELF rather than retrying unchanged.
- 2026-10-04 Card 07 repair cycles 15–18: reconstructed a section-preserving PowerPC big-endian ELF from the exact TU2 guest image; XenonRecomp full generation PASS (300 files, 209,555,542 bytes); MSVC compile rejected due GNU extensions; pinned Clang compile/link PASS for all generated units and native probe. Runtime bootstrap/title execution remains unproven.
- 2026-10-04 Card 07 repair cycle 19: corrected invalid XenonRecomp range constants in the exact-TU2 generated config from the captured PE section map; rebuilt all units with Clang and reran the probe. Correct image/code ranges and mapping now pass; title execution/runtime services remain open.
- 2026-10-04 Card 07 critic: corrected exact-TU2 probe mounts the image and installs 75,636 mappings/302 imports, but entry returns before title init. Exact output contains 256,572 `// ERROR` markers, so compilation is not equivalent to complete translation; next repair is comprehensive generated-call coverage and runtime service dispatch.

## 2026-10-04 — exact-TU2 unresolved-target repair audit

- Builder cycle: extracted 256,572 generated `// ERROR` markers from the exact TU2 translation and isolated 64,777 unique targets inside `.text`; generated `FNC-tu2-complete.toml` without modifying source inputs.
- Repair attempt: pre-created `ppc-tu2-complete` and invoked XenonRecomp against the exact section-preserving ELF. The process remained responsive but produced no output after >90 seconds; it was terminated. This is evidence that bulk insertion of every unresolved target is not a viable repair.
- Second repair attempt: limited insertion to 295 unique unresolved targets found in the entry translation unit (`ppc_recomp.0.cpp`). XenonRecomp again produced zero output and stalled; it was terminated. No generated output was promoted.
- Critic result: successful compilation of the prior 300-unit output remains insufficient; the entry path still contains unresolved control-flow targets (including `0x8229EAF0`, `0x8229F0F0`, and `0x82294EC8`). Native runtime bootstrap remains blocked. Next action is to identify valid function boundaries for the entry targets from the fresh Ghidra program and regenerate only boundary-aligned functions, rather than treating branch labels as 4-byte functions.

- Cycle 22: exact Ghidra metadata confirmed `0x8229EAF0` and `0x8229F0F0` are real function entries (adjacent boundaries imply sizes `0x488` and `0x968`). A boundary-aligned two-function XenonRecomp config was attempted; it again produced zero output and stalled, then was terminated. This indicates the current XenonRecomp build/config path cannot safely expand beyond the original function set even for valid entries. No output was promoted.

- Cycle 23: corrected the generated TOML array separator and retried two metadata-derived functions (`0x8229EAF0`/`0x8229F0F0`) with exact sizes. The corrected config still produced no output after more than 90 seconds; XenonRecomp was terminated. This rules out the earlier missing-comma hypothesis and leaves translator overlap/unsupported control-flow handling as the concrete cause.

## 2026-10-04 — blocker cause fixed; boundary repair compiled

- Root cause of the prior apparent stalls was malformed generated TOML: the repair script inserted a literal `` `r`n `` token instead of a newline. The corrected config serialized valid TOML.
- A boundary-aligned config adding Ghidra-confirmed functions `0x8229EAF0` (size `0x488`) and `0x8229F0F0` (size `0x968`) completed XenonRecomp with exit 0.
- The repaired 300-file output compiled and linked with Clang through the NMake fallback after Ninja became unavailable. `fnc_native_probe.exe --boot` passes the same memory/import/stack/entry checks; unresolved markers fell from 256,572 to 253,974.
- This unblocks controlled translation expansion but does not yet prove title initialization or a playable native game.

## 2026-10-04 — PowerPC instruction coverage expansion

- Fresh critic grouped the exact TU2 warnings and found the dominant unsupported families matched the upstream XenonRecomp issue list. Implemented validated scalar/memory cases and vector aliases in the pinned local recompiler: `addc`, `addme`, `bdzf`, `eqv`, `subfze`, update-form loads/stores, `vsel128`, `vnor128`, `vsububm`, and `vsubuwm`.
- Rebuilt XenonRecomp successfully. Exact TU2 regeneration completed successfully; unrecognized-instruction warnings fell from 6,237 to 1,605 (74.2% reduction).
- The new 300-file output compiled and linked with Clang/NMake. The exact guest boot probe still passes memory, image mount, imports, stack, and entry return. Full runtime initialization is still pending.

## 2026-10-04 — scalar/update instruction pass

- Added validated scalar/update handlers for `lfdu`, `lfdux`, `lwzux`, `lhaux`, `stfdu`, `stfdux`, `stbux`, `stdux`, and `bdnzt`, plus the non-128-bit `vandc` alias.
- XenonRecomp rebuilt successfully. Exact TU2 regeneration completed successfully with warnings reduced from 1,605 to 1,547.
- The new 300-file output compiled and linked in `research/t5bc-nmake`; the exact-image boot probe still passes all current ABI/bootstrap checks.

## 2026-10-04 — zero-unsupported-warning generation and clean native compile

- Builder regenerated all 300 translation units from the exact TU2 section-preserving ELF after fixing two generator defects: immediate-scale emission for `vcfpuxws128` and per-instruction names for vector shift temporaries. XenonRecomp exited 0 and the regeneration log contains **0 unsupported-instruction warnings**.
- Fresh critic inspected the generated source directly: 297 C++ units plus mapping/shared headers are present; the generated source still contains 253,974 `// ERROR` control-flow markers, so zero opcode warnings is not equivalent to complete translation.
- The generated library and native probe compiled and linked with Clang 22.1.8/NMake in `research/t6bc-nmake`; the exact-image ABI probe passes memory reservation, image mount, 75,638 function mappings, 302 imports, stack setup, demand commits, and entry return.
- Compiler warnings remain for tautological carry comparisons in generated code. This is recorded as a correctness defect for the next critic cycle. Title initialization, runtime services, and full-game behavior remain unproven; no completion claim is made.

## 2026-10-04 — full fresh-Ghidra function-set recompilation

- Builder derived 25,487 non-overlapping `.text` function ranges from the fresh TU2 Ghidra metadata and regenerated XenonRecomp output from the exact section-preserving TU2 ELF. XenonRecomp exited 0 with zero unsupported-instruction warnings across 279 generated C++ units.
- Fresh critic found three duplicate generated definitions caused by special save/restore helper overlap (`0x8229B378`, `0x8229B3D0`, `0x82C723A8`). Those three config entries were removed, output regenerated, and a direct symbol audit reports zero duplicate definitions.
- The corrected 71,137-function mapping compiled and linked with Clang 22.1.8/NMake in `research/t8bc-nmake`. The exact-image probe passes with 71,137 mappings, 302 imports, four demand commits, and entry return.
- Direct generated-source audit still finds 183,007 unresolved `// ERROR` control-flow markers. The probe is an ABI/bootstrap harness and returns before title initialization; the native game is not complete or playable.

## 2026-10-04 — Xenon Book-E `mullhwu` correctness repair

- Fresh critic found generated `mullhwu.` instructions were emitted as comments with no operation, despite zero unsupported-opcode warnings. Added the Xenon Book-E low-halfword unsigned multiply handler and regenerated the full 25,487-function output.
- The regenerated 71,137-function library and probe compile/link successfully. The exact-image probe now reaches six demand commits before an access violation at guest function `0x82295D18`; this is useful runtime evidence that the probe has moved beyond the earlier immediate return.
- The access violation is caused by missing real runtime/entry state and unresolved path calls, not an unvalidated “working game” claim. Generated source still contains 183,007 unresolved control-flow markers.

## 2026-10-04 — Full TU2 function-set rebuild and seeded guest-entry probe
- Builder: rebuilt `research/t8bc-nmake` from the 25,487-function Ghidra-derived config after restoring the exact image base/size; generated 71,137 definitions with zero XenonRecomp unsupported-opcode warnings. Outputs: `fnc_generated.lib` SHA256 `C16B9CB30A6CAF002317E73777E185145A344734D338BA6A3FC2778917ED2D24`, `fnc_native_probe.exe` SHA256 `45911B6EA9BEA97758BC25D72E6A2E2DE157BCBCB53705A3C2359C63C3AFDF85`.
- Builder repair: seeded loader-owned r27/r29-r31 argument blocks and demand-committed them before guest entry.
- Critic: seeded probe exited `0`, mounted exact TU2 image, installed 71,137 mappings/302 imports, demand-committed six pages, and returned `guest_entry_return=PASS`; this proves ABI/bootstrap entry only, not title initialization or playability.
- Gate: runtime remains incomplete with 183,007 generated control-flow markers and no complete xboxkrnl/XAM/filesystem/graphics/audio/input/save/DLC runtime.
- Next action: trace the first post-entry guest path and implement the smallest evidence-backed runtime service boundary; preserve this passing probe as a baseline.

## 2026-10-04 — Critic repair cycle 23: shared interior entry experiment
- Hypothesis: the first post-entry `// ERROR 822A0710` is an external call into the interior of the Ghidra function `0x822A0358..0x822A0770`; splitting it into `0x822A0358..0x822A0710` plus `0x822A0710..0x822A0770` could lower the call.
- Repair attempt: generated `FNC-tu2-instructions8.toml` with that split and ran the pinned XenonRecomp binary.
- Critic evidence: XenonRecomp produced an empty log and remained resident with negligible CPU; it was terminated after evidence capture. The prior instructions7 output and passing seeded probe remain the canonical baseline.
- Result: repair cycle failed; no generated output was adopted. This confirms the compiler needs an explicit shared-interior-entry lowering or a validated Ghidra boundary repair, not an overlapping/split config guess.

## 2026-10-04 — Critic repair cycle 24: overlapping interior wrapper experiment
- Hypothesis: retain the canonical `0x822A0358` function and add a supplemental `0x822A0710` entry range so the missing call target becomes a generated symbol.
- Repair attempt: generated `FNC-tu2-instructions8b.toml` and ran the pinned XenonRecomp binary.
- Critic evidence: the process remained resident with zero-byte output and negligible CPU; it was terminated after the bounded observation window. No generated output was adopted.
- Result: overlapping ranges are not a viable repair. The toolchain needs explicit shared-interior-entry support rather than config duplication.

## 2026-10-04 — Full unresolved-target audit
- Independent critic scanned all generated translation units and correlated `// ERROR` targets with the configured function ranges.
- Evidence: `research/generated/EC2C955DB9E55A6207A391A2AB80F60332DC41C3FE4C07968C7F07D5A9621CA4/unresolved-target-audit.csv` and `interior-target-audit.csv`.
- Result: 183,007 markers collapse to 50,602 unique target addresses; 192 sampled/high-frequency targets fall inside another generated function, confirming shared interior-entry lowering is a systemic compiler issue rather than a single missing import.
- The canonical instructions7 output was not modified. Two wrapper/config experiments were terminated after XenonRecomp produced no output and hung; their evidence is retained.

## 2026-10-05 — Compiler interior-entry lowering pass
- Builder repair: added host-only `recomp_entry` dispatch state and compiler-generated interior-entry trampolines. Calls into the middle of another configured function now enter that function at the recorded label instead of emitting an immediate ERROR/return.
- Performance repair: indexed interior targets by owning function; the first implementation had an O(functions × targets) emission loop and was discarded. The indexed implementation generated normally.
- Critic evidence: XenonRecomp exit 0; 71,137 generated definitions; generated markers fell from 183,007 to 4,079. The new native build compiled and linked at `research/t9bc-nmake` with 1,084 compiler warnings but no errors. The seeded probe exits 0, installs 71,137 mappings/302 imports, demand-commits 11 pages, and returns `guest_entry_return=PASS`.
- Remaining: 150 unrecognized instructions (mainly `vpkswss128` and `vpkuwum128`) and 4,079 unresolved targets, mostly data/constant-region addresses; title initialization and platform runtime services remain incomplete.

## 2026-10-05 — Opcode cleanup and native rebuild
- Builder repair: added `vpkswss128`, `vpkuwum/vpkuwus128`, `vnor`, and `lvehx` lowering. The exact TU2 generation now reports zero unsupported instruction warnings.
- Critic evidence: `FNC-tu2-instructions11.toml` generated successfully; the full native library and probe rebuilt at `research/t11bc-nmake` with exit 0. Hashes: library `3AB932440B6AAA647C09021394AC4E1901E72EED2E25B2EBC231D996D9BE3A2F`, probe `FE869AEB24D0A665101DAE3CE93427CEE553852E1F09E666E8498A3EA8FAF04B`.
- Seeded probe evidence: `tu2-native-boot-probe-instructions11.log`; exit 0, exact image mounted, 71,137 mappings and 302 imports installed, 11 demand pages committed, `guest_entry_return=PASS`.
- Remaining: 4,079 ERROR markers are concentrated at addresses in the 0x82EE data/constant region and other non-symbol targets. The probe still only establishes generated ABI entry; title initialization and platform services are not implemented.

- 2026-10-05: instructions12 full-sections regeneration and corrected rebuild. XenonRecomp exit 0, unsupported warnings 0, 71,137 definitions, 4,079 unresolved markers. Fixed copied ppc_config bounds (PPC_IMAGE_BASE=0x82000000, size=0x1360000, PPC_CODE_BASE=0x82290000, size=0xC545A0); t12 native probe now passes mapping=71,137/imports=302/guest_entry_return. Hashes: lib F581EF5B5E00A372DD15B3B9E85D4354FD08A3411914DB8AAAD862EF6180FC30, exe B66530C23806897873E01D3915501895485513F22623516CD9309225A4D19DC0, log EB0BA41D59567395F62DC62AE793F1765A22A9BCDDF855F6303D6B097AFB6E43. Runtime/title init remains pending.

## 2026-10-05 — Exact TU2 zero-marker native build and critic probe
- Builder: regenerated from `tu2-sections-correct2.elf` with the repaired register-restore helper ranges (`+0xC4`) and startup entries. `FNC-tu2-instructions17.toml` produced 71,153 definitions with zero generated `// ERROR` markers and zero unsupported/unrecognized instruction warnings.
- Builder: compiled all 279 generated units and linked `research/t17bc-nmake/fnc_generated.lib` (SHA256 `B05402353A25284E8EBF1D491BF1FBFAC7A02B9AA005D6D7ED66F8CC77896AD6`) and `fnc_native_probe.exe` (SHA256 `7558D863E21D952C9A800ECA270F316C629674E07C33E4498B66CEA25E8164AA`).
- Fresh critic: exact-image probe mounted the TU2 guest and installed 71,153 function mappings/302 imports, but guest entry faulted with `0xC0000005` after seven demand commits at owner `0x82297BC8`. This is a real runtime bootstrap/address-translation failure, not a successful native game; the evidence log is `research/generated/EC2C955DB9E55A6207A391A2AB80F60332DC41C3FE4C07968C7F07D5A9621CA4/tu2-native-boot-probe-instructions17.log`.
- Gate: decompile/regenerate/compile subgate passes; playable native runtime remains blocked. Next action is to fix guest memory/address translation and implement the first required runtime services, preserving this zero-marker build as the current generated baseline.

## 2026-10-05 — Critic repair cycle 25: guest register-width semantics
- Hypothesis: the `0x82297BC8` fault came from stale upper halves of `PPCRegister` after generated `.u32`/`.s32` writes.
- Repair: created private `research/t18src` from the zero-marker output and rewrote 108,810 unsigned and 4,202 signed 32-bit register assignments with explicit zero/sign extension. Rebuilt all 279 units and the probe in `research/t18bc-nmake`.
- Fresh critic evidence: the exact-image probe still faults at owner `0x82297BC8` with the same guest access pattern one 4 GiB window below the reserved base after seven demand commits. Evidence: `research/generated/EC2C955DB9E55A6207A391A2AB80F60332DC41C3FE4C07968C7F07D5A9621CA4/tu2-native-boot-probe-instructions18-register-semantics.log` (SHA256 `8FC983A628D1471D16BCC1E02705B4579A60C7A4B019DACA9D55A025C63CF987`).
- Result: this repair did not change the failure. The remaining defect is missing loader/process/TLS/PCR/heap and kernel/XAM startup state, not generated instruction coverage. The t18 experiment remains quarantined; instructions17 remains the canonical zero-marker generated baseline.

## 2026-10-05 — Critic repair cycle 26: startup list sentinel
- Hypothesis: `sub_82297BC8` faults while walking the startup registration list because the loader has not initialized guest sentinel `0x823FED30`.
- Repair: seeded the self-linked sentinel in the runtime memory copy immediately before entry; the captured guest image remained unchanged.
- Fresh critic evidence: probe still faults at the same owner `0x82297BC8`, same seven demand commits, and the same one-window-below-base access. Evidence: `research/generated/EC2C955DB9E55A6207A391A2AB80F60332DC41C3FE4C07968C7F07D5A9621CA4/tu2-native-boot-probe-listhead-seeded.log` (SHA256 `30DDB8681635477C525DCF24D2A8D05CE060FC0DB7F1267A3B13E12CA9FDFE2D`).
- Result: the isolated probe lacks more loader state than this single list sentinel. The next runtime implementation must reproduce the Xenia process/module, TLS/PCR, heap, and startup registration contract together.

## 2026-10-05 — Critic repair cycle 27: guest-pointer emission and indirect-dispatch tracing
- Builder: patched the private XenonRecomp source to emit `PPC_GUEST_PTR` for raw guest-address operations that bypassed the load/store helpers, then built the quarantined `research/t19bc-nmake` candidate. The full 279-unit library/probe build completed with exit 0. Library SHA256 `343AFB32229ACFD081C26654279FF930A1CC709D2CB2780DC7DAA53CE63C1426`; probe SHA256 `E78C4AA2DE69A0C6145D0F068C39872160DFF86AD1E90EE44BC0B449C272620C`.
- Builder runtime repair: seeded the loader-owned static registration sentinel at `0x82EFEE30` and an empty init array at `0x82EF0000` in the private runtime copy. Added bounded null indirect-dispatch tracing and temporary owner routing for the observed interior callbacks `0x8229CD50` and `0x82438E68`.
- Fresh critic evidence: the pointer-emission candidate moved past the prior `0x82297BC8`/one-window-below-base fault. The trace identifies the next missing runtime/function-boundary targets as `0x8229CD50` (from `sub_82297B50`, `r3=1`, `lr=0x82297AA0`) and `0x82438E68` (from `lr=0x822903CC`). Evidence: `research/generated/EC2C955DB9E55A6207A391A2AB80F60332DC41C3FE4C07968C7F07D5A9621CA4/tu2-native-boot-probe-instructions19-dispatch-map.log`.
- Result: the candidate does not boot a game. Temporary owner routing causes repeated demand commits and was terminated after bounded evidence; it is not promoted. The remaining blocker is the XEX loader's indirect/interior function dispatch and missing runtime initialization, not compilation. The canonical instructions17 build remains unchanged.

- 2026-10-05: Critic repair cycle 28. Generated and rebuilt a 27,425-entry interior dispatch table for the quarantined t19 candidate. The fresh exact-image probe resolved the two prior startup interior targets and the next `0x8229FEB4` boundary. A bounded null/low-service callback experiment reached the runtime service/bootstrap path but exhausted 128 demand commits at owner `0x8229EB38`; no title screen or playable native runtime. Candidate remains quarantined and canonical t17 is unchanged.

- 2026-10-05: Critic repair cycle 29. Corrected null indirect-call handling in the private t19 runtime so callbacks no longer unwind their caller. Rebuilt with zero compiler errors; 27,425-entry interior map and zero direct `base +` generated accesses remain. Exact-image probe reached the loader/platform-service path and stopped at the 128-demand-page safety cap under owner `0x8229EB38`; no title screen. Candidate remains quarantined and native full-game completion is still blocked on the missing XEX loader/service contract.

- 2026-10-05: Critic repair cycle 30. Fixed a harness defect in the private t19 probe: it previously accepted only MZ/PE input even though the exact post-TU2 guest artifact is a raw image (`45410915_tu2_guest_image.bin`). The probe now mounts the exact raw image without modifying it, rebuilt cleanly, and installed 71,153 mappings/302 imports. Fresh run advanced through the startup path to repeated malformed indirect target `0x82006A98` (first observed with `r1=0xFFFFF460`, `r3=0xFFFFF4B0`, `lr=0x82290ED4`) and then `0x8229FEAC`; it did not reach title initialization. Candidate hashes: `fnc_generated.lib` `84EE4D937D078832A76BD545C442FF2C940C98D75BD51A49EE0F21675B6662E3`, `fnc_native_probe.exe` `FB65571D348B0D7A54A672AD82A197E94DDF6A7F8259781CB304C651A5D4F862`. Evidence: `research/t19bc-nmake/cycle30i.log`. The malformed target is evidence of missing loader/process/service state; no arbitrary no-op was promoted.

- 2026-10-05: Critic repair cycle 31. Applied the Xenia `ThreadState` ABI initialization (`r2=0x20000000`, zero non-argument registers) and removed fabricated loader argument blocks from the private probe. The rebuilt exact-image run reproduced the same `0x82006A98` loop and then `0x8229FEAC` after 34 demand commits, with no title screen. This rules out the fabricated r27/r29-r31 seed as the primary cause; the missing state is in the actual loader/process/service initialization. Evidence: `research/t19bc-nmake/cycle31.log`. Candidate remains quarantined.

- 2026-10-05: Critic repair cycle 32. Removed the private probe's blanket clearing of the captured `0x82EF0000..0x82EFEE00` initialization range to test the real TU2 constructor data. The candidate rebuilt cleanly, but the exact-image run became non-terminating with no title output; it was terminated after 20 seconds. This confirms raw constructor data cannot be executed without the real XEX relocation/loader/service contract. The prior bounded trace remains the useful evidence; no candidate was promoted.

## 2026-10-06 — authority alignment

- Updated plan/runbook/checklist/acceptance/risks/README/status to standalone native and browser-native, both normal and true SBS. Streaming is optional.
- TU2 module generation/compile PASS; runtime/gameplay remain open.
- Next: loader/decoder tests, host build and module-binding/entry probe.


2026-10-06 Card 07 PARTIAL: focused suites 4/4 PASS; exact TU2 module binding/startup executed headless with fallback observed, then unsupported-instruction trap at 0x82297D58. Real graphics/audio, import coverage, complete gameplay, SBS and browser remain open. Evidence: docs/evidence/cards/07-native-bootstrap-20261006.json. No acceptance row is promoted by this probe.



## 2026-10-08 — Dynamic XAM token dispatch test and startup probes 95–98
- Builder change: added six XAM party/community export identities and dynamic guest-thunk lookup for XAM exports missing from the XEX static IAT. Added a session-level test that proves repeated lookup stability and calls the resulting thunk through the production session dispatcher.
- Critic evidence: Release CTest `xenon_session_tests`, `xenon_xam_export_ordinal_tests`, and `xenon_runtime_host_tests` passed. Runs95/96 resolved all six late-bound XAM ordinals on exact TU2 to stable addresses `0x3FEF9000..0x3FEF9014`; those title-level callbacks were not observed being invoked.
- Run97: exact TU2/Release headless execution for 90 seconds, 2,840 guest blocks, cooperative stop; no frame or graphics evidence.
- Run98: same host and exact TU2 image, headless with less console logging, roughly six-minute execution and periodic guest-thread samples across IDs 1–5 and 9. It did not reach the dynamic party/UI lookup. Stop timed out after five seconds and the host terminated execution. This may be a slow or looping startup path; the present sample evidence does not distinguish those cases.
- Gate: Card 07 remains PARTIAL/IN_PROGRESS. Title-level XAM callback invocation, first frame, full game, native SBS, browser-native execution, and release acceptance are still open.
- Next: map the sampled thread PCs and startup initializer path in exact TU2 Ghidra; collect bounded per-thread callsite/return evidence around the first divergent path before changing runtime semantics.
- Evidence: `docs/evidence/cards/07-dynamic-xam-dispatch-startup-20261008.json`; raw private artifacts remain under `research/native-host-probe-20261008-*` and `research/ghidra/`.

## Run206 — bounded exact-image regeneration audit — 2026-10-11

- Builder: copied the revision-bound Run167 hint package and ran xenon-prepare against the exact effective-image wrapper in an isolated cache.
- Result: analysis wave 1 expanded to approximately 75,107 newly discovered functions and produced no accepted output before bounded interruption. This attempt is recorded as failed and will not be repeated unchanged.
- Critic: current generated registry and shard_026.cpp already contain xenon_fn_822A5B3C_v2; Run204's active failure is a distinct runtime contract fault at x82A43880, write8 to x80, called through x82A43870. The next repair is a caller/return-register trace, not another broad regeneration.
- Evidence: esearch/repair/startup-readiness/attempt-04/run206/run206-evidence.json; existing Run204 status/log and generated shards.


## Run207 — caller contract analysis — 2026-10-11

- Builder: inspected all generated callsites for x82A43868 and the failing write8(gpr3) at x82A43880.
- Finding: the failing pointer is sourced from the preserved gpr27 path at x8249D4F8 -> 0x8249D504; this is a guest pointer contract failure or an upstream register/import result, not a missing dispatcher entry. Strict page protection stays in place.
- Critic: static inspection alone cannot identify the producer of x80; no source edit or success claim was made. Next is a bounded runtime register trace at the two callsites and callee entry.
- Evidence: esearch/repair/startup-readiness/attempt-04/run207/caller-contract-analysis.json.


## Run213 — register producer trace — 2026-10-11

- Builder: expanded the Debug host observer to capture `r26`–`r29` and targeted `0x8249D470`, `0x8249D478`, `0x8249D4F8`, `0x8249D504`, `0x82A43868`, and `0x82A43870`; rebuilt the `d3d12-probe` runtime host successfully.
- Exact-image run used SHA-1 `b771b41caef2483549bfbc7d726be3eb56943f99` and bound the existing generated module. The run reached the same strict fault at `0x82A43880` after 139,688 guest blocks.
- Evidence: at the failing sample `0x8249D478` receives `r5=0x80` and computes `r27=0x80`; `0x8249D4F8` then copies that value to `r26`, and `0x8249D504` passes it as `r3` to `0x82A43868`. This proves the invalid pointer is produced upstream of the final helper; no page-protection or low-page mapping relaxation is warranted.
- Fresh critic: Run213 has 14 targeted samples, the same effective-image hash, and a reproducible failure. The next trace must follow the caller chain through `0x8249E000`/`0x8249DAD8` and the service path at `0x82293100` to identify the runtime/import contract producing `0x80`.
- Evidence: `research/repair/startup-readiness/attempt-04/run213/run213-evidence.json` and `adaptive-observations.jsonl`.

## Run214 — upstream lookup trace — 2026-10-11

- Builder: added targeted observation of the frame/lookup chain (`0x8249E000`–`0x8249E06C`, `0x8249DAD8`–`0x8249DB14`) and rebuilt the Debug host. The exact-image run again reached the strict `0x82A43880` write fault after 139,688 guest blocks.
- Fresh evidence distinguishes the first valid invocation from the failing second invocation. The first call reaches `0x82A43868` with valid pointers (`r3=0xBFFE0200`, `r4=0xBFFE0780`). On the second invocation, `0x8249E000` enters the lookup/frame path, `0x8249D470` receives `r5=0x80`, and `0x8249D4F8` propagates it into `r27`/`r26`; the callee then receives `r3=0x80`.
- Static inspection shows `0x8249DAE0` calls the generated lookup entry `0x82293100`, which in turn dispatches through the string/object lookup routine at `0x82290C68`. The fault is therefore narrowed to the second-pass lookup/import contract or its returned object/length state, not SBS, page mapping, or the final byte store.
- Critic: no runtime semantics were weakened and no candidate was promoted. Next repair must trace the `0x82290C68` lookup return and the `0x82293100` handoff, then correct the owning runtime contract with a new isolated run.
- Evidence: `research/repair/startup-readiness/attempt-04/run214/adaptive-observations.jsonl`; host build warnings remain non-fatal linker debug-symbol warnings.

- Run215 (2026-10-11): fresh critic passed artifact inspection. The exact TU2 image remains `b771b41caef2483549bfbc7d726be3eb56943f99`; host SHA256 is `75B8FACD13282B091AD7F7C3FD831C5F14A33FCCC6E82E2F21082ED52ADF0D67`. The native probe still terminates at `0x82A43880` writing guest `0x80`, but the new register trace distinguishes the path: `0x82A43868` receives valid `r3=0xBFFE0200` and `r3=0xFF600080` on earlier calls, then the failing `r3=0x80, r4=0x600`; `0x82293100` is fed frame pointers with descriptors `0x8205B418`, `0x8200564C`, and `0x82005494`. The builder/critic conclusion is an upstream lookup/return-contract defect, not a page-protection defect. Evidence: `research/repair/startup-readiness/attempt-04/run215/run215-evidence.json`. Next distinct repair: instrument the actual `0x82290B10` return and `0x82293100` handoff, rebuild, and run a fresh run216.

- Run221 (2026-10-11): fresh critic passed the new trace. The exact TU2 image remains b771b41caef2483549bfbc7d726be3eb56943f99; the run still fails only at the protected low-address write, but the upstream contract is now narrower. Parser inputs are real strings (1M, 64K, 300K, 7M) with length 10. 0x8229CCB0 branches to 0x8229CB50, and Run221 reaches that helper path before the bad 0x80 result. Evidence: research/repair/startup-readiness/attempt-04/run221/run221-evidence.json. Next repair: trace 0x8229B348 and its return value before 0x82293138.

- Runs216-220 (2026-10-11): distinct repair cycles rebuilt the instrumented Debug host and reran fresh effective-image sessions. The probes established that 0x82293130 calls parser 0x82290F40 with valid stack strings, parser helper 0x8229CCB0 branches to 0x8229CB50, and the failure remains the same protected write at 0x80. Runs 216-220 evidence is preserved beside each status file; no low-page mapping or protection relaxation was introduced. The active next repair is the 0x8229B348 return handoff, with the rebuilt host ready for run222.

- Run222 (2026-10-11): fresh critic passed the return-contract trace. The parser helper call is now observed end to end: 0x8229CCB0 -> 0x8229CB50 -> 0x8229B348 -> 0x8229CB58. The helper preserves the parser input pointer; it is not the direct source of the 0x80 value. The parser still fails before 0x82293138 and the low-page protection remains intact. Evidence: research/repair/startup-readiness/attempt-04/run222/run222-evidence.json. Next distinct repair: trace the parser post-helper/FP-conversion blocks and its final return handoff.

## 2026-10-11 — Run236 D470 producer capture

- Rebuilt `research/third_party/xenon-recomp` runtime host after extending the bounded observer to capture the failing `0x8249D470` call's stack fields.
- Executed `research/repair/startup-readiness/attempt-04/run236/` against effective TU2 SHA1 `b771b41caef2483549bfbc7d726be3eb56943f99` using the existing generated DLL; host exited with the same protected low-page write at `0x82A43880` (`r3/r26/r27=0x80`).
- New evidence: the failing D470 pass has `badD470Field54=0`, `badD470Field58=0`, `badD470R31=0`, and `badD470R29=0x3FFDE530`. The zero is therefore already present in the producer frame before the final write; page-0 mapping/protection changes are invalid.
- The producer path `0x8249DAE0 -> 0x82293100` receives a valid source pointer but `r6=0`; the generated `0x82293100` fallback branch returns `r30` (the incoming `r6`) when `0x82290B10` reports no match. The next repair must identify why this exact TU2 parse/lookup misses its key (input bytes, memory object, or missing runtime data) and restore the retail failure/lookup contract; do not inject `0x80` or suppress the write.
- Status: native bootstrap remains `BLOCKED` at the reproducible producer fault; no SBS/browser gate is claimed.

## 2026-10-11 — Run237 lookup table and parser handoff

Run237 confirms the lookup table is present (`size`, `pagesize`) and the key `size` is traversed by the generated linear lookup. The parser receives valid strings such as `7M`; however, the expected post-parser continuation at `0x82293138` is never observed before the zero reaches the D470 producer. This is now a focused AOT call/return propagation defect. The native route remains blocked; no low-page mapping or fake pointer was introduced.

- 2026-10-11 — Run240 narrowed the startup fault to the null-object branch, not parser continuation. The exact TU2 image remains `b771b41caef2483549bfbc7d726be3eb56943f99`. Fresh host-only probes show the failing call sequence `0x8249D4F0 -> 0x8249D4F8 -> 0x8249D504 -> 0x82A43868`: `r26` is zero at `0x8249D4F0`, then `0x8249D4F8` copies the rounded size `0x80` from `r27` into `r26`, and the constructor enters with `r3=0x80`. The final write fault is therefore the generated guest null-allocation/fallback contract; mapping page 0 or skipping the write would be invalid. The diagnostic synthetic parser boundary was inconclusive and is not treated as a fix. Native route remains BLOCKED. Next action: trace and implement the upstream allocation/object-creation runtime service or correct its generated return contract, then rerun with the indexed probes restored.

- 2026-10-11 — Run246 builder/critic cycle: the fresh run preserved the exact TU2 hash and reproduced the same protected write. D470 call-site evidence distinguishes a valid pass from the failing pass at `cia=0x8249E028`: valid `r31` is followed by failing `r31=0`, and `r5=0x80` is only the rounded size. The helper `0x8229B34C` is only a frame prologue/epilogue; the unresolved defect is the E000/E028 caller's object-return/initialization contract. No low-page mapping or pointer fabrication was used. Next action: instrument the E000/E028 return path with an unbounded per-invocation trace or direct caller-state log, then implement the owning TU2 runtime service contract and rerun a new isolated attempt.
- 2026-10-11 — Run251 builder/critic cycle: marked traces show `0x8249DAD8` returns to `0x8249DAE0` correctly and `0x82293100` is entered with a valid pointer/descriptor. The same protected `0x80` write remains. This shifts the active defect to cross-function `ExecutionResult` continuation from `0x82293100`/`0x82290F40` back to `0x8249DB04` and the later E000/E028 branch. Markers restore guest state; no low-page mapping, fake pointer, or protection bypass was introduced. Next action: trace `ExecutionResult.reason`/`next_address` at the 93100 and parser call boundaries, then repair dispatcher continuation semantics.
