# Native TU2 title builder

This directory contains the source-side build recipe and Venom host integration for a private local TU2 title build. The [installer workflow](../installer/README.md) supplies the source-built SDK, verifies the exact generated materials, builds all 219 title objects, and creates the installed launcher. The separate `native/runtime/build.ps1` contract is development scaffolding.

Generated title code comes from a locally owned retail `default.xex` and matching TU2 `default.xexp`, using the pinned ReXGlue SDK and compiler versions in `title-build-profile.json`. Game inputs, generated PPC code, decoded assets, compiled binaries, profiles and logs stay in the caller's private output; they are not distributed in this source recipe.

The complete public source pipeline has passed a development-host build and installation. Final controller shutdown changes and their matching source locks are still being integrated for the installer release. An earlier successful build or a private incremental controller build does not qualify a later source package. Use the final package's receipt and hashes for installation and runtime claims.

The source lineage includes FNC Reborn by Whompay and the FNC Reborn TU2 reference. Attribution and retained licenses are recorded in [third-party notices](../installer/windows/THIRD_PARTY_NOTICES.md), [content-tool NOTICE](../content-tool/NOTICE) and the bundled APT helper notices. These notices do not grant rights to commercial game data or generated game code.

Use the [build/install guide](../installer/README.md) for the supported source workflow and prerequisites. Qualification used Python 3.11.15, LLVM 22.1.8 and 23.1.4, Visual Studio x64 MSVC libraries and a Windows SDK. A successful compile establishes build evidence; gameplay, persistent saves, usable DLC and controller shutdown need separate runtime verification.
