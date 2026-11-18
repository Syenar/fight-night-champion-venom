# Build and install from owned TU2 inputs

`Build-Install-Venom.py` builds the pinned SDK, builds the attributed content helper, qualifies the exact TU2 generator output, builds all 219 title objects, and installs a local launcher. It finishes with installed input checks and does not launch the game. Generated sources, objects, binaries, decoded assets, logs, saves, support tools, and receipts stay below the caller's private output directory.

Use Windows, Python, Git, CMake, Ninja, the Windows SDK/MSVC libraries, LLVM 23.1.4, and LLVM 22.1.8. The qualification used Python 3.11.15. Supply a local SDK Git checkout containing the locked upstream revision and submodule objects. Its working files are never copied. Missing pinned objects stop the build; this command does not fetch over the network.

```powershell
python -B native/installer/Build-Install-Venom.py `
  --sdk-checkout C:/BuildCache/rexglue-sdk `
  --game-data-root D:/Owned/FightNightChampion `
  --update-data-root D:/Owned/FightNightChampion-TU2 `
  --dlc-title-root D:/Owned/45410915 `
  --output D:/PrivateVenomBuild/build-r1 `
  --clang23 C:/LLVM23/bin/clang++.exe `
  --clang22 C:/LLVM22/bin/clang++.exe `
  --jobs 2
```

The game root must satisfy the launcher's exact retail, shader, XP archive, and Legends speech hashes. The distinct update root must always contain the exact TU2 default.xexp. A sibling default.xexp in the game root is optional; if present, it must match the same exact TU2 hash. A mismatched sibling is rejected because the runtime searches it before the separate update mount. The DLC root contains 00000002 and Headers for title 45410915; all 52 manifest files are checked before copying. All owned inputs are read only. Use a fresh private output outside every input tree; the installation defaults to output/installed and must remain inside output.

APT tools are bundled by default. Their source lock binds all four helpers: aptpatch_display.py, ebbig.py, refpack_enc.py, and refpack.py. The optional --apt-tools-root override must supply exactly the same locked source bytes. The driver stages the helpers, LICENSE, NOTICE, and source lock below output/apt-tools. The installed launcher uses that private copy, so it does not depend on the source checkout or an external helper snapshot for XP generation.

The stages run sequentially, with at most the requested compiler job count within each build. The SDK's CLI/GPU imports and all 431 expected generator materials must match before fresh SDK artifact hashes are bound into a private title recipe. The tracked base profile stays unchanged. The title's 219 objects, executed link response, and bundled DLL imports are checked. The normal installer then generates the exact XP and decoded speech overlays and imports the caller's DLC into its user directory. Source media and existing data outside the output remain unchanged.

The installed launcher verifies the new title and helper build receipts and pins their hashes in venom-install.json. It preserves the existing 60 Hz/full-readback defaults; XInput is the controller default and SDL is opt-in with an optional caller mapping. These checks establish build/input identity. Gameplay, save persistence, DLC playability, performance, SBS, browser, shutdown, and release acceptance remain separate gates.

A successful command writes build-install-receipt.json with child receipt hashes, the exact title hash, the installed launcher path, and explicit gameLaunched/runtimeTested false fields. Logs and child command arguments remain in output/logs and output/commands.json. The executedPhases, freshSdkBuild, freshTitleBuild, and reusedCompletedBuildStages fields distinguish a fresh build from reused qualified artifacts.

On a fresh user profile, the first launch can open the Champion Mode prison bout. To reach the main menu immediately, pause, select **Throw in the Towel**, choose **Throw in the Towel** in the confirmation, and confirm. For a quick DLC check, open **Game Modes** and look for **Bare Knuckles Mode**.

Use the same command with --resume to revalidate completed stages. Incomplete folders without receipts are preserved and refused. A content-identical locked APT source directory and a fresh --install-root child may be rebound without rebuilding the SDK/title. Such changes retain the prior input/receipt snapshots and an explicit input-rebinding receipt; compile and media identities must remain unchanged. Existing installations are preserved rather than rewritten.

The initial qualification executed the complete public SDK/helper/title build and normal installer with an explicit APT source directory. The bundled helpers then byte-matched all four tested files and reproduced the exact XP overlay hash. A subsequent normal resume reused the qualified compiled artifacts and created a fresh installation using output-staged bundled tools. The final readable driver is checked by AST equivalence and a further qualified resume/preflight. No game was launched by these checks, and no compilation is claimed for a resume whose fresh-build fields are false.

Only source and non-proprietary manifests belong in these recipes. Game media, generated title sources, binaries, decoded data, and captures must not be committed or bundled with them. See the sibling content-tool attribution and SDK diagnostic limitations.