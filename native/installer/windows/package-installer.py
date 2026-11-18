"""Deterministic source-only and installer-EXE bundles. Never publishes or builds a game."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import zipfile
HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
SUFFIXES = {'.py', '.ps1', '.md', '.json', '.toml', '.cmake', '.patch', '.h', '.cpp', '.txt'}

def digest(data):
    return hashlib.sha256(data).hexdigest()

def encoded(value):
    return (json.dumps(value, indent=2, sort_keys=True) + '\n').encode('utf-8')

def source_files(root):
    pending = [root]
    while pending:
        path = pending.pop()
        attributes = getattr(path.lstat(), 'st_file_attributes', 0)
        if path.is_symlink() or attributes & 1024:
            raise ValueError('Redirected source path refused: ' + str(path))
        if path.is_dir():
            pending.extend(reversed(sorted(path.iterdir())))
        elif path.is_file():
            yield path
        else:
            raise ValueError('Unexpected source filesystem entry: ' + str(path))

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True, help='Fresh private package directory.')
    p.add_argument('--installer-build', type=Path, required=True, help='Private build directory containing verified installer receipt/exe.')
    p.add_argument('--version', required=True, help='Release tag, such as v0.1.0-preview.2.')
    a = p.parse_args()
    if not re.fullmatch(r'v(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)(?:-[0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*)?', a.version):
        p.error('--version must be a release tag such as v0.1.0-preview.2')
    out = a.output.resolve()
    if out.exists():
        raise ValueError('Preserve existing output; select a fresh directory.')
    build = a.installer_build.resolve()
    receipt = json.loads((build / 'installer-build-receipt.json').read_text(encoding='utf-8'))
    exe = (build / 'VenomInstaller.exe').read_bytes()
    if receipt['status'] != 'PASS_NATIVE_COMPILE_AND_STATIC_TESTS' or digest(exe) != receipt['exeSha256']:
        raise ValueError('Installer build receipt/hash did not pass.')
    if digest((HERE / 'VenomInstaller.cpp').read_bytes()) != receipt['sourceSha256']:
        raise ValueError('Installer source changed after compile.')
    for name in ('VenomInstaller.cpp', 'native_ui_self_test.h'):
        if digest((HERE / name).read_bytes()) != receipt.get('sourceHashes', {}).get(name):
            raise ValueError('Installer source/header changed or unbound: ' + name)
    payload = {}
    for folder in ('installer', 'sdk', 'title', 'content-tool'):
        for path in source_files(ROOT / 'native' / folder):
            rel = path.relative_to(ROOT).as_posix()
            if path.is_symlink() or path.name.endswith('.local') or (path.suffix.lower() not in SUFFIXES and path.name not in {'LICENSE', 'NOTICE', 'ReXGlue-LICENSE'}):
                raise ValueError('Unexpected source package material: ' + rel)
            if any((x.lower() in {'research', 'generated', 'captures', 'profiles', 'user-data', '__pycache__', '.git'} for x in path.relative_to(ROOT).parts)):
                raise ValueError('Private/generated directory refused: ' + rel)
            payload[rel] = path.read_bytes()
    for required in (
        'native/sdk/sdk-source-lock.json',
        'native/installer/apt-tools/source-lock.json',
        'native/installer/windows/notices/ReXGlue-LICENSE',
        'native/content-tool/LICENSE',
        'native/content-tool/NOTICE',
    ):
        if required not in payload:
            raise ValueError('Required provenance/notice missing: ' + required)
    payload['README.md'] = (HERE / 'README.md').read_bytes()
    payload['THIRD_PARTY_NOTICES.md'] = (HERE / 'THIRD_PARTY_NOTICES.md').read_bytes()

    def write(name, data, kind):
        manifest = {
            'format': 1,
            'kind': kind,
            'releaseVersion': a.version,
            'published': False,
            'gameMediaIncluded': False,
            'generatedTitleCodeIncluded': False,
            'runtimeBinariesIncluded': False,
            'guiBehaviorVerified': False,
            'members': [{'path': k, 'bytes': len(v), 'sha256': digest(v)} for k, v in sorted(data.items())],
        }
        members = dict(data)
        members['PACKAGE-MANIFEST.json'] = encoded(manifest)
        path = out / name
        with zipfile.ZipFile(path, 'x', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
            for rel, raw in sorted(members.items()):
                info = zipfile.ZipInfo(rel, (1980, 1, 1, 0, 0, 0))
                info.compress_type = zipfile.ZIP_DEFLATED
                info.create_system = 0
                info.external_attr = 32
                z.writestr(info, raw, compress_type=zipfile.ZIP_DEFLATED, compresslevel=9)
        with zipfile.ZipFile(path, 'r') as z:
            if z.testzip() or sorted(z.namelist()) != sorted(members):
                raise ValueError('ZIP inventory/CRC failed')
            for rel, raw in members.items():
                if z.read(rel) != raw:
                    raise ValueError('ZIP payload verification failed: ' + rel)
        (out / (name + '.manifest.json')).write_bytes(encoded(manifest))
        return {
            'file': name,
            'bytes': path.stat().st_size,
            'sha256': digest(path.read_bytes()),
            'members': len(members),
        }
    out.mkdir(parents=True)
    results = [write(f'Venom-{a.version}-source-only.zip', payload, 'source-only')]
    bundle = dict(payload)
    bundle['VenomInstaller.exe'] = exe
    identity = {k: receipt[k] for k in (
        'status',
        'compilerVersion',
        'compilerSha256',
        'windowsSdkVersion',
        'msvcVersion',
        'sourceSha256',
        'sourceHashes',
        'exeSha256',
        'exeBytes',
        'imports',
        'staticCrt',
        'staticTests',
        'guiLaunched',
        'guiBehaviorVerified',
        'gameLaunched',
        'gameBuilt',
        'published',
    )}
    identity['compilerVersion'] = '\n'.join((x for x in identity['compilerVersion'].splitlines() if not x.startswith('InstalledDir:'))) + '\n'
    bundle['INSTALLER-BUILD-IDENTITY.json'] = encoded(identity)
    results.append(write(f'Venom-{a.version}-Windows-installer.zip', bundle, 'native-installer-plus-source'))
    result = {
        'status': 'PASS_PRIVATE_BUNDLE_INVENTORY_AND_HASHES',
        'packages': results,
        'releaseVersion': a.version,
        'published': False,
        'guiBehaviorVerified': False,
        'gameOrDlcRuntimeAcceptance': False,
    }
    (out / 'package-receipt.json').write_bytes(encoded(result))
    print(json.dumps(result, indent=2))
if __name__ == '__main__':
    main()
