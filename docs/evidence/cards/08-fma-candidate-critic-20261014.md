# Card 08: independent FMA candidate critic — 2026-10-14

Reviewer: `callback_critic`, delegated independent source/artifact review. No builds, game runs or input injection were performed by this critic. The candidate is retained under `research/rexglue-qualification-20261012/fma-portability-research48/`; it was not installed in the active SDK at review time. The only critic write was this requested note.

## Scoped verdict

**PASS for the inspected candidate helper's selected arithmetic and alias contracts. Integration and gameplay acceptance remain OPEN.**

The helper computes madd(a,b,c) as one-rounding a*b+c. Its by-value vector parameters and complete loads before the final result store permit destination aliasing any source. nmsub computes fused a*b-c by flipping c's sign before madd, then flips the rounded result's sign. It preserves negative zero for exact cancellation; rewriting it as fma(-a,b,c) would not preserve that selected signed-zero result.

The active vector builder currently uses its decoded operands[1] and operands[2] as multiplicands and operands[3] as addend/subtrahend. The helper parameter names are mathematical roles, not a substitute for verifying architectural vmaddfp/vnmsubfp operand-field ordering in actual generated statements during integration. The active builder remained unfused at this review; candidate tests are not generated-opcode tests.

## Retained tests and independent reasoning

results.json records three successful variants, each with 2,064 checks and zero failures: baseline SSSE3/software std::fma, SSSE3 with FORCE_SOFTWARE, and AVX2/FMA hardware. The retained CPU probe reports v3 before the hardware test. This critic verified the current header/test hashes against results.json and inspected all test logic; it did not rerun executables.

The nonzero exact cancellation fixture is independently justified: (1+2^-23)*(1-2^-23)-1 = -2^-46, exactly representable using the retained bounded double calculation. The unfused negative control produces zero. The overflow fixture returns FLT_MAX for fused FLT_MAX*2-FLT_MAX while the unfused control overflows. These controls distinguish one-rounding arithmetic from merely renaming an intrinsic.

All 4^4 source/destination register arrangements are tested for both operations and all four lanes: 2,048 alias checks. The source values are small dyadic fractions, so their double product and sum/subtraction are exact before the oracle's single float conversion. This is a valid independent oracle for those selected values, not a universal double-based FMA oracle. The tests also cover selected signed zeros, infinity/invalid-operation classes, quiet NaN class, and three explicit guest flush examples.

## Exact artifact identities

Paths below are relative to the candidate directory. SHA-256 values were measured during this critic pass.

| Artifact | SHA-256 |
| --- | --- |
| PortableFma.h | 4F450A7B219D8102310EC05D9D16B1A293ED403D9833DA1FBCFF6D80DE53AB1C |
| Test.cpp | 1529B53179F9985711FE2C6180EDA73487B49F946AB8BDB2B68ADB14AA33F696 |
| RunTests.py | E33153796D82D5591020E77F89496E771F35C450F7CABC26792890FFFCABF2ED |
| results.json | 632CF7E616F7C8B45AA6BAA304BA71827C254A414FA59CDB8C23D363C6F0E917 |

## Gaps before integration

1. The retained FORCE_SOFTWARE variant is compiled with -mssse3, where the native-FMA branch is already unavailable. It is redundant with the ordinary software variant and does not exercise overriding an otherwise enabled native branch. Test -mavx2 -mfma together with FORCE_SOFTWARE before relying on that override. The preprocessor logic itself is correct on inspection.
2. Audit actual decoded/generated vmaddfp and vnmsubfp operand roles, including their 128 forms. Compare every changed instruction block and preserve all unrelated body statements and registrations. Execute focused regressions using actual emitted code with original unfused controls and source/destination aliases.
3. Establish the existing guest FPSCR/MXCSR contract first. The optional guest_* helpers explicitly flush input and output subnormals; that policy must not be assumed equivalent to current execution merely because three selected flush tests pass. Current tests do not explicitly vary rounding modes, FTZ/DAZ combinations, or exception status.
4. Quiet-NaN tests check class only. Signaling NaNs, payload selection/quieting, exception flags, comprehensive subnormal rounding boundaries, and directed rounding remain unverified. The core test's double oracle is sound only for the selected bounded dyadic values and exact cancellation, not every float triple.
5. Hardware selection is compile-time, not runtime dispatch. Preserve the existing SSSE3 baseline; a successful run on this v3 machine does not authorize silently raising deployment ISA requirements. Standard-library std::fma implementation and availability also need validation on intended targets.
6. The timing loop can be scalarized or have dead lanes optimized away. Its reported roughly 0.77 ms hardware and 3.1–3.3 ms software totals for one million loop iterations are helper microbenchmarks only; they do not predict full vector/game throughput or establish a performance budget.

No current get-up input experiment establishes FMA correctness or gameplay failure: an unsuccessful balance/recovery fixture can be a measurement/control limitation. Full native offline gameplay, TU2/DLC, sustained fighter rendering, normal/true-SBS display, browser-native execution and release gates remain **OPEN**.

## Follow-up: forced-software override coverage

The earlier gap1 is now closed for branch selection. Current RunTests.py adds a fourth variant with -mavx2 -mfma and VENOM_RESEARCH_FORCE_SOFTWARE_FMA together. Its retained result reports checks=2064, failures=0 and branch=software_std_fma on the v3 host. Header and Test.cpp hashes remain unchanged. Current results.json SHA-256 is ADB0C22169514E06DC6C41AD4456A7177897DABBA27DEAB9BDAAF807E9BF38F2; RunTests.py is 07EE437630255E6284B38117B257D8E47FC9B9C940DDC109CF8F804884D257C5.

This verifies the fallback source branch can be selected with native-FMA compilation enabled. It does not prove hardware instructions are absent: std::fma itself may use hardware under those compiler flags. The separate SSSE3 variant remains the baseline portability evidence. All other operand-generation, FPSCR/MXCSR, arithmetic-fidelity, throughput and gameplay limits above remain open.
