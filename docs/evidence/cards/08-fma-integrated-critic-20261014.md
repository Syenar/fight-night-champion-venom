# Card 08: independent integrated-FMA critic — 2026-10-14

Reviewer: `callback_critic`, independent delegated review before candidate launch. The critic inspected actual SDK/generated code and pinned baseline artifacts, reran focused preservation/numeric tests, verified candidate hashes and DLL pairing, and performed in-memory adversarial guard tests. No game was launched and no active source was edited. Tests rewrote their own local regression/evidence outputs; this note records the review.

## Verdict

**PASS for this integrated math-only candidate, its existing-body/mapping preservation and focused arithmetic regressions. Ready for a separately admitted bounded runtime experiment; gameplay acceptance remains OPEN.**

vector_fma.h evaluates all vector operands by value and performs fused a*b+c through native FMA only when already enabled by the compile target; otherwise each lane uses std::fma. vector_fms flips c's sign before that fused operation. Generated vnmsub retains its outer sign-bit XOR, including the selected signed-zero behavior. No blanket guest flush helper was installed. The caller's existing FPSCR operations remain unchanged.

Against the frozen SDK copies, vector.cpp changes only the two FMA builder expression templates; operands[1]/[2] remain multiplicands and operands[3] remains the addend/subtrahend. context.h changes only by including vector_fma.h. No ISA/graphics/input/game-state workaround was bundled into these reviewed diffs.

## Independent checks and reruns

The pinned integration-baseline-v2 JSON hashes to 5A698B04337174C815779390A6DCAE98F040060185251C243F34FF59F13D7B04. All 434 ZIP members independently matched its recorded file hashes; ZIP/current inventories matched exactly and had no duplicate members.

The critic reran the hardened AuditIntegratedDelta.py: **15,090 exact FMA substitutions, 69,649 bodies, 69,951 mappings, 211 changed translation units, zero metadata changes.** Every entire generated translation unit equals the frozen version after only the explicit FMA substitutions; non-FMA generated C++/headers are unchanged. Manifest and owned/canonical XEX identity checks pass. The initial ZIP-integrity gap found during review was repaired: the reusable audit now validates exact inventories and every frozen member hash.

TestIntegratedHelper.py was independently rerun against the actual installed helper and substituted emitted expressions. Both SSSE3/software and AVX2/FMA variants passed **33,536 checks**, with 40 exact-cancellation original-bug controls and 320 original/candidate differences. Tests cover the selected 17 actual opcode/alias shapes, all four host rounding modes, signed zeros, coherent exact dyadic/cancellation/overflow/halfway samples and every non-destination vector register. Hardware execution is justified only on the retained v3-capable test host. The source_sha256 field in integrated-helper-tests.json hashes the constructed LF-normalized text; the physical Windows file hash is listed below separately.

The critic reviewed and reran Audit-FmaPreservation.py and the DataLeaf audit's exp2-to-FMA chain. The original pre-exp2 baseline is preserved, the post-exp2/pre-FMA intermediate is pinned, and only exact original-to-final body hash pairs are granted to DataLeaf after both transformations are revalidated. DataLeaf reports **75 entries, old_changes=0, no failures**. The chain guards all 69,649 existing bodies and 69,951 mappings; it does not approve newly added callbacks automatically. The integrated helper's hash is pinned by the campaign guard.

Actual guard executions with **in-memory** altered reads rejected all four cases: frozen ZIP member corruption, existing-body corruption, existing-address mapping corruption, and helper corruption. pathlib/ZIP reads were temporarily wrapped inside a separate Python process and all guard writes suppressed; live artifacts were not changed. Exceptions propagate instead of granting exceptions based on a saved PASS.

## Exact reviewed identities

Paths are relative to `research/rexglue-qualification-20261012/`.

| Artifact | SHA-256 |
| --- | --- |
| toolchain/source-patched/include/rex/ppc/vector_fma.h | AD21AD746E28F8D2A6F5AD7866B307FB74A6BB2BC54D0B887D4520321FB3DE49 |
| toolchain/source-patched/include/rex/ppc/context.h | 4662216E7CFED2AFF046C7C88AFEE3CD3C95BB358E7A02EA93182D63EC6C6BB2 |
| toolchain/source-patched/src/codegen/builders/vector.cpp | B6A0D30C5AB6C921C66E8379EBDE99A0DBD3B5F75E567441DC77032A0FF6B7C7 |
| fma-portability-research48/integration-baseline-v2/generated-before.zip | 79D9F3472C7F87F2C6EA0CC2516E2E06BB4854119DB053BF62FE1A40A8DD31EC |
| fma-portability-research48/AuditIntegratedDelta.py | 87ED0B965F00B6322C740EF2882CCA11676475D2134C1E1E91FF10BC0E08F555 |
| Audit-FmaPreservation.py | B5BF2E9B468360DEB0632956E32F089083A893982C7A1F9689EE5F175FC8412E |
| Audit-Exp2Delta.py | 6E97F43C1B82E6ED7A350E8023E3B7C510B94894252330159E9CC3F5D9CD2F92 |
| Audit-DataLeafSemantics.py | D6C67121F426E5323F6FD3F1FE2920FD0B36764442D5FB97B020372EA0E06CE6 |
| fma-portability-research48/integrated-delta-audit.json | E74152920EAFE48B4ADDFB46616518E35473EE5D8223E14CFE371C614F75C99D |
| fma-portability-research48/IntegratedShapeProbe.cpp, physical bytes | 3BEBC639060C8D2AAC9F9D2E9505753F88806F6A6971E19F6D28CC3AE651BD07 |
| fma-portability-research48/integrated-helper-tests.json | DACD869C409F0197C08EFE54AD27A1DB502BCF878119C1DE6DC7CE61A8864B76 |
| fma-portability-research48/candidate-build-evidence.json | 94DA14968F21CBE5EE9E773F0E82CF9D1A967A0DBB0D8219AED1D50DD6B7CDE5 |

Candidate executable and runtime hashes were independently measured and match candidate-build-evidence.json. Every paired DLL also matches the SDK output directory:

| native-build15/RelWithDebInfo artifact | SHA-256 |
| --- | --- |
| venom_fnc_canonical_tu2.exe | C8CA4CE00D2081941F754AAE7AAC9004AED095BE4785E7154B6677B1728627C1 |
| rexruntimerd.dll | DD1D0CC5D3F24DCA920B7A73E108692A71CE9F1B6D24C3A9F26D413A7C72038E |
| rexgpu-xenosrd.dll | F6B45B040E22DC87545AAA2120DB73ED6360CA1369223D28E03BA5AFACA608B9 |
| TracyClientrd.dll | CDA27CFB99B9D5FD457F5B1E3430A4E1767C04DEFF1D3C82727A61B7AFE24306 |

## Limits

These selected tests and exact-delta checks do not prove comprehensive Xenon signaling-NaN/payload/exception/subnormal fidelity, actual dynamic FPSCR transitions, title hot-site coverage or full-game throughput. A successful build and paired DLLs are not execution evidence. Causal improvement of animation/fighter rendering requires the next controlled runtime comparison, with recovery detector/input measurement changes separately identified.

No full native offline, TU2/DLC, sustained fighting, normal/true-SBS, browser-native or release gate is promoted. All remain **OPEN**.
