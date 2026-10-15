# Card 01 — ReXGlue qualification (2026-10-12)

Status: **REVOKED: INPUT IDENTITY / GENERATED BOUNDARIES INVALID**. See the 2026-10-13 correction below. Historical claims follow.

Previous status: **CODEGEN QUALIFIED / RUNTIME UNQUALIFIED**. This promotes only the isolated code-generation gate. No native runtime, first-frame, SBS, or browser gate is promoted.

The campaign is `research/rexglue-qualification-20261012/`. The pinned SDK commit is `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`. The isolated source copy uses a minimal filesystem fallback for Windows reparse paths and is built with Clang 22.1.8 plus `-mssse3`; the canonical SDK checkout and user-owned inputs were not modified. The patched executable is recorded in `QUALIFICATION_RESULT.json` and was copied only to the campaign outputs.

The fresh CDB investigation showed the original `0xC0000409` was an uncaught `std::filesystem` exception: `canonical` first, then `relative`/`weakly_canonical` on the workspace reparse path. The isolated patch replaces those calls with error-code-aware canonical/lexical fallbacks. The next distinct issue was three unresolved tail branches. Adding three evidence-backed entrypoint gap seeds (`0x82BD8108`, `0x82362218`, `0x8230EA68`) made the base control pass analysis.

Builder evidence: patched `rexgluerd.exe` generated 431 files from the base control in 52.9 seconds with zero unresolved analysis errors. A fresh rerun completed with `431 unchanged`, proving deterministic output. The only warning is one generated function larger than `max_file_size_bytes`; it remains tracked and is not silently promoted as a runtime fix.

Fresh critic evidence: `logs/cdb-patched-codegen.txt`, `logs/patched-codegen-gap-seeds-entrypoint.txt`, and `logs/patched-codegen-gap-seeds-entrypoint-rerun.txt` were inspected together with generated files and hashes. This proves codegen on the base control only. The exact TU2 effective image still requires a patched-tool run, followed by runtime linking, first-frame, services, and complete offline acceptance.

## Exact TU2 effective-image result

The patched tool then ran against the campaign copy of the effective TU2 digest
XEX (SHA-256 `1B8E87A5D9FFBAC50B720EF014E1EA0C8EE8CC065AFDD213F93D3A24E888022C`,
version `0.0.2.3`). The first exact-image pass exposed four new branch gaps;
those were added as evidence-backed entrypoint seeds. A clean rerun generated
433 files and reported no `Unresolved` branch/write lines. A fresh deterministic
rerun reported `433 unchanged`.

The exact-image log still reports 13 unresolved XAM/xboxkrnl ordinals and many
unimplemented `sc` stubs. Those are runtime ABI work, not silently accepted
completion. Therefore Card 01 now qualifies deterministic exact-image codegen
only; runtime linking, first frame, services, DLC, and full-game acceptance are
still open.

## Active correction — 2026-10-13

The 2026-10-12 codegen qualification is **REVOKED** after actual input-byte and generated-body review. The prior synthetic wrapper's body is SHA-256 `EC2C955DB9E55A6207A391A2AB80F60332DC41C3FE4C07968C7F07D5A9621CA4`, not the canonical `EBDCBE683DB2F1C29AA43AC97E79492926C099B2EE253DB502CABCBEC133D52F`. All 617 import records in that wrapper were rewritten by Xenia (including `DEADC0DE` and `sc`), producing bogus ordinals. Do not implement those bogus imports or treat this as missing syscall runtime support. Manual `size=4` gap entries also truncated code: `gap_8230EA6C` executes a compare and returns without the following guest branch.

The retained supervised build compiled the generated translation units and runtime, then failed the final host link on `sub_0`; it did not produce a playable game. Earlier tool-session interruptions lacked reliable lifecycle evidence and are not game failures. The ImGui host include defect was repaired locally; original inputs and canonical SDK remain unchanged.

Current action: qualify `research/rexglue-qualification-20261012/repair15-canonical-tu2/` using the canonical pre-import-resolution TU2 wrapper (SHA-256 `1E4361641D84D3B9CB1C13DD6E208BB079EE76C0E11154687B740EAC19D9616B`). Its body hash matches the canonical raw capture and all 617 import records are valid. Clean analysis exposes four unresolved tail targets; discover full bodies without artificial four-byte boundaries, audit generated coverage and imports, then compile/link and run a supervised normal-mode first-frame probe. Native gameplay, Rust migration, SBS and standalone browser gates remain open.

Evidence: `research/rexglue-qualification-20261012/input-identity-audit-20261013.json`, `native-build-supervised-status.json`, `logs/native-build-supervised.txt`, `logs/codegen-canonical-tu2-clean.txt`. This is self-review, not independent acceptance.


## Current native milestone — 2026-10-13

Corrected canonical TU2 code now compiles and links against the isolated ReXGlue runtime. The runtime wrapper is SHA-256 `6BF4C44B9EBD2D7CA27929E0B6D7DC3308CC420D932511F0CDAD9C48671ED91C`, containing one exact canonical payload; the older `1E4361...` wrapper contains a duplicate body and must not be used at runtime. CDB proved the old body overwrite caused an ASCII-valued invalid host function pointer during `sub_82297B50`. A single-payload launch proceeds beyond it.

Observed indirect entries `0x822C6FA8` and `0x82EB2A10` were added with full reachable-body discovery. Subsequent launches advance to `SetInterruptCallback(82CE6410, ...)` and fail at the next unregistered target `0x826169A8`. That entry is being generated/built. Five private EB archives were extracted to campaign assets with per-member hash verification (12,813 files, zero collisions); the `main.big` missing-path failure disappears, while `alphaoverlay.fxo` and cache alias/service contracts remain unresolved. The existing SDK USB-camera export source was enabled to fix seven final-link errors; webcam functionality remains stubbed/unverified.

**Next action:** finish the build with `0x826169A8`, run a supervised probe on the single-payload XEX and isolated extracted content, and inspect the next exact failure and game window. Runtime/first-frame/full offline, Rust, normal/true SBS and standalone browser acceptance remain open. Evidence: `research/rexglue-qualification-20261012/progress-evidence-20261013.json`, `single-image-repair18.json`, `canonical-codegen-audit-20261013.json`, `archive-extraction-manifest.json`, `probe15` through `probe24` folders. Review is self-review, not independent acceptance.

