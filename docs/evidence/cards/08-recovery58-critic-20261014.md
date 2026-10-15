# Card 08: recovery58 independent measurement critic — 2026-10-14

Reviewer: `callback_critic`. Read-only implementation/runtime-evidence inspection; no build, game launch or input injection. For visual inspection, five completed probe58 PPMs were losslessly converted into private analysis PNGs under `research/rexglue-qualification-20261012/recovery-critic58/`; originals were not changed. This note records the result.

**Measurement verdict: missing recovery state; repair required before interpreting failed get-up as a gameplay/FMA/ABI failure.**

## Biggest concrete issue

Probe58's retained guest-15, guest-16 and guest-25 frames show an upright view/referee, the green center of the balance bar, a bright **RS icon with upward arrows**, and a dark inactive LS icon. Those are genuine rise prompts, not GAME OVER/menu frames. The detector nevertheless marks each rejected `(direction=0,left=-1,right=-1)` because it requires a bright LS disc/lettering combination.

The watcher consequently sends neutral axes instead of right-stick-up. There are no recovery log entries with `ry=32767` across the inspected probe56/58 rotated log sets. Probe58 frames 14..28 are rejected in its vision log despite the visible RS-up prompt in the inspected frames. This closes neither recovery nor sustained-fight testing: the fixture never exercises the indicated rise phase.

Next discriminator: classify LS balance and RS-up as separate validated HUD states, retaining bar/menu/invalid/stale rejection. Include actual probe58 centered RS-up frames as positive controls, and retain corresponding guest-visible `ry=32767` packets before drawing gameplay conclusions. Do not bypass the detector or modify guest state to force progress.

## Evidence and comparison

Recovery-Vision58.py differs from the reviewed Recovery-Vision52.py only in renewing requested lateral input instead of issuing a 150 ms pulse followed by neutral. The 250 ms command expiry, 100 ms renewal, 1.5-second freshness, invalid/stale neutral behavior, ordinary input scope and host 30-second recovery window remain. This is a bounded measurement-policy change, not a new game/SDK/math repair.

Both probe manifests identify the same executable C8CA4CE00D2081941F754AAE7AAC9004AED095BE4785E7154B6677B1728627C1 and same three DLL hashes recorded by the integrated-FMA critic. Both supervisors report their 240-second deadline, close request, exit 0 and no forced termination. That is retained lifecycle evidence, not gameplay or general shutdown acceptance.

Probe56 vision log: 100 frames, 15 right-direction prompts and 85 rejected frames. Its GetState logs show lateral pulse/release packets but no rise input. Probe58 vision log: 100 frames, 4 left-direction prompts and 96 rejected frames. Its logs show initial left corrections and subsequent neutral periods. Visible upright/RS-prompt frames indicate progression in the balance measurement, but this is not proof of complete get-up or gameplay acceptance.

| Artifact, relative to research/rexglue-qualification-20261012 | SHA-256 |
| --- | --- |
| Recovery-Vision58.py | 48C654F6F8EC6FE5EF4FBD01CE7B86FBB8EA81EA0E5B6A733A7BD88F38970D11 |
| probe58-continuous-recovery/guest-15.ppm | 5505568484EA7D87A8C44CD58FACC2B1B821D3E4631E31144D6F97E1B0344A44 |
| probe58-continuous-recovery/guest-16.ppm | 02208EFC9D9C9F0A0D9FD0F3503C735A6E26E9CC15FA95B490C533B286BC6779 |
| probe58-continuous-recovery/guest-25.ppm | 668E0A9C975DEE810743109E7A9AD5377A9F626F5CB259AC95D09BDCCB685A8D |
| probe58-continuous-recovery/vision-log.json | 775BC79FA3F544EBDAB3ADAFCEF181A8B89B607DE588813A200C998D5E6F321D |

## Separate ABI research boundary

The retained unchanged-class extraction reports eight high-u64 truncations with zero failures in its 32-bit/bool/raw-pointer controls (48 checks total). That is a separate isolated negative-control result, not an adopted runtime repair or an explanation of probe58's omitted RS-up input. The copied-class candidate and runtime consequences were not graded in this pass.

Full native offline gameplay, sustained fighter rendering, TU2/DLC, normal/true-SBS, browser-native and release gates remain **OPEN**.
