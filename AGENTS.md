# Fight Night Champion Venom — Xbox project instructions

**Current evidence correction (2026-10-15):** Actual132 independently verifies native Start-to-main-menu and menu-to-side-selection on the exact reviewed pair. The former corrupt-save setup failure is strongly tied to a270-character diagnostic profile path; the corrected258-character path progresses with the same seed/binaries. Actual134 subsequently creates a new profile/header and renders fighter introductions; reload, complete bouts, performance, SBS and browser remain unpassed. Reviewed140 repairs the concurrent snapshot read and launches successfully; actual140 then stops on a dim Press Start OCR false negative with no input/gameplay. Actual146 passes that stop depth and reaches a verified fight HUD; the next active frame exposes a clock OCR false negative. Actual152 then reaches HUD2:49 but fails a generic STAMINA mask contaminated by background pixels; prepare158 text-bounds repair. Timer156 entry telemetry and159 build are separate diagnostics; follow newest STATUS. Full bout, FPS and reload remain open. Historical unpassed-menu statements below are superseded by this actual132 evidence, without relaxing any lifecycle or acceptance requirement.

**Active execution correction (2026-10-13):** Follow [STARTUP_REPAIR_PROTOCOL.md](docs/STARTUP_REPAIR_PROTOCOL.md) for Card 07–08. It supersedes historical startup diagnoses, counts and next actions. Clean native intro, Press Start and autosave captures are observed; Start-to-main-menu remains unpassed. Read the newest numeric probe result before resuming. Require a falsifiable targeted hypothesis, retained process/exit evidence, causal repair and regression; no unchanged broad retries. Native full gameplay precedes SBS and standalone browser work.


Project codename: **Venom**

Start with `docs/AGENT_EXECUTION_RUNBOOK.md`.

Authority:

- `docs/FULL_GAME_BROWSER_SBS_PLAN.md`
- `docs/FULL_GAME_EXECUTION_CHECKLIST.md`
- `docs/STATUS.md`

Scope is Xbox 360 only. Do not read or use the deprecated PS3 tree in the parent project, including `3D Research`, `release`, `harness`, `artifacts`, `tools`, or PS3 screenshots. The PS3 ZIP is retained only as an archive.

Never modify the owned ISO, `default.xex`, title updates, DLC, or Xenia installations. Keep source dumps, Ghidra projects, generated proprietary code, and captures in local gitignored research paths. Do not commit or package them.

Run cards in order and do not promote a failed gate. Diagnose and repair it while continuing independent authorized work. Use docs/COMPLETION_REPAIR_PLAN.md for detailed repair steps and card dependencies. A compile is not runtime proof; each card needs hashes, logs, captures, replays, or other evidence.
