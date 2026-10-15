# Independent callback admission review — 2026-10-13

Read-only fresh critics inspected canonical TU2 bytes, constructor/table relationships, Ghidra disassembly and actual generated code. This is static review, not runtime acceptance.

- Nine `callback_menu_*` entries: PASS. All table slots and constructor bytes match canonical input. Five adjustment tails preserve offsets and dispatch; constant/floating leaves and longer boolean/loop bodies are complete. Latest runtime still fails before main-menu acceptance.
- 373 narrowly selected leaf candidates: PASS as name-only full-body discovery seeds. The critic independently decoded 1,539 relevant PPC words, covering both instructions of every candidate and all 129 constructor spans. Zero mismatch with Ghidra. 361 relative nonlinking adjustment tails reach 88 registered targets; remaining signatures are getters, setters, constants and member addresses. Constructor table-value and object-base registers survive each selected span.
- Admission must skip entries already registered or declared. The moving observed-entry loop added `0x82417D40` and subsequently `0x82396EC0`; a hardcoded expected-new count correctly aborted before mutation and is being replaced by frozen-baseline overlap checks.
- Audit weakness repaired: bounded Ghidra evidence now verifies and records canonical bytes for every decoded instruction, rather than only the entry word. Complete 502-selector audit was rerun successfully. Future batches still require independent constructor/object-base review and generated-body checks.

Private evidence: `research/rexglue-qualification-20261012/leaf-callback-candidates.json`, `ghidra-leaf-callback-audit-complete.json`, `ghidra-menu-callback-audit.json` and corresponding admission manifests. Canonical raw SHA-256 is `EBDCBE683DB2F1C29AA43AC97E79492926C099B2EE253DB502CABCBEC133D52F`.

Full native offline acceptance, main menu, match, Rust migration, normal/true SBS and standalone browser remain open. A static admission pass does not assert that all candidates have executed.
