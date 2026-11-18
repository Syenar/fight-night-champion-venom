# Fight Night Champion Venom

Venom is an Xbox 360 Fight Night Champion native recompilation project. You supply your own game, matching title update and DLC. Commercial game data and generated game code are not distributed here.

## Current state

Development is active. The native TU2 source-build pipeline has built the runtime/GPU SDK, content helper and complete 219-object title, then installed and launched the game. Private tests completed a Jack Johnson Bare Knuckles bout and an XP-pack save/cold-restart check. A separate controller candidate completed a Rocky Marciano versus Joe Louis bout, including the corner prompt and round-two transition. These are bounded test results, not certification of every fighter, controller or game mode.

The [Windows installer preview](https://github.com/Syenar/fight-night-champion-venom/releases/tag/v0.1.0-preview.1) builds the game locally from owned inputs. A clean build and install on the test PC produced all 219 title objects and installed 52 DLC files. The installed game reached the main menu on a fresh profile and completed a Bare Knuckles match as Jack Johnson against Lennox Lewis. The packaged installer also passed its automated form test using the real backend to install into a separate folder.

The preview covers the tested setup and a representative DLC bout. Physical controller compatibility and normal game shutdown still need live testing. Known texture issues, photo import and broader compatibility remain tracked. Side-by-side 3D follows the installer release.

The earlier [November 17 created-boxer milestone](native/GITHUB_MILESTONE_20261117.md) records the user-confirmed LAST versus George Foreman result and its exact scope.

## Build and install the native game

Start with the [native build/install guide](native/installer/README.md). The [Windows installer front end](native/installer/windows/README.md) runs that source workflow and keeps generated code, compiled game binaries, profiles and logs in your selected private output folder.

You need locally owned Xbox 360 game data, the matching TU2 and required DLC. The current source workflow also requires Python, Git, CMake, Ninja, Windows build libraries, the specified LLVM versions and a pinned SDK checkout with its submodules. Follow the guide's exact prerequisites. Attributed APT helpers are bundled as source; the workflow does not download the commercial game.

Prepare the SDK cache in a directory with enough free space. The following commands use the exact upstream commit from the installer lock.

```powershell
git clone https://github.com/rexglue/rexglue-sdk.git D:/BuildCache/rexglue-sdk
git -C D:/BuildCache/rexglue-sdk checkout --detach c94f5ebdcb3c9d1a460ca48e04f9758448f8d518
git -C D:/BuildCache/rexglue-sdk submodule update --init --recursive
```

Pass that checkout to the installer. Select your extracted retail game folder, a separate TU2 folder containing the matching `default.xexp`, the owned DLC title folder containing `00000002` and `Headers`, and a new private output folder. The front end displays the command before starting. When the build and install finish, launch `installed/Start-Installed-TU2.ps1` inside the selected output using PowerShell 7.

For a downloadable front end, use the Windows installer ZIP and extract the full archive. The source-only ZIP supports the command-line workflow. Both builds require the tools and owned inputs listed above.

Keep your source media and private output outside version control. Preserve the selected build output after installation: it contains the locally built runtime and dependencies used by the launcher.

## Other project components

The [runtime contract](native/runtime) is development scaffolding and does not launch the complete game by itself. The [browser launcher](web/launcher) is an SBS transport prototype; it does not provide standalone browser gameplay or game data. Browser and stereo work remain separate later milestones.

## Project guides

- [Installer source workflow](native/installer/README.md)
- [Windows installer front end](native/installer/windows/README.md)
- [Current engineering status](docs/STATUS.md)
- [Execution checklist](docs/FULL_GAME_EXECUTION_CHECKLIST.md)
- [Browser and SBS plan](docs/FULL_GAME_BROWSER_SBS_PLAN.md)

Proprietary files, extracted assets, generated title code and private runtime captures stay local and are excluded from releases.