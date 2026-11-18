from concurrent.futures import ThreadPoolExecutor, as_completed
from hashlib import file_digest
import argparse
import json
import os
from pathlib import Path
import shutil
from string import Template
import subprocess
import sys

PACKAGE = Path(__file__).resolve().parent
TITLE = "venom_fnc_tu2_dlc.exe"
RUNTIME_FILES = ("rexruntimerd.dll", "rexgpu-xenosrd.dll", "TracyClientrd.dll")

def digest(path):
    with path.open("rb") as stream:
        return file_digest(stream, "sha256").hexdigest()

def sync_file(path):
    with path.open("rb+") as stream:
        stream.flush()
        os.fsync(stream.fileno())

def durable_text(path, text):
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        stream.write(text)
        stream.flush()
        os.fsync(stream.fileno())

def require_file(path, expected_hash=None):
    if not path.is_file():
        raise FileNotFoundError(path)
    if expected_hash and digest(path) != expected_hash:
        raise ValueError(f"Unexpected SHA-256 for {path}")

def quoted(path):
    return '"' + path.as_posix() + '"'

def arguments():
    parser = argparse.ArgumentParser(description="Generate and build the Venom TU2 Windows title from owned inputs.")
    parser.add_argument("--retail-xex", type=Path, required=True)
    parser.add_argument("--tu2-xexp", type=Path, required=True)
    parser.add_argument("--rexglue", type=Path, required=True, help="Qualified ReXGlue CLI with its DLLs beside it.")
    parser.add_argument("--sdk-source", type=Path, required=True, help="SDK source and populated third-party directories.")
    parser.add_argument("--sdk-build", type=Path, required=True, help="SDK generated include/ and thirdparty/sdl3/include-revision/.")
    parser.add_argument("--sdk-libs", type=Path, required=True, help="Directory holding the qualified SDK link libraries.")
    parser.add_argument("--runtime", type=Path, required=True, help="Directory holding the qualified runtime/GPU/Tracy DLLs.")
    parser.add_argument("--clang23", type=Path, required=True, help="LLVM 23.1.4 clang++.exe.")
    parser.add_argument("--clang22", type=Path, required=True, help="LLVM 22.1.8 clang++.exe and sibling lld-link.exe.")
    parser.add_argument("--output", type=Path, required=True, help="Fresh local output directory.")
    parser.add_argument("--jobs", type=int, default=6)
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    for name, value in vars(args).items():
        if isinstance(value, Path):
            setattr(args, name, value.resolve())
    return args

def execute(argv, cwd, env, log):
    with log.open("wb") as stream:
        result = subprocess.run(argv, cwd=cwd, env=env, stdout=stream, stderr=subprocess.STDOUT)
        stream.flush()
        os.fsync(stream.fileno())
    if result.returncode:
        raise RuntimeError(f"{Path(argv[0]).name} exited {result.returncode}. See {log}")

def compiler_version(path, expected):
    line = subprocess.check_output([str(path), "--version"], text=True, encoding="utf-8").splitlines()[0]
    if f"clang version {expected} " not in line:
        raise ValueError(f"{path} reports {line}. Expected LLVM {expected}.")
    return line

def main():
    args = arguments()
    profile_path = PACKAGE / "title-build-profile.json"
    profile = json.loads(profile_path.read_text(encoding="utf-8"))
    for path, key in ((args.retail_xex, "retailXex"), (args.tu2_xexp, "tu2Xexp"), (args.rexglue, "rexglue")):
        require_file(path, profile["inputHashes"][key])
    for name, expected in profile["sdkLibraries"].items():
        require_file(args.sdk_libs / name, expected)
    for name in RUNTIME_FILES:
        require_file(args.runtime / name, profile["runtimeFiles"][name])
    for path in (args.sdk_source / "include/rex/rex_app.h",
                 args.sdk_source / "src/ui/windowed_app_main_sdl.cpp",
                 args.sdk_source / "src/ui/rex_app.cpp",
                 args.sdk_build / "include/rex/version.h"):
        require_file(path)
    compilers = {"clang22": args.clang22, "clang23": args.clang23}
    versions = {key: compiler_version(path, profile["compilerVersions"][key]) for key, path in compilers.items()}
    work = args.output
    work.mkdir(parents=True, exist_ok=False)
    project = work / "project"
    assets = project / "assets"
    generated = project / "generated/default"
    objects = work / "objects"
    responses = work / "responses"
    logs = work / "logs"
    temporary = work / "temp"
    for folder in (assets, generated, objects, responses, logs, temporary):
        folder.mkdir(parents=True)
    env = dict(os.environ, TEMP=str(temporary), TMP=str(temporary))

    shutil.copyfile(args.retail_xex, assets / "default.xex")
    shutil.copyfile(args.tu2_xexp, assets / "default.xexp")
    shutil.copytree(PACKAGE / "src", project / "src")
    manifest = PACKAGE / "venom_fnc_canonical_tu2_manifest.toml"
    shutil.copyfile(manifest, project / manifest.name)
    seed = PACKAGE / "codegen.partition.json"
    shutil.copyfile(seed, generated / seed.name)
    print("Applying TU2 and generating title sources locally.", flush=True)
    execute([str(args.rexglue), "codegen", manifest.name], project, env, logs / "codegen.log")
    execute([sys.executable, "-B", str(PACKAGE / "apply-author-hooks.py"), str(generated)],
            project, env, logs / "author-hooks.log")

    for generated_file in generated.iterdir():
        if generated_file.is_file(): sync_file(generated_file)

    expected_sources = {entry["file"] for entry in profile["objects"] if entry["kind"] == "generated"}
    actual_sources = {path.name for path in generated.glob("*.cpp")}
    if actual_sources != expected_sources:
        raise ValueError("Generated translation units differ from the build profile")
    pch_header = work / "guest-pch.hxx"
    pch_input = work / "guest-pch.cxx"
    pch = objects / "guest-pch.pch"
    pch_header.write_text(f'#pragma clang system_header\n#include {quoted(generated / "venom_fnc_canonical_tu2_pch.h")}\n',
                          encoding="utf-8")
    pch_input.write_text("// Guest precompiled header translation unit.\n", encoding="utf-8")
    locations = {
        "sdkSource": args.sdk_source.as_posix(), "sdkBuild": args.sdk_build.as_posix(),
        "sdkLibs": args.sdk_libs.as_posix(), "project": project.as_posix(),
        "pchHeader": pch_header.as_posix(), "pch": pch.as_posix(),
    }

    def response_flags(name):
        return Template(profile["compileProfiles"][name]).substitute(locations)

    def compile_file(compiler, flags, src, result, name):
        response = responses / f"{name}.rsp"
        durable_text(response, f"{flags} -o {quoted(result)} -c {quoted(src)}\n")
        execute([str(compiler), "@" + response.as_posix()], work, env, logs / f"{name}.log")
        sync_file(result)

    compile_file(args.clang23, response_flags("precompile"), pch_input, pch, "precompile")
    source_roots = {"generated": generated, "host": project / "src", "sdk-ui": args.sdk_source / "src/ui"}
    inventory = []
    for index, entry in enumerate(profile["objects"]):
        src = source_roots[entry["kind"]] / entry["file"]
        require_file(src)
        name = f"{index:03d}-{src.name}"
        result = objects / f"{name}.obj"
        inventory.append((entry, src, name, result))

    def compile_unit(job):
        entry, src, name, result = job
        compile_file(compilers[entry["compiler"]], response_flags(entry["profile"]), src, result, name)
        return {"source": str(src), "sourceSha256": digest(src), "object": str(result), "objectSha256": digest(result)}

    print(f"Compiling {len(inventory)} title translation units.", flush=True)
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = [pool.submit(compile_unit, job) for job in inventory]
        for done, future in enumerate(as_completed(futures), 1):
            try:
                future.result()
            except Exception:
                for pending in futures:
                    pending.cancel()
                raise
            if done % 25 == 0 or done == len(futures):
                print(f"{done}/{len(futures)} compiled", flush=True)
    compiled = [future.result() for future in futures]

    title = work / TITLE
    response = responses / "link.rsp"
    link_objects = " ".join(quoted(job[3]) for job in inventory)
    libs = Template(profile["linkLibraries"]).substitute(locations)
    link_flags = profile["linkFlags"]
    durable_text(response, f"{link_flags} {link_objects} {libs} -o {quoted(title)} "
                        f'-Xlinker /MANIFEST:EMBED -Xlinker "/implib:{title.with_suffix(".lib").as_posix()}" '
                        f'-Xlinker "/pdb:{title.with_suffix(".pdb").as_posix()}" -Xlinker /version:0.0\n')
    execute([str(args.clang22), "@" + response.as_posix()], work, env, logs / "link.log")
    sync_file(title)
    for name in RUNTIME_FILES:
        shutil.copyfile(args.runtime / name, work / name)
        sync_file(work / name)
        require_file(work / name, profile["runtimeFiles"][name])
    receipt = {
        "inputs": profile["inputHashes"], "sdkLibraries": profile["sdkLibraries"],
        "sdkUiSourceHashes": {item["source"]: item["sourceSha256"] for item in compiled if item["source"].startswith(str(args.sdk_source))},
        "runtimeFiles": profile["runtimeFiles"], "compilers": versions,
        "manifestSha256": digest(manifest), "partitionSeedSha256": digest(seed),
        "hookSpecSha256": digest(PACKAGE / "author-hook-transforms.json"), "buildProfileSha256": digest(profile_path),
        "compiledObjects": compiled, "executedLinkResponseSha256": digest(response),
        "titleSha256": digest(title), "runtimeTested": False,
    }
    durable_text(work / "build-receipt.json", json.dumps(receipt, indent=2) + "\n")
    print(f"Built {title} with SHA-256 {receipt['titleSha256']}", flush=True)

if __name__ == "__main__":
    main()
