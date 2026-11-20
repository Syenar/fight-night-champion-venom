# Native Windows build/install front end

Extract the installer ZIP and open VenomInstaller.exe. Click **Browse**, choose the folder with your extracted Fight Night Champion files, and click **Install**. Setup looks for the exact retail game, TU2 update, and owned DLC inside or near that folder. The installation goes into **Venom Builds** beside the extracted installer. A verified previous install in the same builds folder can supply your saves automatically.

Simple setup prepares a private Python runtime and finds installed build tools. It downloads and verifies missing portable tools and the pinned public ReXGlue SDK source. The first setup can take a while. If Microsoft C++ Build Tools or the Windows SDK is missing, click **Get C++ Build Tools** and install the Desktop development with C++ workload and a Windows SDK, then return to setup. Keep the owned game, update, and DLC extracted locally.

Use **Advanced** to specify separate game, update or DLC folders, tools outside the normal locations, a custom output, an explicit previous installation, or a build to resume.

Review exact command shows the direct Python command before execution. Build and install asks for confirmation, then runs that command without a shell. The backend retains ownership of media hashes, source locks, overlap checks, generation, compilation, installation and preflight checks. The UI shows stage output and bounded tails of actual compiler log files. It reports the process exit code and requires the final receipt to exist before reporting command completion. Gameplay, DLC playability, save persistence, performance, SBS, browser and publication acceptance are separate gates.

The front end writes no registry or AppData settings. Remember inputs writes installer-ui-inputs.ini only inside the selected output after the backend creates pipeline-inputs.json. Load previous inputs reads that file from an output you select. Resume remains unchecked after loading; explicitly select it to pass --resume. The optional install-folder override remains inside output and supports the backend's fresh-install child selection. Existing incomplete stages remain preserved and refused by the backend. Keep the window open while a build runs. An installer-owned Windows job contains only its own build descendants; a monitoring failure or installer termination stops that owned job and preserves incomplete output. The UI never launches the installed game. Full logs and stage receipts remain below output, even when the display trims older text.

The installer itself uses the static Microsoft CRT and imports only Windows system DLLs; Python and the specified build tools are still required for the source build. It requests the current user's permissions (asInvoker), with no elevation request. Source media, updates, DLC, source-cache objects and existing installations are read only.

## Rebuild and package

From the extracted source root, use an explicit private output on D: or E: with sufficient space. No command below launches a GUI or builds the title.

```powershell
python -B native/installer/windows/build-installer.py `
  --output E:/Venom-QA/installer-compile-r1 `
  --clang-cl C:/LLVM23/bin/clang-cl.exe `
  --msvc 'C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.44.35207' `
  --windows-sdk 'C:/Program Files (x86)/Windows Kits/10' `
  --sdk-version 10.0.26100.0

python -B native/installer/windows/package-installer.py `
  --version v0.1.1 `
  --installer-build E:/Venom-QA/installer-compile-r1 `
  --output E:/Venom-QA/installer-packages-r1
```

The build script pins source/compiler hashes in its private receipt, runs 248 Windows argument-parser round trips, path/CLI checks and an owned-job cleanup check in a console-only test executable, and checks the GUI EXE imports for external CRT/framework dependencies. /Brepro and a zero PE timestamp make builds repeatable with identical sources, tools and libraries. The package script checks directories for reparse points before descending, then uses a bounded source allowlist, sorted inventory, fixed ZIP timestamps and verified per-file SHA-256 values. Running it twice against unchanged inputs produces byte-identical bundles with the same Python/zlib implementation. Use the GitHub release tag for --version. Both ZIP filenames, the package manifests and the packaging receipt record that version. The receipt also records final archive hashes. The source-only ZIP contains no executables. The installer ZIP adds only the independently built UI executable and its nonlocal identity record; no game binary, generated PPC code, SDK library/binary, capture, profile or user data is included.

This is a private review build. Compilation and static checks do not verify visible GUI behavior. Computer Use and GUI automation are prohibited for this work. Use automated native interfaces and command-line evidence for build/install acceptance; visible layout and folder-dialog behavior remain unverified unless separately observed. Publication must wait for the user's required full build, DLC and gameplay acceptance.

## Explicit app-owned native self-test

The opt-in native UI self-test is documented in [NATIVE_UI_SELF_TEST.md](NATIVE_UI_SELF_TEST.md). It uses only this program's own controls and the real pinned Python pipeline, with an explicit scenario/command authorization and private E: receipts. Source/console regressions are available; no GUI/backend run is claimed before the frozen scenario receives root review. [GUI_ACCEPTANCE.md](GUI_ACCEPTANCE.md) separates actual control/build-flow evidence from unverified human layout, text, keyboard and DPI behavior. No Computer Use or generic desktop automation is permitted or required.
