# Venom: isolated ReXGlue comparison execution plan

Prepared 2026-10-07. Executed as an isolated bounded comparison on 2026-10-08; see `research/rexglue-comparison/campaigns/20261008T0043Z/results/COMPARISON_REPORT.md` for completed gates, blockers, and preservation verification. Code generation and runtime comparison remain incomplete.

## Purpose and authority

Determine whether ReXGlue provides a more reliable path to original Fight Night Champion native execution than the current nimauria/Xenon-Recomp runtime, without replacing, rebuilding, resetting, or contaminating current progress.

This is a bounded comparison under Cards 01, 05–08 of `AGENT_EXECUTION_RUNBOOK.md`, subordinate to `FULL_GAME_BROWSER_SBS_PLAN.md` and `COMPLETION_REPAIR_PLAN.md`. It does not replace the active runtime, reduce the full-game scope, authorize release, or advance browser/SBS/full-game acceptance. The initial useful target is correct normal-view native startup; exhibition is the subsequent gameplay test if prerequisites pass.

The stages below define the authorized experiment. This execution completed isolation and SDK build checks, then recorded the generator blocker. It does not claim later runtime or gameplay gates passed.

## Starting evidence to refresh before execution

- Active runtime: `research/third_party/xenon-recomp`, repository `nimauria/Xenon-Recomp`, recorded base commit `2aa583907d6c8962bc11a971f6b9033a19e229ee`, with local patches. Actual current revision, patch hash and build IDs must be captured at execution time.
- Current TU2 pre-map effective-image SHA-256: `EBDCBE683DB2F1C29AA43AC97E79492926C099B2EE253DB502CABCBEC133D52F`; runtime SHA-1: `b771b41caef2483549bfbc7d726be3eb56943f99`. Treat these as expected recorded values, not a replacement for hashing actual inputs.
- Current generated module has compile/binding evidence. Latest recorded elevated Run25 mounted content, loaded TU2, created a window and executed for five minutes, but verified neither title nor first frame. GPU ring/front buffer remained unset. Run26 was executed but reached neither title nor first frame; Runs33/34 later confirmed the restricted-token content-mount repair without closing graphics startup. See `docs/evidence/cards/07-run26-run33-run34-20261008.json` before planning another baseline.
- ReXGlue historical SDK pin: `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`. Historical probe: `research/rexglue-fnc-base/`. It used base-image generation, unresolved control-flow traps and missing content/cache paths. Its `fnc_tu2_probe` name does not prove TU2 identity. It is reference material, not a qualified comparison build.
- `research/TOOLCHAIN_MANIFEST.json` and `research/third_party/LOADER_SELECTION.md` contain historical entries. Record refreshed facts in the experiment manifest instead of silently treating those entries as current.
- Existing post-load Xenia capture differs from the canonical pre-map image in unexplained ranges. Comparing different loader stages without accounting for relocations/import fixups is not source equivalence.

## Isolation contract

Experiment root: `research/rexglue-comparison/`. Each execution campaign gets a unique UTC timestamp plus short ID; never reuse a historical run directory.

```text
research/rexglue-comparison/
  .gitignore                 # ignores all experiment contents
  campaigns/<campaign-id>/
    baseline/                # inventories, patches, preservation snapshots
    inputs/                  # qualified research copies and manifests
    toolchain/               # independent SDK source/dependencies/install
    project/                 # comparison-specific handwritten integration
    generated/<image-id>/    # private generated game code
    build/                   # SDK and candidate output
    harness/                 # guards, launcher, watchdog, result collection
    runs/<run-id>/
      config/
      state/                 # saves, profiles, cache, shader cache, temp
      logs/
      captures/
      result.json
    reference/<run-id>/      # isolated research reference state/captures
    results/                 # gate records, comparisons, final report
```

Tracked output: this plan and a non-proprietary evidence index under `docs/evidence/cards/`. All binaries, generated sources, dumps, patches containing game data, local paths/configuration snapshots, captures and saves stay under ignored research paths. Do not publish or package them.

Protected from writes: owned ISO/XEX/TU/DLC, user Xenia installations, existing generated modules, active toolchain source/build/install, original Ghidra projects, old research runs, existing saves, native/web launchers and governing documents. Read-only access is allowed within the active Xbox scope.

Use an independently populated directory, not a junction/hardlink to a writable active build or save path. If candidate writes cannot be redirected away from protected paths, do not run it until redirection or a separate research build fixes that contract. Permissions must not be changed on owned inputs or active installations.

## Stage 0 — establish preservation and experiment boundaries

Actions:

1. Resolve the absolute Xbox workspace, experiment root and campaign directory. Validate every intended write lies beneath the campaign (except the experiment `.gitignore` and final non-proprietary index). Reject junction/reparse targets escaping that boundary. No recursive deletion is needed.
2. Check the actual enclosing Git repository. Scope status/diff inventories to the active Xbox workspace and active nested toolchain; do not inspect deprecated PS3 trees. Do not assume all valuable work is committed or even tracked.
3. Save revision/status information, tracked diff patches, inventories and hashes for active handwritten source/configs and nested runtime changes. Preserve changed/untracked relevant files with their relative paths. Save the running host/module/config/manifest hashes; record existing build/dependency paths. Patch files alone are insufficient for untracked work.
4. Preserve exact existing host/module binaries and their required adjacent runtime libraries in `baseline/` if a reproducible executable snapshot is feasible. Otherwise mark the baseline restoration claim incomplete; do not represent an arbitrary EXE copy as a portable snapshot.
5. Record owned input hashes from provenance plus live validation of selected XEX/TU/effective image. Schedule full ISO and selected DLC integrity checks before/after the campaign; do not repeatedly hash gigabytes on every short probe.
6. Discover disk space and current activity. Use a process-specific inventory to avoid competing runs/builds; do not terminate another task's processes. Start with four compilation jobs and adapt only to measured resource headroom.
7. Add experiment-local ignore coverage. Verify exclusion with `git check-ignore`; also check nested repository boundaries. Never use `git add -A`, reset, clean, checkout-overwrite, or stash as a preservation shortcut.

Deliverables: `baseline/PRESERVATION_MANIFEST.json`, inventories, source patches/snapshots, protected-path list, ownership/write map and free-space record.

Gate S0: valuable current state is recorded and relevant modified files preserved; intended writes are isolated; research outputs are ignored. Stop dependent work if preservation/isolation cannot be established.

## Stage 1 — freeze a fair comparison contract

Define before running:

| Dimension | Required treatment |
|---|---|
| Executable | Same qualified effective TU2 instructions/data and entrypoint; each toolchain's ABI may differ |
| Content | Same selected disc/TU/DLC inventory; explicit content paths and documented mount semantics |
| State | Separate fresh profiles/saves/caches; compare equivalent state, never share writable stores |
| Graphics | Real normal-view backend; same GPU, dimensions and comparable settings where possible |
| Environment | Same OS/driver and privilege class; record deviations and classify environment failures separately |
| Time budget | Same post-guest-start observation window; separate build/load deadlines |
| Instrumentation | Comparable event coverage; record trace caps and overhead |
| Success | Observable milestones supported by trace and visual evidence |

Define an import disposition table: ordinal/name, function versus data, call site, implementation status, supported/unsupported behavior and observed calls. An unresolved import count is not a compatibility score.

Deliverable: `results/COMPARISON_CONTRACT.json`. Gate S1: source/state/environment and result definitions are explicit. No comparison verdict from retail versus TU2 or null renderer versus real renderer.

## Stage 2 — independently build and pin ReXGlue

1. Populate an independent SDK checkout and submodules from the historical pin. Read its actual CLI, configuration, app lifecycle and build documentation. Record the commit/submodule identities and provenance. Do not run candidate commands in the active nimauria checkout.
2. Reproduce only necessary historical Windows repairs in the experiment checkout: symlink-stub materialization and SSSE3 compiler options, if still needed. Keep each repair as a separately explained diff. Verify it does not alter game behavior.
3. Use installed pinned compiler/CMake/Ninja tools where appropriate; use explicit paths and process-local environment variables. Build and install exclusively into campaign directories. Do not replace global SDKs or modify persistent PATH.
4. Run applicable SDK loader/codegen/runtime tests before introducing Fight Night. Inventory known unsupported functionality.
5. If a newer revision is needed for a specific repair, keep the original pin/candidate intact; record a second revision as a distinct experiment, with the upstream change and reason.

Deliverables: SDK/dependency/patch hashes, configure/build/test logs, CLI help and exact reproducing commands. Gate S2: independent SDK passes relevant available tests and its output/input contract is understood. SDK build success is not game success.

## Stage 3 — qualify exact TU2 input before generation

1. Re-hash the canonical effective image and source packages; record title/media IDs, update version, entrypoint, mapped sections/protections and image size.
2. Determine the pinned ReXGlue loader's accepted updated-image format from source/documentation. A raw memory dump is not automatically a valid XEX. Do not copy the active generated DLL into the candidate; generated ABIs are not interchangeable.
3. Prefer supported base-plus-TU processing if it reproduces the canonical image. If a research effective-image adapter/container is necessary, build it inside `inputs/`, preserve authentic metadata/section semantics, and verify reconstructed bytes and mapping independently.
4. Export candidate bytes at an equivalent pre-map stage and compare with the canonical image. For post-map comparison, enumerate and explain relocation/import-fixup differences rather than blanket-normalizing ranges. Preserve remaining byte differences and block qualification if executable differences remain unexplained.
5. A retail-only run may diagnose the candidate loader, but receives a separate label and cannot decide the TU2 comparison.

Deliverables: `inputs/INPUT_IDENTITY.json`, loader logs, section map and byte-difference report. Gate S3: same effective TU2 source is qualified. If blocked, report an input/loader incompatibility, continue SDK/isolation investigations and do not guess replacement bytes.

## Stage 4 — generate and build the candidate application

1. Create a uniquely named comparison app; start from the pinned SDK's documented app boilerplate. Treat historical probe files as leads, not trustworthy templates.
2. Generate only from S3-qualified input with compatible analysis. Reuse validated metadata where its format/semantics fit; otherwise regenerate independently under the campaign. Validate manual boundaries/jump tables against exact instructions.
3. Inventory warnings, traps, unresolved direct/indirect targets, and unsupported exports. Historical base-image targets are leads only; do not assume the same TU2 addresses.
4. Generate twice into separate outputs and compare normalized content manifests, documenting timestamps/path differences. Do not hand-edit generated gameplay code to suppress traps.
5. Bind correct loader/process/thread/TLS/input/filesystem/graphics/audio services. Explicit unsupported device behavior is acceptable only where accurate; fabricated-success services cannot pass a gate. Audit historical USB-camera stubs before any reuse.
6. Compile using candidate-only prefixes, dependency lookup and caches. Record complete app/SDK/library hashes and build commands.

Deliverables: generation manifests, warning/import tables, app source diff, build log and artifact hashes. Gate S4: reproducible build with explicitly accounted gaps; no automatic runtime acceptance.

## Stage 5 — validate launch containment before game execution

Implement a candidate launcher/watchdog in `harness/` with:

- Unique run ID, exclusive campaign runtime lock and collision rejection; lock is released on all exit paths.
- Validation of image/app/SDK/config hashes and JSON/TOML parsing before launch.
- Explicit working directory and absolute resolved content/state/cache/log/temp paths. Redirect profile/user-data paths through supported APIs; environment variables alone are insufficient proof if native known-folder APIs are used.
- A dry-run write-path inventory and a safe no-game/synthetic fixture that verifies save/cache/log destinations. Observe file writes where supported, including SDK caches and default configuration paths.
- Correct argument quoting for paths containing spaces; explicit stdout/stderr capture and visible diagnostic summary.
- PID plus start-time/process ownership and child-process tracking. Never terminate by executable name, broad process matching or shared PID files.
- Separate build/load/run limits. Initial defaults: 180 seconds to load/start and 300 seconds after guest entry; record hardware-based changes. Signal cooperative stop, allow five seconds, then terminate only owned execution. Report forced termination as shutdown failure.
- Bounded/disk-budgeted traces; preserve saturation/drop counters. Record all guest threads, import events, Vd/GPU setup, ring/front-buffer/present, failures and exit reason.

Resolve required privileges before a runtime comparison. The current normal-token mount failure is an environment issue; compare under the same approved privilege class. Do not loop failed RunAs attempts. If this desktop cannot elevate, prepare a reviewed helper for a user-run elevated session and state the external requirement; dependent live tests wait for it.

Deliverables: dry-run report and harness checks for containment, collisions, malformed config, quoting and owned-process cleanup. Gate S5: writable-state containment and watchdog behavior demonstrated before owned game execution.

## Stage 6 — establish reference and active-runtime baseline

1. Use latest existing trustworthy exact-TU2 evidence first. Do not overwrite or relaunch Run25/26 in their historical directories.
2. If a fresh baseline is needed, run the existing host/module without rebuilding or modifying them, using a new campaign-specific config/state directory. If active runtime cannot redirect all writes, repair only an isolated copy and label it separately.
3. Obtain missing Card 04 reference evidence through a separately configured research Xenia build/copy, preserving user installations. Capture identical source identity, startup, menu input and exhibition where feasible; no reference emulator mutation outside research.
4. Record baseline milestones, event trace limits, shutdown behavior, duration, privilege, GPU/driver, window/frame evidence and fallback use. Absence of a capped log entry is not absence of an event.

Gate S6: baseline provenance is sufficient to compare observed startup. If a reference match is unavailable, startup diagnosis can continue, but gameplay fidelity remains unqualified.

## Stage 7 — run the ordered execution ladder

Run sequentially and retain failures:

| Gate | Evidence required |
|---|---|
| R0: initialization | Valid source identity, content mounts, module binding, thread/service initialization |
| R1: graphics activity | Actual device/ring/command/front-buffer events; not just a window |
| R2: first correct frame | Screenshot/capture plus present trace, reference comparison; exclude blank/loading-only false positives |
| R3: title and menus | Correct title/menu output and reproducible controller navigation |
| R4: fight loading | Two selected fighters and arena correctly assembled, loaded and controllable |
| R5: round | Complete round with movement, punches, defense, opponent behavior, HUD and required audio |
| R6: match lifecycle | Complete exhibition, result, pause/resume, rematch/quit and clean shutdown |

Start with one bounded diagnostic run per candidate. Repair one evidence-backed cause at a time in the isolated candidate, then run relevant regression checks and a fresh probe. Run three equivalent cold starts for repeatability only once a milestone is reached or a symptom varies. Collect a complete match only after menus/fight loading work.

Do not score diagnostic fallback execution as full static qualification. Separate generated-code execution from interpretation/JIT fallbacks, service substitutions and unsupported paths. Performance comparison begins only after both candidates produce equivalent correct output, with instrumentation overhead controlled.

## Stage 8 — compare and decide

For every run record: campaign/run IDs, repository commits/patches, input/host/module/config hashes, content inventory, state initialization, environment/privilege, exact commands, start/load/run times, event coverage/drop counts, guest PC/LR/thread at failure, imports, GPU milestones, captures, fallback use, shutdown, protected-file verification and next hypothesis.

Compare the earliest meaningful observable divergence from reference. Thread scheduling can vary, so do not demand identical wall-time trace ordering; align by initialization events and relevant thread/call relationships.

Possible outcomes:

- **Prefer ReXGlue provisionally:** qualified TU2, intact preservation boundaries, repeatable advancement beyond the active runtime on correct visual/input milestones, and understood fixes rather than concealed traps. A complete stable exhibition is much stronger evidence than a first frame.
- **Retain current route:** candidate regresses, remains blocked on comparable startup, or offers no demonstrated advantage. Transfer only independently validated ideas/fixes through explicit adapters; do not mix generated ABIs.
- **Inconclusive:** identity/reference/environment differences prevent a fair comparison, or progress differs without enough repeatability. State the missing evidence and next bounded test rather than choosing by reputation or compile speed.

Assess browser/SBS implications separately in a source audit: host-only APIs, dynamic module loading, executable-memory/JIT dependencies, guest memory layout, threading, GPU shader translation and camera/render hooks. Native success does not prove WASM/WebGPU feasibility. Do not start the browser port or mark stereo complete during this comparison.

Deliverables: `results/COMPARISON_REPORT.md`, machine-readable matrix, defect ledger and non-proprietary index `docs/evidence/cards/07-rexglue-isolated-comparison.md`.

## Stage 9 — preservation audit, review and handoff

1. Recheck protected source/config/build artifact hashes against S0, including original owned inputs. Expected active logs from unrelated authorized work are not automatically corruption; identify concurrent changes explicitly and rebaseline, never overwrite them.
2. Confirm the active launcher, runtime selection, saves and source patches remain intact. No restoration should be necessary after an isolated experiment. If an unexpected write occurs, stop new launches, retain evidence and propose a narrow repair; never automatically restore a snapshot over newer user work.
3. Review actual logs/source/manifests/captures against all claimed gates. Seek independent review during execution as required by the governing verification process; if unavailable, label self-review and keep independent acceptance pending.
4. Retain the experiment as local research. No cleanup deletion is required. Deliver exact reproduction and limitation paths.
5. If recommending migration, first create a concrete migration proposal listing source/config changes, adapters, regression checks and how the preserved current route remains runnable. Changing the authoritative route is a separate explicit adoption decision; do not silently edit the full-game plan or shared launchers.

Gate S9: evidence supports the recommendation, current progress is intact, and all full-game/SBS/browser gates remain honestly classified.

## Execution checklist

- [ ] S0 preservation snapshots, protected paths, ignore coverage and resource budget.
- [ ] S1 fixed comparison contract.
- [ ] S2 independent pinned SDK build and relevant tests.
- [ ] S3 exact TU2 input/mapping equivalence.
- [ ] S4 deterministic generation, build and gap inventory.
- [ ] S5 contained state/write paths and bounded process-specific harness.
- [ ] S6 refreshed baseline/reference evidence without mutation.
- [ ] S7 ordered startup/visual/input/gameplay tests with fresh run IDs.
- [ ] S8 evidence-based verdict and browser/SBS implications audit.
- [ ] S9 preservation verification, review and explicit migration handoff if warranted.

First execution action: create the unique ignored campaign and preservation manifest. Do not begin with codegen, patching active services, or reusing the historical base-image probe as a TU2 test.
