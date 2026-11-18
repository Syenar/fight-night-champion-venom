"""Build the pinned, patched ReXGlue SDK from Git objects only (no game inputs)."""

import argparse
from hashlib import file_digest, sha256
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile


PACKAGE = Path(__file__).resolve().parent


def digest(path, normalized=False):
    if normalized:
        return sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()
    with path.open('rb') as stream:
        return file_digest(stream, 'sha256').hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def durable(path, data):
    if isinstance(data, str):
        data = data.encode('utf-8')
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('wb') as stream:
        stream.write(data)
        stream.flush()
        os.fsync(stream.fileno())
    require(path.read_bytes() == data, f'Write verification failed: {path}')


def save(path, value):
    durable(path, json.dumps(value, indent=2) + '\n')


def git(repository, *args):
    return ['git', '-c', f'safe.directory={repository.as_posix()}', '-C', str(repository), *args]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        '--upstream',
        type=Path,
        required=True,
        help=('Local SDK Git checkout with the locked submodule Git objects; working files are '
         'never copied.'),
    )
    parser.add_argument(
        '--output',
        type=Path,
        required=True,
        help='Fresh output directory on a local build volume.',
    )
    parser.add_argument(
        '--clang23',
        type=Path,
        required=True,
        help='LLVM23.1.4 clang++.exe for runtime/CLI.',
    )
    parser.add_argument('--clang22', type=Path, required=True, help='LLVM22.1.8 clang++.exe for GPU.')
    parser.add_argument('--cmake', default='cmake')
    parser.add_argument('--ninja', default='ninja')
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument(
        '--stage-only',
        action='store_true',
        help='Verify and export sources without configuring or compiling.',
    )
    parser.add_argument(
        '--configure-only',
        action='store_true',
        help='Stage and configure both stacks without compiling.',
    )
    arguments = parser.parse_args()
    require(arguments.jobs > 0, '--jobs must be positive')
    require(
        not (arguments.stage_only and arguments.configure_only),
        'Choose only one staging/configuration mode',
    )
    for name in ('upstream', 'output', 'clang23', 'clang22'):
        setattr(arguments, name, getattr(arguments, name).resolve())
    require(
        not arguments.output.exists(),
        f'Output already exists; retain it and choose a fresh namespace: {arguments.output}',
    )
    lock_path = PACKAGE / 'sdk-source-lock.json'
    lock = json.loads(lock_path.read_text(encoding='utf-8'))
    versions = {}
    for name, expected in (('clang23', '23.1.4'), ('clang22', '22.1.8')):
        compiler = getattr(arguments, name)
        version = subprocess.check_output([str(compiler), '--version'], text=True).splitlines()[0]
        require(f'clang version {expected} ' in version, f'Expected LLVM {expected}: {version}')
        require(compiler.with_name('clang.exe').is_file(), f'Missing C compiler beside {compiler}')
        versions[name] = version
    for stack_name, stack_lock in lock['stacks'].items():
        patch = PACKAGE / stack_lock['patch']
        require(digest(patch) == stack_lock['patchSha256'], f'Patch hash mismatch: {patch}')
        for name, expected in stack_lock.get('overlayFiles', {}).items():
            require(digest(PACKAGE / stack_name / name) == expected, f'Overlay hash mismatch: {name}')
    pins = {'': lock['revision'], **lock['submodules']}
    for submodule_path, revision in pins.items():
        result = subprocess.run(
            git(arguments.upstream / submodule_path, 'cat-file', '-e', revision + '^{tree}'),
            capture_output=True,
        )
        require(
            result.returncode == 0,
            f'Missing pinned Git object {revision} in {arguments.upstream / submodule_path}',
        )
    arguments.output.mkdir(parents=True)
    temp = arguments.output / 'temp'
    temp.mkdir()
    child_environment = dict(os.environ, TEMP=str(temp), TMP=str(temp))
    commands = []

    def execute(command, log_path, working_directory=None):
        commands.append(
            {
                'argv': [str(argument) for argument in command],
                'cwd': str(working_directory) if working_directory else None,
                'log': str(log_path),
            },
        )
        save(arguments.output / 'commands.json', commands)
        with log_path.open('wb') as stream:
            completed_process = subprocess.run(
                [str(argument) for argument in command],
                cwd=working_directory,
                env=child_environment,
                stdout=stream,
                stderr=subprocess.STDOUT,
            )
            stream.flush()
            os.fsync(stream.fileno())
        require(
            completed_process.returncode == 0,
            f'Command exited{completed_process.returncode}; retain evidence: {log_path}',
        )

    def export(repository, revision, destination_root, archive_path):
        with archive_path.open('wb') as stream:
            subprocess.run(
                git(repository, 'archive', '--format=tar', revision),
                stdout=stream,
                env=child_environment,
                check=True,
            )
            stream.flush()
            os.fsync(stream.fileno())
        with tarfile.open(archive_path) as archive_contents:
            archive_contents.extractall(destination_root, filter='data')
        archive_path.unlink()
        subprocess.run(
            ['git', 'init', str(destination_root)],
            env=child_environment,
            check=True,
            capture_output=True,
        )
        objects = subprocess.check_output(
            git(repository, 'rev-parse', '--path-format=absolute', '--git-path', 'objects'),
            text=True,
        ).strip(
        )
        durable(destination_root / '.git/objects/info/alternates', Path(objects).as_posix() + '\n')
        subprocess.run(
            git(destination_root, 'update-ref', '--no-deref', 'HEAD', revision),
            env=child_environment,
            check=True,
            capture_output=True,
        )
    source_directories = {}
    for stack_name in ('runtime', 'gpu'):
        stack_lock = lock['stacks'][stack_name]
        stack_root = arguments.output / stack_name
        source_root = stack_root / 'source'
        source_root.mkdir(parents=True)
        export(arguments.upstream, lock['revision'], source_root, stack_root / 'sdk.tar')
        for submodule_index, (submodule_path, revision) in enumerate(lock['submodules'].items()):
            destination_root = source_root / submodule_path
            destination_root.mkdir(parents=True, exist_ok=True)
            export(
                arguments.upstream / submodule_path,
                revision,
                destination_root,
                stack_root / f'submodule-{submodule_index}.tar',
            )
        for rel in stack_lock['symlinksToMaterialize']:
            path = source_root / rel
            if path.is_symlink():
                target = path.resolve(strict=True)
                require(
                    target.is_relative_to(source_root),
                    f'Symlink target escapes source tree: {rel}',
                )
                content = target.read_bytes()
                path.unlink()
                durable(path, content)
        patch = PACKAGE / stack_lock['patch']
        execute(
            ['git', '-c', 'core.autocrlf=false', '-C', source_root, 'apply', '--check', patch],
            stack_root / 'patch-check.log',
        )
        execute(
            ['git', '-c', 'core.autocrlf=false', '-C', source_root, 'apply', patch],
            stack_root / 'patch-apply.log',
        )
        for entry in stack_lock['files']:
            require(
                digest(source_root / entry['path'], normalized=True) == entry['afterSha256'],
                f"Patched source hash mismatch: {stack_name}/{entry['path']}",
            )
        if stack_name == 'gpu':
            overlays = stack_root / 'overlays'
            for name, expected in stack_lock['overlayFiles'].items():
                target = overlays / name
                durable(target, (PACKAGE / stack_name / name).read_bytes())
                require(digest(target) == expected, f'Copied overlay hash mismatch: {name}')
            cmake_file = source_root / 'src/graphics/CMakeLists.txt'
            durable(
                cmake_file,
                cmake_file.read_bytes() + ('\ninclude("' + (overlays / 'venom-gpu-overlays.cmake').as_posix() + '")\n').encode(
                ),
            )
        source_directories[stack_name] = str(source_root)
        print(f'{stack_name}: pinned source and patch stage passed', flush=True)
    receipt = {
        'origin': lock['origin'],
        'revision': lock['revision'],
        'submodules': lock['submodules'],
        'patchLockSha256': digest(lock_path),
        'sourceOnly': True,
        'networkUsed': False,
        'sources': source_directories,
        'compilers': versions,
        'stagePassed': ['runtime', 'gpu'],
        'buildPassed': [],
        'runtimeTested': False,
        'gameLaunched': False,
    }
    save(arguments.output / 'sdk-stage-receipt.json', receipt)
    if arguments.stage_only:
        return
    cmake = shutil.which(arguments.cmake)
    ninja = shutil.which(arguments.ninja)
    require(cmake and ninja, 'CMake and Ninja must be installed and resolvable')
    for stack_name in ('runtime', 'gpu'):
        stack_root = arguments.output / stack_name
        compiler = arguments.clang23 if stack_name == 'runtime' else arguments.clang22
        configure = [
            cmake,
            '-S',
            stack_root / 'source',
            '-B',
            stack_root / 'build',
            '-G',
            'Ninja',
            '-DCMAKE_BUILD_TYPE=RelWithDebInfo',
            f'-DCMAKE_CXX_COMPILER={compiler.as_posix()}',
            f"-DCMAKE_C_COMPILER={compiler.with_name('clang.exe').as_posix()}",
            f'-DCMAKE_MAKE_PROGRAM={Path(ninja).as_posix()}',
            '-DCMAKE_CXX_FLAGS=-mssse3',
            '-DREXGLUE_USE_D3D12=ON',
            '-DREXGLUE_USE_VULKAN=OFF',
            '-DREXGLUE_BUILD_TESTS=OFF',
            '-DREXGLUE_ENABLE_FIDELITYFX=OFF',
            '-DREXGLUE_ENABLE_TRACY=ON',
            '-DREXGLUE_ENABLE_PERF_COUNTERS=ON',
        ]
        execute(configure, stack_root / 'configure.log')
        print(f'{stack_name}: configure passed', flush=True)
        if arguments.configure_only:
            continue
        targets = ['rexruntime', 'rexglue'] if stack_name == 'runtime' else ['rexgpu-xenos']
        execute(
            [
                cmake,
                '--build',
                stack_root / 'build',
                '--parallel',
                str(arguments.jobs),
                '--target',
                *targets,
            ],
            stack_root / 'build.log',
        )
        receipt['buildPassed'].append(stack_name)
        save(arguments.output / 'sdk-build-progress.json', receipt)
    if arguments.configure_only:
        save(arguments.output / 'sdk-configure-receipt.json', receipt)
        return
    runtime_artifacts = arguments.output / 'runtime/source/out/win-amd64'
    gpu_artifacts = arguments.output / 'gpu/source/out/win-amd64'
    runtime_bundle = arguments.output / 'sdk-bundle'
    runtime_bundle.mkdir()
    dll_hashes = {}
    for name, root in (
        ('rexruntimerd.dll', runtime_artifacts),
        ('TracyClientrd.dll', runtime_artifacts),
        ('rexgpu-xenosrd.dll', gpu_artifacts),
    ):
        durable(runtime_bundle / name, (root / name).read_bytes())
        dll_hashes[name] = digest(runtime_bundle / name)
    receipt.update(
        {
            'cli': str(runtime_artifacts / 'rexgluerd.exe'),
            'cliSha256': digest(runtime_artifacts / 'rexgluerd.exe'),
            'dllSha256': dll_hashes,
            'runtimeBundle': str(runtime_bundle),
            'sdkSource': source_directories['runtime'],
            'sdkBuild': str(arguments.output / 'runtime/build'),
            'sdkLibs': str(runtime_artifacts),
            'qualificationRequired': [
                'owned-input codegen comparison',
                'title link and static ABI',
                'separate runtime acceptance',
            ],
        },
    )
    save(arguments.output / 'sdk-build-receipt.json', receipt)
    print(f"SDK source build complete: {arguments.output / 'sdk-build-receipt.json'}", flush=True)


if __name__ == '__main__':
    main()
