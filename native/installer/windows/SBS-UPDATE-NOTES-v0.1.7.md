# v0.1.7 SBS update notes

The v0.1.7 package includes the current SBS GPU source payload used by the local build pipeline.

Included source components include:

- `native/sdk/gpu/sdk.patch`
- SBS eye-resource and stereo-boundary headers under `native/sdk/gpu/include-overlay-0`
- Per-file SBS include overlays under `native/sdk/gpu/per-file-include-0`, `per-file-include-1`, and `per-file-include-2`
- SBS source profile and title build metadata

The installer package contains no game media, generated title code, runtime binaries, profiles, or user data. The game and runtime are built locally from the user's owned files.

Private package verification completed with these archive hashes.

- `Venom-v0.1.7-source-only.zip` SHA-256 `1bdf3bb3502a179e8742de239e46dee7d10110f58d1e460de16e40c0afc0e7ca`
- `Venom-v0.1.7-Windows-installer.zip` SHA-256 `8f4f12071ccfd1abe4298013b6c510b1ee53df8d4011938b6764fd1319d902a4`

The package receipt records GUI and gameplay verification separately.