# Card 01 — tool discovery

Status: PASS for tool discovery; title-specific executable feasibility remains pending Card 03.

The required toolchain is available and pinned, with installation directories and hashed executables recorded separately in esearch/TOOLCHAIN_MANIFEST.json. XenonAnalyse, XenonRecomp, and ReXGlue all executed their help/version commands successfully. Ghidra headless executed successfully after redirecting its writable Java user profile to a temporary directory; the default profile is access-restricted in this environment. The exact Fight Night executable has not yet been supplied to the tools, so title-specific compatibility is intentionally not claimed until Card 02 extraction and Card 03 identity.

The external Xenia installation is recorded read-only for reference capture. No parent PS3 analysis, source dump, ISO, executable, title update, DLC, or Xenia installation was modified.

- Manifest: esearch/TOOLCHAIN_MANIFEST.json
- Version record: esearch/toolchain/versions.txt
- Next card: Card 02 — read-only ISO extraction

