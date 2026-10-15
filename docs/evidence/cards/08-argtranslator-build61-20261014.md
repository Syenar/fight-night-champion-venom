# Card 08: isolated GPR argument translation candidate61 — 2026-10-14

Status: **BUILD_PASSED_RUNTIME_UNQUALIFIED; fresh paired-artifact review required before any title run**.

Builder: leaf critic reassigned to a scoped builder after expanded candidate57 wrapper review. This is not an independent review of the builder's own final artifacts.

## Scope

Only `research/rexglue-qualification-20261012/toolchain/source-patched/include/rex/ppc/function.h` changed: eight GPR extraction reads use `.u64`; small plain values and both guest-pointer overloads explicitly retain low-32-bit narrowing. No stack-width, floating-point, content/filesystem, title-function, import-return or game-data changes.

Original header SHA-256: `A132711CDDF466C1945916260BCF41C3E5AFB4FBCA74A81B70BB01FB4CB274B6`.

Reviewed candidate header SHA-256: `3F92D5AC5246AE9C90E317366A8B0258233D31894B06201E11C187193B1F88B5`.

## Regression and retained controls

`research/rexglue-qualification-20261012/argtranslator-research61/` contains the complete original header, original native15 EXE/DLL pair, exact source diff, configure failures/repair, source fingerprints, tests, build supervisor and final exact-preservation guard.

Unlike candidate57's extracted-class fixture, `integrated.cpp` directly includes the actual complete SDK header and uses the real `PPCContext` and `GuestPtr`. A frozen-original include mirror supplies the failing control without changing the active SDK. Both compile and execute successfully:

- Original: 48 direct typed checks; eight expected u64 failures, zero small-type/null-pointer control failures; mixed and aggregate capture predicates false; wrapper return and nonnull mapped handle write controls true.
- Candidate: all 48 direct typed checks pass; actual mixed and aggregate-signature capture predicates true; return and mapped handle write controls true.

Both tests use the actual `HostToGuestFunction` implementation. The aggregate callback has the integer types/order of the SDK export; it does not execute the kernel/content manager/filesystem. Stack widths, be64 arguments, actual save enumeration and save/reload remain unqualified.

Exact commands and binary hashes: `integrated-original-evidence.json` and `integrated-evidence.json`.

## Build isolation and outcome

The canonical repair15 CMake helper attaches title codegen to compilation. A build61-only CMake scaffold references the exact existing repair15 main and generated sources and preserves SDK/runtime/settings while omitting only title-codegen dependencies. No canonical CMake/generated file was edited. Scaffold and explicit diff: `argtranslator-research61/scaffold/`.

Compiler: Clang 22.1.8; generator: Ninja Multi-Config; CXX flags: `-mssse3`; RelWithDebInfo flags: `-O2 -DNDEBUG`; parallelism: `-j 2`. Graphics/profiling settings match the frozen native15 cache. The first configure failed because RC was undiscovered; the retained retry supplied the same native15 RC, linker and archive-tool paths and passed.

Build command: `cmake --build native-build61 --config RelWithDebInfo --target venom_fnc_canonical_tu2 -j 2`. Retained child PID 11792; start `2026-10-14T20:33:10.873334+00:00`; end `2026-10-14T20:40:43.327866+00:00`; exit 0. The title was not launched.

The old user-controlled native15 session was never stopped or controlled. Native15's original EXE/DLL files were neither replaced nor copied into. Paired candidate DLLs were staged only into `native-build61/RelWithDebInfo/`.

| Candidate61 file | SHA-256 |
| --- | --- |
| `venom_fnc_canonical_tu2.exe` | `53481BAC6C1889D61B104FD214E596E760FA14E83B623A4322BC8C00F5B4602F` |
| `rexruntimerd.dll` | `C5A644C838C75990B27C710F7567ECD29BD28E7B85AF65E91CA25817F9177491` |
| `rexgpu-xenosrd.dll` | `64B57F53658E64873F2978D019B406475E8D5DAF2B58043DB1A8B33E03F410C7` |
| `TracyClientrd.dll` | `850E4CBE76C5729812160AD3E34774A039262E48F983C8AC70A611F9C98783C7` |

After link, `Finalize61.py` independently reran exact inventory/hash guards because the running supervisor had loaded its earlier guard before hardening. All passed: all 435 canonical generated files with no additions/removals/changes, canonical title manifest, reviewed candidate header, exact original native15 pair and retained main/FMA helper source fingerprints. Final result: `argtranslator-research61/build-status.json`; combined provenance: `progress-evidence.json`.

## Next action

Obtain a fresh paired-artifact critic before a future candidate61 probe. Preserve the original native15/manual-session files. A build and typed-wrapper test do not prove that actual save enumeration, profile behavior, startup or gameplay improved; any runtime causal claim needs the bounded XUID/path/enumerator discriminator described in the independent read-only review.
