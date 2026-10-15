# Independent callback critic — 2026-10-13

Fresh-context read-only critic `/root/callback_critic` inspected raw canonical bytes, wrapper payload, constructors, Ghidra instruction evidence, actual generated bodies/registration, compiled vector regression, runtime pairing, probe logs and captures.

**Static callback admission/tails: PASS. Start-to-main-menu: FAIL. Full-game acceptance: OPEN.**

The critic independently verified canonical SHA-256 `EBDCBE683DB2F1C29AA43AC97E79492926C099B2EE253DB502CABCBEC133D52F`, wrapper payload offset 16384, and constructor `0x828EE0F4` loading table `0x820DF3E8` and storing it at object offset zero at `0x828EE10C`. All 27 emitted callback sequences match the retained Ghidra instructions. Complete generated tails include 25 indirect thunks, `0x828B88E0 -> 0x82898550`, and `0x8291DB40` returning `r3+140`; all are registered, with no forced four-byte function sizes.

The largest runtime defect was probe42's fatal unregistered entry `0x82E1A030` (`runtime.log:12105`, exit `0xC0000409`). Its `guest-3.png` is the autosave A OKAY dialog, not the menu. Start press/release is recorded at log lines 4715/4761. Later cycle16 failed at `0x824DFF78` before its scheduled A replay. These addresses are historical once repaired; follow the newest numeric loop result.

Probe42's older DLL hashes reflect the later addition of A replay, not a proven pairing defect. Retained cycle16/17 build statuses match their respective probe manifests for all three DLLs. The critic correctly limited vector proof to seven actual emitted `vpkuwus128` sites, two source-alias cases, 2,000 checks and the original-bug negative control. Other pack opcodes/alias classes are not validated by that test.

The automated callback audit alone checks registration, call/return presence and absence of `REX_UNIMPLEMENTED`; semantic correctness here is supported by the critic's direct instruction/body comparison. No files were changed, builds run or processes launched by the critic.

Private evidence: `research/rexglue-qualification-20261012/callback-table-admission.json`, `ghidra-callback-audit.json`, `callback-batch-audit.json`, `vpk-alias-regression/`, `probe42-callback-start-replay/` and per-cycle manifests/logs.
