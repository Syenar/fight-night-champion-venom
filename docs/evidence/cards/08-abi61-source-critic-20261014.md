# ABI61 independent source and scaffold critic — 2026-10-14

Scoped static PASS for the reviewed GPR repair and separate frozen-source build scaffold. Native61 build/pair qualification remains pending; no game was launched, closed, or controlled by this review. Gameplay, full-game, SBS, and browser gates remain OPEN.

Actual `research/rexglue-qualification-20261012/toolchain/source-patched/include/rex/ppc/function.h` SHA256 is `3f92d5ac5246ae9c90e317366a8b0258233d31894b06201e11c187193b1f88b5`. Retained original SHA256 is `a132711cddf466c1945916260bcf41c3e5afb4fbca74a81b70bb01fb4cb274b6`. Independently diffing these complete headers confirms only r3–r10 full-width reads and explicit low-32-bit conversion for narrow scalar, mapped pointer, and raw guest pointer reads. Floating-point argument handling and stack handling are unchanged. The narrow conversion before the pointer null check correctly preserves a null low word even when the high word is nonzero.

Reviewed complete-header `integrated.cpp` uses actual PPCContext/GuestPtr and typed wrappers, including aggregate XUID input and a guest big-endian output handle. Retained original evidence reports eight wide failures, no narrow failures, and failed wrapper captures; candidate evidence reports 48 checks with zero failures and both captures/returns correct. These are retained compile/run results; the critic did not launch test executables during the concurrent build. Test source SHA256: `71cedcfbfdf3595bfa14e0089d9e689f781f50fac0d3420e88c2b5959d4788cf`.

Independently compared exact generated inventory and all hashes with `argtranslator-research61/freeze.json`: 435 files expected and actual, no extras, missing files, or changed contents. Canonical manifest hash matches the freeze. All four native15 EXE/DLL artifacts still match the freeze. The scaffold references the same canonical `src/main.cpp`, generated sources, PCH, metadata, and DLL targets. Its helper delta removes the codegen command and dependencies and redirects source-relative references to the canonical root; no alternate runtime implementation was introduced. Both native15/native61 caches retain Clang, `-mssse3`, RelWithDebInfo `-O2 -DNDEBUG`, and generated line-table debug information. SDK source is shared, while build/output directories are separate.

Remaining ABI limitation: both stack integer getter and setter transfer only 32 bits at `r1 + 0x54 + ((arg - 8) * 8)`. This repair does not fix wide stack arguments. A comment-stripped source scan matched 460 actual `REX_EXPORT` function definitions; no matched export has a wide integer scalar beyond the eighth argument. Fourteen matched exports do use later stack slots, all for narrow scalars or guest pointers. In particular, internal `xeXamContentCreate` has `u64 content_size` at index 8, but it is called directly in C++; exported `XamContentCreateEx_entry` receives that value at index 7/r10 and `XamContentCreateInternal_entry` at index 6/r9. The internal helper is not evidence of an exposed wide-stack wrapper defect. This source scan is not an exhaustive ABI proof for unknown future exports or title-specific signatures.

Build supervisor guard limitation: `Build61.py` verifies hashes of frozen members but does not compare full generated inventory, does not recheck the manifest, and only asserts the integrated header hash before the build. Current independently observed files pass all those additional checks. The supervisor alone could miss an extra generated file, manifest drift, or mid-build header drift; qualify the completed pair against fresh hashes rather than only its PASS string.

Last observed build state was `BUILD_RUNNING_NO_RUNTIME_LAUNCH`, with log progress 792/914. Post-build EXE/DLL hashes and SDK-to-output pairing have not yet been reviewed.

## Completed artifact review

Fresh post-build artifact review PASS. The preceding pending-build observation is historical. Actual build61 EXE/DLL inventory exactly matches `build-status.json`, and all three actual DLLs exactly match the compiler SDK output directory. Actual hashes:

| Artifact | SHA256 |
| --- | --- |
| venom_fnc_canonical_tu2.exe | 53481bac6c1889d61b104fd214e596e760fa14e83b623a4322bc8c00f5b4602f |
| rexruntimerd.dll | c5a644c838c75990b27c710f7567ecd29bd28e7b85af65e91ca25817f9177491 |
| rexgpu-xenosrd.dll | 64b57f53658e64873f2978d019b406475e8d5daf2b58043db1a8b33e03f410c7 |
| TracyClientrd.dll | 850e4cbe76c5729812160ad3e34774a039262e48f983c8ac70a611f9c98783c7 |

Independently rehashed the exact 435-file generated inventory, canonical manifest, reviewed header, and exact native15 EXE/DLL inventory after linking: all preserved. Actual build log records EXE linking and contains no codegen invocation. The retained supervisor reports exit 0 and `BUILD_PASSED_RUNTIME_UNQUALIFIED`.

Reviewed `Finalize61.py` SHA256 `8cc0764f62cc3155a8d91fc77ba56278ed7b958ea88df87555e83f238fd92521`, which matches the recorded final guard hash. This separate finalizer compares complete generated and old-pair inventories, manifest, and reviewed header after build completion, marks drift as failure, and returns nonzero for a failed build or guard. It addresses the earlier supervisor gaps for this completed build without modifying the already-running process. Current candidate pairing is also independently verified above; the finalizer itself checks preservation rather than the new DLL pair.

No candidate launch or manual-session interaction occurred. This is build/artifact qualification only, not runtime, saving, gameplay, wide-stack ABI, SBS, or browser acceptance.
