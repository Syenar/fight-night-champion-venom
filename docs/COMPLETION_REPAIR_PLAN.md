# Detailed repair and completion plan

**Active execution correction (2026-10-08):** Follow [STARTUP_REPAIR_PROTOCOL.md](STARTUP_REPAIR_PROTOCOL.md) for Card 07–08. It supersedes historical startup diagnoses, counts and next actions. Current gate: exact TU2/module execute but no verified first frame. Require a falsifiable targeted hypothesis, retained process/exit evidence, causal repair and regression; no unchanged broad retries. Native full gameplay precedes SBS and standalone browser work.


Updated 2026-10-06. Implementation companion to `FULL_GAME_BROWSER_SBS_PLAN.md`, executed through `AGENT_EXECUTION_RUNBOOK.md`. This document expands the cards; it does not replace source identity, acceptance or risk authorities. All paths below are relative to the Xbox workspace unless explicitly absolute.

## 1. Deliverable and completion boundary

Deliver the complete supported offline Xbox 360 Fight Night Champion game, using the frozen owned source, TU2 and selected DLC, as:

1. A standalone Windows recompiled runtime launched from a writable folder.
2. A standalone browser-native WASM/WebGPU runtime executing the same recompiled game locally, with Bun for development/serving/build orchestration.
3. Normal single-view and optional true left/right stereoscopic SBS modes in both runtimes, sharing simulation, saves, controls and content.

Required features: menus/profiles/settings, exhibition, Champion Mode through completion, Legacy Mode through its lifecycle, training, replays, audio, video, saves/reloads, selected DLC/unlocks, supported local multiplayer, controller reconnect, recovery and long sessions. Enumerate actual offline modes from the frozen reference before signing off. Online services are unsupported unless separately implemented; that must not prevent offline play.

Xenia is a reference/diagnostic instrument. Xenia hosting, WebRTC, identical images in both halves, depth-based stereo reconstruction, or a gameplay remake cannot close this goal. C++ is the generated CPU core; Rust is an integration option across an explicit stable ABI, not a requirement to translate every generated function. Preserve source inputs and original installations; use only local research copies/builds. Never read the PS3 archive as implementation reference.

This is an evidence-based engineering sequence, not a guarantee that every runtime/browser gap has already been solved. Unknowns have experiments, acceptance conditions and repair branches below. A passing plan review is not a passing game.

## 2. Historical baseline identity ledger — 2026-10-06

The historical baseline evidence is `docs/evidence/cards/07-native-bootstrap-20261006.json`, `research/verify-tu2/RESULTS.xml`, and `research/native-host-probe-20261006-tu2/{status.json,log.txt,adaptive-observations.jsonl,SUMMARY.json}`.

| Item | Current evidence / constraint |
|---|---|
| Base ISO SHA-256 | `75E9D3EAA8440A24BFE1052B6E051F2A29DF189DDBE854969C1406E31D2E8242` |
| Base XEX SHA-256 | `90A698C71AA714E383CE092750CE767E07145F1CA38D8A9C9B3BD8005EA29FF6` |
| TU package SHA-256 | `10D3B95C630F774D376FF504574E6107457A0A24768079D69BDD343BEF88BB68` |
| Extracted XEXP SHA-256 | `8429D60620012112285809E6B6545AF646056B9FCCBA3D1B88BB71C32EA9A8AE` |
| Effective TU2 image identity | `b771b41caef2483549bfbc7d726be3eb56943f99` (runtime image-hash field; do not label as SHA-256) |
| Title / media / versions | `45410915` / `2DC50868`; base `0x3`, effective `0x203` |
| Generated core | 65,376 functions, 4,086 shards; compiled/linked, not fully qualified |
| Native DLL SHA-256 | `50258050c0ff8c779385558e110013bd92d76401f828783e919dd2f5e3fea162` |
| Loader checks | CPU/decoder, XEX loader, crypto and LZX: four suites passed after repairs |
| Headless boot probe | TU2 applied, identity matched, DLL bound, startup code executed |
| Current runtime state | Latest bounded TU2 probe ran about two minutes with no guest trap, then was stopped by the probe. It remained in heap/pool startup activity and produced no first frame; dynamic fallback remains enabled. Earlier trap at `0x82297D58` / `0x7C6A5910` is historical, not reproduced by the latest probe. |
| Other open runtime evidence | Latest probe reports 214 unresolved imports and 1,617 adaptive observations; import names/kinds still need coverage classification. No rendered first frame. |
| Reference capture | Card 04 incomplete; user authorized advancing runtime work, not skipping final visual/reference acceptance |

Package, XEXP, normalized effective image and relocated/mapped memory dumps are different artifacts. Record each algorithm, byte length, address range and normalization procedure. Their hashes are not interchangeable.

Workflow revision (2026-10-12): `research/third_party/xenon-recomp` remains the current generator/reference checkout, remote `https://github.com/nimauria/Xenon-Recomp.git`, base commit `2aa583907d6c8962bc11a971f6b9033a19e229ee`, plus local patches. The next runtime qualification must pin and test a ReXGlue SDK/runtime revision against the exact TU2 image. This is not an instruction to mix repositories: `hedge-dev/XenonRecomp`, nimauria's checkout and ReXGlue have different manifests, generated ABIs and runtime contracts. Record an explicit adapter/compatibility result before using any alternative output. The native bootstrap target is a generated Venom title module linked to the qualified shared ReXGlue/Xenia-derived runtime; the custom minimal host remains a diagnostic comparison only. Pin local patch and dependency hashes.

Current module: `research/xenon-prepare-fnc/cache-tu2-cross-edge-run1/entries/0fb0bcb0d6e27a24cbe93daf7e53bfe7206170ae/xenon_game_module.dll`.

Current generated workspace: `research/xenon-prepare-fnc/cache-tu2-cross-edge-run1/workspaces/f09c64c07f043e626279b7cc9ab0a4c9/generated`.

Current loader repairs: bound optional FFI fields by declared record size; correct synthetic descriptor key binding; deliver explicit metadata-free XEXP to the validated loader; fail on missing explicit update. Retain these regressions. The DLL predates the latest host repairs: identify host and module separately, and rebuild affected artifacts when their dependencies change.

## 3. Dependency order and safe work protocol

| Work package | Runbook cards | Must precede | Closure evidence |
|---|---|---|---|
| A: reconcile identity/build provenance | 00–03, 05–07 | all promoted runtime results | signed-off ledger and repeatable commands |
| B: fix generated routing and CPU trap | 06–08 | boot ladder | trace past trap; required paths execute generated code |
| C: qualify runtime/imports | 07–08 | full native game | tested services and correct process/thread lifecycle |
| D: correct native graphics/audio/video | 07–08 | full native/SBS acceptance | reference-matched first frame and real fight |
| E: whole offline game | 04, 08–09 | SBS sign-off and browser implementation | every native feature row passes |
| F: true native SBS + normal | 10 | desktop/display release | synchronized independent gameplay eyes |
| G: portable Windows package | 11 | browser/release qualification | clean-machine cold-start/save/recovery |
| H: browser feasibility and complete port | 12–14 | browser package | complete locally executing game in both modes |
| I: package/release review | 15–16 | completion declaration | full evidence matrix and release manifest |

Within A–D, independent analysis/test/dependency preparation can proceed while a failing subgate is repaired. Do not promote dependent gates. Browser architecture constraints may be documented now, but Cards 12–14 implementation/promotion remain after native full-game/display gates. A one-match diagnostic is an intermediate test of the full core, not reduced scope.

For each defect: save the failing command, exact input/build hashes, trace, guest PC and expected contract; make one causal change; run its regression plus affected tests; rerun the same failing scenario; compare traces/screenshots; review the diff adversarially. Preserve each attempt in `research/repair/<issue-id>/attempt-N/`. Avoid blind unchanged retries, wide unrelated patches or erasing failed runs. If unresolved after three distinct evidence-backed attempts, record the precise cause and continue independent tasks; three attempts are not permission to give up on a repairable problem.

Keep the last known test/boot milestone intact. Use new build/session directories for configuration changes; save builds by identity. No original dump/TU/DLC/Xenia installation is an edit target. Use source overrides/generator changes, not hand-patched generated shards. Scripts discover paths from manifests and reject missing/mismatched inputs before launching. They must not assume the user's current working directory or require administrator privileges for ordinary use.

## 4. Package A — reproducible source and toolchain qualification

1. Reconcile `research/SOURCE_MANIFEST.json`, toolchain manifest, current TU2 prepare entry and native-host evidence. Preserve historical records; add a versioned current effective-image/build record instead of rewriting history.
2. Verify package-to-XEXP extraction mapping, base/version/signature binding and effective-image bytes against an independently captured Xenia image. Normalize only documented loader relocation/import-fixup regions; retain a byte-difference report. A hash equal to another output of the same parser is not an independent oracle.
3. Confirm a fresh Ghidra analysis uses this normalized exact TU2 image and correct base/sections. If existing fresh analysis matches, reuse its archive; otherwise import fresh. Export entry/TLS/imports, function/block ranges, jump tables, exception/unwind data, address-taking sites and confidence-tagged symbols.
4. Freeze generator settings, manual boundaries, indirect-target metadata and input hashes. Run generation twice in separate clean output directories; compare semantic/source manifests with volatile timestamps/absolute paths handled explicitly. Scan for forced traps, unresolved control transfers, duplicate/overlapping ownership, invalid bounds, unsupported opcodes and unexpected host architecture assumptions.
5. Record Debug and Release configurations and exact compiler/linker flags, especially floating-point behavior. Verify relevant semantic suites in both configurations with assertions active. The present 1.35 GB Debug DLL is not a browser-size prediction; measure optimized code separately.
6. Build a local source patch/dependency ledger and save commands in `research/toolchain/`. Verify that a fresh local checkout plus those patches reproduces tests/module identity without fetching floating branch heads.
7. Qualify the pinned ReXGlue SDK/runtime with a minimal Venom module built from the exact TU2 image. Record accepted/rejected input format, manifest requirements, runtime ABI, plugin versions, and the first observable milestone. Keep this qualification isolated from the current custom-host artifacts.

**Pass:** exact source and patched image independently qualified; reproducible generation; versioned toolchain+patch provenance. No native-playability claim follows from this pass.

## 5. Package B — qualify the shared runtime before repairing title code

The current custom-host null-object fault is evidence of an incomplete runtime contract, not proof that every generated function must be rewritten. Before adding another title-specific workaround, run the same exact image and manifest through the qualified ReXGlue/Xenia-derived runtime. If it reaches farther, make that the native bootstrap path. If it rejects the image or fails at a runtime contract that is demonstrably absent, preserve the evidence and repair the shared runtime or manifest at the owning layer. Do not fork a second emulator inside generated shards.

### B0. Shared-runtime qualification

1. Build the pinned ReXGlue runtime and its Xenos/audio/input plugins from recorded commits.
2. Generate the Venom title module with the pinned manifest and run the manifest collision check before linking. A guest symbol that shadows an XAPI import is a hard failure.
3. Link and stage matching Debug/Release runtime, UI, Xenos, audio and input plugins. Record their hashes and reject mixed configurations.
4. Initialize the runtime in its documented dependency order: exception support, clock, memory, export resolver, function dispatcher, VFS, kernel state, input, kernel modules, audio, mounts, graphics and function mappings.
5. Run a bounded launch ladder: loader identity, process/module state, first kernel call, filesystem read, first window, first frame.
6. Compare each reached boundary with Xenia using the same TU2/DLC inputs. Record whether the failure is generator metadata, runtime service, content mapping, renderer/plugin, or title logic.
7. Only promote this path to Package C when it reaches a real first frame or provides stronger evidence than the custom host.

## 5A. Repair the current CPU/routing failure when the shared-runtime path is unavailable

### B1. Decode and reproduce the trap

1. Read `0x82297D58` from the locked effective image, not retail bytes. Run the selected toolchain disassembler and Ghidra independently; record opcode, operands, OE/Rc bits and surrounding block. Catalog inspection identifies the word as a `subfex`/subfe-family candidate; verify the decoded instruction before implementing a fix.
2. Correlate the original six observations and the newer probe's 1,617 adaptive observations with generated-function registry and branch sites. Some records are expected import-thunk redirects; distinguish those from data/function mismatches, interior generated entries, and actual missing code. Do not infer one cause from the diagnostic label alone.
3. Preserve the log showing kernel calls before the trap, register/XER/CR state, stack/PCR/TLS, memory accesses and the last compiled-to-fallback transfer. Add bounded trace instrumentation if necessary; never log key material.

### B2. Fix primary generated routing

1. Inspect generation metadata and function ownership around `0x82297D44` through `0x82297D68`, the returning callee and the originating transfer. Is the return address an interior label unavailable to the dispatcher? Is a range truncated? Is a function-table entry absent? Is LR/CTR routing wrong?
2. Correct the ownership/registry/continuation contract in the generator or runtime. Represent valid mid-function entry/continuation states without inventing a new four-byte function at every branch target. Distinguish import thunks from executable function entries.
3. Check manual bound `0x82C4BCC8` → exclusive `0x82C4BCFC` and call at `0x82C4BCF8` to `0x82C79050`. Prove whether the callee returns. Padding beyond the range is not proof of a non-returning callee. Audit its indirect call at `0x82C4BCEC` as well.
4. Regenerate affected metadata/output, rebuild only required dependencies and rerun the exact probe. Measure fallback counts and sites. Final static-runtime tests must fail if a required game path falls back to interpretation; diagnostic fallback is allowed only with explicit labeling during bring-up.

### B3. Repair diagnostic execution semantics where needed

The lifter already has subfe-family arithmetic in `src/cpu/ppc/lifter_integer.cpp`. An unsupported diagnostic fallback is a different defect from unsupported AOT generation.

1. Compare decoder → lifter → IR → generated semantics against `src/cpu/codegen/dynamic_fallback.cpp`. Repair missing fallback support only if it assists diagnosing real game behavior; it does not replace B2.
2. Validate `RB + ~RA + CA` with carry-in/out, 64-bit wrap, applicable OE/overflow/SO, Rc/CR0, register aliasing and correct mode behavior against an independent arithmetic oracle/ISA reference. Include boundaries, negative values, carry 0/1 and randomized inputs. Do not copy expected values from the implementation under test.
3. Test a small compiled control-flow fixture that returns/branches to an interior continuation, plus the real startup trace. Preserve PC/next-PC/LR behavior across compiled dispatch and diagnostic fallback.

**Pass:** exact startup no longer stops at this trap; affected instruction semantics pass; generated routing reaches the next real milestone with fallback explicitly accounted for. Later traps are new issues, not closure of the whole CPU core.

## 6. Package C — complete runtime contracts and imports

### C1. Classify imports from the active hash-identified host/module

Export `research/runtime/IMPORT_COVERAGE.json`: library, ordinal, name from independently verified export tables, function/data kind, guest thunk/storage address, calling convention, argument/result layouts, runtime implementation location, required configuration, observed call sites, tests, feature dependencies and status. Classify resolved entries too; a name match alone does not prove semantics. Assign each contract to the shared ReXGlue/Xenia-derived runtime or the Venom title module; do not implement a duplicate service in generated shards.

Separate missing implementation from disabled subsystem, missing registration, ordinal mismatch, data import, wrong ABI and unavailable online service. Verify current names and ordinals against pinned Xenia export declarations and local runtime code. Do not count all imports as missing implementations. Implement in startup call order, then expand by full-matrix feature use. Supply documented offline failure behavior for online APIs; never return invented success.

### C2. Process/memory/thread bootstrap

1. Qualify image mappings/protection/endianness/aliases; stack alignment/guard pages; module header handles; process type; heap/allocation contracts and file/image pointer lifetimes.
2. Reconstruct process/module state, PCR/TLS slots, thread IDs, CPU initial register state, thread startup/exit, callbacks, APC/exception paths and synchronization from actual startup/reference traces. Audit `RtlImageXexHeaderField` and other already-resolved calls: a successful return of zero can still be wrong for a requested field.
3. Test zero-fill and reserve/commit/decommit behavior, atomic reservations, wait/event/semaphore/timeouts, monotonic timebase, guest-to-host address translation and CPU/GPU coherence. Stress cross-thread contention and shutdown.
4. Add crash evidence containing guest PC, opcode, last service, thread, registers, identity and replayable command. A bounded launcher must handle failures/hangs during initialize/load/start as well as during the normal session loop; `stop.signal` alone does not currently prove load-time cancellation.

### C3. Filesystem/content/profile/saves/input

1. Mount the exact owned base content read-only with correct Xbox path/case behavior, async I/O/completion and guest-buffer layouts. Implement local profile/content interfaces without needing a real online sign-in.
2. Explicit TU files must reach binding validation even without catalog execution metadata. Directory searches must select deterministically; incompatible or unreadable explicit updates must fail instead of choosing retail code. Add tests for missing/truncated/wrong-base/wrong-version updates and selected-update consistency in content graph/status.
3. Inventory each selected DLC package/container ID and inner hashes. Implement discovery, content/license responses for the user's supplied content, mount paths and persistent unlock/profile behavior. Validate boxers/features in-game and after reload; installed files alone do not pass.
4. Save to per-user isolated directories with atomic replacement, backup/recovery, disk-full errors and schema checks. Test new save, reload, corruption, interrupted write and migration. Never edit the user's known-good saves in place.
5. Map controller polls, deadzones, vibration, hotplug, pause/focus and supported local player slots. Check startup, menus and live fighting. Keep keyboard mapping useful without declaring unavailable local-controller modes passed.
6. Audit existing `native/runtime/` Rust scaffolding and FFI readiness checks only after the shared C++/ReXGlue reference is qualified. Establish one authoritative runtime; use a tested adapter if retaining Rust. Test ownership, buffers/endianness, errors, callback thread affinity and ABI version/struct size. A second disconnected "ready" runtime is not integration.

**Pass:** startup contracts are correct, required offline imports behave as specified, content/save/input tests and real scenarios pass. Every remaining import has an honest scoped disposition; unresolved required offline exports cannot remain hidden.

## 7. Package D — correct native rendering, audio and video

The current `ninja-min` host has no production graphics backend and no audio; `null` is diagnostic only. Build a separate fully configured candidate so existing probe evidence remains reproducible.

1. Pin and verify SDL/presentation, Vulkan SDK or D3D12, DXC/shader compiler and the selected runtime's compatible FFmpeg/XMA dependency. Inspect its CMake options and licensing requirements. Do not use a generic FFmpeg package unless required decoder capabilities are present.
2. Enable a real graphics backend and audio in the new build. Start in normal mode with default-correct settings; retain strict loader/memory validation. Verify native window creation, foreground visibility and render-device selection on the real user's desktop. Native-window automation availability is separate from emulator/runtime correctness; use supported capture, renderer readback or user-visible validation without pretending a hidden headless process is visible.
3. Test the shared Xenos frontend/backend with synthetic PM4/draw/shader/resolve fixtures before debugging game scenes. Then capture the first game frame and compare with exact reference state.
4. For corruption: inspect draw submission, vertex/index formats, shader constants, texture tiling/swizzle/endian/format, EDRAM ownership/MSAA/resolve, pitch/offset/upload range, resource transitions and feedback hazards in that order guided by the failing capture. Bisect draw groups/passes, not unrelated flags. Never globally allow invalid ranges as the final repair.
5. Keep normal mode correct before adding eyes. Use capture/replay tests to distinguish frontend errors from backend errors; try a second backend only as a controlled comparison. A backend switch is not proof the original defect is understood.
6. Qualify audio context/voice allocation, XMA decode, sample rates/resampling, timing/mixing, underruns, pause and end-of-stream. Compare commentary/music/effects and A/V sync. Add video decode/upload/colorspace/seek/subtitle and cutscene-transition tests using owned local media only.

**Pass:** correct title/menu/fight/cutscene frames, audible expected audio and synchronized video, with actual screenshots/captures and no unexplained corruption. This is required before stereo acceptance.

## 8. Package E — finish the complete offline game

Create a per-feature test schedule mapped to the existing acceptance rows. Every scenario records build/source, initial profile/save, controller input or replay, runtime trace, screenshot/video, expected result and disposition.

1. Boot and every menu/settings path; persist preferences and profile between clean launches.
2. Exhibition: roster/arena/options selection, full match, round transitions, knockdowns/recovery, pause, restart, decisions/KO/TKO and return to menus as supported by the reference.
3. Champion: start new, all story stages/cutscenes/tutorials/challenges, checkpoint failures/retries, progression unlocks, save/reload and completion. A checkpoint-only smoke test is not campaign completion.
4. Legacy: create boxer, edit/configure, training, schedule/progression, repeated fights, stats, saves and retirement/end-state. Use a documented complete lifecycle; shortcuts cannot replace coverage of skipped code paths.
5. All training, replay/highlight, unlock/statistics, selected DLC, local multiplayer and controller-layout paths found in the reference.
6. Repeated cold starts, mode transitions, controller reconnect, language/settings variants and long sessions. Baseline stress protocol: 10 cold starts and an uninterrupted 2-hour mixed gameplay session per candidate, plus complete campaign/Legacy coverage; extend when leaks/timing issues appear. These durations are review criteria, not proof of every possible state.
7. Track memory, handles, threads, audio voices, resource/cache growth, frame-time percentiles and error counts; investigate unbounded growth even if the session does not crash. Replay parity must allow documented floating-point/timing differences rather than requiring arbitrary cross-platform hashes to match.

**Pass:** all required native offline rows have reviewed evidence. Open defects keep their feature/gate incomplete. Unknown offline scope must be resolved, not removed to make the matrix green.

## 9. Package F — true two-eye SBS without regressions

1. Use Ghidra plus runtime draw/constant tracing to locate view/projection/camera data and classify world, shadow, reflection, screen-space/postprocess, HUD and video passes. Document exact source sites and evidence; do not apply eye offsets to every matrix indiscriminately.
2. Freeze simulation once per game tick. Render both eyes from that same state; rendering twice must not advance AI/physics/audio/random state twice. Audit draw-time state mutation and CPU/GPU synchronization.
3. Use independent left/right view transforms displaced along camera-local right, with parallel cameras and documented off-axis projection/convergence. Verify engine units/sign/conventions; expose sensible bounds and a zero-IPD diagnostic. Avoid unverified toe-in or world-axis offsets.
4. Allocate explicit per-eye color/depth/resolve and required temporal/history resources. Record ownership, frame ID, clear/load/store, barriers, viewport/scissor and sampling dependencies. Never read a mono or previous-eye target accidentally. Stereo shadows/reflections/effects must have an explicit correct reuse/recompute policy.
5. Define per-layer behavior: world depth is stereoscopic; menus/HUD/subtitles use a readable chosen plane; prerecorded video is intentionally mono presented consistently in both eyes. Full- and half-width SBS packing must preserve each eye's aspect policy, resolution and order.
6. Prove pipeline wiring with near/far synthetic geometry, then real fights, camera moves/replays and cutscene/menu transitions. Capture simultaneous left/right/packed readbacks with identical frame IDs. Measure object disparity versus depth; inspect vertical alignment, occlusion, edges, UI clipping, stale frames and eye order. A screenshot of two panes is insufficient.
7. Test zero IPD, adjustable IPD/convergence, swapped eyes, resolution/fullscreen, mode switching without resetting the session, GPU resize/device-loss recovery and prolonged use. Compare normal output before/after stereo changes and qualify both modes across the entire offline matrix.

**Pass:** two complete synchronized real gameplay views with measurable geometrically correct parallax, clean screenshots, no corruption and preserved full-game behavior. Review on a real SBS viewer where available; capture/readback proof remains mandatory.

## 10. Package G — portable Windows delivery

Build a Release candidate with runtime dependencies bundled according to licenses, launcher, isolated writable state/cache/logs, checksums and explicit normal/SBS settings. Data selection reads owned inputs; extraction/conversion is resumable and content-addressed. No ordinary administrator install or development shell is required. Support spaces/non-ASCII paths, unavailable devices, disk-full, source mismatch and corrupted caches.

Test on a clean Windows environment: unzip, supply owned data, first launch, controllers, both display modes, full-game scenarios, save/quit/reload, crash/hang recovery and update rollback. Keep original data and saves untouched during cache reset/uninstall. Separate distributable runtime/tools from private generated module/assets. If module rights are not established, provide a private local-build recipe/package and label it accurately; do not publish retail-derived WASM/DLL code by default either.

**Pass:** reproducible writable-folder launch/recovery and correct provenance/package boundaries.

## 11. Package H — standalone browser implementation

### H1. Browser feasibility gate (Card 12)

Use a no-content shell and synthetic fixtures after native gates pass. Pin Emscripten/LLVM, Rust WASM target only if used, Bun and WebGPU binding/shader tools. Record compiler/linker flags and browser/driver capabilities.

1. Build representative generated code and then the complete optimized core for WASM; inventory every Win32/host-only API, native DLL/JIT/executable-memory dependency, SIMD/FP assumption and function-pointer table. Browser builds need a static compatible module/registry; Windows `LoadLibrary` and host virtual pointers are not portable.
2. Design guest memory as a logical Xbox address space mapped into compact WASM backing storage. Do not reserve a dense WASM array up to guest address `0x82000000` or mirror all native aliases as duplicate physical memory. Preserve 32-bit guest addresses, alias coherence, protection/MMIO semantics and endian handling through explicit translation/accessors. Validate bounds and pointer-sized casts.
3. Measure RAM, compiled-code size, function table/link limits, compilation/download time and cache budget on the target browser. Evaluate wasm32 first; memory64 is a separately qualified option, not a presumed cure. Stream game data from local cache instead of loading a 7.8 GB ISO into memory. Browser limits are hard feasibility conditions.
4. Prove worker/shared-memory synchronization with cross-origin-isolated serving. Keep blocking guest waits and CPU execution off the UI thread. Specify COOP/COEP and local resource policies; verify browser worker startup and event-loop yielding. A single-thread variant requires separate scheduler correctness and acceptance, not an automatic fallback claim.
5. Prove WebGPU synthetic upload/render/readback and recovery, local storage streaming, AudioWorklet output, video capability checks and Gamepad input. Preserve frontend Xenos semantics; native SPIR-V/HLSL does not directly become WGSL without translation/validation.

**Pass:** documented solutions and resource budgets for each native-to-browser contract, a functioning synthetic shell and a complete-core WASM build. A failure stays explicit; no WebRTC substitution.

### H2. Port the complete platform layer (Cards 13–14)

| Native contract | Browser implementation / proof |
|---|---|
| Game/module loading | Static/link-compatible WASM registry plus owned local data initialization; effective TU2 identity remains checked |
| Guest memory/threads/time | Compact backing + guest address translation; worker scheduler/atomics and monotonic time; reservation/wait tests |
| Filesystem/TU/DLC | Picker/import, chunked read-only VFS, OPFS or qualified IndexedDB storage; local conversion/resume/integrity |
| Saves/profiles/settings | Transactional persistent store, explicit export/import/backup, schema/quota/crash tests |
| Xenos/native graphics | Canonical frontend + WebGPU resources/WGSL shaders; EDRAM/resolve equivalence fixtures and reference frames |
| Audio | Browser-compatible XMA decoder path, bounded ring/AudioWorklet, user-gesture start and underrun/pause tests |
| Video | Capability-qualified decoder (WebCodecs only for supported codecs, otherwise licensed portable decode), color/A/V/seek parity |
| Input | Gamepad/keyboard, normalized layouts/deadzones, focus/reconnect/local multiplayer; unsupported vibration stated |
| Window/present | One canvas; normal/SBS, resize/fullscreen/device loss, worker-to-UI controls without blocking |
| Crash recovery | Worker failure/watchdog, persistent diagnostics and transactional save recovery without data upload |

Build a Bun-served production frontend with local-data import, readiness/progress/errors, controller/display/settings and recovery. Existing `web/launcher/` is streaming-oriented: audit before reusing UI pieces; it does not supply the WASM game backend. Use a separately named browser-native entry point/build output, and keep the streaming route optional and visibly separate. No Windows host is needed for play; a static local/HTTPS server may serve the app itself.

Qualify Chromium first on an identified GPU/OS/browser revision, then Firefox/WebKit where capabilities suffice. Specify the supported device/browser floor honestly. Run the full offline matrix in both modes, plus offline-after-install behavior, storage permissions/quota/eviction, reload, tab backgrounding, reconnect, cache version rollback, local save export and browser/device-loss recovery. Verify network traffic contains no owned data and browser execution stops only when the local session stops, not when a game server disappears.

**Pass:** same complete recompiled game runs locally in browser, normal/SBS and full feature matrix pass. A shell, title screen or streamed native process does not pass.

## 12. Package I — final audit and release manifest

Require a fresh-context reviewer when available; otherwise label self-review honestly and keep independent release sign-off pending. Reviewer reads actual source/configuration, generated manifests, exact inputs, logs, captures, replays, save results, browser surface, package contents and diffs. Summaries are indexes, not proof.

`research/RELEASE_MANIFEST.json` must identify source/container/effective-image hashes, toolchain commit plus patch/dependency hashes, host/module/browser build IDs, configuration, all acceptance row dispositions/evidence, normal/SBS parameters, supported platforms/devices, online-service boundary, provenance/licenses, package contents, checksums and private/distributable status. Scan both native and web output for prohibited data/code; generated proprietary code is not made publishable by changing its format to WASM.

Final delivery includes reproducible build/launch instructions, owned-data setup, controls/display settings, save location/export, troubleshooting/recovery and exact limitations. Completion is declared only after every required offline row passes native and browser-native in both modes under independent review. Keep user-facing claims narrower than the evidence.

## 13. Immediate work queue and commands

Execute next in this order:

1. A: reconcile exact TU2 manifests, current source patch identity and independent-image evidence.
2. B1/B2: trace the fresh run's startup loop around `0x82290ED4` and `0x82297D44` using the exact effective image, disassembly, guest registers, and last service call; confirm the prior `0x7C6A5910` trap remains fixed across a longer reproducible run.
3. C1: export/name/classify the current 214 unresolved imports plus resolved startup calls from the pinned XenonRecomp and Xenia tables. Separate runtime implementation gaps from uncalled imports and distinguish functions from variables.
4. Rerun the exact-image headless probe with a bounded watchdog and inspect whether startup passes pool initialization. Then attempt a real-backend normal-mode first frame; menus and a full match remain subsequent gates. Do not begin SBS fixes on an unverified mono frame.
5. Continue E → F → G → H → I only after their prerequisites pass. Backfill Card 04 visual reference before gameplay acceptance.

From the Xbox workspace in PowerShell, the existing focused regression commands are:

```powershell
& 'C:\Program Files\CMake\bin\cmake.exe' --build research/verify-tu2/build --parallel 4
& 'C:\Program Files\CMake\bin\ctest.exe' --test-dir research/verify-tu2/build --output-on-failure --output-junit ../RESULTS.xml
& 'C:\Program Files\CMake\bin\cmake.exe' --build research/third_party/xenon-recomp/out/build/ninja-min --config Debug --target xenon_runtime_host --parallel 4
git -C research/third_party/xenon-recomp diff --check
```

These depend on the already configured compiler/SDK environment; discover pinned paths from manifests on another machine. Create a NEW session directory/config from the recorded TU2 probe, verify its input hashes, and launch `runtime_host/Debug/xenon_runtime_host.exe --launch-config <absolute-new-config>`. Do not reuse a stale `stop.signal`, overwrite historical logs, or run several probes against the same saves. Add a watchdog with bounded load/start/run phases and structured timeout evidence; terminate only the process created by that probe if graceful stop fails. Headless/null/audio-disabled configuration remains diagnostic.

### Required issue/result record

Each issue/result has ID, package/card, severity, source/host/module/config hashes, exact reproduction, expected/actual behavior, evidence paths, cause confidence, changed files, regression result, runtime result, fallback observations, review disposition and next action. Update STATUS, checklist, acceptance evidence and risk log after a milestone. Promote completed prerequisites from real evidence only; do not infer completion from historical PASS labels.

## 14. Primary references and what they establish

Checked 2026-10-06. Local pinned source and title-specific execution evidence override assumptions from current upstream documentation.

- [Selected Xenon Recomp runtime](https://github.com/nimauria/Xenon-Recomp): experimental shared native runtime; its README does not claim completed retail qualification. Use its actual module/runtime ABI and pinned source.
- [hedge-dev XenonRecomp](https://github.com/hedge-dev/XenonRecomp): a separate recompiler candidate. Do not conflate its generated ABI/tool commands with the selected checkout.
- [Emscripten pthreads](https://emscripten.org/docs/porting/pthreads.html): shared memory/workers, cross-origin isolation and blocking/event-loop restrictions affect the browser scheduler design.
- [Emscripten settings](https://emscripten.org/docs/tools_reference/settings_reference.html): configure/measure memory limits, growth, threading and linking rather than assuming native resource layouts fit.
- [Emscripten filesystem overview](https://emscripten.org/docs/porting/files/file_systems_overview.html): native filesystem calls need browser storage/VFS integration; persistence is a separate contract.
- [WGSL specification](https://www.w3.org/TR/WGSL/): WebGPU shader types/numerics/validation require an explicit shader translation and conformance path.

Research each new uncertain contract using pinned implementation, official specifications and primary upstream issues/tests. An online example is a lead to reproduce, not evidence that Fight Night Champion already supports the path.

### Current Card 07 finding — 2026-10-08 Run49

Run49 adds a concrete null-item failure after earlier heap/pool and file-path traces: guest thread 8 faults at `0x825EBFD4` while dereferencing an item associated with global queue `0x83002B34`. The captured `end=begin-4` came after the generated consumer stored `end-4`, so it does not establish pre-consumption queue corruption. The wait helper result and producer chronology were not captured. Treat empty-queue consumption, concurrent mutation, queue/event ordering and generated `sc 2`/import dispatch as competing hypotheses, not a diagnosis. The Release session tests pass but do not validate a game boot. The probe has no Vd calls, configured ring/front buffer, title, or first frame, and it was force-terminated after stop. Next repair loop: (1) enumerate exact TU2 stores to the queue endpoints and instrument those accesses plus wait return/status; (2) validate syscall/import thunk metadata and dispatch using a fixture that proves arguments, results, and caller continuation; (3) rerun with the smallest causal repair supported by those traces. Keep Card 07 IN_PROGRESS until a repeatable first frame is reached; do not promote graphics/gameplay/SBS/browser gates. Full record: `docs/evidence/cards/07-run49-queue-snapshot-20261008.json`.

Run50/51 progressed the trace without validating the consumer: both recorded one wait-helper result `0xFFFFFFFFFFFFFFFE`, while the consumer pre-pop hook remained unreachable. Run51 shows the call passed wait object `0x83002B90` with infinite timeout `0xFFFFFFFF`; the object's `+0xC` byte was 1, global byte `0x83002B2C` was 0 after return, and queue globals pointed to `0xBD80B0E0`/`0xBD80B0C0`. These are header pointers; no queue cursor fields were recorded. The `-2` return's meaning is still unproven. The guest ran for the 305-second watchdog and required forced termination, with no Vd call, ring/front buffer, title, or frame. Next repair loop: (1) instrument both queue headers/cursors, producer writes and signal transitions; (2) trace the exact wait helper's syscall/import dispatch and validate the exact TU2 mapping and return contract; (3) make only a causally supported repair, then repeat the same probe and independent review. Do not patch the return value or insert a signal as an experiment. Keep Card 07 IN_PROGRESS. Full record: `docs/evidence/cards/07-run50-run51-wait-trace-20261008.json`.
