# Installer form and build-flow acceptance

Status: app-owned native self-test implemented and compiled; **GUI/backend execution remains unverified and is not authorized until root reviews the frozen source and exact scenario**. Computer Use, accessibility automation, generic desktop automation, external-window control and screenshots of other applications are prohibited. No such route is required as a workaround.

The explicit `--ui-self-test <scenario.ini> --scenario-sha256 <SHA256>` interface creates only this program's own form without activation. It fills and reads its actual controls, checks required action-control rectangles at the tested client size, and calls the same production validation, command builder, review/build handler, owned job/process monitor and asynchronous completion handler. Scenario authorization replaces only the confirmation dialog after exact validated command/hash matching. There is no fake backend or success injection.

Before any native run, prepare a fresh private E QA scenario that pins the installer, real packaged Python backend, Python executable, source-package manifest, prior full-pipeline receipt and exact production command. The scenario requires resume, disabled input persistence and a nonexistent install child within that qualified output. The console descriptor uses the real C++ command builder without creating controls or launching the backend. The template/descriptor is not itself GUI acceptance.

After root source review and explicit run authorization, collect the app's `ui-self-test-result.json`, process exit evidence, command/source/executable hashes, the actual new pipeline receipt and fresh installation manifest. Independently parse the backend receipts and verify the intended fresh child and qualified reused stages. A positive form-flow result requires real child completion with the expected exit code, restored controls, an empty owned job before close, unchanged form values, and—when exit0 is expected—changed pipeline-receipt bytes plus a fresh install manifest. A requested deadline or own-window close cancels only the installer-owned job and yields failure. A negative backend test must use the actual pinned backend; expected failure-flow acceptance is labeled separately.

| Evidence | What it can establish | What remains unverified |
| --- | --- | --- |
| Console regressions | 248 Windows argument round trips, scenario rejection/escaping/hash checks, owned-child job containment | Form creation, rendering and real backend flow |
| Own-form native self-test, once run | Actual stored control values, required action rectangles, shared production validation/review/build flow, real async child completion and private receipts | Visible text/font rendering, clipping, human readability, keyboard navigation and DPI behavior |
| Real backend/install receipts | Source/media identity, actual phase execution or qualified reuse, fresh install/preflight | Gameplay, physical-controller/default-backend behavior and normal game shutdown |

Folder-picker Cancel behavior, manual confirmation Cancel, remembered-input loading, resizing/scrolling and human keyboard/DPI observations are still unverified unless separately observed. Do not promote own-control rectangle/value checks into visible GUI certification. The initial self-test deliberately disables Remember inputs and does not exercise those persistence/dialog branches. Bitmap capture is not implemented.

Preserve the earlier frozen GUI/package evidence and failed build namespaces. Current build/install evidence does not authorize publication or claim complete DLC/gameplay/controller acceptance. FPS is accepted by the user; no further FPS work is required. SBS follows installer publication.
