"""Build the native installer only. Never builds or launches the game or GUI."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
HERE = Path(__file__).resolve().parent

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True, help='Fresh private artifact directory; no default C: output.')
    p.add_argument('--clang-cl', type=Path, default=shutil.which('clang-cl'))
    p.add_argument('--msvc', type=Path, required=True, help='VC/Tools/MSVC/version containing include and lib/x64.')
    p.add_argument('--windows-sdk', type=Path, default=Path(os.environ.get('ProgramFiles(x86)', 'C:/Program Files (x86)')) / 'Windows Kits/10')
    p.add_argument('--sdk-version', default='10.0.26100.0')
    a = p.parse_args()
    output = a.output.resolve()
    if output.exists():
        raise ValueError('Output exists; preserve it and select a fresh directory.')
    if not a.clang_cl:
        raise ValueError('Select clang-cl.exe.')
    compiler = a.clang_cl.resolve()
    readobj = compiler.with_name('llvm-readobj.exe')
    msvc = a.msvc.resolve()
    sdk = a.windows_sdk.resolve()
    includes = [
        msvc / 'include',
        *[sdk / 'Include' / a.sdk_version / x for x in ('ucrt', 'shared', 'um')],
    ]
    libs = [
        msvc / 'lib/x64',
        *[sdk / 'Lib' / a.sdk_version / x / 'x64' for x in ('ucrt', 'um')],
    ]
    for path in [compiler, readobj, *includes, *libs]:
        if not path.exists():
            raise FileNotFoundError(path)
    output.mkdir(parents=True)
    temp = output / 'temp'
    temp.mkdir()
    env = dict(os.environ, TEMP=str(temp), TMP=str(temp))
    for name in ('CL', '_CL_', 'LINK', '_LINK_', 'CCC_OVERRIDE_OPTIONS'):
        if env.get(name):
            raise ValueError('Inherited compiler override refused: ' + name)
    source_hashes = {name: digest(HERE / name) for name in ('VenomInstaller.cpp', 'native_ui_self_test.h')}
    build_script_hash = digest(Path(__file__))
    version = subprocess.check_output([str(compiler), '--version'], cwd=output, env=env, text=True)
    commands = []

    def build(name, test=False):
        exe = output / (name + '.exe')
        cmd = [
            str(compiler),
            '/nologo',
            '/std:c++20',
            '/EHsc',
            '/O2',
            '/MT',
            '/W4',
            '/DUNICODE',
            '/D_UNICODE',
            '/D_WIN32_WINNT=0x0A00',
            '/Brepro',
            '/utf-8',
            '/c',
            str(HERE / 'VenomInstaller.cpp'),
            '/Fo' + str(output / (name + '.obj')),
        ]
        cmd += ['/imsvc' + str(x) for x in includes]
        if test:
            cmd += ['/DVENOM_STATIC_TEST']
        commands.append(cmd)
        with (output / (name + '-compile.log')).open('wb') as log:
            subprocess.run(cmd, cwd=output, env=env, stdout=log, stderr=subprocess.STDOUT, check=True)
        link = [
            str(compiler.with_name('lld-link.exe')),
            '/nologo',
            '/out:' + str(exe),
            str(output / (name + '.obj')),
            '/machine:x64',
            '/subsystem:' + ('console' if test else 'windows'),
            '/opt:ref',
            '/opt:icf',
            '/Brepro',
            '/timestamp:0',
            '/manifest:embed',
            "/manifestuac:level='asInvoker' uiAccess='false'",
            'kernel32.lib',
            'user32.lib',
            'gdi32.lib',
            'shell32.lib',
            'ole32.lib',
            'uuid.lib',
            'advapi32.lib',
            'bcrypt.lib',
        ]
        link += ['/libpath:' + str(x) for x in libs]
        commands.append(link)
        with (output / (name + '-link.log')).open('wb') as log:
            subprocess.run(link, cwd=output, env=env, stdout=log, stderr=subprocess.STDOUT, check=True)
        return exe
    exe = build('VenomInstaller')
    test = build('installer-static-tests', True)
    static_result = subprocess.run([str(test)], cwd=output, env=env, check=False, capture_output=True, text=True)
    (output / 'installer-static-tests.log').write_text(static_result.stdout + static_result.stderr, encoding='utf-8')
    if static_result.returncode:
        raise ValueError('Console regressions failed: ' + static_result.stdout + static_result.stderr)
    match = re.search(r'PASS 248 Windows argument round trips; (\d+) diagnostic schema cases; owned-job cleanup', static_result.stdout)
    if not match or int(match.group(1)) < 29:
        raise ValueError('Console regression summary missing')
    abi = subprocess.check_output([str(readobj), '--file-headers', '--coff-imports', str(exe)], cwd=output, env=env, text=True)
    (output / 'VenomInstaller.abi.txt').write_text(abi, encoding='utf-8')
    imported = re.findall('\\n  Name: ([^\\n]+)', abi)
    if 'IMAGE_FILE_MACHINE_AMD64' not in abi:
        raise ValueError('Expected AMD64 executable')
    if any((re.search('(vcruntime|msvcp|ucrtbase|rex|tracy|python|clang)', n, re.I) for n in imported)):
        raise ValueError('Unexpected external runtime dependency: ' + str(imported))
    if source_hashes != {name: digest(HERE / name) for name in source_hashes}:
        raise ValueError('Installer inputs changed during build/test')
    if build_script_hash != digest(Path(__file__)):
        raise ValueError('Build script changed during build/test')
    receipt = {
        'status': 'PASS_NATIVE_COMPILE_AND_STATIC_TESTS',
        'compiler': str(compiler),
        'compilerVersion': version,
        'compilerSha256': digest(compiler),
        'windowsSdkVersion': a.sdk_version,
        'msvcVersion': msvc.name,
        'sourceSha256': source_hashes['VenomInstaller.cpp'],
        'sourceHashes': source_hashes,
        'sourceHashesBeforeAfterMatch': True,
        'buildScriptSha256': build_script_hash,
        'cwd': str(output),
        'TEMP': str(temp),
        'TMP': str(temp),
        'exeSha256': digest(exe),
        'exeBytes': exe.stat().st_size,
        'staticTestExeSha256': digest(test),
        'staticTestsLogSha256': digest(output / 'installer-static-tests.log'),
        'imports': imported,
        'staticCrt': True,
        'staticTests': {
            'windowsArgumentRoundTrips': 248,
            'diagnosticScenarioCases': int(match.group(1)),
            'pathContainment': True,
            'bundledAptOmission': True,
            'resumeArgument': True,
            'customInstallArgument': True,
            'ownedJobKillsOwnChildOnClose': True,
        },
        'nativeUiSelfTestPreparedButNotExecuted': True,
        'guiLaunched': False,
        'guiBehaviorVerified': False,
        'gameLaunched': False,
        'gameBuilt': False,
        'published': False,
        'commands': commands,
    }
    (output / 'installer-build-receipt.json').write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({k: v for k, v in receipt.items() if k != 'commands'}, indent=2))
if __name__ == '__main__':
    main()
