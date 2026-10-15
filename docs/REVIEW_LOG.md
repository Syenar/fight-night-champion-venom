# Documentation review log

Review date: 2026-10-01

Scope: the active `xbox-full-game-sbs` workspace and the parent documents that could route an agent into the Xbox or deprecated PS3 tracks.

## Pass 1 findings and fixes

- The Card 00 command used an ISO filename that is not present. It now discovers exactly one `Fight Night Champion*.iso` beside the clean workspace and stops on ambiguity.
- Card 09 used a path that failed from the documented workspace root. It now points to `docs/FULL_GAME_EXECUTION_CHECKLIST.md`.
- The plan listed missing `FULL_GAME_RISKS.md` and evidence directories. A risk register and evidence-record rules were added.
- Source, reference, and toolchain manifests were mentioned inconsistently. Their ownership and paths are now explicit.
- The plan treated a framework listing as stronger proof than it is. Toolchain verification now requires a build, exact commits, and a title-specific result before P1 passes.
- The recompiler failure path implied that an emulator/WebRTC route was automatically complete. The documents now require full acceptance and SBS proof for that route.
- The acceptance rows and execution checklist used slightly different names. The offline multiplayer row is now aligned.
- Parent duplicate plans and the old mixed-route documents were archived under `archive/superseded-xbox-plans/` and removed from the active parent tree.
- Root README and agent instructions now route directly to the Xbox workspace. Deprecated PS3 instructions remain outside the active route.

## Pass 2 critic checks

- 13 active workspace Markdown files plus the two root routing files were scanned.
- Local Markdown links: 0 broken.
- Required workspace directories and documents: present.
- Runbook cards: contiguous Card 00 through Card 16.
- Stale parent `docs/full-game` directory: absent.
- Stale ISO filename in the runbook: absent.
- Parent-plan archive: ZIP stream checked and SHA-256 recorded.
- Exactly one Xbox ISO is present beside the clean workspace.

The active document set passes the structural review. Future implementation work must update the acceptance matrix and card evidence as soon as a gate is exercised; a green compile alone cannot close a row.

## Pass 3 findings and fixes — 2026-10-12

- Compared the active workflow with current Xbox 360 recompilation projects using XenonRecomp/ReXGlue and Xenia-derived runtimes.
- Corrected the strategic plan, runbook, checklist, risks, completion repair plan, status record, and methodology audit so ReXGlue shared-runtime qualification is the primary native bootstrap candidate.
- Separated generator qualification, shared-runtime qualification, first-frame boot, complete offline acceptance, Rust migration, SBS, and browser-native execution into distinct gates.
- Added the Venom manifest requirements: manual boundaries, gap seeds, invalid/data ranges, special control flow, imports, TU2/DLC identities, runtime overrides, and deterministic generation evidence.
- Classified the existing custom host as diagnostic comparison evidence and retained the historical ReXGlue attempt as unqualified evidence; no historical result was promoted or deleted.
- Added explicit safeguards against treating generated C++ units, zero unresolved symbols, successful linking, or emulator output as proof of a complete native recompile.

Critic result: the governing documents now agree on the shared-runtime/module ownership model and the next qualification gate. Fight Night/ReXGlue compatibility remains unproven and is intentionally left unchecked.

## Pass 4 findings and fixes — 2026-10-12

- Rechecked the workflow against the ReXGlue documentation, including its experimental status, runtime setup order, injected input/audio/graphics backends, function dispatcher, and manual-boundary requirements.
- Corrected wording that incorrectly treated graphics, audio, and input as wholly owned by the shared runtime; the plans now require injected backend/plugin identity and Debug/Release pairing.
- Added runtime initialization-order checks, generated guest-symbol/XAPI collision checks, matching plugin hashes, and a hard pre-link collision gate.
- Confirmed the old custom-host path remains diagnostic and that the 2026-10-08 ReXGlue attempt remains unqualified rather than being promoted.

Critic result: the active documents now match the researched ReXGlue architecture and retain an explicit exact-TU2 compatibility gate. No native, SBS, browser, or release gate was promoted by this review.
- 2026-10-02 Card 07 critic cycle 07: independently reran the native probe after fixing `/MAP` linker syntax and PPC guest-pointer semantics. Verified 183 generated C++ units remain compiler-clean, 302 XEX import function thunks are cataloged, runtime CTest remains 1/1, and the probe returns only after demand-committing guest pages. Review result: Card 07 IN_PROGRESS; Card 08 BLOCKED because no title screen or process startup occurs.
- 2026-10-02 Card 07 critic cycle 08: independent scan of all generated translation units recorded `unresolved-direct-calls.json` with 30,102 distinct unresolved direct-call targets. This invalidates any claim that the generated core is bootable; Card 08 remains blocked until those targets are resolved or proven unreachable.

- 2026-10-02 Card 05 critic cycle 03: fresh headless Ghidra run after clearing stale compiled-script cache executed ExportFncMetadata.java successfully; output contains 25,170 functions and 2,394,723 bytes. Metadata subgate PASS.
- 2026-10-02 Card 07 critic cycle 09: fresh scan verified the targeted config includes all 30,102 prior unresolved direct-call addresses, expanded XenonRecomp output contains 300 C++ units and 76,270 mappings, unresolved scan count is 0, and CMake/Ninja links the native probe. Boot probe still returns without title; startup ABI/runtime services remain unproven.

- 2026-10-02 Card 07 critic cycle 10: independently checked the Xenia-shaped stack reserve, LR, PCR, and TLS initialization in the probe source and reran the binary. The probe remains a harness PASS only; no title screen or real game boot. Card 08 remains blocked.

## 2026-10-02 — Card 07 online research / boot-contract diagnosis

Reviewed official Xenia kernel, thread, processor, thread-state, XEX-module, and kernel-module sources; official XenonRecomp documentation; and the Saints Row community reimplementation milestone list. The sources agree that generated C++ plus address mapping is only the translation stage. Xenia creates a process/module record, kernel export state, XThread, TLS/PCR, stack, and real xboxkrnl/xam services before the entry point can behave like a game. The native probe currently passes mapping, PE mount, stack/PCR scaffolding, import-slot installation, and zero-unresolved scanning, but does not implement those launch services. See `research/CARD_07_XENIA_BOOT_CONTRACT.md` and `research/generated/90A698C71AA714E383CE092750CE767E07145F1CA38D8A9C9B3BD8005EA29FF6/CARD_07_BOOT_PROBE.json`. No boot or completion claim is made.

### Card 07 repair cycle 11 — Title Update entry mismatch

Compared the active Xenia Netplay log with the frozen source manifest. Xenia logged successful Title Update 2 application and reported entry `0x82297338`, image size `0x01360000`; the native image under test came from the unpatched retail XEX with entry `0x82297370`. This is a real source-revision blocker. The required next gate is verified patch application, patched-image hashing, and regeneration from that patched image. The diagnostic hash-bypass attempt was discarded and the patch tool was restored to strict hash validation.

## Review 2026-10-02 — Card 07 cycle 12

Builder/critic result: the executable-revision gate is still blocked. The Xenia log proves a TU2 image different from the retail input, while the strict local verifier proves the selected patch stream does not reconstruct from the supplied retail XEX and fails a deterministic decrypted block hash. The source verifier was rebuilt after temporary diagnostics were removed; crypto self-tests pass. The next repair must acquire the exact base/patch stream used by Xenia or a traceable patched-image capture. Do not analyze or compile the retail-only image as if it were TU2.

## Review cycle 13 — 2026-10-02
- Reviewed the non-strict experiment independently: the only mismatch override crashed during patch application and emitted no candidate image.
- Decision: retain strict hash verification; do not weaken the source identity gate or treat the retail native build as TU2.


## Review cycle 14 — pivot authority
- Found and corrected a contradiction between the user-authorized Xenia SBS pivot and the plan's previous native-primary wording.
- Native recompilation remains explicitly blocked and quarantined; no retail-derived output is promoted.


## Review cycle 15 — host freeze and stereo design
- Fresh-context checks verified every manifest artifact hash and launcher validation.
- Found no code defect in the launcher after correcting the content-root layout and the readback assertion.
- Remaining blockers are visible DLC acceptance and access to the Xenia source revision needed for a render-boundary patch.


## Review cycle 16 — stereo source mapping
- Verified the pinned source tree is clean and shallow at the recorded revision.
- Confirmed the presenter is only a final-image boundary; a duplicate-frame compositor would violate the acceptance standard.
- Kept implementation unmodified pending a valid per-eye transform boundary.

- 2026-10-05: Display-mode requirement clarification. Updated the strategic plan, execution runbook, checklist, and acceptance record to require both normal single-view play and true left|right SBS from the same complete runtime. Added separate acceptance rows and browser/WebRTC mode-selection gates; a one-mode implementation is incomplete.

2026-10-06 plan/native-bootstrap review: governing documents now require standalone native and standalone browser-native execution in normal/SBS modes. Optional streaming is excluded from completion. Current exact TU2 compile/link is distinguished from open execution/rendering/full-game gates; prior route decisions remain labeled historical. Loader field-size regression and invalid synthetic key binding repaired; four focused suites pass including negative key-binding coverage. Native host now supplies explicit metadata-free XEXP and rejects missing paths. Exact TU2 hash/module binding/startup proved headless, but six dynamic-fallback observations, 228 unresolved imports, and trap at 0x82297D58 prevent gameplay acceptance. Source/doc diff checks pass. Critic was an adversarial self-review, not a separate independent agent; independent release review remains open. Evidence: evidence/cards/07-native-bootstrap-20261006.json.

2026-10-06 detailed completion-plan review: created COMPLETION_REPAIR_PLAN.md and mapped Packages A-I to Cards 00-16. Reviewed current exact-TU2 probe, import/fallback evidence, selected runtime remote/commit, opcode catalog/lifter, native build configuration and streaming-oriented web scaffold. Corrected toolchain naming, stale module-binding status, failed-gate wording, and unqualified single-thread fallback language. Added specific CPU/routing, import/runtime, graphics/audio/video, full-game, stereo, portable-package and standalone-browser repair/qualification steps. Coverage/link checks pass; no new gameplay/runtime milestone is claimed. Review was a separate adversarial self-review pass, not an independent agent. Unknown contracts retain tests/repair branches; there is no feasibility guarantee.
