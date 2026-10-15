# Card 08: recovery-vision measurement critic — 2026-10-14

Reviewer: `callback_critic`, independent delegated source/artifact inspection. No implementation changes, builds, game launches or input injection were performed. Probe52 was already running under its separate supervisor. Only this evidence note and the FMA critic addendum were written.

**Measurement verdict: REPAIR REQUIRED; no gameplay gate promotion.** Defaults remain inert: recovery start/duration are zero, balance-file path is empty, and the existing connected-user-0/Start opt-in gates remain. Active overrides are bounded ordinary LX/RY controller axes, not guest-state changes.

## Concrete input-contract defect

In inspected input_system.cpp, balance-file changes replace thumb_lx and the fixture replaces thumb_ry, but packet_number receives only recovery_stage. That stage is constant 1 for the entire 120..150-second active window. With unchanged physical-controller packet numbers, corrections and releases can therefore change gamepad axes while retaining the same returned packet number. This contradicts the documented XInput change-indication contract; a consumer that ignores unchanged packets may miss the recovery corrections. Whether this title actually does so has not been established. See [Microsoft XINPUT_STATE documentation](https://learn.microsoft.com/en-us/windows/win32/api/xinput/ns-xinput-xinput_state).

Repair measurement before interpreting recovery failure: maintain a stable sequence for changes in the returned controller state, including recovery pulse/release transitions, and retain packet numbers alongside the actual guest-visible axes. Probe52's VENOM_RECOVERY_BALANCE logs establish that the GetState fixture observed LX transitions; they do not establish that title input processing consumed them.

## Detector, file and timing limitations

- The classifier uses fixed 1280x720 color boxes and a white-pixel count as an LS-icon proxy. Three known frames are useful controls, not a robust rejection test for every menu, transition or scene. Resize/missing icon yields neutral, but unrelated bright/color content can still mimic those thresholds. Add recovery-state confidence and additional negative frames before treating detections as reliable closed-loop state.
- Formatted integer extraction accepts a valid prefix such as `32767junk`; it does not enforce the stated one-value/invalid-file-neutral contract. Truncate-and-write command updates also are not atomic. Require the entire validated record, atomic replacement and a short freshness deadline if this fixture is retained. The existing numeric bounds do prevent arbitrary out-of-range axes.
- The watcher processes every unseen filename in modification-time order. A backlog can apply corrections from stale frames; no frame-age limit or latest-frame-only selection is enforced. Its 150 ms Python sleep is not proof of a 150 ms guest-consumed input pulse.
- `finally` restores neutral on normal exit/Python exceptions, not forceful process termination or a failed final write. A stale nonzero command has no file-level expiry; the host's 30-second active gate is the remaining bound. The watcher starts its 260-second timer separately from the game's first-poll clock, so launch/hash delays and early game exit are not synchronized automatically.
- The command file is opened synchronously on each active GetState poll. File-system latency and reading while writing can perturb or shorten the measurement. Record parser status, command freshness, frame age and guest-visible packet/axes to distinguish these from gameplay behavior.
- Run-NativeGetupVision52.ps1's manifest says captures115..233s, but its actual settings are 100 frames, one-second intervals, delay115: nominally115..214s plus scheduling delay. Its broad exp2/disappearing-fighters hypothesis does not isolate this recovery-control measurement. Correct the prospective bounds and discriminator.

At inspection, vision-log.json contained 75 processed frames with 12 non-neutral decisions. Retained runtime logs showed LX=-32767/0 transitions around ages134..140 and stage2 at age150.009. This confirms bounded fixture activity at GetState, not successful balance recovery, sustained fighting, or an FMA/exp2 rendering cause.

## Inspected artifact SHA-256

Paths are relative to `research/rexglue-qualification-20261012/`. These identify the reviewed version; later builder repairs require fresh hashes/regression evidence.

| Artifact | SHA-256 |
| --- | --- |
| Recovery-Vision52.py | 9C33915BD1422BDDCB293C16276F467655A06C7D964A7ED5684F653131C0400B |
| Prepare-VisionProbe52.py | 9473DACCB61476ECBD67525AA1E78159F0E343C49F12B8DC73E97D7708AA7479 |
| Run-NativeGetupVision52.ps1 | 9250AC5DD0CCCFD5B02F4969935955089F03860CB566B802B95A27C1F2F38344 |
| toolchain/source-patched/src/input/input_system.cpp | CF59E735185306C0F6FD1E300009CB9619C10AAB04907C22AE13AE1D9664299D |

Full native gameplay, TU2/DLC, visual acceptance, true SBS, browser-native and release gates remain **OPEN**.
