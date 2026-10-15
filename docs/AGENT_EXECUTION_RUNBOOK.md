# Agent execution runbook

**Newest bounded observation (2026-10-15, actual162):** Integration159 produces live guarded effective60 timer proof, six qualified HUD clock decreases and four script requests. Blank numeric OCR ends the script safely; title/observer exit0. Full bout, normal game timing and displayed FPS remain unpassed. Prepare168 bounded neutral HUD retry, integration166 default-off GPU warning rate limit, and163 exact-root metadata save tracing. Actual155 reaches main menu with its new seed unchanged, but successful profile/header reads remain UNKNOWN. See newest STATUS and [actual162 card](evidence/cards/08-runtime162-exploratory60-20261015.json); historical observations below do not supersede it.

**Newest bounded observation (2026-10-15):** Actual151 runs integration137 through native startup and active Tyson/Ali boxing, with three verified decreasing HUD clocks. Title and observer close normally with exit0. The next valid-looking HUD frame is rejected by the strict reader before scripted punches; full bout and FPS remain unpassed. Reader150 passes independent preparation review; combine it with151 as152 and review the new2:36 frame before launch. See newest STATUS and the actual151 card; the historical corrections below do not supersede them.

**Latest execution correction (2026-10-15):** Actual134 creates a new profile/header, validates one-round/one-minute settings and renders Ali/Tyson intro cards; no HUD before90-second observer bound. Runtime138 independently reviewed180-second UNKNOWN/LOADING wait stops before title at concurrent controller snapshot sharing. Runtime140 repairs that read contract; fresh review and bounded regression precede further launch. Reload139 and combined recompile137 remain separately unadmitted. See newest STATUS; full gameplay/reload/FPS/SBS/browser gates stay open.

**Current evidence (2026-10-15):** Actual132 independently observes Start-to-main-menu and side selection on the reviewed native pair. Same seed/pair progresses after shortening the diagnostic profile path below260 characters and staging the fs-debug config at the effective user-root location. Publisher heartbeat contention stops the script before gameplay. Use newest STATUS, actual132 evidence/review and separately reviewed runtime134 repair. Full native game/saves/FPS/SBS/browser gates remain open; historical menu-unpassed statements are superseded for this bounded milestone only.

**Active execution correction (2026-10-13):** Follow [STARTUP_REPAIR_PROTOCOL.md](STARTUP_REPAIR_PROTOCOL.md) for Card 07–08. It supersedes historical startup diagnoses, counts and next actions. Clean native intro, Press Start and autosave captures are observed; Start-to-main-menu remains unpassed. Use the latest numeric retained probe result. Require a falsifiable targeted hypothesis, retained process/exit evidence, causal repair and regression; no unchanged broad retries. Native full gameplay precedes SBS and standalone browser work.


This is the operational companion to `FULL_GAME_BROWSER_SBS_PLAN.md`. An agent must execute the active cards in order, record evidence, and keep independent work moving when one gate is blocked. **Workflow revision (2026-10-12):** the native qualification route is a generated Venom title module linked to a pinned ReXGlue/Xenia-derived shared runtime. The existing custom host is diagnostic evidence only until it beats or explains the shared-runtime result. The active route is full native recompilation followed by standalone WASM/WebGPU browser execution, both with normal and true SBS. Xenia is reference tooling; streaming cannot satisfy completion.

Use [COMPLETION_REPAIR_PLAN.md](COMPLETION_REPAIR_PLAN.md) for Packages A-I, mapped to these cards. Its immediate work queue follows STARTUP_REPAIR_PROTOCOL.md: reconcile current identities, map actual startup callers, compare a specific reference contract and test a causal repair. Preserve card dependencies; repair failed prerequisites while keeping independent work moving.

**Link provenance rule (2026-10-15):** Resolve every executed linker `@response` argument to its absolute file, pin its hash, and verify that actual object vector against the claimed replacements. A separately staged response or map basename is insufficient. For new host telemetry, also check its own known literal/CVar in the linked artifact and require live invocation before a runtime claim. Integration137 had an unused prepared response; see its post160 provenance audit.

## Rules every agent must follow

1. Work from the Xbox workspace root: the directory containing this `docs` directory (`xbox-full-game-sbs`).
2. Never modify the source ISO, `default.xex`, title updates, DLC, or any original dump.
3. Keep extracted data, Ghidra projects, generated proprietary code, and captures under local `research/`; only manifests and non-proprietary evidence indexes may be tracked.
4. Never publish original game data or generated code derived from it.
5. A compile is not runtime proof. Every claim needs a log, capture, replay, or hash.
6. If a command or tool is missing, record the exact error, obtain or repair the pinned dependency, and retry. Keep independent work moving; do not substitute an unverified tool silently.
7. Preserve failed experiments. Do not delete evidence to make the matrix look green.

## REA-style investigation contract

For every active reverse-engineering hypothesis, retain one evidence record that binds the exact input hashes, provider/tool versions, analysis profile, target addresses, commands, generated-source locations, runtime observations, expected discriminator, result (`PASS`, `FAIL`, or `UNKNOWN`), limitations, and next action. Join static Ghidra facts, generated-code facts, and runtime/Xenia observations by address and artifact hash. A missing observation is `UNKNOWN`, never proof of equivalence. Use an isolated read-only analysis snapshot for each fresh critic pass; do not silently change providers, language profiles, or source images between retries.

## Card format

Every completed card writes a JSON or Markdown result containing: date, agent, input hashes, commands, output paths, status (`PASS`, `PARTIAL`, `BLOCKED`), and next card.

## Card 00 — workspace and source lock

**Input:** the supplied Xbox ISO.

**Actions:**

```powershell
New-Item -ItemType Directory -Force research, research\source, research\captures, research\ghidra, research\toolchain, research\reference | Out-Null
$parent = Split-Path -Parent (Get-Location)
$candidates = @(Get-ChildItem -LiteralPath $parent -Filter 'Fight Night Champion*.iso' -File)
if ($candidates.Count -ne 1) { throw "Expected exactly one Xbox Fight Night Champion ISO beside the workspace; found $($candidates.Count)." }
$iso = $candidates[0].FullName
Get-FileHash $iso -Algorithm SHA256 | Out-File research\source\iso.sha256
Get-Item $iso | Select-Object FullName,Length,LastWriteTime | Out-File research\source\iso.metadata.txt
```

**Output:** `research/source/iso.sha256`, `iso.metadata.txt`.

**Pass condition:** hash and size are recorded; the selected path is explicit; the original ISO remains unchanged.

## Card 01 — tool discovery

Find and record versions for Ghidra, XenonAnalyse, XenonRecomp, ReXGlue, CMake, Ninja, Clang/MSVC, Rust, Bun, and Xenia. Treat ReXGlue as the primary runtime qualification candidate because current Xbox 360 recompilation projects use its Xenia-derived shared runtime; treat the selected XenonRecomp checkout as a code-generation candidate until a title-specific manifest and runtime are proven. Verify that the selected generator and ReXGlue runtime can be built and that the documented input format matches this Xbox executable. Use pinned commits once a repository is selected. Save executable paths, repositories, commits, dependency hashes and version output in `research/TOOLCHAIN_MANIFEST.json` and `research/toolchain/versions.txt`.

**Blocked condition:** neither the selected generator nor a shared runtime can be obtained, built, or shown to accept the exact Xbox 360 image. Record the missing contract and keep the alternative generator/runtime qualification isolated; an emulator stream cannot replace the goal.

## Card 02 — inspect ISO without mutation

Use an approved read-only Xbox ISO extractor. Extract to `research/source/extracted/<iso-hash>/`. If no extractor is installed, obtain and pin a suitable read-only extractor before retrying. Do not mount the ISO read-write.

**Outputs:** extraction log, file listing, `default.xex` path, title-update/DLC inventory, and a read-only extraction manifest under `research/source/`.

## Card 03 — executable identity

Hash `default.xex` and all selected content packages. Record title ID, media ID, version, region, update assumptions, and the hashes of every selected package in `research/SOURCE_MANIFEST.json`. Record any expected Xenia title-update or DLC mapping in `research/reference/REFERENCE_MANIFEST.json`; do not compare against an undocumented manifest.

**Pass condition:** one exact source build is selected. If multiple revisions exist, choose one and freeze it before analysis.

## Card 04 — reference boot

Run the frozen source in Xenia using the recorded settings. Capture title screen, menus, one complete exhibition fight, save creation, reload, replay, and quit. Store captures under `research/captures/reference/`, write `research/reference/REFERENCE_MANIFEST.json`, and link each capture from `docs/FULL_GAME_ACCEPTANCE.md`.

**Blocked condition:** the source does not boot or cannot reach an in-game fight. Keep Card 04 incomplete. The user explicitly authorized native recompilation/runtime work while capture is unavailable; capture remains required before gameplay acceptance passes.

## Card 05 — fresh Ghidra project

Create a fresh Ghidra project for the exact effective TU2 image, preserving base/update hashes and loader memory map, instead of retail-only analysis of `default.xex`. Save the project archive and import report. Run analysis without inventing symbols. Export memory map, imports, strings, entry points, and function count.

REA may be used as a read-only evidence provider for this card. Follow
[REA_INTEGRATION.md](REA_INTEGRATION.md): pin the package, run the Codex/Ghidra
doctor checks, isolate each provider session, and join every result to the
exact image hash. REA output supplements the saved Ghidra project; it does not
replace it or perform recompilation.

**Output:** `research/ghidra/<source-hash>/` plus `ghidra-import.md`.

## Card 06 — manifest-driven analysis and code generation

**Generator repair regression:** If generator implementation changes, invoke `codegen --ignore-stamp` and inspect actual emitted statements before rebuilding. The existing stamp can report a module up to date despite a semantic generator fix. Preserve a pre-fix binary and before/after capture, and execute a focused regression with a failing original-bug control. The 2026-10-13 vector-pack repair demonstrated this requirement; see `evidence/cards/08-vector-pack-repair-20261013.json`.

**2026-10-13 input/boundary admission:** Hash the payload actually consumed by the loader, not just its XEX container. It must equal canonical pre-import-resolution TU2 SHA-256 `EBDCBE683DB2F1C29AA43AC97E79492926C099B2EE253DB502CABCBEC133D52F`. Reject post-load captures whose import records contain host-patched values or syscall placeholders. For the uncompressed research wrapper, require one payload and container length equal to header length plus declared image size; a duplicated payload can overwrite the runtime function table. Never use `size=4` merely to silence unresolved-target errors. Use reachable-body discovery or byte-verified complete bounds, then inspect the generated tail and indirect entries. See the 2026-10-13 STATUS evidence before reusing historical qualified artifacts.

Run XenonAnalyse, then the selected pinned generator using the exact source hash and a Venom manifest. The manifest must record manual function boundaries, gap seeds, invalid/data ranges, special control-flow addresses, imports, title-update/DLC identities, runtime overrides and generated-symbol collision checks. A guest function name must not shadow a ReXGlue/XAPI import thunk. Preserve generated-code manifest, warnings, unresolved symbols, boundary corrections and compiler output. Generated files stay under local `research/`, either `research/generated/<source-hash>/` or the exact manifest-recorded prepare cache/workspace, until licensing/provenance review is complete. Do not duplicate or regenerate a proven artifact merely to rename its directory.

**Pass condition:** deterministic generation succeeds twice from the same inputs, the manifest has no unexplained boundary/data warnings on required paths, and the output is accepted by the selected shared runtime. A successful translation or link alone is only `PARTIAL`.

## Card 07 — native compile and runtime scaffold

Run existing loader/crypto/LZX and decoder semantic tests after toolchain changes. Bind the exact TU2 title module through the selected ReXGlue/shared-runtime ABI and verify identities. Follow the runtime setup order: exception support, clock, guest memory, export resolver, function dispatcher, VFS, kernel state, input, kernel modules, audio, mounts, graphics, then function mappings. Keep guest memory, process/module state, TLS/PCR, threads, kernel/XAM imports, filesystem and timing in the shared runtime unless evidence assigns a contract to the Venom module. Treat input, audio and graphics as injected backends and record the plugin/runtime ABI and Debug/Release pairing. Keep title symbols, boundaries, TU2/DLC handling and compatibility hooks in Venom. Use generated C++ plus the shared runtime as the first native reference; only after it reaches a first frame and complete offline matrix may subsystem-by-subsystem Rust migration proceed behind stable, tested FFI interfaces. Do not claim completion from a wholesale unvalidated translation.

A DLL link is compile evidence only. Never use successful no-ops for unresolved exports. Validate manual function bounds, non-return assumptions and indirect routing against exact bytes/execution. Record dynamic fallback use separately.

## Card 08 — native execution ladder

Advance: module ABI binding; process/memory initialization; TU2 identity/content mount; guest entry/first service call; correct first rendered frame; title; menus/controller; fight loading; complete round/match; save/reload; progression; TU2/DLC. Preserve failures with guest PCs, module, service traces and commands. Headless probes cannot close visual/gameplay gates.

## Card 09 — full native offline matrix

Run every required acceptance row against the recompiled runtime with reference comparisons and separate feature evidence. One match/title screen does not pass the complete game.

## Card 10 — native normal and stereo modes

Preserve normal output. Implement independent eye cameras/targets, depth/clears, SBS composition, intentional UI/video policy, IPD/eye order. Capture real gameplay with measurable parallax, complete synchronized eyes and no regressions. Pass both modes across the full matrix.

## Card 11 — portable native package

Create a writable-folder package with read-only owned-data selection, hash checks, isolated saves, controller settings, caches, logs and crash recovery. Generated proprietary code/binaries remain private unless rights review permits distribution. Verify a clean Windows environment.

## Card 12 — browser feasibility

After full native/display gates pass, test a no-content WASM shell using Bun, WebGPU, workers, OPFS/IndexedDB, WebAudio/video, Gamepad API and saves. This is a feasibility probe only. Browser failures remain blockers; streaming is not an automatic substitution.

## Card 13 — standalone browser backend

Port the same complete runtime to WASM/WebGPU with local data/cache, audio/video/input/storage/timebase/worker services. It executes locally without a Windows game host or streaming server.

## Card 14 — full browser matrix

Repeat full offline acceptance in normal and true SBS. Test served production routes, supported browsers, reload, quota/cache recovery, resize/fullscreen, controller reconnect, saves, eye alignment/IPD and long sessions.

## Card 15 — browser package and optional transport

Package the standalone browser runtime with reproducible Bun commands and local provenance/data handling. Optional WebRTC remains separately labeled and cannot close Cards 13/14.

## Card 16 — release audit

Every required offline row passes on native and browser-native in both display modes. Audit source/toolchain hashes, provenance, packaging, checksums, SBOM, recovery, reproducibility and independent gameplay evidence. Online services remain unsupported unless implemented. Record RELEASE_MANIFEST.json with private/distributable status.

## Blocker handling

Preserve evidence, repair concrete causes without unchanged retry loops, and continue independent tasks. Never substitute an emulator stream, duplicate eye, depth fake or partial remake. Historical pivot results are diagnostic only.

