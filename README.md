# Fight Night Champion Venom

Venom is an Xbox 360 Fight Night Champion native recompilation project. You supply your own game, matching title update and DLC. Commercial game data and generated game code are not distributed here.

## Current state

Development is active. The native TU2 source-build pipeline has built the runtime/GPU SDK, content helper and complete 219-object title, then installed and launched the game. Private tests completed a Jack Johnson Bare Knuckles bout and an XP-pack save/cold-restart check. A separate controller candidate completed a Rocky Marciano versus Joe Louis bout, including the corner prompt and round-two transition. These are bounded test results, not certification of every fighter, controller or game mode.

The Windows installer builds the game locally from owned files. The preview reached the main menu on a fresh profile and completed a Bare Knuckles match as Jack Johnson against Lennox Lewis. The v0.1.3 update fixed invisible fighters in the opening Bishop–Frost bout and verified the fix after reloading a migrated Champion save. [Windows installer releases](https://github.com/Syenar/fight-night-champion-venom/releases)

Physical controller compatibility, texture bugs, photo import and true side-by-side 3D remain in development.

The earlier [November 17 created-boxer milestone](native/GITHUB_MILESTONE_20261117.md) records the user-confirmed LAST versus George Foreman result and its exact scope.

## Build and install the native game

1. Download the Windows installer ZIP and extract the full archive into a short path. Open VenomInstaller.exe. [Windows installer releases](https://github.com/Syenar/fight-night-champion-venom/releases)
2. Click **Browse** and choose the folder containing your extracted Xbox 360 game, matching TU2, and owned DLC, or their common parent. Click **Install**. Setup locates nearby inputs and prepares the required portable build tools.
3. If Setup asks for Microsoft C++ Build Tools, install the **Desktop development with C++** workload and a Windows SDK, then run Setup again.
4. Open the new output folder in **Venom Builds** and run its installed launcher using PowerShell 7. Keep the output folder because it contains the locally built game and its dependencies.

Setup can automatically copy profiles and saves from a verified previous install in the same **Venom Builds** folder. Use **Advanced** to select a previous install in a different location or specify the SDK and media paths manually. [Windows installer guide](native/installer/windows/README.md)

The source-only ZIP supports the manual command-line build, including the pinned SDK and LLVM versions. Keep game data and build output outside version control. [Native installer source workflow](native/installer/README.md)

## Other project components

The [runtime contract](native/runtime) is development scaffolding and does not launch the complete game by itself. The [browser launcher](web/launcher) is an SBS transport prototype; it does not provide standalone browser gameplay or game data. Browser and stereo work remain separate later milestones.

## Project guides

- [Installer source workflow](native/installer/README.md)
- [Windows installer front end](native/installer/windows/README.md)
- [Current engineering status](docs/STATUS.md)
- [Execution checklist](docs/FULL_GAME_EXECUTION_CHECKLIST.md)
- [Browser and SBS plan](docs/FULL_GAME_BROWSER_SBS_PLAN.md)

Proprietary files, extracted assets, generated title code and private runtime captures stay local and are excluded from releases.