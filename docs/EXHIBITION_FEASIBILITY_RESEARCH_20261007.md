# Two-fighter exhibition feasibility research

Research date: 2026-10-07. Advisory only: this report does not change the full-game plan, runbook order, or acceptance gates. No game inputs or installations were modified. External projects were inspected through their documentation and selected source files, not built or independently played.

## Decision

Yes: a standalone two-fighter exhibition reconstruction is technically feasible and is a substantially narrower engineering target than the full native/browser Fight Night Champion port. Rust/Bevy is a credible implementation choice. The viral Rust mashups support this architecture, but do not establish Fight Night asset compatibility or faithful boxing gameplay.

An exhibition using the original recompiled Fight Night simulation is also a sensible first gameplay milestone. However, restricting modes does not remove its shared startup/runtime/rendering barriers. There is no evidence here that selecting two fighters makes the current recompiler boot problem disappear.

Recommendation: if earlier playable browser/SBS boxing is the priority and reconstructed behavior is acceptable, pursue a separately identified exhibition reconstruction, first qualifying asset/animation conversion and one complete combat interaction. Preserve the original recompilation as the route to original behavior and full offline modes. A reconstruction is not a completed portion of the original executable merely because it uses the same fighter assets.

## What the Rust mashups actually do

1. [2010 Rust Rewrite Mashup](https://github.com/chasmlol/2010-rust-rewrite-mashup) combines a from-scratch MW2 Rust runtime with Skate 3 mechanics and a Minecraft world. Its documentation describes local conversion of skater models, animation banks, state graphs, and input/physics settings. The architecture connects a skate simulation worker to the host's map collision and soldier posing. This is the most likely example behind the question. The README lists visible defects; its feature claims are not independent parity certification.
2. [Skate integration documentation](https://github.com/chasmlol/2010-rust-rewrite-mashup/blob/main/docs/SKATE.md) identifies the imported Rust skate crates and host integration points. [The integration source](https://github.com/chasmlol/2010-rust-rewrite-mashup/blob/main/crates/render_anim/src/skate.rs) and [Cargo workspace](https://github.com/chasmlol/2010-rust-rewrite-mashup/blob/main/Cargo.toml) provide concrete code/structure behind that description. Importing an XEX as a setup input does not establish that the original executable is being executed.
3. [SK8 Engine](https://github.com/SK8-ENGINE/skate-3-rust-engine) explicitly describes a Rust/Bevy rewrite with incomplete gameplay parity. Its history credits more than two years of prior reverse engineering by dumbad. This is important: the apparent speed of a rewrite excludes much of the foundational format and behavioral research.
4. [IW4L](https://github.com/vladtrc/iw4L) loads owned MW2 maps/models/textures into its own Rust/Bevy engine. Its README explicitly acknowledges incomplete gameplay, bugs, and desynchronization. Its asset readers and gameplay code are title-specific; this is not a generic Fight Night importer.
5. [ghostdisk/mashup](https://github.com/ghostdisk/mashup) demonstrates the more general composition pattern: locally imported assets, interchangeable controllers/animations/mechanics, and shared engine plugins. Its README distinguishes procedural demos, asset viewers, and playable mechanics, and states exact source-game equivalence remains under validation.

The transferable lesson is to construct a small new runtime around decoded assets and selected behaviors. Rust helps organize this runtime; it does not automatically translate an Xbox executable, decode every proprietary asset, or reproduce the original game's simulation.

## Current Venom evidence

The active plan is a complete offline native recompile followed by standalone WASM/WebGPU, in normal and true SBS. The plan already includes an exhibition on the native execution ladder.

Current evidence reviewed:

- `docs/STATUS.md`, `docs/FULL_GAME_EXECUTION_CHECKLIST.md`, and `docs/COMPLETION_REPAIR_PLAN.md`.
- `docs/evidence/cards/07-run25-launch-diagnostics-20261007.json`.
- `research/SOURCE_MANIFEST.json` and `research/source/extract-xiso-list.txt`.

The exact-TU2 generated core has compiled and earlier probes have executed guest startup. A correct rendered first frame and complete match remain unverified. Latest Run25 evidence fails at content graph mounting before guest boot; earlier runs lacked verified GPU ring/front-buffer/present progress. This report relies on those recorded runs and does not claim a fresh runtime test.

Disc extraction is proven, but disc extraction is not fighter conversion. The Xbox listing contains `.big` archives, venue archives, shared gameplay/boot/frontend/shader packages, and video/speech data. This review found no verified Xbox fighter-to-modern-engine conversion or exported boxing animation acceptance evidence in the active plan/evidence examined. Archive names alone do not prove internal models, skeletons, clips, or simulation configuration are decoded.

The selected [Xenon runtime](https://github.com/nimauria/Xenon-Recomp) describes real-title execution qualification as ongoing. [ReXGlue](https://github.com/rexglue/rexglue-sdk) is another early-development toolkit and translates PPC into C++; replacing the current toolkit is not evidence that Fight Night would work. No verified ready-made Fight Night Rust exhibition implementation or complete Xbox animation importer was found in this research. This is a search result, not proof none exists.

## Comparison of routes

| Route | What it delivers | Main remaining dependency | Assessment |
|---|---|---|---|
| Original executable, exhibition first | Original fight behavior, narrower acceptance milestone | Xbox services, startup, graphics, loading, animation, simulation | Best continuity with full port; limited relief for immediate blockers |
| Rust exhibition with owned imported assets | New boxing runtime with original appearance where conversion succeeds | Fighter/rig/clip conversion plus authored combat and AI | Strong smaller target; appearance and gameplay fidelity remain conditional |
| Rust exhibition with original/replacement assets | Independent playable boxing prototype | Animation production, combat quality, AI, presentation | Highest confidence of obtaining a playable slice; does not promise original likeness |
| Original match in Xenia | Reference/emulated Fight Night gameplay | Title-specific reference qualification and emulator behavior | Useful baseline; does not satisfy native recompile or standalone browser target |

Removing careers, story videos, roster breadth, online features, elaborate menus, DLC, and commentary meaningfully reduces a reconstruction's scope. It does not remove two-body combat, responsive animation, contact detection, defense, stamina, knockdowns, opponent behavior, or match rules. Those systems determine whether it feels like boxing rather than two animated models.

## Suggested initial contract

Two fixed fighters, one ring, one camera, player versus basic AI, a short configurable bout, movement, jab/cross/hook/uppercut, head/body targeting, guard and dodge, stamina, hit reactions, knockdown/count-out, timer, basic scoring, victory, pause, and rematch. Use original sound effects or properly sourced replacements initially. Commentary, cutscenes, careers, online play, and a full roster are deferred. Local two-player input can follow once the fight loop is stable.

This is a proposed product scope, not a change to the governing plan. Two named original fighters should be promised only after their models/rigs are validated.

## First gates and stopping criteria

1. **Asset gate:** from the owned Xbox extraction, identify and decode one fighter's geometry, textures, bone hierarchy, bind pose, skin weights, and at least idle, movement, punch, and reaction clips. Render the assembled fighter and compare it to reference behavior. If clips or rigs remain undecoded, choose replacement animations/assets explicitly; do not claim original fidelity.
2. **Combat gate:** two assembled fighters in a ring; a reproducible jab can miss, be guarded, or connect once, consumes stamina, produces a coherent reaction, and returns both fighters to a usable stance. Verify contact at different distances and update rates. This tests the central dependency far better than a model viewer.
3. **Match gate:** basic AI, the complete short bout, knockdown/count-out or decision, pause/resume, restart, and repeated play without corrupt state. Compare feel and measurable timing against a captured original exhibition.
4. **Browser gate:** run the same small runtime locally in a served WASM build with controller input and local converted-asset import. Measure memory, load time, GPU performance, and audio. Avoid loading the entire ISO into browser RAM. Inspect desktop-only dependencies before promising portability.
5. **SBS gate:** same simulation tick, separate left/right cameras and targets, correct projection/depth/clears, measured parallax, configurable IPD/eye order, and a deliberate HUD policy. Re-test complete fights in normal and SBS. Two equal images are not stereo.

Browser feasibility has primary-source support: Bevy publishes [WASM/WebGPU examples](https://bevy.org/examples-webgpu/), including [skinned meshes](https://bevy.org/examples-webgpu/gltf/gltf-skinned-mesh/), [animation graphs](https://bevy.org/examples-webgpu/animation/animation-graph/), and [rendering to textures](https://bevy.org/examples-webgpu/3d-rendering/render-to-texture/). These demonstrate components, not a tested boxing game or guaranteed support on every browser/device. A small authored runtime avoids porting the Xbox address space, kernel, GPU command frontend, and full native host to the web; local files, audio, input, and performance still need implementation and verification.

## Confidence and limits

- High confidence: a deliberately simplified two-fighter boxing game can be built; Rust/Bevy supports the necessary broad rendering/animation structure.
- Conditional confidence: it can use faithful Fight Night fighter appearance and original clips. A validated Xbox conversion must establish that.
- Unproven: it can reproduce Fight Night's original combat feel and simulation closely. Models and animation clips do not contain the whole game logic.
- Unproven: a bypass directly into the original exhibition removes current startup failures. Such a bypass requires a known initialization dependency graph and tested hooks, not guessing a function address or reusing an emulator snapshot.
- No defensible completion date or percentage savings follows from current evidence. A generic prototype, a polished reconstruction, and a faithful original recompile have different costs. Estimate after the asset and combat gates.

Conclusive engineering recommendation: adopt exhibition as the next original-game gameplay milestone. If the priority is a smaller standalone browser/SBS experience, a separately labeled reconstruction is the practical scope reduction suggested by the Rust mashups. Begin with asset/animation and combat feasibility gates, not a wholesale C++-to-Rust translation. Preserve the full-game goal and its honest open gates unless the user explicitly replaces it.
