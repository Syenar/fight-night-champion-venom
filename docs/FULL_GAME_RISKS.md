# Full-game Xbox risk register

This register keeps route decisions tied to evidence. A risk is not closed by a build that only reaches the title screen.

| Risk | Trigger | Mitigation | Gate |
|---|---|---|---|
| Diagnostic save path exceeds Windows API boundary | Actual127 payload path270units fails while header259 succeeds; native131 reproduces raw260/270 failure and258/259 success | Short258-unit path reaches menu132 and creates profile/header134. Effective user-root config fixes missing fs debug. Preserve old failure; verify new save title-side reload before closing save gate. [Evidence](evidence/cards/08-runtime134-intro-20261015.json) | Card08–09 |
| Concurrent controller-file snapshot read denies atomic replacement | Actual138 stops before title with PermissionError13 at final neutral snapshot read; cleanup completes | Runtime140 must reuse existing bounded Windows shared-read helper while preserving packet validation and neutral/freshness gates; fresh controls/review then bounded regression | Card08–09 |
| Optimizing generated core does not reach graphics startup | Run160 loaded an independently built exact-TU2 `-O1` module but reached only 634880 sampled main blocks, with no Vd/ring/front-buffer transition or frame | Trace guest target `0x82EE369C` used by the alertable wait wrapper and compare its import contract with pinned Xenia; retain successful wait returns and require a concrete state/status mismatch before changing semantics. [Evidence](evidence/cards/07-run160-optimized-startup-20261009.json) | C / Card 07 |
| Native core startup trace does not reach the late caller target within a bounded probe | Run159's `-O0` generated module reached 630,784 sampled main blocks in 420 seconds without entering `0x82EAEAD8`; a watchdog ended the process | Build the exact generated module separately at `-O1`, pin its hash, and compare a bounded same-TU2 run; if still shallow, capture host/guest progress and resource evidence before more semantic changes. Do not infer unreachable code from a capped trace. [Evidence](evidence/cards/07-run159-caller-probe-20261008.json) | C / Card 07 |
| Wrong source revision | ISO, `default.xex`, title update, or DLC hashes do not match the frozen manifest | Stop, select one exact build, and regenerate the source manifest | P0/P1 |
| Recompiler/runtime incompatibility | XenonRecomp or ReXGlue cannot build, cannot parse the executable, or cannot satisfy required imports | Repair the exact native contract; emulator streaming cannot close the gate | P1/P2 |
| Custom runtime divergence | A title-specific host reimplements Xenia contracts incorrectly and stalls before first frame | Qualify the pinned ReXGlue/Xenia-derived shared runtime first; keep the custom host diagnostic only and assign each service to shared runtime or Venom module | P1/P2 |
| Manifest/function-boundary error | Generated code compiles but an overlapping/truncated function, missed gap seed, invalid data range, or special control-flow address sends execution to the wrong continuation | Maintain a versioned Venom manifest with manual boundaries, gap seeds, invalid ranges, `setjmp`/`longjmp`, indirect targets and boundary tests; compile success cannot close this risk | P1/P2 |
| Generated code is not distributable | Provenance or licensing review rejects public distribution of generated game code or assets | Keep a private local build or distribute only after rights review; do not label a source-only toolchain as a portable release | P4/P6 |
| Xenos graphics gap | Native boot works but renderer, shaders, video, or post-processing diverges | Track each graphics contract, preserve a reference capture, and block stereo work until native rendering is stable | P2/P3 |
| Browser platform gap | WebGPU, WebAudio, workers, storage, or video APIs cannot satisfy the native contracts | Repair browser contracts and record unresolved platform blockers; optional WebRTC cannot satisfy standalone browser completion | P5A/P6 |
| Storage or input limits | Browser quota, file picker, controller, reload, or reconnect behavior loses user data or input | Use content hashes, resumable conversion, explicit local-only storage, and browser-specific acceptance rows | P5B |
| Frame-rate-dependent behavior | Timing changes break animation, physics, replays, or saves | Preserve the source timebase, use deterministic replay checkpoints, and test long sessions | P2B/P3/P5B |
| Evidence drift | A result references an old executable, settings file, or capture | Put input hashes and exact commands in every card result; invalidate dependent rows after source changes | All gates |

The risk register is reviewed whenever the source revision, recompiler/runtime, graphics backend, or browser storage design changes.

Detailed repair owner/gate mapping is in `COMPLETION_REPAIR_PLAN.md`. Additional explicit risks:

| Risk | Trigger | Repair/qualification | Gate |
|---|---|---|---|
| Toolchain identity confusion | Commands/ABI from hedge-dev or ReXGlue used against the selected nimauria checkout | Freeze actual remote/commit/local patch hashes; qualify adapters before changing toolchain | A / Cards 01, 06, 07 |
| ReXGlue title compatibility unknown | The shared runtime accepts Xbox 360 code generally but has not accepted this exact TU2 image and DLC/content contract | Run an isolated exact-image Venom module qualification and record input format, manifest, plugin ABI, first milestone and failure class before migrating the native route | A / Cards 01, 06, 07 |
| ReXGlue experimental API/backend drift | ReXGlue is pre-1.0 and graphics, audio and input backends are injected and evolving | Pin SDK/submodules/plugins together, record Debug/Release pairing and hashes, and rerun the qualification after every dependency change | A / Cards 01, 07 |
| ReXGlue executable identity/codegen crash | Original binaries hit `0xC0000409` because `std::filesystem` followed a workspace reparse path; the isolated patched build now passes base-control analysis | Keep the patched build isolated and hash-pinned, run exact TU2 next, and keep runtime/first-frame qualification separate; track the remaining large-function file-size warning | A / Card 01 |
| Guest symbol shadows runtime import | A manifest name collides with a generated XAPI import thunk and silently routes calls into guest code | Run the manifest collision check before every link and fail generation on collisions | B / Card 06 |
| Generated interior routing / diagnostic interpreter dependence | Unsupported fallback trap, absent return continuation, or manual truncated range | Exact-image disassembly and routing/semantics tests; diagnostic fallback cannot close full static execution | B / Cards 06-08 |
| Incorrect resolved service | Export returns success but produces incorrect pointer/state | Validate contracts and actual call arguments/results, not just unresolved-export counts | C / Cards 07-09 |
| Load-time hang and ineffective cancellation | Host blocks before stop-signal polling | Bounded phase watchdog, structured evidence and controlled process-specific termination/recovery | C, G / Cards 08, 11 |
| Exact TU2 guest executes without reaching graphics startup | Exact-image run executes guest code but Vd calls, command-ring setup, and front-buffer publication remain absent | Continue bounded per-thread call-path/import tracing on the self-run Release host; inspect the exact guest startup path and service readiness before repairing a proven contract gap | C / Card 07 |
| Disc archive lookup or merged loose-file precedence is incorrect | Earlier Run37 could not find `D:\data\xenon\fe\main.big` or `D:\data\common\ai\`; the extracted image stores assets in five EB archives alongside loose files | Addressed: EB indexes are validated and exposed through a read-only merged overlay; real-content tests locate `eng_us.db` and the FE member, and Run117 guest opens FE main and AI directory successfully. Continue checking subsequent startup paths and preserve source archives read-only. | C / Card 07; verification [evidence](evidence/cards/07-archive-overlay-run116-run117-20261008.json) |
| Guest code ignores stop request during startup | Run118 proves the runtime-host's five-second stop watchdog calls `std::_Exit(0)` when guest execution remains active; process exit code 0 is not a guest-clean shutdown and `status.json` remains failed | Use bounded, evidence-preserving probe termination and label it accurately. Trace the guest threads' wait/worker state to determine whether stop semantics, wait services, or the startup loop prevent cooperative exit; do not treat watchdog exit as successful session cleanup. | C / Card 07 |
| Guest consumer dereferences a null queue item | Run49 faults on a null item after the consumer's end decrement, which does not prove pre-consumption corruption. Run50/51 captured wait result `-2` for object `0x83002B90` with infinite timeout; the consumer hook remained unreachable, global signal byte `0x83002B2C` was zero, and queue globals held pointers whose header fields are not yet decoded. | Instrument both pointed-to queue headers/cursors, every producer write and signal transition; trace the exact wait-helper syscall/import path and verify its exact TU2 contract; only then test a causal fix and repeat the probe. | C / Card 07 |
| WASM memory/link/table scale | Native guest aliases/addresses, Debug DLL size or registry layout do not fit browser | Compact guest-address translation, optimized complete-core build and measured resource budgets; no presumed memory64/single-thread cure | H / Cards 12-14 |
| Stereo advances simulation twice | Draw replay mutates guest state or eye rendering advances a game tick | Freeze same-tick state, trace mutations, separate per-eye resources/history and validate frame IDs | F / Card 10 |
| Streaming frontend mistaken for browser runtime | Existing web launcher connects to a native host rather than local WASM | Separate browser-native entry/build and prove local execution of full game | H / Cards 13-15 |


2026-10-06 Card 07 PARTIAL: focused suites 4/4 PASS; exact TU2 module binding/startup executed headless with fallback observed, then unsupported-instruction trap at 0x82297D58. Real graphics/audio, import coverage, complete gameplay, SBS and browser remain open. Evidence: docs/evidence/cards/07-native-bootstrap-20261006.json. No acceptance row is promoted by this probe.


## Execution drift controls — 2026-10-08

Detached runs without reliable exit evidence are INCONCLUSIVE, not crashes. Repeated broad probes without discriminating evidence must give way to caller/contract inspection. Historical traps/counts cannot drive current repairs without reproduction. Exit 0 and headless execution cannot close shutdown or visual acceptance. Apply [STARTUP_REPAIR_PROTOCOL.md](STARTUP_REPAIR_PROTOCOL.md); maintain one dated next action in STATUS. Run157 proves only its own cooperative stop, not general shutdown stability.

## 2026-10-13 qualification correction

Post-load Xenia import records are not valid codegen metadata. Require payload hash equality to the canonical pre-map TU2 image and validate import record types/ordinals before generation. Manual function sizes must preserve full reachable flow; zero unresolved diagnostics cannot pass truncated bodies. The prior qualification is revoked. See STATUS and input-identity-audit-20261013.json.

2026-10-13 vector-pack repair: two observed startup entries regenerated; exact emitted-pack regression 2000 checks PASS and original-bug negative control FAIL as expected; renderer capture39 clean intro frame, captures37/38 green. Full title/menu/gameplay/audio/SBS/browser acceptance remains OPEN. Evidence docs/evidence/cards/08-vector-pack-repair-20261013.json.

2026-10-13 Start-to-menu: 27 constructor/table-verified callback entries generated and registered; optional Start replay confirmed; readable autosave dialog captured. Main menu NOT PASSED. Probe42 later hit 0x82E1A030; use latest replay-loop result. Evidence docs/evidence/cards/08-start-to-autosave-20261013.json.

2026-10-13 reviewed leaf batch: 368 new/5 preserved, 373 generated semantics independently PASS; cycle29 runtime evidence retained, main-menu/full-game acceptance remains open. Evidence docs/evidence/cards/08-leaf-generated-20261013.json.

2026-10-13 post-autosave progression: EA Canada production frame independently verified; 373+9+75 scoped generated callback audits PASS and complete old-body/mapping preservation PASS. Main menu/full offline/SBS/browser gates remain OPEN. Evidence docs/evidence/cards/08-post-autosave-film-20261013.json.

2026-10-13 native tutorial: prison fight/HUD captured and user confirms punch input; cycle39 survived180seconds then supervisor closed it. Tutorial/menu completion, saves and entire offline/SBS/browser matrix remain open. Evidence docs/evidence/cards/08-native-tutorial-20261013.json.

2026-10-13 sustained tutorial rendering FAIL: fighters disappear before a later82DAFF88 callback crash. CauseUNKNOWN; single-variable synchronous-memexport comparison inprogress. Visible tutorial/user-confirmed punches do not close visual or stability acceptance. Evidence docs/evidence/cards/08-fighter-disappearance-20261013.json.


2026-10-14 recovery measurement: exp2 instruction repair has independent scoped PASS; scripted knockdown/recovery blocks unattended sustained-fight measurement. Button fixture removed after menu confound. Full native/SBS/browser acceptance stays OPEN. Evidence docs/evidence/cards/08-recovery-measurement-20261014.json.


2026-10-14 FMA integration: compiled portable single-rounding candidate; exact frozen-body/archive checks and prior numeric regressions PASS. Probe55 fails recovery; prompt detector false-negative under repair. No gameplay or final-goal gate promotion. Evidence docs/evidence/cards/08-fma-integration-20261014.json.


2026-10-14 recovery58: normal lateral input centers view; missing RS-up detector state identified from actual renderer frames. No get-up/gameplay gate passes. ABI copied-class negative control identifies separate64-bit value truncation; not adopted. Evidence docs/evidence/cards/08-recovery58-20261014.json.


2026-10-14 manual60: visible responding user-session native15 game opened, automaticinput/timeoutclose disabled. Separate native61ABIbuild preserves manual15pair; detector59 criticPASS but no recovery runtimepass. Window existence is not gameplay acceptance. Evidence docs/evidence/cards/08-manual60-20261014.json.


2026-10-14 handoff: native61 reviewed build/pair PASS, never launched. Manual native15 pause frame narrowPASS; subsequent exit0xC0000409 without automaticstop is unresolved, no long-session acceptance. Detector59 reviewed, not runtimepassed. Next admitted61probe/crashstack/save-content evidence. See docs/HANDOFF_2026-10-14.md.


2026-10-14 probe62: native61 first runtime, loaded matching pair verified; ordinary balance/RS-up recovery progresses into prison fight/cutscene/tutorial prompts. 100captures retained, independent all-frame review pending. Profile creation/enumeration partial; reload/fullXUIDtrace absent. Deadlinewindowclose exit0/no watchdog; hardexit lifecycle is not general shutdown acceptance. Manual60 finalfatal identifies missing82DAFF58; additive generated repair/native64 pending. Fullnative/offline/Rust/SBS/browser/release remainOPEN. Evidence: [probe62](evidence/cards/08-native61-probe62-20261014.json).


2026-10-14 profile65: preexistingprofileenumerates1item; realmainmenu,Tyson/Ali/arena selection,bout43–72 andpause73–110 retained. Primaryall-frame review only; livecontrollerinput makesautomaticcausalmeasurementinconclusive. Callback63 additivebody qualified; separatefixture66 contract/build/exportsPASS, runtimeunqualified. Fullfight/offline/Rust/SBS/browser/release remainOPEN. [Evidence](evidence/cards/08-profile-reload65-20261014.json).

### 2026-10-14 measurement and performance correction

Probe70 reached settings only; no fight or optimization improvement is established. Approximately30guest swaps/s and p9936.19ms refer to menu/settings cadence. Observer timing and partial-file readiness require repaired scripted setup before gameplay/performance comparisons. CSV lacks first-swap UTC and final hard-exit may lose buffered rows. See evidence/cards/08-scripted-profile70-20261014.json.


### Retry72 — 2026-10-14

Publisher recovered313actual file-sharing collisions, all input remained neutral. Bright PressStart detector false-negative prevented setup; no gameplay gate promoted. Eleven raw captures retained; loaded-module measurement failed, live identities UNKNOWN. Exact reviewed source freezing and prompt recognition repair required before fresh74. See evidence/cards/08-setup-retry72-20261014.json.


### Probe76 — 2026-10-14

Verified responsive round changes 10→9→8→7→6→5→4. Mapped OCR omitted gold 4; independent visual review confirms it. Both minima and gameplay remain unpassed. Frozen source and loaded pair identities match; close exits 0 without force, general shutdown remains unqualified. Actual glyph measurement repair precedes the next probe. See evidence/cards/08-setup76-20261014.json.
