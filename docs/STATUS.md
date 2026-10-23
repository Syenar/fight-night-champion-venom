# Fight Night Champion — current status

Updated: 2026-10-23

This project is an active engineering build for a fan made Fight Night Champion recompilation and browser viewing experience. Work is moving forward, but it is not a finished game release.

## Gameplay playability

The game remains playable in qualified local runs, with boxing interaction and HUD progression demonstrated. The latest launch path has a temporary startup regression while a shared failure is repaired. The project remains a playable-game effort, but a complete bout, save/reload coverage, and a distributable build remain open.

## Working now

- The native runtime scaffold builds successfully in debug and release configurations.
- Native lifecycle, foreign function interface, and stereo bridge checks pass.
- The local launcher and browser entry points bundle successfully.
- The launcher signaling test suite passes when run with its isolated test ports.
- Source and evidence are organized for continued decompilation and renderer work.
- A bounded decompilation batch completed with independent review: 32 callers plus seven controls were captured in paired baseline and candidate runs. It is retained as raw representation evidence only; it adds zero semantic coverage and does not prove behavior, runtime, or gameplay.

## Being investigated

- Completing the full game decompilation and proving that reconstructed code matches the original behavior.
- Diagnosing why some custom fighters and outfit pieces can appear without their intended textures.
- Connecting renderer resource tracing to a verified left and right eye presentation path.
- A controlled renderer launch was attempted with the current diagnostic build and with the preserved comparison build. Both stopped at startup with the same Windows `0xC0000409` error before producing a renderer trace. This points to a shared title/runtime startup issue, not a proven texture regression.

## Not proven yet

The following acceptance gates remain open:

- A complete, behaviorally equivalent decompilation.
- A complete playable game build with saved progress and a full bout.
- Correct custom character and outfit textures in all relevant scenes.
- Independent left and right eye rendering, verified parallax, and a playable stereo experience.
- Standalone browser execution without a separately running local game host.
- A self contained downloadable or installable game package from this repository.

The repository does not contain the commercial game image or other proprietary game files. It cannot download or install those files. Anyone testing the project must provide their own legally obtained game inputs and keep them outside version control.

## Next step

Diagnose the shared title/runtime startup issue, then rerun the bounded custom versus stock texture diagnostic only when the local launch contract is repaired. Record the resulting evidence before making any runtime, texture, stereo, or completion claim.

The reviewed decompile selection is currently exhausted. The next batch requires a new disjoint selection and source-pin review before any new GO or capture.

## Decompilation evidence

A bounded decompilation batch completed with independent review. It captured 32 callers and seven retained controls in paired baseline and candidate runs, with all 39 raw instruction arrays matching. This is raw representation evidence only: it adds zero semantic coverage, does not establish equivalent behavior or exception handling, and does not prove runtime or gameplay acceptance.

The current reviewed function selection is exhausted. The next batch requires a new disjoint source selection and independent review before execution.


