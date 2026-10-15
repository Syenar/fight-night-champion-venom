# Fight Night Champion Venom

Codename: **Venom**. This is the Xbox 360 full-game recompilation and browser SBS project.

This is the active project workspace for the Xbox 360 version of Fight Night Champion.

The target is complete standalone native recompilation and browser-native WASM/WebGPU execution, each supporting regular single-view and optional true left/right SBS. Prefer Bun. Xenia is reference tooling; WebRTC is optional and cannot complete the standalone browser goal. Follow the strategic plan, runbook and checklist in docs/.

The PS3 track is deprecated and is not an input, reference, compatibility target, or implementation base for this workspace. Its saved archive is at `..\archive\ps3-deprecated\Fight_Night_Champion_PS3_legacy_2026-10-01.zip`.

The owned Xbox ISO, title updates, DLC, Xenia installations, Ghidra projects, extracted data, and generated recompilation output stay private in local research paths or their original locations. Do not copy them into releases.

Detailed end-to-end repair steps: [COMPLETION_REPAIR_PLAN.md](docs/COMPLETION_REPAIR_PLAN.md).

## Repository contents

This repository contains engineering source, contracts and project documentation. Native research builds depend on locally generated files and user-owned game data, which remain outside version control. The browser interfaces are scaffolding; standalone WASM/WebGPU and true SBS acceptance are still in progress. See [current status](docs/STATUS.md) and the [execution checklist](docs/FULL_GAME_EXECUTION_CHECKLIST.md).
