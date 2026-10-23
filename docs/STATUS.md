# Current project status

The Xbox 360 recompilation and browser SBS project is active and incomplete.

## Progress

- Reviewed raw decompilation batches for the current selection are complete.
- Native runtime and launcher contract checks pass.
- The current renderer diagnostic source compiled and linked.
- A fresh isolated diagnostic launch was attempted with a strict time bound. It exited with Windows status 0xC0000409 before producing a renderer trace.
- The failed launch is preserved as evidence. It does not establish gameplay, texture correctness, stereo rendering, parallax, or browser acceptance.

## Remaining gates

- Semantic equivalence and exception-handling coverage remain open.
- Full-game native execution, saves, reload, and complete offline feature coverage remain open.
- Custom-outfit texture identity and custom-versus-stock draw comparison remain open.
- True left/right SBS with independent eye resources, depth, and parallax remains open.
- Standalone WASM/WebGPU game execution remains open.

## Installation boundary

This repository contains source, runtime scaffolding, browser tooling, and documentation. It does not download, install, or distribute the proprietary game image, title updates, DLC, generated game code, or research captures. Those inputs must remain local and user-owned.

See the execution checklist and repair plan for the next engineering steps.