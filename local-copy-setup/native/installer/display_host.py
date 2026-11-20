"""Build the display-capable SDK runtime from a retained local SDK build."""

from ctypes import POINTER, WinDLL, byref, c_int, c_void_p, c_wchar_p
import os
from pathlib import Path
import re
import shutil
import subprocess

from save_upgrade import check_path, file_hash, read_json

HERE = Path(__file__).resolve().parent
WINDOW_OBJECT = "src/ui/CMakeFiles/rexui.dir/window_sdl.cpp.obj"
WINDOW_SOURCE_HASH = "9607ebfefb9e5ba4a82c2681b5725f0bb52a2263bf5dbcc92e57ea299bbf3f08"


def windows_argv(command):
    shell = WinDLL("shell32")
    kernel = WinDLL("kernel32")
    shell.CommandLineToArgvW.argtypes = [c_wchar_p, POINTER(c_int)]
    shell.CommandLineToArgvW.restype = POINTER(c_wchar_p)
    kernel.LocalFree.argtypes = [c_void_p]
    kernel.LocalFree.restype = c_void_p
    count = c_int()
    ptr = shell.CommandLineToArgvW(command, byref(count))
    if not ptr:
        raise ValueError("Could not parse the retained SDK command.")
    try:
        return [ptr[i] for i in range(count.value)]
    finally:
        kernel.LocalFree(ptr)


def replace_once(text, old, new):
    if text.count(old) != 1:
        raise ValueError("The selected build uses an unsupported SDK or launcher layout.")
    return text.replace(old, new, 1)


def local_tool(raw):
    found = shutil.which(raw)
    if not found:
        raise ValueError(f"The retained SDK tool is unavailable: {Path(raw).name}")
    path = Path(found).resolve(strict=True)
    check_path(path)
    return path


def upgrade_runtime(source_build, original_hash, output, temporary):
    sdk = source_build / "sdk" / "runtime"
    work = sdk / "build"
    stock = sdk / "source" / "out" / "win-amd64" / "rexruntimerd.dll"
    source = sdk / "source" / "src" / "ui" / "window_sdl.cpp"
    for selected in (work, stock, source):
        check_path(selected)
    if (not (work / "build.ninja").is_file() or not stock.is_file() or not source.is_file()
            or file_hash(stock) != original_hash.lower() or file_hash(source) != WINDOW_SOURCE_HASH):
        raise ValueError("For the display update, select the original build folder with its matching SDK files.")
    arguments = read_json(source_build / "pipeline-inputs.json")["args"]
    clang = local_tool(arguments["clang23"])
    ninja = local_tool(arguments["ninja"])
    temporary.mkdir()
    env = dict(os.environ, TEMP=str(temporary), TMP=str(temporary))

    def run(command, captured=False):
        return subprocess.run(command, cwd=work, env=env, check=True, timeout=180,
                              capture_output=captured, text=True).stdout

    commands = run([str(ninja), "-t", "commands", WINDOW_OBJECT], True).splitlines()
    compile_args = windows_argv(commands[-1])
    source_index = compile_args.index("-c") + 1
    object_index = compile_args.index("-o") + 1
    if (Path(compile_args[0]).resolve() != clang or
            Path(compile_args[source_index]).resolve() != source.resolve() or
            (work / compile_args[object_index]).resolve() != (work / WINDOW_OBJECT).resolve()):
        raise ValueError("The retained SDK window compile recipe did not match.")
    candidate_object = temporary / "window_sdl.cpp.obj"
    compile_args[source_index] = str(HERE / "display" / "window_sdl.cpp")
    compile_args[object_index] = str(candidate_object)
    for flag in ("-MT", "-MF"):
        if flag in compile_args:
            index = compile_args.index(flag)
            del compile_args[index:index + 2]
    if "-MD" in compile_args:
        compile_args.remove("-MD")
    print("Building the SDL window support for your existing runtime.", flush=True)
    run(compile_args)

    rule = "CXX_SHARED_LIBRARY_LINKER__rexruntime_RelWithDebInfo"
    import json
    entries = json.loads(run([str(ninja), "-t", "compdb", "-x", rule], True))
    if len(entries) != 1 or (work / entries[0]["output"]).resolve() != stock.resolve():
        raise ValueError("The retained SDK runtime link recipe did not match.")
    _, command, end = entries[0]["command"].split(" && ")
    if end.strip() != 'cd ."':
        raise ValueError("Unknown SDK link command.")
    prefix, inputs = command.split(" -Xlinker /version:0.0 ", 1)
    link_args = windows_argv(prefix)
    if Path(link_args[0]).resolve() != clang:
        raise ValueError("The retained SDK runtime linker differs.")
    inputs = replace_once(inputs, WINDOW_OBJECT, f'"{candidate_object.as_posix()}"')
    candidate = temporary / stock.name
    for flag, name in (("-o", stock.name), ("/implib:", "rexruntimerd.lib"), ("/pdb:", "rexruntimerd.pdb")):
        index = link_args.index(flag) + 1 if flag == "-o" else next(
            i for i, arg in enumerate(link_args) if arg.startswith(flag))
        prior = link_args[index] if flag == "-o" else link_args[index][len(flag):]
        if (work / prior).resolve() != stock.with_name(name).resolve():
            raise ValueError("The retained SDK link output differs.")
        new = str(temporary / name)
        link_args[index] = new if flag == "-o" else flag + new
    response = temporary / "display-link.rsp"
    response.write_text(inputs + "\n", encoding="utf-8")
    run(link_args + ["-Xlinker", "/version:0.0", "@" + str(response)])

    def exports(binary):
        readobj = clang.with_name("llvm-readobj.exe")
        text = run([str(readobj), "--coff-exports", str(binary)], True)
        return re.findall(r"Ordinal: (\d+)\s+Name: ([^\r\n]*)", text)

    before = exports(stock)
    if not before or exports(candidate) != before or file_hash(stock) != original_hash.lower():
        raise ValueError("The display runtime changed the existing export ABI.")
    shutil.copy2(candidate, output)
    return file_hash(output)


def patch_launchers(installed, installed_hashes):
    local = installed / "Start-Local-TU2.ps1"
    entry = installed / "Start-Installed-TU2.ps1"
    parameter = '[ValidateSet("Windowed","Borderless","Fullscreen")][string]$WindowMode = "Windowed",'
    for path in (local, entry):
        code = path.read_text(encoding="utf-8-sig")
        code = replace_once(code, "    [switch]$CheckOnly,",
                            f"    {parameter}\n    [switch]$CheckOnly,")
        if path == entry:
            code = replace_once(code, "'ControllerMappingFile','CheckOnly'",
                                "'ControllerMappingFile','WindowMode','CheckOnly'")
        else:
            code = replace_once(code, "        [int]$FrameRate,",
                                f"        {parameter}\n        [int]$FrameRate,")
            code = replace_once(code, "        '--fullscreen=false',",
                                '        "--fullscreen=$($WindowMode -ne \'Windowed\')",\n'
                                '        "--venom_exclusive_fullscreen=$($WindowMode -eq \'Fullscreen\')",')
            code = replace_once(code, "        -InputBackend $InputBackend " + chr(96),
                                "        -InputBackend $InputBackend " + chr(96) +
                                "\n        -WindowMode $WindowMode " + chr(96))
        path.write_text(code, encoding="utf-8")
        installed_hashes[path.name] = file_hash(path)