"""Install an existing local build without a compiler or a game rebuild."""

import argparse
import json
import os
from pathlib import Path, PureWindowsPath
from hashlib import sha256
import shutil
import subprocess
import sys
from tempfile import TemporaryDirectory

from disc_image import XboxDisc
from display_host import patch_launchers, upgrade_runtime
from save_upgrade import (
    LOCAL_INSTALL_RECEIPT, PreviousOutputRequired, check_path,
    file_hash, prepare_saves, previous_profile, read_json,
)

from portable_tools import ensure_tool


HERE = Path(__file__).resolve().parent
PACKAGE = HERE.parents[1]


def checked_copy(source, output, expected):
    check_path(source)
    if not source.is_file():
        raise ValueError(f"Missing installed file: {source}")
    output.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, output)
    if file_hash(output) != expected.lower():
        raise ValueError(f"Copied file did not match its receipt: {source}")


def write_record(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")


def leaf_path(value):
    path = PureWindowsPath(value)
    if len(path.parts) != 1 or path.anchor or value in {".", ".."}:
        raise ValueError(f"Invalid installed filename: {value}")
    return value


WINDOW_MODES = ("Windowed", "Borderless", "Fullscreen")


def supported_modes(powershell, launcher):
    # Parse the installed script without running it to detect a mode-capable build.
    probe = """
$tokens = $null
$errors = $null
$ast = [System.Management.Automation.Language.Parser]::ParseFile(
    $env:VENOM_VALIDATE_LAUNCHER, [ref]$tokens, [ref]$errors
)
if ($errors.Count) { exit 1 }
if ($ast.ParamBlock.Parameters.Name.VariablePath.UserPath -contains 'WindowMode') {
    Write-Output 'WindowModes'
}
"""
    mode_probe = subprocess.run(
        [str(powershell), "-NoProfile", "-NonInteractive", "-Command", probe],
        env=dict(os.environ, VENOM_VALIDATE_LAUNCHER=str(launcher)),
        check=True, capture_output=True, text=True,
    )
    return WINDOW_MODES if mode_probe.stdout.strip() == "WindowModes" else ()

def check_installed_launcher(powershell, launcher):
    modes = supported_modes(powershell, launcher)
    verify = [str(powershell), "-NoProfile", "-NonInteractive", "-File", str(launcher),
              "-InputBackend", "xinput", "-CheckOnly"]
    for mode in modes or (None,):
        flags = ["-WindowMode", mode] if mode is not None else []
        subprocess.run(verify + flags, check=True)
    return modes


def write_play_commands(stage, powershell, modes):
    base = (
        '@echo off\n"' + str(powershell) +
        '" -NoProfile -File "%~dp0installed\\Start-Installed-TU2.ps1" -InputBackend xinput'
    )
    (stage / "Play Venom.cmd").write_text(base + "\n", encoding="utf-8")
    for mode in modes:
        (stage / f"Play Venom ({mode}).cmd").write_text(
            base + f" -WindowMode {mode}\n", encoding="utf-8",
        )


def install(args):
    check_path(args.output.absolute())
    check_path(args.game_folder.absolute())
    check_path(args.previous_output.absolute())
    iso = args.game_folder.resolve(strict=True)
    output = args.output.resolve()
    source_build = args.previous_output.resolve(strict=True)
    if not iso.is_file() or iso.suffix.casefold() != ".iso":
        raise ValueError("Choose your Xbox disc image (.iso).")
    if output.exists():
        raise ValueError("Choose a fresh installation destination.")

    profile = previous_profile(source_build)
    if profile is None:
        raise ValueError("Select a completed existing Venom build.")
    if (source_build / LOCAL_INSTALL_RECEIPT).is_file():
        source_installed = source_build / "installed"
    else:
        receipt = read_json(source_build / "build-install-receipt.json")
        old_root = Path(read_json(source_build / "pipeline-inputs.json")["args"]["output"])
        source_installed = source_build / Path(receipt["installedLauncher"]).relative_to(old_root).parent

    manifest = read_json(source_installed / "venom-install.json")
    paths = manifest["Paths"]
    game = Path(paths["GameDataRoot"])
    update = Path(paths["UpdateDataRoot"])
    dlc = profile / "0000000000000000" / "45410915"
    dlc_items = read_json(source_installed / "owned-dlc-manifest.json")
    runtime = Path(paths["RuntimeRoot"])
    receipt = Path(paths["RuntimeReceipt"])
    protected = (
        PACKAGE, source_build, runtime, game, update, dlc, Path(paths["AptToolsRoot"]),
        Path(paths["ContentToolRoot"]), Path(paths["PythonExecutable"]).parent,
    )
    if any(output.is_relative_to(path) or path.is_relative_to(output) for path in protected):
        raise ValueError("The install folder must be separate from the source build, media and tools.")

    check_path(receipt)
    if file_hash(receipt) != manifest["InstalledReceiptHashes"]["RuntimeReceipt"].lower():
        raise ValueError("The existing build receipt did not match its manifest.")
    with iso.open("rb") as media:
        disc = XboxDisc(media)
        executable = Path("default.xex")
        if executable not in disc.files:
            raise ValueError("The disc has no Xbox 360 executable.")
        retail_hash = sha256()
        for block in disc.chunks(executable):
            retail_hash.update(block)
        if retail_hash.hexdigest() != read_json(receipt)["inputs"]["retailXex"].lower():
            raise ValueError("Your ISO does not match the selected Venom build.")

    print("Verified your ISO against the existing build.", flush=True)
    powershell = ensure_tool("powershell", output.parent / ".venom-inputs")
    output.parent.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(prefix=".venom-local-stage-", dir=output.parent) as temporary:
        stage = Path(temporary) / "build"
        stage.mkdir()
        (stage / "runs").mkdir()
        installed = stage / "installed"
        for name, expected in manifest["InstalledFiles"].items():
            leaf_path(name)
            checked_copy(source_installed / name, installed / name, expected)

        copied_receipt = stage / "runtime" / "build-receipt.json"
        checked_copy(receipt, copied_receipt, manifest["InstalledReceiptHashes"]["RuntimeReceipt"])
        build = read_json(copied_receipt)
        print("Copying the existing game executable and runtime.", flush=True)
        runtime_files = {"venom_fnc_tu2_dlc.exe": build["titleSha256"], **build["runtimeFiles"]}
        for name, expected in runtime_files.items():
            leaf_path(name)
            checked_copy(runtime / name, copied_receipt.parent / name, expected)

        upgraded_host = False
        if not supported_modes(powershell, installed / "Start-Installed-TU2.ps1"):
            name = "rexruntimerd.dll"
            digest = upgrade_runtime(source_build, runtime_files[name],
                                     copied_receipt.parent / name, Path(temporary) / "display-build")
            runtime_files[name] = digest
            build["runtimeFiles"][name] = digest
            write_record(copied_receipt, build)
            manifest["InstalledReceiptHashes"]["RuntimeReceipt"] = file_hash(copied_receipt)
            patch_launchers(installed, manifest["InstalledFiles"])
            upgraded_host = True

        print("Preserving the selected profile and installing owned DLC.", flush=True)
        migration = prepare_saves(stage, source_build)
        for item in dlc_items:
            relative = Path(item["Path"])
            if relative.is_absolute() or relative.anchor or ".." in relative.parts:
                raise ValueError("Invalid DLC manifest path.")
            checked_copy(dlc / relative, stage / "user-data" / "0000000000000000" /
                         "45410915" / relative, item["SHA256"])

        destination_paths = {"RuntimeRoot": "runtime", "RuntimeReceipt": "runtime/build-receipt.json",
                             "RunRoot": "runs", "UserDataRoot": "user-data"}
        manifest.pop("PreparationRun", None)
        manifest_path = installed / "venom-install.json"
        paths.update({name: str(stage / relative) for name, relative in destination_paths.items()})
        write_record(manifest_path, manifest)
        print("Checking the installed launcher with controller input enabled.", flush=True)
        modes = check_installed_launcher(powershell, installed / "Start-Installed-TU2.ps1")

        paths.update({name: str(output / relative) for name, relative in destination_paths.items()})
        write_record(manifest_path, manifest)
        write_record(stage / LOCAL_INSTALL_RECEIPT, {
            "format": 1, "output": str(output), "installedManifestSha256": file_hash(manifest_path),
        })
        write_record(stage / "build-install-receipt.json", {
            "status": "PASS_EXISTING_BUILD_INSTALL", "runtimeFiles": runtime_files,
            "sourceBuild": str(source_build), "saveUpgrade": migration, "compiled": False,
            "windowModes": modes, "hostRuntimeRebuilt": upgraded_host,
        })
        write_play_commands(stage, powershell, modes)
        check_path(output)
        if output.exists():
            raise ValueError("The selected install destination already exists.")
        stage.rename(output)
    print(f"Installed the existing build. Start it from {output / 'Play Venom.cmd'}", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-folder", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--previous-output", type=Path, required=True)
    try:
        install(parser.parse_args())
    except PreviousOutputRequired as error:
        print(error, file=sys.stderr)
        return 79
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        print(f"Setup needs attention: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())