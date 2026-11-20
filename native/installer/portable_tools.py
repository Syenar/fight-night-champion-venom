"""Install official, pinned Windows build tools into the private Venom cache."""
from hashlib import file_digest
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


def cached_tool(tool, cache):
    url, checksum, relative = TOOLS[tool]
    folder = cache / "tools" / f"{tool}-{checksum[:12]}"
    exe = folder / relative
    receipt = folder / "venom-tool.json"
    check_path(exe)
    check_path(receipt)
    if not folder.exists():
        return None
    record = json.loads(receipt.read_bytes())
    if (record.get("archiveSha256") != checksum or
            record.get("executables", {}).get(relative) != digest(exe)):
        raise ValueError(f"The cached {tool} files differ from the verified download.")
    for name, expected in record["executables"].items():
        path = member_path(folder, name)
        check_path(path)
        if digest(path) != expected:
            raise ValueError(f"The cached {tool} toolchain changed: {name}")
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
        exe = unpacked / relative
        required = [exe]
        if tool in {"clang23", "clang22"}:
            required.extend(exe.with_name(name) for name in ("clang.exe", "llvm-readobj.exe", "lld-link.exe"))
        if not all(item.is_file() for item in required):
            raise ValueError(f"The official {tool} archive did not have the expected toolchain files.")
        record = {"archiveSha256": checksum, "url": url,
                  "executables": {str(item.relative_to(unpacked).as_posix()): digest(item) for item in required}}
        (unpacked / "venom-tool.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
        check_path(folder)
        try:
            unpacked.rename(folder)
        except FileExistsError:
            # Another installer may commit the same download before this one.
            return cached_tool(tool, cache)
    return cached_tool(tool, cache)
