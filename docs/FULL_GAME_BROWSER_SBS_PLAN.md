# Fight Night Champion Venom — full-game browser SBS plan

**Active execution correction (2026-10-13):** Follow [STARTUP_REPAIR_PROTOCOL.md](STARTUP_REPAIR_PROTOCOL.md) for Card 07–08 and the newest numeric retained probe result. Clean native intro, Press Start and autosave output are observed after a causal vector-pack repair. Start-to-main-menu and complete gameplay remain unpassed. Require a falsifiable targeted hypothesis, retained process/exit evidence, causal repair and regression; no unchanged broad retries. Native full gameplay precedes SBS and standalone browser work.


Project codename: **Venom**.

**Goal:** Complete supported offline Xbox 360 Fight Night Champion as a standalone native recompile and a browser-native WASM/WebGPU runtime. Both normal single-view and optional true left/right SBS are mandatory over the same complete game state. A browser requiring a Windows game host does not satisfy standalone browser completion.

**User performance requirement (2026-10-14):** Address sustained low frame rate alongside recompilation. Provide a visible, persistent frame-rate limit with Unlimited and common FPS presets. Do not enable application-controlled display VSync. Preserve internal guest timing and bout speed independently of host presentation limits. Measure active gameplay and distinguish simulation, guest swaps and displayed output; a limiter or successful build is not an optimization result. Normal and SBS modes must retain these controls; count each complete stereo pair as one output frame. Native full-game gates still precede SBS/browser implementation. Platform compositor limits must be reported honestly.

**Primary source:** Frozen user-owned Xbox executable, verified TU2, DLC and game data. Exact effective TU2 bytes drive analysis and generation.

**Primary route (workflow revision 2026-10-12):** Qualify a pinned ReXGlue/Xenia-derived Xbox 360 runtime against the exact TU2 image before expanding the custom host. Use Ghidra/XenonAnalyse for evidence and a pinned XenonRecomp or ReXGlue code generator for the title module. The native bring-up target is the shared-runtime/module split used by current Xbox 360 recompilation projects: the shared runtime provides guest memory, CPU dispatch, XEX/module state, kernel/XAM, threads, filesystem and timing, while injected application backends provide input, audio and Xenos graphics; the Venom module owns generated game code, title-specific symbols, function-boundary metadata, TU2/DLC manifests and compatibility hooks. Generated C++ plus the qualified ReXGlue runtime is the first native bootstrap target. The final project may then migrate the game-facing boundary and platform services to Rust behind tested FFI, but Rust migration cannot begin by replacing an unvalidated runtime and cannot be called complete until the C++/runtime reference passes first-frame and offline-game gates. Xenia remains reference/diagnostic evidence only.

**Workflow rule:** A generated translation unit, zero unresolved symbols, or a successful link is never evidence of a complete decompilation. Track function coverage, boundary corrections, fallback sites, runtime contracts and behavioral equivalence separately. Use the original guest behavior as a compatibility fallback only inside private research builds while functions are being validated; do not use fallback to claim a standalone native completion.

**Browser route:** Port the same full runtime to WASM/WebGPU, browser storage/input, WebAudio/video and workers. Prefer Bun tooling. Local browser execution is required; WebRTC is optional transport only.

Detailed implementation steps, issue-specific repair branches, commands and pass criteria are in [COMPLETION_REPAIR_PLAN.md](COMPLETION_REPAIR_PLAN.md). Use it with the runbook cards and acceptance matrix, not as a separate route.

## Active native contract

Historical Run157 loaded the exact TU2 image and native module without verified graphics. The current shared-runtime candidate produces clean intro, Press Start and autosave-confirmation captures; missing indirect callbacks still prevent verified main-menu access. Follow current STATUS and numeric probe results rather than the old Run157 address queue. Earlier import counts, CPU traps and archive-open failures are historical unless reproduced. Full Card 08 gameplay, display, browser and release acceptance remain incomplete.

Advance to a correct first frame, menus/controller, complete match and full offline matrix. Never fabricate successful services. Record interpreter/dynamic fallback use separately; it cannot prove complete static recompilation. Normal mode renders one correct view. SBS uses independent eye transforms/targets, correct depth/clears, intentional UI/video policy, adjustable IPD and eye order. Duplicated frames/depth fakes do not pass either runtime's SBS gate.

## Non-negotiable scope

The completion target is the complete supported game, not an exhibition-only remake. The acceptance matrix must cover title screens, menus, Champion Mode, Legacy Mode, exhibition, training, replays, saves, audio, video sequences, DLC, controller settings, and any supported offline modes. Online services are a separate compatibility boundary and must be marked unavailable unless independently implemented.

The original executable, extracted assets, keys, copyrighted audio/video, and user game dumps remain local inputs. They must not be committed, hosted, or included in release packages. Releases contain project code, runtime libraries, tools, and replacement or user-local asset handling only.

The plan does not assume that a working Fight Night Champion recompilation already exists. A framework listing or a successful compiler run is evidence of a candidate route, not evidence that this game boots. The project must prove the exact Xbox build, runtime coverage, and native boot ladder before claiming successful recompilation.

## Repository layout

```text
docs/
  FULL_GAME_BROWSER_SBS_PLAN.md
  FULL_GAME_ACCEPTANCE.md
  FULL_GAME_RISKS.md
  evidence/
    README.md
    cards/
rewrite/
  game/                 # integration only; generated source stays under research/
  runtime/              # ReXGlue integration and platform services
  stereo/               # eye cameras, dual render, SBS pack
  tools/                # manifests, conversion, verification
web/
  launcher/             # browser picker, cache, settings, diagnostics
  wasm/                 # browser platform backend
  tests/                # served-route and browser smoke tests
native/
  launcher/             # portable desktop setup and runtime launcher
research/               # local dumps, Ghidra projects, traces, captures
  reference/             # Xenia settings, manifests, and reference captures index
```

## Phase 0 — freeze scope and provenance

### Tasks

1. Identify the user-owned Xbox 360 ISO and keep it outside this source tree.
2. Record title ID, region, `default.xex` hash, title update hashes, DLC identifiers, and emulator reference settings.
3. Create a provenance manifest for every input.
4. Add packaging checks that reject EBOOT/XEX/ISO files, console keys, and known proprietary asset signatures.
5. Define the supported offline feature matrix in `docs/FULL_GAME_ACCEPTANCE.md`.

### Gate P0

The exact source build is reproducible, inputs are separated from source, the source path is explicit rather than guessed, and every release artifact can be checked for forbidden files.

## Phase 1 — reference and Ghidra baseline

### Tasks

1. Create a fresh Ghidra project for the Xbox executable; do not reuse analysis from another console build as implementation state.
2. Import the executable and title-update variants; document load addresses, sections, imports, exports, TLS, exception data, and thread entry points.
3. Run XenonAnalyse and save jump-table/function metadata.
4. Catalog kernel/XAM/XAPI imports, graphics, audio, video, filesystem, input, timing, save, DLC, and network boundaries.
5. Identify engine initialization, main loop, render submission, asset loading, fight simulation, menu, save, and shutdown paths.
6. Create named Ghidra symbols with confidence and evidence links.
7. Capture reference behavior from Xenia for every acceptance-matrix row and write `research/reference/REFERENCE_MANIFEST.json`.
8. Optionally attempt the pinned REA MCP/evidence provider against the same exact image, but honor its target-format/architecture capability result. The current Windows provider rejects the Xbox PPC/XEX image, so preserve that typed limitation and continue with the PPC/XEX-aware Ghidra project; use REA for supported native helpers and runtime artifacts. Preserve provider/version/profile and compare every returned fact with saved Ghidra and runtime observations. REA is an evidence layer, not the recompiler or loader. See `docs/REA_INTEGRATION.md`.

### Gate P1

The executable fingerprint is fixed, major subsystem boundaries are cataloged, a clean reference capture exists for every required mode, and the selected recompiler/runtime has been built or explicitly marked BLOCKED for this exact title.

## Phase 2 — shared-runtime qualification and whole-program static recompilation

### Tasks

1. Pin and build the selected ReXGlue SDK/runtime and its dependency commits. Record whether the exact TU2 image is accepted; do not silently substitute a different title update or retail revision.
2. Create a Venom manifest for the exact image: source hash, entry point, sections, manual function boundaries, gap seeds, invalid/data ranges, `setjmp`/`longjmp` addresses, imports, title-update/DLC identities, runtime overrides and known renderer requirements. Include generated-symbol collision checks so guest function names cannot shadow ReXGlue/XAPI import thunks.
3. Run the selected code generator against the exact executable and manifest. Preserve generated code as reproducible private output; durable corrections belong in manifest/generator/runtime/module layers, never ad-hoc edits to generated shards.
4. Link the generated title module against the shared ReXGlue runtime and matching injected input/audio/graphics plugins. Implement missing Fight Night contracts in the shared runtime or Venom module according to ownership; do not grow a second disconnected emulator inside the generated shards. Record Debug/Release runtime-plugin identity and reject mismatched DLL/plugin combinations.
5. Keep a private compatibility/reference path for unreconstructed or diagnostic functions while the native bootstrap is being established. Label every fallback site and make release/static-runtime tests fail if a required path still depends on it.
6. Add title-update/DLC selection, crash logging, guest-address diagnostics, deterministic boot logs, and replayable startup commands to the manifest/launcher.
7. Build native Windows x64 first. Only after the C++/ReXGlue reference reaches a first frame and complete offline matrix should subsystem-by-subsystem Rust migration begin behind stable, tested FFI boundaries.

Repair exact native contract failures using source evidence and tests. The native bootstrap is complete only when the generated module plus shared runtime passes the first-frame and full offline gates. The Rust-owned deliverable is complete only when each migrated subsystem passes differential tests against that qualified reference and no required release path depends on diagnostic fallback. Mark the precise gate BLOCKED if unresolved; an emulator or partial remake cannot close the gate.

### Gate P2A — boot

The native build launches, initializes the runtime, reaches the title screen, accepts controller input, and exits without corrupting the source data.

### Gate P2B — complete native game

Run the full acceptance matrix: menus, all offline modes, Champion Mode progression, Legacy Mode progression, training, replay, saves, audio, video, DLC, settings, pause, restart, and repeated cold starts. Every failure becomes a Ghidra/runtime issue with a reproducible capture.

Do not begin browser porting until P2B is passed or a documented feature is explicitly outside scope.

## Phase 3 — native stereo renderer

### Tasks

1. Identify the native recompilation renderer boundary where camera state, world draws, post-processing, and present targets are available.
2. Implement a stereo configuration object: IPD, convergence, eye order, crop, and SBS output size.
3. Render the complete frame twice using left/right camera transforms.
4. Keep HUD, menus, subtitles, video, and other mono-safe layers single-rendered where appropriate.
5. Pack left and right eye images into one stable SBS target.
6. Add a debug mode showing eye labels, camera offsets, render timing, and frame counters.
7. Verify gameplay stability, eye order, disparity, no eye tearing, no hangs, no stale right-eye frame, and resolution changes.

### Gate P3

The native full game displays correct L|R SBS in multiple modes, with measurable non-zero disparity and no regression in input, saves, audio, or progression.

## Phase 4 — portable desktop distribution

### Tasks

1. Package a self-contained launcher and runtime ZIP only after provenance and distribution rights are reviewed.
2. On first run, ask for the user’s ISO or extracted `default.xex` directory.
3. Extract/cache data into a per-install directory without modifying the source dump.
4. Verify hashes, title update, DLC, and runtime version before launch.
5. Provide controller setup, SBS/IPD settings, logs, safe mode, cache reset, and crash recovery.
6. Ensure the package works from a writable folder without installation privileges.
7. Build a reproducible release archive and checksum.

### Gate P4

A clean Windows machine can extract the ZIP, select owned game data, launch the complete native game, enable SBS, save, quit, and relaunch without manual developer tooling. If generated game code cannot be distributed, this gate is a private local-build gate and the release record must say so.

## Phase 5 — browser platform backend

### Tasks

1. Compile the recompiled runtime and platform layer to WebAssembly.
2. Replace native filesystem calls with a virtual filesystem backed by OPFS/IndexedDB.
3. Use an explicit directory/file picker for user-owned game data; never upload it by default.
4. Cache converted data with content hashes and resumable progress.
5. Port the graphics backend to WebGPU; retain a WebGL2 fallback only if the renderer can support it without changing behavior.
6. Port audio to WebAudio and video to a browser-compatible decoder path.
7. Port input to Gamepad API, keyboard, and touch fallback.
8. Port threading/timebase behavior to Web Workers and WASM threads where available; qualify a single-thread variant separately if required; it cannot silently replace unimplemented thread semantics.
9. Add browser-safe save storage, settings, logs, and cache deletion.
10. Add a Bun-served PWA with versioned assets, service worker caching, and no remote code execution.

The browser backend must document the maximum supported input size, storage quota behavior, and whether the user supplies an extracted directory or a converted local cache. A browser cannot be treated as having unrestricted access to an ISO or a native filesystem.

### Gate P5A — runtime boot

The WASM build loads through a served Bun route, initializes the virtual filesystem, accepts selected user data, and reaches the title screen.

### Gate P5B — complete browser game

Repeat the entire P2B acceptance matrix in the browser. Test Chromium, Firefox, and WebKit where supported, plus controller, keyboard, resize, fullscreen, reload, cache recovery, and storage-quota behavior.

### Gate P5C — browser display modes

The complete browser game supports normal single-view rendering and true independent left/right views packed into one SBS canvas. SBS exposes IPD/eye-order controls and holds target frame time during fights, replays, menus, and video transitions. Both modes use the same complete game state and input path.

## Phase 6 — optional transport

WebRTC may transport a proven native build as an additional labeled feature. It cannot replace standalone WASM/WebGPU acceptance. Browser platform failures remain blockers unless the user explicitly changes the goal.

## Verification loop

Every subsystem follows:

```text
Ghidra hypothesis
  -> native/reference capture
  -> implementation or runtime fix
  -> deterministic replay/test
  -> fresh-context critic
  -> acceptance evidence
```

Required evidence includes source diff, generated-code/build log, runtime log, replay hash, screenshot or video capture, and acceptance-matrix row. A green compile is never sufficient proof of a working game.

## Final definition of done

Completion requires the entire supported offline game to pass on native Windows and browser-native runtimes in both normal and true SBS. Both load owned data locally and support controller input, saves, audio/video, TU2/DLC, crash recovery and long sessions under independent review. Xenia/streaming results cannot substitute.

The release must include reproducible builds, provenance/licensing documentation, checksums, forbidden-file scans, crash recovery instructions, and a clear statement that users supply their own game data.

## Toolchain references and verification

The current selected checkout is [nimauria/Xenon-Recomp](https://github.com/nimauria/Xenon-Recomp), base commit 2aa583907d6c8962bc11a971f6b9033a19e229ee plus local patches. The workflow revision adds a qualification branch for the pinned [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk) and its Xenia-derived runtime. [hedge-dev/XenonRecomp](https://github.com/hedge-dev/XenonRecomp) remains a code-generation comparison candidate. These projects have different manifests, generated ABIs and runtime contracts; commands and generated outputs must never be mixed without an explicit adapter and tests. Pin exact repositories, commits, patches, dependencies, commands and Fight Night compatibility results in `research/TOOLCHAIN_MANIFEST.json`. Upstream runtime coverage is not a Fight Night compatibility guarantee.

