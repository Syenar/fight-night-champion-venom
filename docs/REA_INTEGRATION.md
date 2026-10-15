# REA integration boundary

REA is an evidence and workflow layer for the Venom investigation. It is not a
recompiler, an Xbox loader, or proof that the game has been rebuilt. Its native
provider can drive an existing Ghidra installation and return pseudocode,
assembly, imports, calls, xrefs, and evidence records. XenonAnalyse,
XenonRecomp, the runtime host, and the acceptance matrix remain separate gates.

## Required use

When the package is available, use a pinned REA release and the exact effective
TU2 image. Run read-only provider queries in an isolated session, record the
source hash, provider version, language/profile, address, command, output path,
and result, then independently compare the result with the saved Ghidra
project, generated code, and runtime trace. A REA answer never authorizes a
runtime semantic change by itself.

The correct setup command for the Codex client is:

```powershell
npx rea-agents@latest setup --client codex
```

Setup is a reviewed user-level configuration transaction. It must not be run
against the game workspace as a substitute for analysis, and it must not alter
the ISO, XEX, title updates, DLC, Xenia, or generated proprietary data. After
setup, restart/reconnect the agent and verify the registration with:

```powershell
npx -y rea-agents@latest doctor --client codex --json
npx -y rea-agents@latest doctor --provider ghidra --json
```

On Windows the provider uses an existing Ghidra 12.1.x installation and a
64-bit JDK; it does not install either one. The current environment has
Ghidra 12.1.2 and Temurin 21, which satisfy that provider version range.

## Current setup result

On 2026-10-10, the public package-runner attempt was made from the Xbox
workspace. Node 24.12.0 and npm 11.12.0 satisfy REA's supported Node range, but
the sandbox could not resolve `registry.npmjs.org` (`ENOTFOUND`). The reviewed
dry-run therefore planned changes without applying them. The already-cached
REA 6.3.0 package was then run with the user's approved elevated setup action:

- Codex registration was applied to `C:\Users\samsa\.codex\config.toml`.
- The pre-change backup is `C:\Users\samsa\.codex\config.toml.rea.backup`.
- The REA skill is installed at `C:\Users\samsa\.agents\skills\reverse-engineer-anything`, version 34, with 139 tools.
- The elevated doctor verified Ghidra 12.1.2, Temurin JDK 21.0.11, the native
decompiler, native-authority controls, and an aligned Codex registration.

The full doctor JSON and a concise setup record are in
`research/rea/setup-20261010/`. The doctor still reports
`environment_healthy: false` because Hopper and IDA are optional missing
analysis engines and other clients are unconfigured; the requested Ghidra/Codex
scope is healthy. A non-elevated doctor may report native-authority access denied
because of the Codex sandbox token; that is an inspection limitation, not a
Ghidra failure. Restart or reconnect Codex before expecting the new MCP server
to appear. REA availability does not close the native runtime startup blocker.
## Scope discipline

REA-style evidence is already required by the runbook's investigation contract.
Until the setup and provider doctor pass, continue with the pinned local Ghidra
workflow and preserve the same evidence fields. Do not pause the startup repair
or claim that REA is required for XenonRecomp. Do not use REA's pseudocode as a
wholesale C++-to-Rust conversion or as a license to bypass the exact-image,
runtime, SBS, and browser gates.


