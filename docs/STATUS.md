# Fight Night Champion Venom — current status

Updated: 2026-10-23

This project is an active engineering build for a fan made Fight Night Champion recompilation and browser viewing experience. Work is moving forward, but it is not a finished game release.

## Working now

- The native runtime scaffold builds successfully in debug and release configurations.
- Native lifecycle, foreign function interface, and stereo bridge checks pass.
- The local launcher and browser entry points bundle successfully.
- The launcher signaling test suite passes when run with its isolated test ports.
- Source and evidence are organized for continued decompilation and renderer work.

## Being investigated

- Completing the full game decompilation and proving that reconstructed code matches the original behavior.
- Diagnosing why some custom fighters and outfit pieces can appear without their intended textures.
- Connecting renderer resource tracing to a verified left and right eye presentation path.
- Preparing a controlled texture comparison and a bounded renderer run using locally supplied test inputs.

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

Finish the independent review of the newest renderer readiness packet, then run the bounded custom versus stock texture diagnostic only when its local prerequisites are satisfied. Record the resulting evidence before making any runtime, texture, stereo, or completion claim.

