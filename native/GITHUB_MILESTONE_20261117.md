# Native TU2 gameplay milestone · November 17, 2026

The native Windows recompilation reached an in-ring bout with XInput enabled. A player confirmed that created boxer LAST rendered correctly against George Foreman in round one. The face, neck, torso, wrists, and visible legs rendered without the black and cyan artifacts previously reported in created-boxer bouts. The same run logged 12 successful DLC mounts.

## Verified local build

| Component | SHA-256 |
| --- | --- |
| Native TU2 title | `D51F35A85A83F98E1335BD07F349D7E3E2533F0E9FD0F48360A6022AD3A02444` |
| XInput-repaired runtime | `CE5F6C0AC6D885C8B86FB093A9D0806B16F0B9175736835D1DC7410F0A1794A6` |
| Paired D3D12 GPU module | `BCEA017C264DF07AE6EB1FE3F5B2CB82A7A2C45DCC40FBD300B8F6120A46E346` |

The test used the exact TU2 content setup, a 60 FPS setting, and full GPU readback. Work continues on the earlier Ali and Liston texture cases, created-boxer save and reload, and the remaining DLC, performance, SBS, and installer checks. The locally tested executable and owned game content remain private.
