# Fight Night Champion — Xbox 360 recompile and SBS work

This repository contains source code, browser interface, native runtime contracts, and documentation for an Xbox 360 Fight Night Champion recompilation project. The intended result is a locally run native game and a browser version with normal single-view output and true left/right side-by-side (SBS) rendering.

## Current state

Development is active and incomplete. The repository currently provides buildable runtime and launcher scaffolding. Full-game semantic decompilation, complete native gameplay, texture correctness, independent stereo rendering, parallax, and standalone browser acceptance still require additional work.

**Gameplay status:** The game has previously reached playable boxing in qualified local runs. The current launch path is temporarily unplayable while a shared startup failure is repaired. The project remains a playable-game effort; full-game acceptance, save/reload coverage, and a distributable build are still open.

## What you need

The repository does not contain or download the commercial game. To perform a private local run, you must supply game data that you own, including the Xbox 360 game image or extracted executable, the matching title update, and any required downloadable content. Keep those files outside version control and do not redistribute them.

## Build the native runtime contract

On Windows, install the tools listed in `research/TOOLCHAIN_MANIFEST.json`, then run:

```powershell
Set-Location native/runtime
.\build.ps1
```

This builds and checks the runtime contract. It does not launch the complete game by itself; a qualified game module and locally supplied game data are still required.

## Run the browser launcher

Install [Bun](https://bun.sh/), then run:

```powershell
Set-Location web/launcher
bun install
bun run start
```

The launcher serves local host and client pages for the current SBS transport prototype. It expects a separately running local game host and does not provide game data or standalone browser gameplay.

## Project guides

- [Current status](docs/STATUS.md)
- [Execution checklist](docs/FULL_GAME_EXECUTION_CHECKLIST.md)
- [Browser and SBS plan](docs/FULL_GAME_BROWSER_SBS_PLAN.md)

All proprietary game files, extracted assets, generated game code, and private runtime captures remain local and are excluded from releases.



