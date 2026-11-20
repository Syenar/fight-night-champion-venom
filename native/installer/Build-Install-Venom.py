"""Build the pinned Venom SDK, helper and exact TU2 title, then install without launching."""

import argparse
from hashlib import file_digest, sha256
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import datetime

from save_upgrade import prepare_saves


PACKAGE = Path(__file__).resolve().parent
NATIVE = PACKAGE.parent
TITLE = 'venom_fnc_tu2_dlc.exe'
RUNTIME = ('rexruntimerd.dll', 'rexgpu-xenosrd.dll', 'TracyClientrd.dll')
APT_FILES = ('aptpatch_display.py', 'ebbig.py', 'refpack_enc.py', 'refpack.py')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def digest(path):
    with path.open('rb') as stream:
        return file_digest(stream, 'sha256').hexdigest()


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


def load(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def expect(path, expected_sha256):
    require(
        bool(re.fullmatch('[a-fA-F0-9]{64}', expected_sha256 or '')),
        f'Invalid expected hash: {path}',
    )
    actual_sha256 = digest(path)
    require(actual_sha256 == expected_sha256.lower(), f'Hash mismatch: {path}: {actual_sha256}')
    return actual_sha256


def inspect_abi(readobj, artifact_paths, logs_root):
    exports = {}
    imports = {}
    artifact_records = {}
    for path in artifact_paths:
        inspection_output = subprocess.check_output(
            [str(readobj), '--file-headers', '--coff-imports', '--coff-exports', str(path)],
        )
        durable(logs_root / f'{path.name}.abi.txt', inspection_output)
        inspection_text = inspection_output.decode('utf-8')
        require('IMAGE_FILE_MACHINE_AMD64' in inspection_text, f'Expected AMD64: {path}')
        exports[path.name.lower()] = {
            match.group(1)
            for match in re.finditer('Export \\{\\s*Ordinal: \\d+\\s*Name: ([^\\n]+)', inspection_text)
        }
        imports[path.name.lower()] = {}
        for block in re.findall('Import \\{(.*?)\\n\\}', inspection_text, re.S):
            name = re.search('\\n  Name: ([^\\n]+)', block).group(1).lower()
            imports[path.name.lower()][name] = re.findall('\\n  Symbol: (.*?) \\(\\d+\\)', block)
        artifact_records[path.name] = {
            'sha256': digest(path),
            'length': path.stat().st_size,
            'abiLogSha256': sha256(inspection_output).hexdigest(),
        }
    resolved = []
    for consumer, modules in imports.items():
        for provider, imported_symbols in modules.items():
            if provider not in exports:
                continue
            missing_symbols = sorted(set(imported_symbols) - exports[provider])
            require(
                not missing_symbols,
                f'Unresolved imports: {consumer}->{provider}:{missing_symbols}',
            )
            resolved.append(
                {
                    'consumer': consumer,
                    'provider': provider,
                    'count': len(imported_symbols),
                    'unresolved': [],
                },
            )
    return {
        'files': artifact_records,
        'privateImports': resolved,
        'imports': imports,
        'exportCounts': {k: len(v) for k, v in exports.items()},
    }


def verify_runtime_imports(abi, expected_by_consumer, consumer):
    provider = 'rexruntimerd.dll'
    actual = abi['imports'].get(consumer, {}).get(provider, [])
    expected = expected_by_consumer[consumer]
    actual_set, expected_set = set(actual), set(expected)
    require(
        bool(expected) and len(expected_set) == len(expected),
        f'Invalid pinned runtime import list for {consumer}',
    )
    require(
        actual_set == expected_set and len(actual) == len(expected),
        f'Runtime imports differ for {consumer}: '
        f'missing={sorted(expected_set - actual_set)}, '
        f'unexpected={sorted(actual_set - expected_set)}, '
        f'expected count={len(expected)}, actual count={len(actual)}',
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name, helptext in [
        (
            'sdk-checkout',
            'Local pinned SDK checkout with its populated submodule Git objects; read only.',
        ),
        (
            'game-data-root',
            'Caller-owned extracted game root containing the exact retail default.xex.',
        ),
        ('update-data-root', 'Caller-owned verified TU2 root containing default.xexp.'),
        (
            'dlc-title-root',
            'Caller-owned title 45410915 directory containing 00000002 and Headers.',
        ),
        ('apt-tools-root', 'Caller-provided aptpatch_display.py,refpack_enc.py,andrefpack.py.'),
        (
            'output',
            'Fresh private build/install output; all generated code and binaries remain here.',
        ),
        ('clang23', 'LLVM 23.1.4 clang++.exe.'),
        ('clang22', 'LLVM 22.1.8 clang++.exe.'),
    ]:
        if name == 'apt-tools-root':
            parser.add_argument(
                '--' + name,
                type=Path,
                default=PACKAGE / 'apt-tools',
                help='Optional locked APT source directory (default: bundled apt-tools).',
            )
        else:
            parser.add_argument('--' + name, type=Path, required=True, help=helptext)
    parser.add_argument(
        '--install-root',
        type=Path,
        help='Fresh installation directory inside output (default:output/installed).',
    )
    parser.add_argument(
        '--previous-output',
        type=Path,
        help='Optional earlier Venom output folder. A single existing install beside the new output is detected automatically.',
    )
    parser.add_argument('--cmake', default='cmake')
    parser.add_argument('--ninja', default='ninja')
    parser.add_argument('--powershell', default='pwsh')
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument(
        '--resume',
        action='store_true',
        help=('Reuse completed, hash-verified stages; equivalent APT sources and a fresh '
         'install child may be rebound.'),
    )
    arguments = parser.parse_args()
    require(arguments.jobs > 0, '--jobs must be positive')
    for name, value in vars(arguments).items():
        if isinstance(value, Path) and name != 'previous_output':
            setattr(arguments, name, value.resolve())
    output_root = arguments.output
    arguments.install_root = arguments.install_root or output_root / 'installed'
    require(
        arguments.install_root.is_relative_to(output_root),
        'Install root must stay within the private output',
    )
    for owned in (
        arguments.game_data_root,
        arguments.update_data_root,
        arguments.dlc_title_root,
        arguments.sdk_checkout,
        arguments.apt_tools_root,
    ):
        require(
            not output_root.is_relative_to(owned) and (not owned.is_relative_to(output_root)),
            f'Output overlaps read-only input:{owned}',
        )
    require(
        arguments.game_data_root != arguments.update_data_root,
        'Game and update roots must be distinct',
    )
    for ancestor in (output_root, *output_root.parents):
        if ancestor.exists():
            require(
                not getattr(ancestor.lstat(), 'st_file_attributes', 0) & 1024,
                f'Output path redirection:{ancestor}',
            )
    base_profile_path = NATIVE / 'title/title-build-profile.json'
    base_title_profile = load(base_profile_path)
    sdk_source_lock_path = NATIVE / 'sdk/sdk-source-lock.json'
    runtime_import_lock_path = PACKAGE / 'runtime-import-lock.json'
    runtime_import_lock = load(runtime_import_lock_path)
    require(runtime_import_lock['format'] == 1, 'Unexpected ABI lock format')
    require(
        runtime_import_lock['sdkSourceLockSha256'] == digest(sdk_source_lock_path),
        'Runtime import lock does not match the SDK source lock',
    )
    expected_runtime_imports = runtime_import_lock['rexruntimerd.dll']
    retail_xex = arguments.game_data_root / 'default.xex'
    tu2_patch = arguments.update_data_root / 'default.xexp'
    expect(retail_xex, base_title_profile['inputHashes']['retailXex'])
    expect(tu2_patch, base_title_profile['inputHashes']['tu2Xexp'])
    apt_lock_path = PACKAGE / 'apt-tools/source-lock.json'
    apt_lock = load(apt_lock_path)
    require(
        set(apt_lock['BundledSha256']) == set(APT_FILES),
        'APT source lock must bind all four helpers',
    )
    for name, expected_hash in apt_lock['BundledSha256'].items():
        expect(PACKAGE / 'apt-tools' / name, expected_hash)
        expect(arguments.apt_tools_root / name, expected_hash)
    dlc_manifest = load(PACKAGE / 'owned-dlc-manifest.json')
    for entry in dlc_manifest:
        rel = Path(entry['Path'])
        require(not rel.is_absolute() and '..' not in rel.parts, f'Invalid DLC manifest path: {rel}')
        expect(arguments.dlc_title_root / rel, entry['SHA256'])
    input_identity = {
        'args': {
            k: str(v) if isinstance(v, Path) else v
            for (k, v) in vars(arguments).items()
            if k not in ('resume', 'previous_output')
        },
        'retailSha256': digest(retail_xex),
        'tu2Sha256': digest(tu2_patch),
        'sdkLockSha256': digest(sdk_source_lock_path),
        'runtimeImportLockSha256': digest(runtime_import_lock_path),
        'immutableTitleProfileSha256': digest(base_profile_path),
        'dlcManifestSha256': digest(PACKAGE / 'owned-dlc-manifest.json'),
        'aptToolsSha256': {
            runtime_filename: digest(arguments.apt_tools_root / runtime_filename)
            for runtime_filename in ('aptpatch_display.py', 'refpack_enc.py', 'refpack.py')
        },
    }
    if output_root.exists():
        require(
            arguments.resume,
            ('Existing output refused; choose a fresh output or explicitly resume '
             'completed stages'),
        )
        previous_input_identity = load(output_root / 'pipeline-inputs.json')
        comparison_identity = json.loads(json.dumps(previous_input_identity))
        input_changes = []
        for argument_name in ('apt_tools_root', 'install_root'):
            if comparison_identity['args'][argument_name] != input_identity['args'][argument_name]:
                input_changes.append(
                    {
                        'field': 'args.' + argument_name,
                        'before': comparison_identity['args'][argument_name],
                        'after': input_identity['args'][argument_name],
                    },
                )
                comparison_identity['args'][argument_name] = input_identity['args'][argument_name]
        require(
            comparison_identity == input_identity,
            'Resume compile/media inputs differ from the original invocation',
        )
        if input_changes:
            if any((change['field'] == 'args.install_root' for change in input_changes)):
                require(
                    not arguments.install_root.exists(),
                    'A rebound install destination must be fresh',
                )
            before_sha = digest(output_root / 'pipeline-inputs.json')
            durable(
                output_root / f'pipeline-inputs-before-{before_sha[:12]}.json',
                (output_root / 'pipeline-inputs.json').read_bytes(),
            )
            if (output_root / 'build-install-receipt.json').exists():
                prior_receipt_sha = digest(output_root / 'build-install-receipt.json')
                durable(
                    output_root / f'build-install-receipt-before-{prior_receipt_sha[:12]}.json',
                    (output_root / 'build-install-receipt.json').read_bytes(),
                )
            save(
                output_root / f'input-rebinding-{before_sha[:12]}.json',
                {
                    'changes': input_changes,
                    'preservedInputsSha256': before_sha,
                    'unchangedCompileAndMediaIdentity': True,
                    'aptSourceLockSha256': digest(apt_lock_path),
                    'allFourNewAptSourcesMatchLock': True,
                },
            )
            save(output_root / 'pipeline-inputs.json', input_identity)
    else:
        require(not arguments.resume, 'Cannot resume a missing output')
        output_root.mkdir(parents=True)
        save(output_root / 'pipeline-inputs.json', input_identity)
    temp_root = output_root / 'temp'
    temp_root.mkdir(exist_ok=True)
    logs_root = output_root / 'logs'
    logs_root.mkdir(exist_ok=True)
    child_environment = dict(os.environ, TEMP=str(temp_root), TMP=str(temp_root))
    staged_apt = output_root / 'apt-tools'
    staged_apt.mkdir(exist_ok=True)
    for name in (*APT_FILES, 'LICENSE', 'NOTICE', 'source-lock.json', 'README.txt'):
        source_path = arguments.apt_tools_root / name if name in APT_FILES else PACKAGE / 'apt-tools' / name
        target_path = staged_apt / name
        if target_path.exists():
            expect(target_path, digest(source_path))
        else:
            durable(target_path, source_path.read_bytes())
    save(
        output_root / 'apt-tools-validation.json',
        {
            'sourceRoot': str(arguments.apt_tools_root),
            'installedToolRoot': str(staged_apt),
            'sourceLockSha256': digest(apt_lock_path),
            'helperSha256': {name: digest(staged_apt / name) for name in APT_FILES},
            'allFourSourcesMatchLock': True,
        },
    )
    current_phases = ['stage-apt-tools']
    commands = load(output_root / 'commands.json') if (output_root / 'commands.json').exists() else []

    def execute(phase_name, command, working_directory=None):
        print(f'{phase_name}: starting', flush=True)
        current_phases.append(phase_name)
        log_path = logs_root / (phase_name + '.log')
        commands.append(
            {
                'phase': phase_name,
                'argv': [str(argument) for argument in command],
                'cwd': str(working_directory) if working_directory else None,
                'log': str(log_path),
            },
        )
        save(output_root / 'commands.json', commands)
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
            (
                f'{phase_name}'
                f'failed('
                f'{completed_process.returncode}'
                f');retainoutputsandinspect'
                f'{log_path}'
            ),
        )
        print(f'{phase_name}: passed', flush=True)
    readobj = arguments.clang23.with_name('llvm-readobj.exe')
    require(readobj.is_file(), f'Missing LLVM object inspector: {readobj}')
    sdk_output_root = output_root / 'sdk'
    sdk_receipt_path = sdk_output_root / 'sdk-build-receipt.json'
    if not sdk_receipt_path.exists():
        require(
            not sdk_output_root.exists(),
            'Incomplete SDK stage retained; it has no completed receipt',
        )
        execute(
            'sdk-build',
            [
                sys.executable,
                '-B',
                NATIVE / 'sdk/build-sdk.py',
                '--upstream',
                arguments.sdk_checkout,
                '--output',
                sdk_output_root,
                '--clang23',
                arguments.clang23,
                '--clang22',
                arguments.clang22,
                '--cmake',
                arguments.cmake,
                '--ninja',
                arguments.ninja,
                '--jobs',
                str(arguments.jobs),
            ],
        )
    sdk_build_record = load(sdk_receipt_path)
    require(sdk_build_record['buildPassed'] == ['runtime', 'gpu'], 'Both SDK source builds required')
    require(
        sdk_build_record['patchLockSha256'] == digest(sdk_source_lock_path),
        'SDK patch lock differs',
    )
    runtime_bundle = Path(sdk_build_record['runtimeBundle'])
    sdk_source_root = Path(sdk_build_record['sdkSource'])
    sdk_build_root = Path(sdk_build_record['sdkBuild'])
    sdk_libraries_root = Path(sdk_build_record['sdkLibs'])
    sdk_cli = Path(sdk_build_record['cli'])
    expect(sdk_cli, sdk_build_record['cliSha256'])
    for name, expected_hash in sdk_build_record['dllSha256'].items():
        expect(runtime_bundle / name, expected_hash)
    sdk_abi = inspect_abi(
        readobj,
        [sdk_cli, *[runtime_bundle / runtime_filename for runtime_filename in RUNTIME]],
        logs_root / 'sdk-abi',
    )
    verify_runtime_imports(sdk_abi, expected_runtime_imports, 'rexgluerd.exe')
    verify_runtime_imports(sdk_abi, expected_runtime_imports, 'rexgpu-xenosrd.dll')
    save(output_root / 'sdk-abi-receipt.json', sdk_abi)
    qualification = output_root / 'codegen-qualification'
    q_receipt = qualification / 'codegen-qualification-receipt.json'
    expected_sources = load(NATIVE / 'title/codegen-source-hashes.json')
    if not q_receipt.exists():
        require(not qualification.exists(), 'Incomplete codegen qualification retained without receipt')
        generated = qualification / 'generated/default'
        generated.mkdir(parents=True)
        assets = qualification / 'assets'
        assets.mkdir()
        durable(assets / 'default.xex', retail_xex.read_bytes())
        durable(assets / 'default.xexp', tu2_patch.read_bytes())
        manifest = NATIVE / 'title/venom_fnc_canonical_tu2_manifest.toml'
        durable(qualification / manifest.name, manifest.read_bytes())
        durable(
            generated / 'codegen.partition.json',
            (NATIVE / 'title/codegen.partition.json').read_bytes(),
        )
        execute('sdk-owned-codegen', [sdk_cli, 'codegen', manifest.name, "--ignore-stamp"], qualification)
        for name, expected_hash in expected_sources.items():
            expect(generated / name, expected_hash)
        require(len(expected_sources) == 431, 'Expected 431 qualified codegen materials')
        save(
            q_receipt,
            {
                'cliSha256': digest(sdk_cli),
                'retailSha256': digest(retail_xex),
                'tu2Sha256': digest(tu2_patch),
                'expectedSourceHashesSha256': digest(NATIVE / 'title/codegen-source-hashes.json'),
                'materialsMatched': 431,
                'runtimeTested': False,
            },
        )
    qualification_record = load(q_receipt)
    require(
        (
            qualification_record['cliSha256'] == digest(sdk_cli)
            and qualification_record['materialsMatched'] == 431
        ),
        'Stale SDK codegen qualification',
    )
    for name, expected_hash in expected_sources.items():
        expect(qualification / 'generated/default' / name, expected_hash)
    content_tool_output_root = output_root / 'content-tool'
    helper_receipt_path = content_tool_output_root / 'content-tool-receipt.json'
    if not helper_receipt_path.exists():
        require(
            not content_tool_output_root.exists(),
            'Incomplete content-helper stage retained without receipt',
        )
        execute(
            'content-helper-build',
            [
                sys.executable,
                '-B',
                NATIVE / 'content-tool/build-content-tool.py',
                '--sdk-receipt',
                sdk_receipt_path,
                '--clang23',
                arguments.clang23,
                '--output',
                content_tool_output_root,
            ],
        )
    content_tool_record = load(helper_receipt_path)
    content_tool_bin_root = Path(content_tool_record['ToolRoot'])
    require(
        content_tool_record['SdkBuildReceiptSha256'] == digest(sdk_receipt_path),
        'Content helper SDK binding differs',
    )
    for name, expected_hash in content_tool_record['ToolFiles'].items():
        expect(content_tool_bin_root / name, expected_hash)
    helper_abi = inspect_abi(
        readobj,
        [
            content_tool_bin_root / runtime_filename
            for runtime_filename in content_tool_record['ToolFiles']
        ],
        logs_root / 'content-helper-abi',
    )
    save(output_root / 'content-helper-abi-receipt.json', helper_abi)
    # Bind fresh SDK hashes only after static ABI and owned-codegen qualification.
    # The public title profile remains immutable; this recipe is private output.
    bound_title_recipe = output_root / 'bound-title-recipe'
    binding_path = bound_title_recipe / 'recipe-binding-receipt.json'
    if not binding_path.exists():
        require(
            not bound_title_recipe.exists(),
            'Incomplete bound title recipe retained without receipt',
        )
        bound_title_recipe.mkdir()
        for source_path in (NATIVE / 'title').rglob('*'):
            if source_path.is_file():
                durable(
                    bound_title_recipe / source_path.relative_to(NATIVE / 'title'),
                    source_path.read_bytes(),
                )
        bound_title_profile = load(base_profile_path)
        bound_title_profile['inputHashes']['rexglue'] = digest(sdk_cli)
        bound_title_profile['runtimeFiles'] = sdk_build_record['dllSha256']
        for name in bound_title_profile['sdkLibraries']:
            bound_title_profile['sdkLibraries'][name] = digest(sdk_libraries_root / name)
        for name in ('objects', 'compileProfiles', 'compilerVersions', 'linkFlags', 'linkLibraries'):
            require(
                bound_title_profile[name] == base_title_profile[name],
                f'Illegal profile binding change: {name}',
            )
        save(bound_title_recipe / 'title-build-profile.json', bound_title_profile)
        save(
            binding_path,
            {
                'baseProfileSha256': digest(base_profile_path),
                'boundProfileSha256': digest(bound_title_recipe / 'title-build-profile.json'),
                'sdkBuildReceiptSha256': digest(sdk_receipt_path),
                'sdkAbiReceiptSha256': digest(output_root / 'sdk-abi-receipt.json'),
                'codegenQualificationReceiptSha256': digest(q_receipt),
                'contentHelperAbiReceiptSha256': digest(output_root / 'content-helper-abi-receipt.json'),
                'bindingChanges': ['inputHashes.rexglue', 'runtimeFiles', 'sdkLibraries'],
            },
        )
    binding_record = load(binding_path)
    expect(bound_title_recipe / 'title-build-profile.json', binding_record['boundProfileSha256'])
    require(
        binding_record['sdkBuildReceiptSha256'] == digest(sdk_receipt_path),
        'Stale title SDK binding',
    )
    require(
        digest(base_profile_path) == input_identity['immutableTitleProfileSha256'],
        'Public base profile changed',
    )
    title_output_root = output_root / 'title'
    title_receipt_path = title_output_root / 'build-receipt.json'
    if not title_receipt_path.exists():
        require(not title_output_root.exists(), 'Incomplete title build retained without receipt')
        execute(
            'title-build',
            [
                sys.executable,
                '-B',
                bound_title_recipe / 'build-venom-tu2.py',
                '--retail-xex',
                retail_xex,
                '--tu2-xexp',
                tu2_patch,
                '--rexglue',
                sdk_cli,
                '--sdk-source',
                sdk_source_root,
                '--sdk-build',
                sdk_build_root,
                '--sdk-libs',
                sdk_libraries_root,
                '--runtime',
                runtime_bundle,
                '--clang23',
                arguments.clang23,
                '--clang22',
                arguments.clang22,
                '--output',
                title_output_root,
                '--jobs',
                str(arguments.jobs),
            ],
        )
    title_receipt = load(title_receipt_path)
    require(len(title_receipt['compiledObjects']) == 219, 'All 219 title objects required')
    for compiled_object in title_receipt['compiledObjects']:
        expect(Path(compiled_object['object']), compiled_object['objectSha256'])
        expect(Path(compiled_object['source']), compiled_object['sourceSha256'])
        coff_result = subprocess.run(
            [
                str(readobj),
                '--file-headers',
                '--sections',
                '--symbols',
                compiled_object['object'],
            ],
            capture_output=True,
        )
        require(
            (
                coff_result.returncode == 0
                and not coff_result.stderr
                and b'IMAGE_FILE_MACHINE_AMD64' in coff_result.stdout
            ),
            f"Invalid title COFF object: {compiled_object['object']}",
        )
    link_response_path = title_output_root / 'responses/link.rsp'
    expect(link_response_path, title_receipt['executedLinkResponseSha256'])
    linked_objects = re.findall('"([^"\\n]+\\.obj)"', link_response_path.read_text())
    require(
        [str(Path(element)) for element in linked_objects] == [str(Path(element['object'])) for element in title_receipt['compiledObjects']],
        'Actual title link object vector differs',
    )
    expect(title_output_root / TITLE, title_receipt['titleSha256'])
    for name, expected_hash in title_receipt['runtimeFiles'].items():
        expect(title_output_root / name, expected_hash)
    hooks = {
        e['file']: e['afterSha256']
        for e in load(bound_title_recipe / 'author-hook-transforms.json')
    }
    for name, expected_hash in expected_sources.items():
        expect(title_output_root / 'project/generated/default' / name, hooks.get(name, expected_hash))
    title_abi = inspect_abi(
        readobj,
        [
            title_output_root / TITLE,
            *[title_output_root / runtime_filename for runtime_filename in RUNTIME],
        ],
        logs_root / 'title-abi',
    )
    verify_runtime_imports(title_abi, expected_runtime_imports, TITLE)
    save(output_root / 'title-abi-receipt.json', title_abi)
    run_root = output_root / 'runs'
    user_data_root = output_root / 'user-data'
    run_root.mkdir(exist_ok=True)
    save_upgrade = prepare_saves(output_root, arguments.previous_output)
    print(f"save upgrade: {save_upgrade['status']}, {save_upgrade['filesPreserved']} files preserved", flush=True)
    powershell = shutil.which(arguments.powershell)
    require(bool(powershell), 'PowerShell 7 is required')
    if not arguments.install_root.exists():
        execute(
            'install',
            [
                powershell,
                '-NoProfile',
                '-NonInteractive',
                '-File',
                PACKAGE / 'Install-Local-TU2.ps1',
                '-InstallRoot',
                arguments.install_root,
                '-RuntimeRoot',
                title_output_root,
                '-GameDataRoot',
                arguments.game_data_root,
                '-UpdateDataRoot',
                arguments.update_data_root,
                '-AptToolsRoot',
                staged_apt,
                '-ContentToolRoot',
                content_tool_bin_root,
                '-RunRoot',
                run_root,
                '-DlcTitleRoot',
                arguments.dlc_title_root,
                '-UserDataRoot',
                user_data_root,
                '-RuntimeReceipt',
                title_receipt_path,
                '-ContentToolReceipt',
                helper_receipt_path,
                '-PythonExecutable',
                sys.executable,
            ],
        )
    require(
        Path(load(arguments.install_root / 'venom-install.json')['Paths']['AptToolsRoot']).resolve() == staged_apt.resolve(),
        ('Existing installation uses another APT root; preserve it and choose a fresh '
         '--install-root child'),
    )
    installed_launcher = arguments.install_root / 'Start-Installed-TU2.ps1'
    execute(
        'installed-preflight',
        [powershell, '-NoProfile', '-NonInteractive', '-File', installed_launcher, '-CheckOnly'],
    )
    installed_config = load(arguments.install_root / 'venom-install.json')
    for name, expected_hash in installed_config['InstalledFiles'].items():
        expect(arguments.install_root / name, expected_hash)
    for entry in dlc_manifest:
        expect(user_data_root / '0000000000000000/45410915' / entry['Path'], entry['SHA256'])
    preparation_run_root = Path(installed_config['PreparationRun'])
    expect(
        preparation_run_root / 'offline-xp/boosts.big',
        'd7eb8297f20c58e1dc9d5e5699499816e2028fa33cba90ce39c65a544557df9f',
    )
    for name, expected_hash in [
        (
            'speech_eng_dlc_samples.sbr',
            '08b7bb2c4aa798669aaa14f0976cea4a0dc7f7871d886dfdcb5db382e6da4a2b',
        ),
        (
            'speech_eng_dlc_samples.sbs',
            '68f0609aa77d7c20234fe601774c2ab2e660c05cc7be0531947a37e4d1eaa654',
        ),
    ]:
        expect(preparation_run_root / 'legends-speech' / name, expected_hash)
    expect(retail_xex, input_identity['retailSha256'])
    expect(tu2_patch, input_identity['tu2Sha256'])
    for entry in dlc_manifest:
        expect(arguments.dlc_title_root / entry['Path'], entry['SHA256'])
    receipts = {
        name: digest(path)
        for (name, path) in {
            'inputs': output_root / 'pipeline-inputs.json',
            'aptTools': output_root / 'apt-tools-validation.json',
            'sdk': sdk_receipt_path,
            'sdkAbi': output_root / 'sdk-abi-receipt.json',
            'ownedCodegen': q_receipt,
            'contentTool': helper_receipt_path,
            'contentToolAbi': output_root / 'content-helper-abi-receipt.json',
            'titleBinding': binding_path,
            'title': title_receipt_path,
            'titleAbi': output_root / 'title-abi-receipt.json',
            'installedManifest': arguments.install_root / 'venom-install.json',
        }.items(
        )
    }
    pipeline_receipt = {
        'status': 'PASS_FULL_SOURCE_BUILD_AND_INSTALL_PREFLIGHT',
        'completedUtc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
        'runtimeRoot': str(title_output_root),
        'installedLauncher': str(installed_launcher),
        'titleSha256': digest(title_output_root / TITLE),
        'titleObjects': 219,
        'codegenMaterialsMatched': 431,
        'installedDlcFiles': len(dlc_manifest),
        'saveUpgrade': save_upgrade,
        'ownedInputsUnchanged': True,
        'publicTitleProfileUnchanged': True,
        'receiptHashes': receipts,
        'pipelineSourceSha256': digest(Path(__file__)),
        'resumed': arguments.resume,
        'executedPhases': current_phases,
        'freshSdkBuild': 'sdk-build' in current_phases,
        'freshTitleBuild': 'title-build' in current_phases,
        'reusedCompletedBuildStages': [
            name
            for name in ('sdk-build', 'sdk-owned-codegen', 'content-helper-build', 'title-build')
            if name not in current_phases
        ],
        'gameLaunched': False,
        'runtimeTested': False,
        'published': False,
    }
    save(output_root / 'build-install-receipt.json', pipeline_receipt)
    print(json.dumps(pipeline_receipt, indent=2), flush=True)


if __name__ == '__main__':
    main()
