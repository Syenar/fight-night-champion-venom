# Fight Night Champion Venom

Codename: **Venom**. This is the Xbox 360 full-game recompilation and browser SBS project.

This is the active project workspace for the Xbox 360 version of Fight Night Champion.

The target is complete standalone native recompilation and browser-native WASM/WebGPU execution, each supporting regular single-view and optional true left/right SBS. Prefer Bun. Xenia is reference tooling; WebRTC is optional and cannot complete the standalone browser goal. Follow the strategic plan, runbook and checklist in docs/.

The PS3 track is deprecated and is not an input, reference, compatibility target, or implementation base for this workspace. Its saved archive is at `..\archive\ps3-deprecated\Fight_Night_Champion_PS3_legacy_2026-10-01.zip`.

The owned Xbox ISO, title updates, DLC, Xenia installations, Ghidra projects, extracted data, and generated recompilation output stay private in local research paths or their original locations. Do not copy them into releases.

Detailed end-to-end repair steps: [COMPLETION_REPAIR_PLAN.md](docs/COMPLETION_REPAIR_PLAN.md).

## Current progress — 2026-10-23

The decompilation work is active and incomplete. The reviewed raw capture batches and their artifact audits are complete for the current selection, but semantic equivalence, exception-handling closure, and cumulative coverage admission remain open. The current renderer diagnostic source compiled and linked successfully; its isolated host package passed independent readiness review (`PASS_READY_FOR_BOUNDED_DIAGNOSTIC_NO_RUN`). The next experiment is a bounded custom-vs-stock texture diagnostic capture using the package recorded in `research/texture-sbs-bounded-run-readiness-2026-10-23-v2.json` (packet SHA-256 `D4F5B3AC37EF137EF1054370196896E8C09863FE79166CE8764153327EC7E7BF`; review SHA-256 `7E3AE84C6BBB9DE882B06EE2B34E0D8339184E9F61B6FBB5ADF5E075ECF57F24`).

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

The verified private Candidate607 diagnostic package is stored at `D:\Fight Night Champion Venom\xbox-full-game-sbs\research\active-work\sbs521-draw-binding-trace-candidate607-r1`. It is research-only, must stay outside releases, and does not establish true SBS or playable runtime acceptance.

## Repository contents

This repository contains engineering source, contracts and project documentation. Native research builds depend on locally generated files and user-owned game data, which remain outside version control. The browser interfaces are scaffolding; standalone WASM/WebGPU and true SBS acceptance are still in progress. See [current status](docs/STATUS.md) and the [execution checklist](docs/FULL_GAME_EXECUTION_CHECKLIST.md).


