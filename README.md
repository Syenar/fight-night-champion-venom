# Fight Night Champion Venom

Codename: **Venom**. This is the Xbox 360 full-game recompilation and browser SBS project.

This is the active project workspace for the Xbox 360 version of Fight Night Champion.

The target is complete standalone native recompilation and browser-native WASM/WebGPU execution, each supporting regular single-view and optional true left/right SBS. Prefer Bun. Xenia is reference tooling; WebRTC is optional and cannot complete the standalone browser goal. Follow the strategic plan, runbook and checklist in docs/.


The owned Xbox ISO, title updates, DLC, Xenia installations, Ghidra projects, extracted data, and generated recompilation output stay private in local research paths or their original locations. Do not copy them into releases.

Detailed end-to-end repair steps: [COMPLETION_REPAIR_PLAN.md](docs/COMPLETION_REPAIR_PLAN.md).

## Current progress — 2026-10-23

The decompilation work is active and incomplete. Reviewed raw capture batches are complete for the current selection, while semantic equivalence, exception handling, cumulative coverage, texture correctness, parallax, linked two-eye runtime, playable SBS, and standalone browser acceptance remain open.

Current gates: native/runtime and launcher contract tests pass; the full game is not yet a complete semantic decompile; no texture correctness, parallax, linked two-eye runtime, playable SBS, or standalone browser acceptance has passed. The repository contains source, scaffolding, and evidence only. The proprietary ISO, `default.xex`, TU2/DLC, Xenia/ReXGlue runtime inputs, and generated research builds remain local and are never downloaded or packaged by this repository.

## Install and run from this repository

This repository is source-only. It does not download, install, or contain the proprietary game image. A user must supply their own legally owned Xbox 360 ISO or extracted `default.xex`, the exact TU2 data, and any owned DLC through the private local research workflow. Keep those inputs outside version control and do not place them in a release archive.

For the native runtime contract build on Windows, install the pinned tools listed in `research/TOOLCHAIN_MANIFEST.json`, then run:

```powershell
Set-Location native/runtime
.\build.ps1
```

This builds and tests the runtime scaffold. It does not launch the complete game; a qualified TU2 title module, shared runtime, and user-owned inputs are still required.

For the browser signaling client, install Bun and run:

```powershell
Set-Location web/launcher
bun install
bun run start
```

Open `/host?session=localtest01` on the machine running the already-qualified Vulkan SBS Xenia host, select its window, then open `/?session=localtest01` in a browser. The launcher contains no game data and is a local signaling/display bridge; it is not standalone browser game execution.


## Repository contents

This repository contains engineering source, contracts and project documentation. Native research builds depend on locally generated files and user-owned game data, which remain outside version control. The browser interfaces are scaffolding; standalone WASM/WebGPU and true SBS acceptance are still in progress. See [current status](docs/STATUS.md) and the [execution checklist](docs/FULL_GAME_EXECUTION_CHECKLIST.md).




