# App-owned installer self-test

This opt-in internal test interface uses only the installer's own controls. Ordinary no-argument launch keeps the normal interactive form. The diagnostic window uses WS_EX_NOACTIVATE/SW_SHOWNOACTIVATE. No SendInput, FindWindow, accessibility, desktop screenshots, game launch, controller setting or registry/AppData persistence is added.

Prepare data without running the GUI/backend:

```powershell
python -B native/installer/windows/prepare-ui-self-test.py `
  --installer-build E:/Venom-Installer-QA/installer-native-ui-selftest-r1/compile-r5 `
  --package-root E:/Venom-Installer-QA/installer-ui-r1/package-r1/verified-source `
  --pipeline-output E:/Venom-Installer-QA/public-build-install-r1 `
  --run-root E:/Venom-Installer-QA/installer-native-ui-selftest-r1/prepared `
  --fresh-install-name installed-native-ui-selftest-r1 `
  --timeout-seconds 600
```

The preparer checks the entire supplied source manifest and prior qualified219-object pipeline, reads the original input arguments, and chooses a fresh install child. It runs only the hash-bound console descriptor to obtain the real C++ production command/hash. It does not mirror the argument builder or execute the UI/backend. The generated `prepared-scenario-receipt.json` contains the exact future GUI argv for root review.

The scenario is UTF-8 key=value data, at most64KiB, with exact known keys and duplicate/control-character rejection. It includes all14 form fields, explicit `I_AUTHORIZE_HASH_PINNED_RESUME`, resume1, remember-inputs0, timeout1..1800seconds, expected backend exit0..255, and SHA256 pins for the host installer, backend, Python, package manifest, baseline receipt and exact command. The GUI CLI additionally requires the exact scenario hash. Writable run/output/install/temp/receipt locations are under nonredirected physical E:/Venom-Installer-QA. Owned source inputs and tools are read from their original paths. The app rechecks backend/Python/manifest/baseline/fresh install before confirmation. TEMP/TMP alone are rebound for its child; no controller-disable environment is inserted.

**Do not run the generated GUI argv before source/scenario review and explicit root authorization.** Once authorized, the app calls production review then build, monitors the real Python job, records its own form/rectangle/control states and actual process completion, and closes its own window. The child starts suspended, is assigned to the installer-owned kill-on-close job, then resumes. Test cancellation uses that owned cleanup path. The finite message-loop deadline and5second cleanup budgets are not a universal guarantee against a stalled OS/filesystem API. No unrelated process is searched or controlled.

The final receipt uses exclusive creation in the scenario run directory. It records actual child/expected/completion codes, command/source pins, form values, state transitions, job-zero evidence, baseline/new pipeline hashes and install-manifest hash. Exit0 form-flow success additionally requires new receipt bytes and a new install manifest. The native interface does not parse the pipeline JSON semantics; the root must independently inspect those bound receipts. Expected real-backend failure tests use a separate PASS_EXPECTED_BACKEND_FAILURE_UI_FLOW label. Missing/stale receipts or monitor/cleanup/deadline failures cannot become a positive result.

Console tests and the source package's native backend remain separate evidence from actual GUI execution. No bitmap or human visual/DPI/keyboard certification is supplied. See GUI_ACCEPTANCE.md for exact remaining gates. Previous frozen packages are unchanged; the newer self-test host can point at a separately pinned read-only source package without changing it. Reprepare the scenario if any pinned binary, source package, baseline receipt or install target changes.
