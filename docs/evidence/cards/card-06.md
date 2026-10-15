# Card 06 — Xenon analysis and code generation

Date: 2026-10-02  |  Status: **PASS**

The repaired local XenonRecomp build generated the complete native translation set from the prepared Fight Night Champion image. The output directory was created explicitly because XenonRecomp does not create it. The first run exited 0 at `Recompiling functions... 100%` and wrote 186 files: 183 C++ translation units plus three headers/mapping artifacts, totaling 204,947,209 bytes. A second full run also exited 0 and produced byte-identical output across all 186 files (zero changed hashes).

Known warnings remain in the generation log for unsupported or undecoded PowerPC instructions. They are preserved in `full-generation.log` and must be handled during the Card 07 runtime/compiler audit; they do not invalidate deterministic code generation.

Evidence: `research/generated/<source-hash>/CARD_06_RESULT.json`, `generation-pass1-hashes.json`, `generation-pass1-summary.json`, `full-generation.log`, `full-generation-pass2.log`, and `ppc-elf/`.

Next: Card 07 runtime scaffold and unresolved-export classification.
