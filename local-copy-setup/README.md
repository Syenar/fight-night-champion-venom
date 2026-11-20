# Venom local-copy installer v0.1.5

This Windows installer copies an existing Venom build already installed on your PC. Keep your game files, Title Update 2, helper tools and verified installation available. This download contains Setup and its source.

1. Extract the ZIP and open **VenomInstaller.exe**.
2. Drop your Xbox 360 ISO into Setup or click **Browse** next to **Game source**.
3. Under **Existing Venom build**, choose the completed build folder containing user-data and the installation receipt.
4. Choose **Install location**, then click **Install**.
5. Open the new build folder and run **Play Venom.cmd**. The launcher defaults to XInput.

Setup checks the ISO against your existing build, copies its executable and runtime, and keeps game data and shared tools at their existing locations. It copies saves and verified DLC into a separate profile and leaves the original installation available.

The first run may require internet access for hash-checked portable Python and PowerShell. A first-time install with only an ISO requires an existing Venom installation first.
