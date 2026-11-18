"""Prepare a pinned real-resume UI scenario; never run the GUI or backend."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys


def digest(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def private_path(path):
    path = Path(os.path.abspath(path))
    allowed = Path('E:/Venom-Installer-QA')
    if not path.is_relative_to(allowed) or path == allowed:
        raise ValueError('Use an explicit private E:/Venom-Installer-QA child')
    for parent in [path, *path.parents]:
        if parent.exists() and getattr(parent.lstat(), 'st_file_attributes', 0) & 1024:
            raise ValueError('Redirected QA path refused: ' + str(parent))
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--installer-build', type=Path, required=True)
    parser.add_argument('--package-root', type=Path, required=True)
    parser.add_argument('--pipeline-output', type=Path, required=True)
    parser.add_argument('--run-root', type=Path, required=True)
    parser.add_argument('--python', type=Path, default=Path(sys.executable))
    parser.add_argument('--fresh-install-name', default='installed-native-ui-selftest-r1')
    parser.add_argument('--timeout-seconds', type=int, default=600)
    args = parser.parse_args()
    build = private_path(args.installer_build)
    package = private_path(args.package_root)
    output = private_path(args.pipeline_output)
    run = private_path(args.run_root)
    if run.exists() or not run.parent.is_dir():
        raise ValueError('Run must be a fresh child of an existing private QA parent')
    if not re.fullmatch(r'[A-Za-z0-9_-]{1,64}', args.fresh_install_name):
        raise ValueError('Choose a simple fresh install child name')
    if not 1 <= args.timeout_seconds <= 1800:
        raise ValueError('Finite timeout must be1..1800seconds')
    install = output / args.fresh_install_name
    if install.exists():
        raise ValueError('Preserve existing installations; choose another fresh child')
    build_receipt = json.loads((build / 'installer-build-receipt.json').read_text())
    exe = build / 'VenomInstaller.exe'
    descriptor = build / 'installer-static-tests.exe'
    if build_receipt['status'] != 'PASS_NATIVE_COMPILE_AND_STATIC_TESTS':
        raise ValueError('Installer build did not pass')
    if digest(exe) != build_receipt['exeSha256'] or digest(descriptor) != build_receipt['staticTestExeSha256']:
        raise ValueError('Installer/console descriptor hash changed')
    manifest_path = package / 'PACKAGE-MANIFEST.json'
    manifest = json.loads(manifest_path.read_text())
    backend = package / 'native/installer/Build-Install-Venom.py'
    manifest_members = {entry['path']: entry for entry in manifest['members']}
    if manifest_members.get('native/installer/Build-Install-Venom.py', {}).get('sha256') != digest(backend):
        raise ValueError('Backend is not bound by the supplied source package manifest')
    for entry in manifest['members']:
        relative = Path(entry['path'])
        if relative.is_absolute() or '..' in relative.parts:
            raise ValueError('Unsafe source manifest member')
        member = package / relative
        if not member.resolve().is_relative_to(package.resolve()) or getattr(member.lstat(), 'st_file_attributes', 0) & 1024:
            raise ValueError('Redirected/outside source manifest member')
        if member.stat().st_size != entry['bytes'] or digest(member) != entry['sha256']:
            raise ValueError('Package member mismatch: ' + entry['path'])
    baseline_path = output / 'build-install-receipt.json'
    baseline = json.loads(baseline_path.read_text())
    if baseline['status'] != 'PASS_FULL_SOURCE_BUILD_AND_INSTALL_PREFLIGHT' or baseline['titleObjects'] != 219:
        raise ValueError('Resume scenario requires an already qualified full public pipeline output')
    original = json.loads((output / 'pipeline-inputs.json').read_text())['args']
    field_names = ['sdk-checkout', 'game-data-root', 'update-data-root', 'dlc-title-root',
                   'clang23', 'clang22', 'python', 'output', 'apt-tools-root', 'cmake',
                   'ninja', 'powershell', 'jobs', 'install-root']
    values = {name: str(original.get(name.replace('-', '_'), '')) for name in field_names}
    values.update({'python': str(args.python.resolve()), 'output': str(output), 'install-root': str(install)})
    data = {'format': '1', 'mode': 'resume-build', 'authorization': 'I_AUTHORIZE_HASH_PINNED_RESUME',
            'run-root': str(run), 'package-root': str(package), 'installer-sha256': digest(exe),
            'backend-sha256': digest(backend), 'python-sha256': digest(args.python.resolve()),
            'package-manifest-sha256': digest(manifest_path), 'baseline-receipt-sha256': digest(baseline_path),
            'authorized-command-sha256': '0' * 64, 'timeout-seconds': str(args.timeout_seconds),
            'expected-exit-code': '0', 'resume': '1', 'remember-inputs': '0'}
    data.update({'field.' + name: values[name] for name in field_names})
    if any(any(character in value for character in '\r\n\0') for value in data.values()):
        raise ValueError('Scenario values cannot contain line breaks/NUL')
    run.mkdir()
    descriptor_temp = run / 'descriptor-temp'
    descriptor_temp.mkdir()
    scenario_path = run / 'scenario.ini'

    def write_scenario():
        scenario_path.write_text(''.join(key + '=' + value + '\n' for key, value in data.items()),
                                 encoding='utf-8', newline='\n')
    write_scenario()
    descriptor_argv = [str(descriptor), '--describe-ui-self-test', str(scenario_path)]
    environment = dict(os.environ, TEMP=str(descriptor_temp), TMP=str(descriptor_temp))
    result = subprocess.run(descriptor_argv, cwd=run, env=environment, capture_output=True, text=True)
    (run / 'descriptor.stdout.json').write_text(result.stdout, encoding='utf-8')
    (run / 'descriptor.stderr.txt').write_text(result.stderr, encoding='utf-8')
    if result.returncode:
        raise ValueError('Console descriptor refused data: ' + result.stderr)
    description = json.loads(result.stdout)
    if hashlib.sha256(description['commandLine'].encode('utf-8')).hexdigest() != description['commandSha256']:
        raise ValueError('Production command descriptor hash mismatch')
    data['authorized-command-sha256'] = description['commandSha256']
    write_scenario()
    (run / 'production-command.txt').write_text(description['commandLine'] + '\n', encoding='utf-8')
    gui_argv = [str(exe), '--ui-self-test', str(scenario_path), '--scenario-sha256', digest(scenario_path)]
    receipt = {'status': 'PREPARED_ONLY_REAL_RESUME_SCENARIO_GUI_BACKEND_NOT_EXECUTED',
               'installerBuildReceiptSha256': digest(build / 'installer-build-receipt.json'),
               'installerSha256': digest(exe), 'consoleDescriptorSha256': digest(descriptor),
               'descriptorArgv': descriptor_argv, 'descriptorExitCode': result.returncode,
               'scenarioSha256': digest(scenario_path), 'backendSha256': digest(backend),
               'packageManifestSha256': digest(manifest_path), 'baselineReceiptSha256': digest(baseline_path),
               'productionCommandSha256': description['commandSha256'], 'guiArgvForReviewOnly': gui_argv,
               'freshInstallRoot': str(install), 'guiExecuted': False, 'backendExecuted': False,
               'gameLaunched': False, 'controllerSettingsChanged': False}
    (run / 'prepared-scenario-receipt.json').write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(receipt, indent=2))


if __name__ == '__main__':
    main()
