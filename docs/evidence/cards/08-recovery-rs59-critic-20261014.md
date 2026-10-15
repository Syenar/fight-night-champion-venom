# Card 08: RS recovery detector59 independent critic — 2026-10-14

Reviewer: `callback_critic`, delegated source/retained-frame inspection and isolated classifier regression. No SDK edit, build, watcher loop, controller injection, game launch or process-close action was performed. The manual game session was left untouched.

**Scoped PASS for detector59's retained-frame classification repair. Successful recovery and gameplay acceptance remain OPEN.**

The source adds a distinct rise phase: bright RS disc, dark RS lettering, upward chevrons, inactive dark LS disc and the recovery bar must all be present. Only that complete phase returns the explicit rise sentinel `(0,0,0)`. Valid directional LS prompts remain balance corrections. An LS prompt with a neutral arrow difference alone is rejected; wrong-size/menu/invalid states retain `(0,-1,-1)`. This resolves the specific omitted-RS-state measurement defect identified in probe58's actual upright frames.

The watcher loop is byte-text identical to Recovery-Vision58.py from its validated output-directory assertion onward. Atomic replacement, 250 ms expiry, 100 ms renewal, latest-successful-frame freshness of 1.5 seconds, rejected/stale neutral behavior and bounded controller-only input policy are unchanged. The new default output directory is separate. No inference of an active automatic fixture in the manual session follows from this source review.

The critic independently reran the AST-isolated TestDetector.py: **420 retained frames, 50 balance, 13 rise, 357 rejected, 11 extra controls; PASS.** The corpus has 113 distinct frame hashes. The new positive rise interval uses probe58 frames 15..27, including the RS-up frames previously independently viewed. Negative controls include wrong-size/menu/uniform frames and four independent feature-removal cases, each rejecting removal of a required rise feature. AST execution loads only imports plus score/direction, so this regression does not start the watcher or emit command files/input. No blocking source defect was found in this scoped repair.

| Artifact, relative to research/rexglue-qualification-20261012 | SHA-256 |
| --- | --- |
| Recovery-Vision59.py | 75D848E908B911B6B2940C23349F357183E8CBDB2BFCCFB1957D7B7E5F4FC186 |
| recovery-rs-detector59/TestDetector.py | A3838C19AE7F8639771CEC6C1104208DDA9B8DBB25E5213FF2FB1F21E7E0D328 |
| recovery-rs-detector59/evidence.json | FAE386363B70B34C1AA090AB16FA8C33F78CAA3A488553E595B2021D6B88761E |

Remaining limits: a fixed 1280x720 HUD layout and selected retained sequences do not prove arbitrary-scene robustness, input timing sufficiency, title consumption, successful get-up, sustained fighter rendering or full offline gameplay. A later separately authorized runtime measurement must retain actual rise-axis packets and the resulting game transition. The current manual session must not be launched, interrupted or closed by this critic.

Full native offline, TU2/DLC, normal/true-SBS, browser-native and release gates remain **OPEN**.
