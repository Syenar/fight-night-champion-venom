# Evidence card 07 — isolated ReXGlue comparison

**Status:** bounded comparison executed; no route change. ReXGlue remains unqualified for Venom runtime use.

Campaign `research/rexglue-comparison/campaigns/20261008T0043Z/` preserves the existing Xenon and ReXGlue states, then configured the pinned ReXGlue SDK in an independent ignored copy. The CLI target built and help ran. The full build stopped when Windows could not launch its bundled PPC assembler. The unit-test binary built after campaign-only environment adjustments; 205 of 213 cases passed, 4 failed, and 4 known-broken BitStream cases were skipped.

ReXGlue initialized a fresh project from the same read-only TU2 effective image recorded by Run25. Its code generator exited with `0xC0000409`; a known-valid XEX control and a PTY run returned the same status. No generated candidate, runtime launch, first frame, controller test, or exhibition match was produced. The exact image is a bare `MZ` PE rather than a normal XEX container, so input-format qualification also remains open.

The active Xenon and original ReXGlue repositories’ full Git porcelain snapshots exactly match their S0 baselines. The campaign input SHA-256 remains `EBDCBE683DB2F1C29AA43AC97E79492926C099B2EE253DB502CABCBEC133D52F` and it is read-only. No owned ISO, XEX, TU, DLC, emulator install, active save, or current Xenon source was changed.

The comparison does not show that ReXGlue is better than the current route. Keep the current route authoritative while the Windows codegen crash, PPC assembler launch, unit-test failures, and TU2 image format path are investigated. Runtime and exhibition conclusions are pending.

Full evidence: `research/rexglue-comparison/campaigns/20261008T0043Z/results/COMPARISON_REPORT.md`.
