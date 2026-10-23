# Fight Night Champion — current status

Updated: 2026-10-23

This project is an active engineering build for a fan made Fight Night Champion recompilation and browser viewing experience. Work is moving forward, but it is not a finished game release.

## Gameplay playability

The game opens and reaches playable boxing in the current local test runtime, with boxing interaction and HUD progression demonstrated. The normal launch path still needs its logging-path repair before it is the standard one-click route. The project remains a playable-game effort, but a complete bout, save/reload coverage, and a distributable build remain open.

## Working now

- The native runtime scaffold builds successfully in debug and release configurations.
- Native lifecycle, foreign function interface, and stereo bridge checks pass.
- The local launcher and browser entry points bundle successfully.
- The launcher signaling test suite passes when run with its isolated test ports.
- Source and evidence are organized for continued decompilation and renderer work.
- The current local test path opens the game and reaches active boxing; no automatic input or capture is used.
- Seven bounded decompilation batches have completed with independent review: 224 callers plus seven retained controls were captured in paired baseline and candidate runs. The newest batch is retained as raw representation evidence only; cumulative semantic coverage remains unchanged and behavior is not claimed.

## Being investigated

- Completing the full game decompilation and proving that reconstructed code matches the original behavior.
- Diagnosing why some custom fighters and outfit pieces can appear without their intended textures.
- Connecting renderer resource tracing to a verified left and right eye presentation path.
- The normal launcher still stops before startup with Windows `0xC0000409` because its rotating log sink cannot open the configured path. A separate writable local test runtime, launched under the debugger with exception handling, opened the game and reached active boxing; that test was cleanly shut down and is recorded privately. This confirms current playability while leaving the normal launcher repair and texture trace open.

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

The current reviewed successor selection has been consumed by the latest bounded capture. The next batch requires another disjoint selection and source-pin review before any new GO or capture.

## Decompilation evidence

A bounded decompilation batch completed with independent review. Across the seven reviewed batches, 224 callers and seven retained controls were captured in paired baseline and candidate runs, with raw instruction arrays matching. This is raw representation evidence only: it adds zero semantic coverage, does not establish equivalent behavior or exception handling, and does not prove runtime or gameplay acceptance.

The current reviewed function selection has been consumed. The next batch requires a new disjoint source selection and independent review before execution.


