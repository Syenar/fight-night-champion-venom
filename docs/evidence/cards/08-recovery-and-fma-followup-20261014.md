# Card 08: recovery repair and actual-shape FMA follow-up — 2026-10-14

Reviewer: `callback_critic`, independent delegated read-only implementation/artifact review. No builds, game launches, input injection or active-source edits were performed. This evidence note is the only write for this pass.

## Recovery measurement

The strict four-field command helper closes the prior numeric-prefix issue: it requires expiry, positive sequence, LX and RY; consumes trailing whitespace; rejects trailing data, out-of-range axes, expired commands, negative current time and lifetime beyond 500 ms. Invalid commands yield both axes neutral. The retained test source contains 16 coherent valid/invalid cases. Atomic command-file replacement, 250 ms expiry, 100 ms refresh, newest-frame selection and stale-file rejection substantially improve measurement bounds. Defaults remain zero/empty and ordinary connected-user-0 input only.

**Remaining defects require correction before recovery failure is interpreted as gameplay evidence:**

1. Wrong-sized images return `(0,0,0)` from direction(), but the new watcher treats nonnegative left score as balanced recovery and requests RY=32767. Thus a rejected non-1280x720 frame triggers recovery rather than neutral. Use an explicit validity result or the rejected sentinel `(0,-1,-1)`.
2. Failed image decode continues without clearing desired axes. The heartbeat's freshness decision uses the newest file's modification time, not the last successful classification. Fresh corrupt/unreadable files can keep renewing an earlier balanced RY-up command without a fresh valid balanced frame. Neutralize decode failures and bound desired state by successful-classification time.
3. Packet tracking fixes single-thread corrections/releases, but exchange(previous axes), fetch_add(delta), and load(delta) are separate operations. Concurrent GetState calls with different axes can both return the same loaded delta after their updates. A synchronized axes/packet assignment is needed if concurrent polls are possible. This is a code-level interleaving risk; this critic has not established that it occurred in probe54.

The detector remains a fixed-box/color heuristic, and actual title consumption/recovery success remains unverified. Completion of probe53 and sequential execution of probe54 do not cure these measurement defects or pass gameplay.

## FMA actual-shape candidate

**PASS for the inspected 17 extracted opcode/alias substitutions and retained focused tests. Ready for an isolated SSSE3-preserving integration experiment, not gameplay or complete Xenon semantic acceptance.**

The critic independently checked all 17 source locations, opcode comments, original executable statements, statement hashes and recorded flush-prefix lines against current generated files. All match. The original/candidate functions in ActualShapeProbe.cpp also exactly match the recorded substitutions. No active FMA generator installation was present at review time.

Retained SSSE3/software and AVX2/FMA runs each report 33,536 checks, zero failures, 40 exact-cancellation negative controls and 320 original/candidate differences. The fixture uses all four host rounding modes with FTZ/DAZ enabled, coherent source assignments, original unfused controls, bitwise signed-zero comparison and preservation checks for every non-destination vector register. Its bounded double oracle is exact for the selected dyadic/cancellation/overflow/halfway samples; it is not a universal FMA oracle.

Integration should preserve existing decoded multiplicand/addend roles and ctx.fpscr calls, keep the SSSE3 target, scope the builder change to FMA emission, inspect every emitted changed block, and retain full unrelated-body/registration preservation plus original-bug controls. These shape tests do not execute actual FPSCR state transitions, cover every NaN payload/exception/subnormal edge, demonstrate dynamic hot-site coverage or establish game throughput. Do not install the optional blanket guest_* flush wrappers merely because host-MXCSR fixtures passed.

## Artifact identities

Paths are relative to `research/rexglue-qualification-20261012/`; hashes identify the reviewed versions.

| Artifact | SHA-256 |
| --- | --- |
| toolchain/source-patched/include/rex/input/research_recovery_command.h | 947DECD661B99B834B2792DB821E2157777AA55BE6ED921AAEB792D6286EB936 |
| RecoveryCommandTest54.cpp | 99FA3ECB5B33DA3C355BFED0D13647E2481F4C7F32A0DF1C948EBA786FC4FC3E |
| Recovery-Vision52.py | 2E66EB2E09D8263F3338C69F8F621C2D07D703AE409E6A21ACC0BE18808E68E7 |
| toolchain/source-patched/src/input/input_system.cpp | CECD749E5A1CED2A60F6B01B3B5F05BBD8C0F8CB4118835CD7F9ED977D2BDAB4 |
| fma-portability-research48/ActualShapeProbe.cpp | E426AD74E7C2260648CE9A158FCADDE0857C8A32B17FEC3FC310342B0D58E2B7 |
| fma-portability-research48/actual-shape-substitutions.json | 08C482BB008AFEA89C8580F63EBAB98350F21594771ABFE17BD4CB223CDE39F5 |
| fma-portability-research48/actual-shapes-evidence.json | C47E1A3E1F7EAECFE8A978AB7D82BF31EA1D81089CD4E61056922E6916BE9906 |

Full native offline gameplay, sustained fighter rendering, TU2/DLC, normal/true-SBS display, browser-native execution and release gates remain **OPEN**.

## Recovery repair reinspection: cited source defects resolved

The later inspected versions resolve all three concrete findings above. Wrong-size images now return the rejected `(0,-1,-1)` sentinel. Decode failures clear desired axes and last_valid_at; heartbeat renewal now expires by the last successfully classified frame's time. A mutex owns complete returned-gamepad comparison and stable packet assignment, replacing the separate axes exchange/counter increment/counter load. Each changed complete gamepad state receives an incremented packet, while unchanged state retains its packet. This closes the identified concurrent-assignment interleaving in the private recovery fixture.

The classifier additionally requires the LS icon's warm ring, reducing the observed GAME OVER false positives. This critic independently executed only `python research/rexglue-qualification-20261012/Recovery-Vision52.py test`, which exits before watcher creation or command writes. Exit was 0 across six assertions: right prompt from probe49, left prompt from probe51, gameover from probe47, wrong-size neutral image, and actual gameover probe54 frames 32/90. Positive scores were right 145/left 0 for probe49 and left 503/right 22 for probe51; rejected probe47 returned `(0,-1,-1)`. No game or live input loop was run by this critic.

The strict parser helper remains unchanged and coherent for the retained 16 cases. Defaults, numeric bounds, atomic replacement and short expiry remain as reviewed. **Scoped PASS for these concrete recovery-source repairs and classifier controls; no new blocking source defect found in the reinspection.**

Latest reviewed SHA-256 values:

| Artifact | SHA-256 |
| --- | --- |
| Recovery-Vision52.py | 59BBFD2CF4772EF807980ABE46F1577AC3B19DCC4B67268B9E5B7D3583CBD6A3 |
| toolchain/source-patched/src/input/input_system.cpp | 4FCD1546370FE3A922B84110A85E95BBD820C6C8491B29F473A2986435E0EA93 |
| toolchain/source-patched/include/rex/input/research_recovery_command.h | 947DECD661B99B834B2792DB821E2157777AA55BE6ED921AAEB792D6286EB936 |

Remaining open limits: six controls do not establish classifier reliability across every scene/transition; one-second captures and 150 ms pulses do not establish control-loop sufficiency; packet/axis emission does not prove title consumption; and recovery success/sustained fighting need new retained runtime evidence. The earlier nominal capture-bound/manifest-discriminator discrepancy also remains a documentation limit unless corrected in the next probe's actual manifest. The simultaneously regenerating FMA snapshot was deliberately not graded by this reinspection. No gameplay, visual, SBS or browser gate is promoted.
