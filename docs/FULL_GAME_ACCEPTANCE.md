# Full-game Xbox acceptance record

This file is the evidence index for the Xbox recompilation and browser SBS gates. It uses the same row names as `FULL_GAME_EXECUTION_CHECKLIST.md`. Each row must link to reference evidence, implementation evidence, input trace or replay, runtime log, and a PASS/PARTIAL/BLOCKED decision. A compiler result alone does not close a row. Online services are recorded as a separate compatibility boundary and are not silently counted as offline passes.

## Evidence location

Large captures, save data, and proprietary source material stay under local `research/` paths. This file links to their local evidence records; it must not embed or commit game data.

All required offline rows must pass on native and browser-native in normal and true SBS. Emulator/streaming evidence cannot close these requirements.

## Status summary

2026-10-15 actual151: integration137 reaches active Tyson/Ali boxing with independently reviewed HUD clocks2:59→2:50→2:43. The strict observer stops at the next HUD rejection before scripted punches; title/observer exit0. Complete bout, save reload, effective timer hook and displayed FPS remain open. [Evidence](evidence/cards/08-runtime151-recompile-fight-20261015.json).

2026-10-15 actual134: [settings, fighter intros and new profile/header observed](evidence/cards/08-runtime134-intro-20261015.json). This is partial save-creation evidence; title-side reload, verified fight HUD/completion and displayed FPS remain open. Runtime138 stops before title on a concurrent controller-file read; no gameplay result.

2026-10-15 actual132: [main menu and side selection independently observed](evidence/cards/08-runtime132-main-menu-20261015.json) after correcting the diagnostic save-path length and effective logging config. Publisher contention stops the script before a bout; save creation/reload and FPS remain open.

2026-10-15 diagnostic: [actual127 save denial](evidence/cards/08-runtime127-save-denial-20261015.json) reaches observed SETTINGS creation and PROFILE Retry but the new payload open returns access denied. New header alone does not pass save creation/reload. No FPS result; all gameplay/SBS/browser rows retain their existing open status.

| Row group | Native | Browser | Optional streaming (not completion) | Evidence record |
|---|---|---|---|---|
| Normal single-view display mode | PARTIAL | BLOCKED | BLOCKED | [native tutorial evidence](evidence/cards/08-native-tutorial-20261013.json); full row remains open |
| True left\|right SBS display mode | BLOCKED | BLOCKED | BLOCKED | pending Card 10/14/15 |
| Boot, language, profile, settings | PARTIAL | BLOCKED | BLOCKED | [native tutorial evidence](evidence/cards/08-native-tutorial-20261013.json); full row remains open |
| Exhibition and complete fight | BLOCKED | BLOCKED | BLOCKED | pending Card 04 |
| Champion Mode | PARTIAL | BLOCKED | BLOCKED | [native tutorial evidence](evidence/cards/08-native-tutorial-20261013.json); full row remains open |
| Legacy Mode | BLOCKED | BLOCKED | BLOCKED | pending Card 04 |
| Training and practice | BLOCKED | BLOCKED | BLOCKED | pending Card 04 |
| Replay and highlights | BLOCKED | BLOCKED | BLOCKED | pending Card 04 |
| All offline multiplayer modes | BLOCKED | BLOCKED | BLOCKED | pending Card 04 |
| Menus, tutorials, unlocks, statistics | PARTIAL | BLOCKED | BLOCKED | [native tutorial evidence](evidence/cards/08-native-tutorial-20261013.json); full row remains open |
| Fighters, arenas, crowds, commentary, music, sound | PARTIAL | BLOCKED | BLOCKED | [native tutorial evidence](evidence/cards/08-native-tutorial-20261013.json); full row remains open |
| DLC and title update | BLOCKED | BLOCKED | BLOCKED | pending Card 04 |
| Resolution, controls, vibration, accessibility, pause | PARTIAL | BLOCKED | BLOCKED | [native tutorial evidence](evidence/cards/08-native-tutorial-20261013.json); full row remains open |
| Cold starts and long sessions | PARTIAL | BLOCKED | BLOCKED | [native tutorial evidence](evidence/cards/08-native-tutorial-20261013.json); full row remains open |

`BLOCKED` means evidence has not been collected yet; it is not a claim that the feature is broken.

## Required rows

- [ ] Boot, language, profile, and settings
- [ ] Exhibition setup and complete fight
- [ ] Champion Mode: new game, cutscenes, progression, checkpoint, reload
- [ ] Legacy Mode: create boxer, training, progression, retirement/save
- [ ] Training and practice modes
- [ ] Replay and highlight systems
- [ ] All offline multiplayer modes
- [ ] Menus, tutorials, unlocks, statistics, and records
- [ ] Fighters, arenas, crowds, commentary, music, and sound effects
- [ ] DLC and title-update content
- [ ] Resolution, controller layouts, vibration, accessibility, and pause
- [ ] Frame Rate Limit in the game's own settings, reachable from both the main menu and paused gameplay without F4; live changes and saved/reloaded preference; application display VSync off; unchanged bout timing and measured active-play performance
- [ ] Repeated cold starts and long-session stability
- [ ] Normal single-view mode: complete game session on one composed view
- [ ] True left|right SBS mode: complete game session with measurable two-eye disparity

## Evidence record template

```text
Row:
Target backend: Xenia reference | native recompile | browser-native WASM/WebGPU | optional streaming
Display mode: normal single-view | true left|right SBS
Status: PASS | PARTIAL | BLOCKED
Reference capture:
Implementation capture:
Input trace/replay:
Runtime log:
Save/reload evidence:
Notes and known limitations:
```


2026-10-06 Card 07 PARTIAL: focused suites 4/4 PASS; exact TU2 module binding/startup executed headless with fallback observed, then unsupported-instruction trap at 0x82297D58. Real graphics/audio, import coverage, complete gameplay, SBS and browser remain open. Evidence: docs/evidence/cards/07-native-bootstrap-20261006.json. No acceptance row is promoted by this probe.

## User-confirmed native output — 2026-10-13

The user supplied three screenshots of the recompiled Venom window showing recognizable Fight Night Champion/EA Sports startup artwork and copyright text. **Game output is observed; correct rendering FAILS** because all captures contain severe green/black corruption. The user reports that it cannot proceed past the start screen. This replaces the previous uncertainty about whether any game pixels were produced; it does not pass correct-first-frame, menus or gameplay acceptance.

The exact interactive session exited `0xC0000409`, and its terminal log confirms unregistered indirect target `0x82B00028`. Repeated invalid texture-fetch constants are also recorded. **Next:** repair that exact generated entry and trace texture-constant producer/transfer/interpretation byte order against pinned Xenia, with a causal regression and repeat screenshots. The graphics cause is still a hypothesis; no global byte-swap or invalid-fetch bypass is approved as a proven fix.

Evidence: [native startup screenshot index](evidence/cards/08-native-start-screen-corrupt-20261013.json); original screenshots copied unchanged and hashed under `research/rexglue-qualification-20261012/user-start-screen-evidence-20261013/`. Native full-game, Rust, SBS, browser and release gates remain incomplete.


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


### 2026-10-14 probe70 correction

Scripted setup is incomplete: retained settings10rounds/2minutes, no fight. Guest swap CSV mean33.33ms/p9936.19ms is a menu/settings baseline only. Watcher OCR, pulse timing and freshness repair precedes a new bounded probe. No gameplay or performance optimization gate promoted. See evidence/cards/08-scripted-profile70-20261014.json.


### Retry72 — 2026-10-14

Publisher recovered313actual file-sharing collisions, all input remained neutral. Bright PressStart detector false-negative prevented setup; no gameplay gate promoted. Eleven raw captures retained; loaded-module measurement failed, live identities UNKNOWN. Exact reviewed source freezing and prompt recognition repair required before fresh74. See evidence/cards/08-setup-retry72-20261014.json.


### Probe76 — 2026-10-14

Verified responsive round changes 10→9→8→7→6→5→4. Mapped OCR omitted gold 4; independent visual review confirms it. Both minima and gameplay remain unpassed. Frozen source and loaded pair identities match; close exits 0 without force, general shutdown remains unqualified. Actual glyph measurement repair precedes the next probe. See evidence/cards/08-setup76-20261014.json.
