# Card 08: independent ArgTranslator ABI review — 2026-10-14

Status: **STATIC_DEFECT_CONFIRMED; runtime causality UNKNOWN; isolated regression DESIGN_ONLY**.

Reviewer: fresh leaf critic. This record preserves read-only source and retained-log findings. No runtime, game build, ABI patch, original-data mutation, or new acceptance claim was performed. The current candidate56 FMA-only experiment remains unchanged.

## Exact inputs and identity

All paths below are relative to the canonical workspace:

`C:\Users\samsa\Desktop\Workplace\Projects\Fight Night Champion Mods\xbox-full-game-sbs`

| Input | Identity |
| --- | --- |
| Isolated SDK source | `research/rexglue-qualification-20261012/toolchain/source-patched/`; Git HEAD `36d48086a3e3f9369cbff9d0c6ecb04fc48268fb`, locally modified qualification checkout |
| Current translator | `research/rexglue-qualification-20261012/toolchain/source-patched/include/rex/ppc/function.h`; SHA-256 `A132711CDDF466C1945916260BCF41C3E5AFB4FBCA74A81B70BB01FB4CB274B6` |
| FNCReborn downloaded patch | `research/rexglue-qualification-20261012/upstream-research/FNCReborn-7f68ee3c1e8d/patches/rexglue-sdk.patch`; recorded upstream commit `7f68ee3c1e8d9f68be05b672dfd5ede379fb5521`; SHA-256 `D7D357889A4B3FB5E0E753FAE5D0C132D163AF971503C8BF025FB917991339FA` |
| Primary local Xenia source | `research/xenia-source/`; Git HEAD `b083312b8b18e07e6e410b82104191f126722794` |
| Xenia shim source | `research/xenia-source/src/xenia/kernel/util/shim_utils.h`; SHA-256 `F6224D5FFF961F79F12570C2AF094788A92DB2B6E86B22315DD94E4013190EE0` |
| Retained execution observation | `research/rexglue-qualification-20261012/probe47-exp2-dense/runtime.2.log:37975-37977`; retrospectively inspected, not a new trace |

Git HEAD identifies the base checkout, not all local modifications or the exact executing binary. Header hashes above were rechecked while writing this record. No downloaded third-party script was executed.

## Demonstrated defect and affected contract

In the isolated SDK `include/rex/ppc/function.h:103-132`, `GetIntegerArgumentValue` returns `uint64_t` but obtains arguments 0-7 from `ctx.r3.u32` through `ctx.r10.u32`. The plain typed overload at lines 279-283 casts that already-truncated result to `u64`; upper bits cannot be recovered.

The actual export path uses this helper: `include/rex/hook.h:43-44,72-74` connects `REX_EXPORT` to `HostToGuestFunction`; `include/rex/ppc/function.h:380-383,406-410` extracts each typed argument using its ordinal before invoking the native export.

`src/kernel/xam/xam_content_aggregate.cpp:82` declares the first argument of `XamContentAggregateCreateEnumerator_entry` as `u64 xuid`. Lines 117-118 forward it to `ContentManager::ListContent`. `src/system/xam/content_manager.cpp:88-99,139-150` uses that XUID to select a 16-digit hexadecimal content directory.

The SDK profile initializes a nonzero-high-word XUID in `src/system/xam/user_profile.cpp:28`. Under the present translation, such a value deterministically becomes a different 32-bit XUID widened to 64 bits. Consequently aggregate save enumeration can search a different user directory than save creation, which obtains the full profile XUID internally.

The pinned FNCReborn patch at lines 65-146 changes register reads to `.u64` and explicitly narrows small typed values and guest pointers. Both parts matter. Changing only the register reads can change `bool` and null-pointer behavior when high GPR bits are nonzero but the low 32 bits are zero.

Primary Xenia evidence agrees: `shim_utils.h:63-71` reads full GPR values for 64-bit arguments; lines 139-147 perform typed conversion and width-specific stack loading; line 355 defines `qword_t` as `ParamBase<uint64_t>`. `src/xenia/kernel/xam/xam_content_aggregate.cc:80` declares the XUID argument as `qword_t`. One integer argument consumes one ordinal, including a 64-bit argument; this is not a pair-of-32-bit-register ABI.

## Limits and discriminator still missing

The retained probe47 log records export resolution and an aggregate enumerator with zero items. It does not record the guest GPR value, the translated XUID or the searched directory. Empty enumeration alone does not prove that truncation caused a current failure. A corresponding save may be absent, and other content/runtime contracts can also differ.

`XamUserGetXUID_entry` at `src/kernel/xam/xam_user.cpp:39-64` receives a mapped output pointer and writes a full XUID to guest memory. `XamUserReadProfileSettings_entry` at lines 287-293 and its Ex variant use mapped arrays of 64-bit values. `XamUserWriteProfileSettings_entry:304` receives profile-setting data through a pointer. Those pointee contents are not directly truncated by this by-value argument defect.

Other by-value `u64` exports are exposed, including notification masks, timestamps, tile IDs and content sizes. This record does not audit all of them or classify them as current blockers.

There is a separate stack-width mismatch: `function.h:129-132` always reads a 32-bit word for ordinal >=8, whereas Xenia performs a width-specific load. With a big-endian 64-bit stack argument this is the first word, not necessarily the low word. The FNCReborn register repair does not repair that path. Keep stack translation in a separate hypothesis and regression scope.

## Isolated actual-wrapper regression design

Prepare future test artifacts only beneath `research/rexglue-qualification-20261012/argtranslator-regression/`. Do not replace the active SDK header, modify game code, link the title, change save locations or mix this with candidate56. No test artifacts or binaries were created during this review.

Build the same small C++ harness twice in a later authorized ABI experiment:

1. **Original failing control:** include the exact hashed current SDK headers and instantiate the actual `rex::ppc::HostToGuestFunction<Capture>` template.
2. **Isolated candidate:** shadow only `rex/ppc/function.h` in a private include mirror with the scoped register-width and typed-narrowing changes. Use unchanged actual SDK headers for all other dependencies. Save both header hashes and exact include search order. Do not execute or apply the downloaded patch wholesale.

The harness should use typed capture callbacks rather than a rewritten translator. Capture arguments into a fixture without accessing kernel singleton state. A `void Capture8(u64, u64, u64, u64, u64, u64, u64, u64)` callback checks all eight GPR positions in one actual wrapper call. Include different nonzero high and low halves, a high-only value, and all-ones bits. Initialize the `PPCContext` explicitly and verify input GPRs remain unchanged for a void capture.

| Case | Required observation |
| --- | --- |
| Eight by-value `u64` arguments | Capture every original 64-bit value from r3-r10; original control must fail high-half preservation |
| Mixed `u32`, `u64`, `i32`, `u64` signature | Consume one GPR ordinal per integer argument; narrow u32 correctly and preserve both u64 arguments |
| Signed `i64`, `i32` and `be<u64>` values | Preserve 64-bit bits/sign and retain expected low-32-bit signed conversion |
| `bool` with only high GPR bits set | Capture false under the existing guest 32-bit small-value convention |
| Raw pointer and `mapped_u64` with high bits set, low 32 bits zero | Capture guest null without forming a host pointer from upper bits |
| Raw pointer and `mapped_u64` with high bits set, valid low guest offset | Resolve the fixture address using only the guest low 32 bits; mapped pointee read/write retains all 64 bits and big-endian representation |
| Return-value control | An actual wrapper returning u64 writes the full value to r3, independently of input extraction |

Use a synthetic guest memory fixture with sufficient backing for chosen low offsets; validate the actual `GuestPtr` mapping rules before allocating it. Keep fixture contents synthetic. Make an unexpected original-control pass or candidate failure a regression failure, not a reason to loosen expected values.

Add a second contract fixture only after the pure wrapper discriminator passes: create separate temporary content directories for two synthetic XUIDs with identical low halves but different high halves, and distinguish their content items. Invoke the actual aggregate export through its wrapper using isolated kernel/content state. Verify that each full XUID selects only its intended fixture and that the original control selects the wrong/empty directory. Exercise the common-content path separately so an unrelated common item cannot hide the failure.

Before a later title probe, retain bounded observations joining guest `r3.u64`, translated export XUID, content path and enumerator count. Predict a specific save enumeration or reload transition. Keep the current FMA-only experiment intact; complete its result before proposing a separate causal ABI change.

## Evidence acquisition and next action

Read-only commands used: `Get-Content`, focused `rg`, `Get-FileHash -Algorithm SHA256`, `git rev-parse HEAD` for both source checkouts, and `git diff -- include/rex/ppc/function.h`. The translator had no local diff at review time; other checkout modifications were not classified by this review.

Next action: preserve this as an independent queued ABI hypothesis; prepare and run the isolated actual-wrapper original-control/candidate regression after the active FMA-only experiment concludes. No startup, menu, save/reload, gameplay or release gate is closed by this source review.
