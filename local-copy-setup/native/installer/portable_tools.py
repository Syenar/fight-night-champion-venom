"""Install official, pinned Windows build tools into the private Venom cache."""
from hashlib import file_digest, sha256
import json
from pathlib import Path
import shutil
import stat
import tarfile
from tempfile import TemporaryDirectory
import urllib.request
import zipfile

from save_upgrade import check_path

TOOLS = {
    "ninja": (
        "https://github.com/ninja-build/ninja/releases/download/v1.13.2/ninja-win.zip",
        "07fc8261b42b20e71d1720b39068c2e14ffcee6396b76fb7a795fb460b78dc65",
        "ninja.exe",
    ),
    "git": (
        "https://github.com/git-for-windows/git/releases/download/v2.56.1.windows.1/MinGit-2.56.1-64-bit.zip",
        "4a92850b6e917da4fd010669b614b55dccf5b0ec333a534cc4be4700dfefdafd",
        "cmd/git.exe",
    ),
    "cmake": (
        "https://github.com/Kitware/CMake/releases/download/v4.2.3/cmake-4.2.3-windows-x86_64.zip",
        "eb4ebf5155dbb05436d675706b2a08189430df58904257ae5e91bcba4c86933c",
        "cmake-4.2.3-windows-x86_64/bin/cmake.exe",
    ),
    "powershell": (
        "https://github.com/PowerShell/PowerShell/releases/download/v7.6.6/PowerShell-7.6.6-win-x64.zip",
        "02fe458be20493fbdf43f61ea20610b811ee6c738ab1676c61b9cfcd1a33c860",
        "pwsh.exe",
    ),
    "clang23": (
        "https://github.com/llvm/llvm-project/releases/download/llvmorg-23.1.4/clang%2Bllvm-23.1.4-x86_64-pc-windows-msvc.tar.xz",
        "854bffcfc84528e0ee92490ff5c178920abd358a79c3e98f053e41e3e8e4c6d2",
        "clang+llvm-23.1.4-x86_64-pc-windows-msvc/bin/clang++.exe",
    ),
    "clang22": (
        "https://github.com/llvm/llvm-project/releases/download/llvmorg-22.1.8/clang%2Bllvm-22.1.8-x86_64-pc-windows-msvc.tar.xz",
        "d96c2cc1736f4eb7fa43cb9bbdf56d93551a9ae0a9aadb9c99c3c3b2b712a234",
        "clang+llvm-22.1.8-x86_64-pc-windows-msvc/bin/clang++.exe",
    ),
}


# SHA-256 of the sorted case-folded paths and binary file hashes from each pinned archive.
TOOL_TREE_HASHES = {
    "07fc8261b42b20e71d1720b39068c2e14ffcee6396b76fb7a795fb460b78dc65": "1a2401bba7ed68a72c39803d3d7c43cdfcb55ceb8df94a76910a99c653de1eaf",  # ninja
    "4a92850b6e917da4fd010669b614b55dccf5b0ec333a534cc4be4700dfefdafd": "5e8a975f40c0706f80d482026b03b0ea69eb23fc311ca4521f3c39edec56a311",  # git
    "eb4ebf5155dbb05436d675706b2a08189430df58904257ae5e91bcba4c86933c": "f6aedf8e95c0b0d8f4c998f833ce9a07e93387ce6b04e18fe3fc50fc2fc52833",  # cmake
    "02fe458be20493fbdf43f61ea20610b811ee6c738ab1676c61b9cfcd1a33c860": "9518cb14e211040f19565cef11434f5841fd2ba12dd108deff28804dfd6e29a7",  # powershell
    "d96c2cc1736f4eb7fa43cb9bbdf56d93551a9ae0a9aadb9c99c3c3b2b712a234": "1fe11c27b602fbb84ccf3ba0f58bba6a0e22fafd202394383668685a47d4d94c",  # clang22
    "854bffcfc84528e0ee92490ff5c178920abd358a79c3e98f053e41e3e8e4c6d2": "cc837decc843cb8de5cc987a897544f8310eec95ffe9c50807248dc543c1566f",  # clang23
}


def digest(path):
    with path.open("rb") as stream:
        return file_digest(stream, "sha256").hexdigest()


def member_path(location, name):
    trimmed = name.rstrip("/")
    parts = trimmed.split("/")
    if (not trimmed or ":" in trimmed or "\\" in trimmed or
            any(part in {"", ".", ".."} or part.endswith((" ", ".")) for part in parts)):
        raise ValueError(f"Invalid portable tool archive member: {name}")
    return location.joinpath(*parts)


def extract(archive, destination, url):
    def transfer(handle, name):
        output = member_path(destination, name)
        output.parent.mkdir(parents=True, exist_ok=True)
        with handle, output.open("xb") as target:
            shutil.copyfileobj(handle, target, length=1024 * 1024)

    if url.endswith(".zip"):
        with zipfile.ZipFile(archive) as folder:
            for member in folder.infolist():
                mode = member.external_attr >> 16
                if stat.S_ISLNK(mode):
                    raise ValueError("The portable tool ZIP contains a symbolic link.")
                if member.is_dir():
                    member_path(destination, member.filename).mkdir(parents=True, exist_ok=True)
                else:
                    transfer(folder.open(member), member.filename)
    else:
        with tarfile.open(archive, mode="r|xz") as folder:
            for member in folder:
                if member.isdir():
                    member_path(destination, member.name).mkdir(parents=True, exist_ok=True)
                elif member.isfile():
                    transfer(folder.extractfile(member), member.name)
                else:
                    raise ValueError(f"Unsupported file type in portable tool archive: {member.name}")


def required_executables(tool, relative):
    if tool in {"clang23", "clang22"}:
        parent = relative.rsplit("/", 1)[0]
        return (relative, *(f"{parent}/{name}" for name in ("clang.exe", "llvm-readobj.exe", "lld-link.exe")))
    return (relative,)


def tool_tree_digest(folder):
    files = {}
    pending = [folder]
    while pending:
        for path in pending.pop().iterdir():
            info = path.lstat()
            if stat.S_ISLNK(info.st_mode) or getattr(info, "st_file_attributes", 0) & stat.FILE_ATTRIBUTE_REPARSE_POINT:
                raise ValueError(f"Redirected tool cache entry: {path}")
            if stat.S_ISDIR(info.st_mode):
                pending.append(path)
                continue
            if not stat.S_ISREG(info.st_mode):
                raise ValueError(f"Unexpected tool cache entry: {path}")
            name = path.relative_to(folder).as_posix().casefold()
            if name == "venom-tool.json":
                continue
            if name in files:
                raise ValueError(f"Duplicate tool cache path: {path}")
            files[name] = digest(path)
    result = sha256()
    for name, file_hash in sorted(files.items()):
        result.update(name.encode("utf-8") + b"\0" + bytes.fromhex(file_hash))
    return result.hexdigest()


def cached_tool(tool, cache):
    url, checksum, relative = TOOLS[tool]
    folder = cache / "tools" / f"{tool}-{checksum[:12]}"
    exe = folder / relative
    receipt = folder / "venom-tool.json"
    check_path(receipt)
    if not folder.exists():
        return None
    try:
        record = json.loads(receipt.read_bytes())
    except (OSError, UnicodeError, ValueError) as error:
        raise ValueError(f"The cached {tool} receipt could not be read: {receipt}") from error
    recorded = record.get("executables") if isinstance(record, dict) else None
    required = required_executables(tool, relative)
    if (not isinstance(recorded, dict) or record.get("archiveSha256") != checksum or
            record.get("url") != url or recorded.keys() != set(required)):
        raise ValueError(f"The cached {tool} receipt does not match the required toolchain: {receipt}")
    for name in required:
        path = member_path(folder, name)
        check_path(path)
        if not path.is_file() or digest(path) != recorded[name]:
            raise ValueError(f"The cached {tool} toolchain changed: {name}")
    if not folder.is_dir() or tool_tree_digest(folder) != TOOL_TREE_HASHES[checksum]:
        raise ValueError(f"The cached {tool} payload differs from the pinned archive: {folder}")
    return exe


def ensure_tool(tool, cache):
    ready = cached_tool(tool, cache)
    if ready:
        return ready
    url, checksum, relative = TOOLS[tool]
    home = cache / "tools"
    check_path(home)
    home.mkdir(parents=True, exist_ok=True)
    folder = home / f"{tool}-{checksum[:12]}"
    print(f"Downloading {tool} from its official release…", flush=True)
    with TemporaryDirectory(prefix=f"{tool}-download-", dir=home) as tmp:
        staging = Path(tmp)
        check_path(staging)
        archive = staging / "download"
        with urllib.request.urlopen(url, timeout=80) as response, archive.open("xb") as output:
            if not response.geturl().startswith("https://"):
                raise ValueError("The official tool download redirected away from HTTPS.")
            shutil.copyfileobj(response, output, length=1024 * 1024)
        if digest(archive) != checksum:
            raise ValueError(f"The {tool} download did not match its SHA-256.")
        unpacked = staging / "unpacked"
        unpacked.mkdir()
        extract(archive, unpacked, url)
        required = required_executables(tool, relative)
        if not all((unpacked / name).is_file() for name in required):
            raise ValueError(f"The official {tool} archive did not have the expected toolchain files.")
        record = {"archiveSha256": checksum, "url": url,
                  "executables": {name: digest(unpacked / name) for name in required}}
        (unpacked / "venom-tool.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
        check_path(folder)
        try:
            unpacked.rename(folder)
        except FileExistsError:
            # Another installer may commit the same download before this one.
            return cached_tool(tool, cache)
    return cached_tool(tool, cache)
