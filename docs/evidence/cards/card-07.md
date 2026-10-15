# Card 07 — runtime scaffold

Status: IN PROGRESS.

The deterministic Card 06 output was audited before runtime work. It contains 186 generated files (183 C++ units), 48,847 indirect call sites, and 6,204 `Unrecognized instruction` warnings. The generated context still marks MMIO behavior as TODO and requires a host function mapping for indirect calls. These are native-runtime requirements, not evidence of a playable game.

The warning audit is reproducible with `research/analyze_generation_log.ts` and records 6,204 warnings across 45 opcode categories in `generation-warning-categories.json`. The first runtime scaffold is under `native/runtime/`. Its Rust crate implements lifecycle, guest tick accounting, writable-folder creation, and explicit contract reporting. Kernel, memory, timebase, threads, and filesystem contracts report `Ready`; input, audio, video, graphics, saves, and DLC deliberately report `Unsupported` until real backends are bound. The scaffold test passed with `cargo test` (1 test). The C++ companion headers/source and test are present for the generated C++ integration boundary; configuring CMake with the workspace Ninja, `C:\LLVM\bin\clang++.exe`, and Windows SDK `rc.exe` produced a passing native build and CTest run.

The repeatable `native/runtime/build.ps1` path now configures the pinned tools, builds the C++ boundary, runs CTest, and builds the Rust static library for the C ABI. The Rust runtime has deterministic local backends for input, audio, video, graphics, saves, and DLC, a Windows XInput polling adapter that loads `xinput1_4.dll` dynamically, and exported lifecycle/tick/SBS functions in `include/fnc_runtime_ffi.h`. Their status is recorded in `runtime-contracts.json`. These are contract implementations, not proof that the generated game calls them correctly; title-specific import mapping, real audio/video/GPU adapters, and generated-game execution remain open.

The recompiler was repaired to skip zero-filled `.pdata` slots and return from counted branches whose targets are outside the analyzed function, matching the analyzer's existing tail-branch treatment. A fresh generation produced 186 files, and all 183 generated C++ units now compile into `fnc_generated.lib`. `fnc_native_probe.exe` links against that library and reports the expected image constants. The probe does not execute the game entry point.

The first controlled entry probe reserves the full guest address space, mounts the prepared PE sections, installs 46,390 generated function pointers, and calls the generated `_xstart` symbol under an exception boundary. Reservation, mount, and lookup installation pass; `_xstart` raises an access violation before the title screen. The exact result is preserved in `CARD_07_BOOT_PROBE.json` and `CARD_08_BOOT_PROBE.log`; the next fix is the first missing kernel/heap/thread/import service, not another compiler change.
The follow-up probe commits a valid 128 KiB guest stack and passes a 32-bit guest r1, then returns after demand-committing two pages. This is only an entry-call contract test; it does not demonstrate process startup, imports, title-screen initialization, or playability. Card 08 therefore remains blocked on the XEX startup/import ABI.
The follow-up probe commits a valid 128 KiB guest stack and passes a 32-bit guest r1, then returns after demand-committing two pages. This is only an entry-call contract test; it does not demonstrate process startup, imports, title-screen initialization, or playability. Card 08 therefore remains blocked on the XEX startup/import ABI.

Repair cycle 09: generated a targeted XenonRecomp config containing all 30,102 previously unresolved direct-call targets as explicit 4-byte functions. The output contains 76,270 mappings and the unresolved-call scan is count=0; the expanded generated core compiled and linked. The entry probe still returns without a title, so startup ABI/runtime initialization remains blocked.

Repair cycle 09: generated a targeted XenonRecomp config containing all 30,102 previously unresolved direct-call targets as explicit 4-byte functions. The output contains 76,270 mappings and the unresolved-call scan is count=0; the expanded generated core compiled and linked. The entry probe still returns without a title, so startup ABI/runtime initialization remains blocked.

Repair cycle 10: matched Xenia's Processor::Execute stack/LR semantics and added a minimal big-endian PCR-to-TLS guest layout. The rebuilt probe passes setup but still returns before title initialization; no false boot claim was made.

## Exact TU2 guest capture repair cycle 14 — 2026-10-04

- Builder: added an opt-in research-only Xenia switch that writes the post-`FinishLoadingUserModule` guest image under `research/pivot/research/patched_guest/`. The isolated Vulkan run logged `XEX patch applied successfully: base version: 0.0.0.3, new version: 0.0.2.3` and wrote a 0x01360000-byte image at base `0x82000000`.
- Evidence: `research/pivot/TU2_PATCHED_GUEST_CAPTURE_20261004.json`; guest image SHA-256 `EC2C955DB9E55A6207A391A2AB80F60332DC41C3FE4C07968C7F07D5A9621CA4`; metadata reports title version `0.0.2.3`; source XEX/TU2 and Xenia executable hashes are recorded.
- Fresh Ghidra critic: a new project `research/ghidra/EC2C955DB9E55A6207A391A2AB80F60332DC41C3FE4C07968C7F07D5A9621CA4/FNCXboxTU2Guest3` imported and analyzed the captured image; metadata export reports 25,470 functions, 13 memory blocks, and 47,781 strings. PE-loader warnings/errors are preserved in the import log and do not invalidate the import result.
- Recompiler critic: XenonAnalyse accepts the captured file but emits the same empty jump-table sections. A copied all-targets XenonRecomp configuration stalled with zero translation output, so the raw loaded-memory image is not yet proven to be a valid XenonRecomp input. Native Card 07 remains blocked at input-format conversion/recompiler acceptance; no native completion claim is made.

## Exact TU2 recompilation repair cycles 15–18 — 2026-10-04

- Repair 15: raw guest bytes were wrapped as a flat ELF; XenonRecomp still produced no output. Critic found the wrapper lacked section semantics.
- Repair 16: reconstructed a section-preserving big-endian PowerPC ELF from the captured PE sections and restored executable `.text` flags. A controlled target test completed with 186 files; the full all-target configuration completed with 300 files and 209,555,542 bytes.
- Repair 17: compiled the exact generated set with MSVC; critic rejected the result because XenonRecomp output requires Clang/GNU extensions. No output was promoted.
- Repair 18: reran with pinned Clang 22.1.8. All 300 generated translation units compiled, `fnc_generated.lib` linked, and `fnc_native_probe.exe` linked. Probe evidence is `research/generated/EC2C955DB9E55A6207A391A2AB80F60332DC41C3FE4C07968C7F07D5A9621CA4/tu2-native-probe.log`.
- Critic result: exact TU2 image capture, fresh Ghidra analysis, XenonRecomp generation, and generated-core compilation/link are now PASS. Native runtime bootstrap is still open: the probe reports metadata/mapping only and does not boot the full game or title screen. The 6,237 unsupported-instruction warnings are preserved and require title/runtime review.

## Exact TU2 runtime metadata repair cycle 19 — 2026-10-04

- Fresh critic found the first exact-image build emitted invalid range constants (`PPC_IMAGE_BASE=0`, `PPC_CODE_BASE=0xFFFFFFFFFFFFFFFF`) because the reconstructed ELF section flags were not reflected in XenonRecomp's generated config.
- Builder corrected the private generated config from the captured PE map (`image_base=0x82000000`, `image_size=0x01360000`, `code_base=0x82290000`, `code_size=0xC545A0`) and rebuilt all 300 units with Clang.
- Critic reran the exact probe: it now reports the correct image/code ranges and mapping `0x82290070`. This is a valid generated-image ABI setup, but it still does not execute the game entry or provide title-screen evidence; runtime bootstrap remains the next gate.

## Exact TU2 boot-probe critic — 2026-10-04

- Corrected probe with the exact guest image mounts the PE and installs 75,636 generated mappings plus 302 import stubs. It commits two demand pages and returns from `0x82297338` without an exception or title initialization.
- Fresh source scan finds 256,572 `// ERROR` markers in the exact generated output, including unresolved calls from the entry stub such as `0x8229EAF0`; therefore the generated set is not yet a faithful executable runtime despite successful compilation/link.
- Status: native runtime bootstrap remains blocked on complete call-target/helper coverage and service implementation. The corrected metadata is retained; no playable-game claim is made.

### Cycles 20–21 — unresolved-target regeneration stalls

Cycle 20 extracted 256,572 `// ERROR` markers and attempted a complete config with 64,777 unique `.text` targets. XenonRecomp produced no output after more than 90 seconds and was terminated. Cycle 21 limited the config to 295 targets from the entry translation unit; it again produced zero output and was terminated. These are distinct evidence-backed failures. The critic rejected both approaches because the unresolved addresses are branch targets and not proven function boundaries. Native runtime remains blocked pending fresh-Ghidra boundary mapping.

Cycle 23 corrected the TOML separator before retrying two exact Ghidra function ranges. The recompiler again stalled with zero output after more than 90 seconds; the corrected syntax hypothesis is rejected. The native gate remains blocked by translator control-flow overlap/unsupported expansion.

### Cycle 24 — malformed repair config corrected

Fresh critic review found the prior repair TOML contained a literal `` `r`n `` token. Rebuilt with actual newline serialization. XenonRecomp completed successfully for two Ghidra-confirmed entry functions; the repaired output compiled/linked and the boot probe passed. This resolves the tooling stall but not the runtime gate: 253,974 unresolved markers remain and title initialization has not been demonstrated.

### Cycle 25 — instruction coverage expansion

The critic matched dominant warnings against the upstream XenonRecomp unsupported-opcode list. Added and rebuilt 18 instruction families, regenerated exact TU2 output, and verified a 74.2% warning reduction (6,237 to 1,605). The generated library and probe compile/link successfully; runtime title initialization remains unverified.

### Cycle 26 — scalar/update instruction pass

Added handlers for remaining high-frequency scalar/update instructions and rebuilt the toolchain. Exact TU2 regeneration, compilation, linking, and boot-probe validation pass. Warning count fell from 1,605 to 1,547; remaining failures are predominantly VMX/VMX128 semantics.

### Zero-warning exact-TU2 generation — 2026-10-04

Recompiled from the pinned exact TU2 image with the repaired XenonRecomp. Generation exits 0 with zero unsupported opcode warnings; 297 generated C++ units compile and link through Clang/NMake (esearch/t6bc-nmake). The native probe passes image/memory/import/stack/entry ABI checks. Critic scan still finds 253,974 // ERROR markers and tautological carry warnings, so runtime/title initialization and full-game acceptance remain blocked.

### Full Ghidra function-set recompile — 2026-10-04

A fresh function-range derivation from the exact TU2 Ghidra metadata generated 25,487 ranges. XenonRecomp completed with zero unsupported-instruction warnings. Three duplicate special-helper definitions were removed by excluding overlapping save/restore addresses; the regenerated output has zero duplicate definitions. Clang/NMake compiled and linked 71,137 mappings, and the exact-image probe passed. Critic scan still finds 183,007 unresolved control-flow markers and no title initialization, so Card 07 remains incomplete at runtime bootstrap.

### `mullhwu` repair and entry fault evidence — 2026-10-04

The critic caught a silent semantic omission: `mullhwu.` was translated to no code. The handler now multiplies the low unsigned halfwords and the full generated library was rebuilt. The probe advances through six demand commits before faulting at `0x82295D18`; this is a runtime blocker requiring real loader/thread/service state. 183,007 unresolved control-flow markers remain.

### 2026-10-04 seeded probe critic pass
- Rebuilt `research/t8bc-nmake` with exact `PPC_IMAGE_BASE=0x82000000`, image size `0x1360000`; build exit 0.
- Full generated set: 25,487 configured Ghidra functions, 71,137 definitions, 0 unsupported XenonRecomp warnings, 183,007 unresolved control-flow markers.
- Seeded probe log: `research/generated/EC2C955DB9E55A6207A391A2AB80F60332DC41C3FE4C07968C7F07D5A9621CA4/tu2-native-boot-probe-instructions7-seeded.log`; exit 0, `guest_entry_return=PASS`, six demand commits.
- Critic result: ABI entry is reproducible; full runtime/title initialization is still blocked by unresolved generated control flow and unimplemented platform services.
