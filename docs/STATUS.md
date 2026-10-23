# Fight Night Champion Venom — current status

Updated: 2026-10-23

This project is an active engineering build for a fan made Fight Night Champion recompilation and browser viewing experience. Work is moving forward, but it is not a finished game release.

## Gameplay playability

The game has previously reached playable boxing in qualified local runs. The current launch path is temporarily unplayable while a shared startup failure is repaired. The project remains a playable-game effort, but full-game acceptance, save/reload coverage, and a distributable build remain open.

## Working now

- The native runtime scaffold builds successfully in debug and release configurations.
- Native lifecycle, foreign function interface, and stereo bridge checks pass.
- The local launcher and browser entry points bundle successfully.
- The launcher signaling test suite passes when run with its isolated test ports.
- Source and evidence are organized for continued decompilation and renderer work.

## Being investigated

- Completing the full game decompilation and proving reconstructed code matches original behavior.
- Diagnosing missing or incorrect custom fighter and outfit textures.
- Connecting renderer resource tracing to verified left and right eye presentation.
- Repairing the shared startup failure that currently stops bounded game launches before a renderer trace.

## Not proven yet

- A complete behaviorally equivalent decompilation.
- A complete playable game build, saved progress, or a full bout in the current launch path.
- Correct custom character and outfit textures in all relevant scenes.
- Independent left and right eye rendering, verified parallax, and a playable stereo experience.
- Standalone browser execution without a separately running local game host.
- A self-contained downloadable or installable game package from this repository.

The repository does not contain the commercial game image or other proprietary game files. It cannot download or install those files. Anyone testing the project must provide their own legally obtained game inputs and keep them outside version control.

## Next step

Diagnose the shared startup issue, then restore the temporary gameplay regression and rerun the bounded texture comparison before making any runtime, texture, stereo, or completion claim.