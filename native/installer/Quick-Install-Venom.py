"""Resolve owned inputs and public tools for the normal Windows installer."""
import argparse
from collections import deque
from itertools import islice
from hashlib import file_digest
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
from tempfile import TemporaryDirectory

INSTALLER = Path(__file__).resolve().parent
NATIVE = INSTALLER.parent
PROFILE = json.loads((NATIVE / "title/title-build-profile.json").read_bytes())
SDK = json.loads((NATIVE / "sdk/sdk-source-lock.json").read_bytes())
DLC = json.loads((INSTALLER / "owned-dlc-manifest.json").read_bytes())
REPARSE_POINT = 1024
from save_upgrade import check_path, choose_previous
from portable_tools import cached_tool, ensure_tool

# Match the installed launcher's exact owned startup files before beginning a long build.
STARTUP = {
    "data/xenon/shaders/adaptfixedshader.fxo": "95975518eaaa000e6148a5a59141627a3a48631bb8b98bae5617de6cd5a41109",
    "data/xenon/fe/game/screens/createboxer/boxergrowth/boosts.big": "2a464fadcf5e4a07de5a0eba2c16892b47656bacf96856b01323023057443330",
    "datadlc/dlc_pack1/dlc/data/xenon/loc/eng/speech/speech_eng_dlc_samples.sbr": "13ee8bf8835b9f6f11d6b9a4599d094bdb4452da3002df2471574d747f276d6e",
    "datadlc/dlc_pack1/dlc/data/xenon/loc/eng/speech/speech_eng_dlc_samples.sbs": "71c9602d5982f47f2095da6de1bff276e30739a6430a1ead78066d918b172dd2",
}


def digest(path):
    with path.open("rb") as file:
        return file_digest(file, "sha256").hexdigest()


def directories(root):
    """Check a selected folder and nearby standard layouts, with a fixed search limit."""
    def list_children(path):
        try:
            for child in sorted(path.iterdir(), key=lambda entry: entry.name.casefold()):
                if (child.is_dir() and not child.is_symlink() and
                        not getattr(child.lstat(), "st_file_attributes", 0) & REPARSE_POINT):
                    yield child
        except OSError:
            return

    queue = deque([(root, 0)])
    if root.parent != Path(root.anchor):
        for name in ("TU2", "Update", "Title Update", "DLC", "Content", "45410915"):
            neighbor = root.parent / name
            if neighbor.is_dir() and not getattr(neighbor.lstat(), "st_file_attributes", 0) & REPARSE_POINT:
                queue.append((neighbor, 0))
    seen = set()
    while queue and len(seen) < 500:
        path, depth = queue.popleft()
        key = str(path).casefold()
        if key in seen:
            continue
        seen.add(key)
        yield path
        if depth < 4:
            children = (child for child in list_children(path)
                        if child.name.casefold() not in {"data", "datadlc", ".git", "venom builds"})
            queue.extend((child, depth + 1) for child in islice(children, max(0, 500 - len(queue) - len(seen))))


def media(root):
    retail, update, dlc = [], [], []
    hashes = PROFILE["inputHashes"]
    for path in directories(root):
        if (path / "default.xex").is_file() and digest(path / "default.xex") == hashes["retailXex"]:
            retail.append(path)
        if (path / "default.xexp").is_file() and digest(path / "default.xexp") == hashes["tu2Xexp"]:
            update.append(path)
        if (path / "00000002").is_dir() and all(
                (path / row["Path"]).is_file() and digest(path / row["Path"]) == row["SHA256"].lower()
                for row in DLC):
            dlc.append(path)
    missing = [label for label, paths in (("retail game", retail), ("TU2 update", update),
                                          ("owned DLC", dlc)) if not paths]
    if missing:
        raise ValueError("Could not find verified " + ", ".join(missing) +
                         ". Select the parent containing your extracted files, or use Advanced.")
    # Identical retail code can accompany different extracted data; defer that choice.
    if root in retail:
        game = root
    elif len(retail) == 1:
        game = retail[0]
    else:
        raise ValueError("Multiple retail folders match. Select the extracted retail game folder directly.")
    for relative, expected in STARTUP.items():
        path = game / relative
        if not path.is_file() or digest(path) != expected:
            raise ValueError(f"The extracted retail game is missing the verified file {relative}.")
    sibling = game / "default.xexp"
    if sibling.exists() and (not sibling.is_file() or digest(sibling) != hashes["tu2Xexp"]):
        raise ValueError("The update beside default.xex differs from the required title update.")
    return game, next((path for path in update if path != game), update[0]), dlc[0]


def existing(command, *locations):
    for name in (*locations, shutil.which(command)):
        if name:
            path = Path(name)
            if path.is_absolute() and path.is_file() and not getattr(path.lstat(), "st_file_attributes", 0) & REPARSE_POINT:
                return path
    return None


def llvm(version, variable):
    user = Path(os.environ.get("USERPROFILE", "C:/Users/Default"))
    programs = Path(os.environ.get("ProgramFiles", "C:/Program Files"))
    roots = (
        user / "scoop/apps/llvm" / version / "bin",
        programs / f"LLVM-{version}" / "bin",
        programs / "LLVM/bin",
        Path(f"C:/LLVM{version.split('.')[0]}/bin"),
        Path("C:/LLVM/bin"),
    )
    on_path = (Path(item.strip('"')) / "clang++.exe" for item in os.environ.get("PATH", "").split(os.pathsep))
    for candidate in (os.environ.get(variable), *(root / "clang++.exe" for root in roots), *on_path):
        if not candidate:
            continue
        path = Path(candidate)
        if not path.is_absolute() or not path.is_file():
            continue
        if not all(path.with_name(tool).is_file() for tool in ("clang.exe", "llvm-readobj.exe", "lld-link.exe")):
            continue
        try:
            output = subprocess.check_output([str(path), "--version"], stderr=subprocess.DEVNULL,
                                             text=True, timeout=8)
        except (OSError, subprocess.SubprocessError):
            continue
        if re.search(r"\bclang version " + re.escape(version) + r"(?=\s|$)", output):
            return path
    return None


def compiler_tools(cache, check_only):
    pf = Path(os.environ.get("ProgramFiles", "C:/Program Files"))
    tools = {
        "git": existing("git", pf / "Git/cmd/git.exe"),
        "cmake": existing("cmake", pf / "CMake/bin/cmake.exe"),
        "ninja": existing("ninja"),
        "powershell": existing("pwsh", pf / "PowerShell/7/pwsh.exe"),
        "clang23": llvm("23.1.4", "VENOM_QUICK_LLVM23"),
        "clang22": llvm("22.1.8", "VENOM_QUICK_LLVM22"),
    }
    labels = {"git": "Git", "cmake": "CMake", "ninja": "Ninja", "powershell": "PowerShell 7",
              "clang23": "LLVM 23.1.4", "clang22": "LLVM 22.1.8"}
    for key in tools:
        cached = cached_tool(key, cache)
        if cached is not None:
            tools[key] = cached
    missing = [key for key, path in tools.items() if path is None]
    if missing and check_only:
        print("Tools to download: " + ", ".join(labels[key] for key in missing), flush=True)
    for key in missing:
        if not check_only:
            tools[key] = ensure_tool(key, cache)
    return tools


class MissingMicrosoftTools(Exception):
    """The installer needs a system C++ toolchain and Windows SDK."""


def check_microsoft_tools():
    programs = Path(os.environ.get("ProgramFiles(x86)", "C:/Program Files (x86)"))
    developer_msvc = os.environ.get("VCToolsInstallDir")
    developer_sdk = os.environ.get("WindowsSdkDir")
    kits = Path(developer_sdk) if developer_sdk else programs / "Windows Kits/10"
    visual_studio = programs / "Microsoft Visual Studio"
    msvc = list(visual_studio.glob("*/*/VC/Tools/MSVC/*"))
    if developer_msvc:
        msvc.append(Path(developer_msvc))
    vswhere = visual_studio / "Installer/vswhere.exe"
    if vswhere.is_file():
        probe = subprocess.run(
            [str(vswhere), "-products", "*", "-requires",
             "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"],
            text=True, encoding="utf-8", errors="replace", capture_output=True, timeout=12)
        if probe.returncode == 0:
            for line in probe.stdout.splitlines():
                msvc.extend((Path(line) / "VC/Tools/MSVC").glob("*"))
    present = any((path / "bin/Hostx64/x64/cl.exe").is_file()
                  and (path / "lib/x64/libcpmt.lib").is_file()
                  and (path / "include/vector").is_file() for path in msvc)
    sdk = any((version / "ucrt/x64/ucrt.lib").is_file()
              and (version / "um/x64/kernel32.lib").is_file()
              and (kits / "Include" / version.name / "um/Windows.h").is_file()
              for version in (kits / "Lib").glob("10.0.*"))
    if not present or not sdk:
        raise MissingMicrosoftTools("Install Microsoft C++ Build Tools with the Desktop development "
                                   "with C++ workload and a Windows 10 or 11 SDK, then retry.")

def sdk_checkout(git, cache, env):
    configured = os.environ.get("VENOM_QUICK_SDK")
    if configured:
        return Path(configured).resolve(strict=True)
    checkout = cache / ("r-" + SDK["revision"][:8])
    check_path(checkout)
    if checkout.is_dir():
        verified = subprocess.run([str(git), "-C", str(checkout), "cat-file", "-e",
                                   SDK["revision"] + "^{commit}"], env=env, capture_output=True)
        if verified.returncode == 0:
            return checkout
        raise ValueError("A previous SDK cache is incomplete. Choose a different SDK in Advanced.")
    cache.mkdir(parents=True, exist_ok=True)
    print("Downloading the public ReXGlue SDK source…", flush=True)
    with TemporaryDirectory(dir=cache, prefix="") as staging:
        temp = Path(staging)
        subprocess.run([str(git), "clone", "--no-checkout", SDK["origin"], str(temp)],
                       check=True, env=env)
        subprocess.run([str(git), "-C", str(temp), "checkout", "--detach", SDK["revision"]],
                       check=True, env=env)
        subprocess.run([str(git), "-C", str(temp), "submodule", "update", "--init", "--recursive", "--jobs", "4"],
                       check=True, env=env)
        temp.rename(checkout)
    return checkout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-folder", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--check", action="store_true", help="Resolve inputs without preparing an install.")
    args = parser.parse_args()
    root = args.game_folder.resolve(strict=True)
    output = args.output.resolve()
    cache = output.parent / ".venom-inputs"
    if not root.is_dir() or output.exists():
        raise ValueError("Select an extracted game folder and a fresh installation output.")
    check_path(args.output.absolute())
    check_path(cache)
    print("Checking your game folder…", flush=True)
    game, update, dlc = media(root)
    for path in (game, update, dlc, INSTALLER.parent.parent):
        check_path(path)
        if any(target.is_relative_to(path) or path.is_relative_to(target) for target in (output, cache)):
            raise ValueError("The installation output and cache must be separate from game files and this package.")
    if output.parent.is_dir():
        choose_previous(output, None)
    print("Found verified game, TU2 and owned DLC. Checking build tools…", flush=True)
    check_microsoft_tools()
    tools = compiler_tools(cache, args.check)
    if args.check:
        print(json.dumps({"game": str(game), "update": str(update), "dlc": str(dlc),
                          "tools": {key: str(path) if path is not None else None for key, path in tools.items()},
                          "requiresUpdateStaging": game == update}))
        return 0
    # The backend resolves Git in child PATH. Keep changes local to this build.
    env = dict(os.environ, PATH=str(tools["git"].parent) + os.pathsep + os.environ["PATH"])
    # Git for Windows must propagate long-path support into submodule and pack helpers.
    # Add it only to this installer's child environment.
    index = int(env.get("GIT_CONFIG_COUNT", "0"))
    env[f"GIT_CONFIG_KEY_{index}"] = "core.longpaths"
    env[f"GIT_CONFIG_VALUE_{index}"] = "true"
    env["GIT_CONFIG_COUNT"] = str(index + 1)
    if update == game:
        update = cache / ("tu2-" + PROFILE["inputHashes"]["tu2Xexp"][:12])
        final = update / "default.xexp"
        check_path(final)
        update.mkdir(parents=True, exist_ok=True)
        if not final.exists():
            with TemporaryDirectory(dir=update, prefix="tu2-copy-") as temporary:
                copied = Path(temporary) / final.name
                shutil.copy2(game / final.name, copied)
                if digest(copied) != PROFILE["inputHashes"]["tu2Xexp"]:
                    raise ValueError("The owned title update changed while it was copied.")
                try:
                    copied.rename(final)
                except FileExistsError:
                    pass
        check_path(final)
        if digest(final) != PROFILE["inputHashes"]["tu2Xexp"]:
            raise ValueError("A staged title update does not match the owned input.")
    sdk = sdk_checkout(tools["git"], cache, env)
    # The embeddable runtime does not add the script directory to sys.path.
    entry = ("import runpy,sys; from pathlib import Path; script=sys.argv.pop(1); "
             "sys.argv[0]=script; sys.path.insert(0,str(Path(script).parent)); "
             "runpy.run_path(script,run_name='__main__')")
    cmd = [sys.executable, "-u", "-B", "-c", entry, str(INSTALLER / "Build-Install-Venom.py")]
    for name, value in {
        "sdk-checkout": sdk, "game-data-root": game, "update-data-root": update,
        "dlc-title-root": dlc, "output": output, "jobs": "2",
        **{key: tools[key] for key in ("clang23", "clang22", "cmake", "ninja", "powershell")},
    }.items():
        cmd.extend(("--" + name, str(value)))
    return subprocess.run(cmd, env=env, check=False).returncode


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except MissingMicrosoftTools as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(78)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(f"Setup needs attention: {error}", file=sys.stderr)
        raise SystemExit(1)
