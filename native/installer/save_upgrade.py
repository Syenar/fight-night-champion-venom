"""Carry local Venom profiles into a fresh side-by-side installation."""

from hashlib import file_digest
import json
import os
from pathlib import Path
import shutil
import stat
from tempfile import TemporaryDirectory


SAVE_RECEIPT = '.venom-save-upgrade.json'
PUBLIC_DLC = Path('0000000000000000/45410915')


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def file_hash(path):
    with path.open('rb') as stream:
        return file_digest(stream, 'sha256').hexdigest()


def check_path(path):
    for item in (path, *path.parents):
        try:
            info = item.lstat()
        except FileNotFoundError:
            continue
        if stat.S_ISLNK(info.st_mode) or getattr(info, 'st_file_attributes', 0) & 1024:
            raise ValueError(f'Save upgrade path is redirected: {item}')


def previous_profile(output):
    """Find a completed installer manifest and its local user data."""
    check_path(output)
    receipt = output / 'build-install-receipt.json'
    if not receipt.is_file():
        return None
    check_path(receipt)
    build = read_json(receipt)
    if build.get('status') != 'PASS_FULL_SOURCE_BUILD_AND_INSTALL_PREFLIGHT':
        return None
    inputs_path = output / 'pipeline-inputs.json'
    check_path(inputs_path)
    inputs = read_json(inputs_path)['args']
    old_output = Path(inputs['output'])
    old_launcher = Path(build['installedLauncher'])
    if not old_output.is_absolute() or not old_launcher.is_relative_to(old_output):
        raise ValueError(f'Previous launcher is outside its build output: {old_launcher}')
    relative_launcher = old_launcher.relative_to(old_output)
    if '..' in relative_launcher.parts or old_launcher.name != 'Start-Installed-TU2.ps1':
        raise ValueError(f'Invalid previous launcher: {old_launcher}')
    launcher = output / relative_launcher
    manifest = launcher.with_name('venom-install.json')
    check_path(manifest)
    check_path(launcher)
    expected = build['receiptHashes']['installedManifest']
    if file_hash(manifest) != expected.lower():
        raise ValueError(f'Previous installation manifest differs: {manifest}')
    config = read_json(manifest)
    if config.get('Format') != 1 or file_hash(launcher) != config['InstalledFiles'][launcher.name].lower():
        raise ValueError(f'Previous launcher identity differs: {launcher}')
    old_user = Path(config['Paths']['UserDataRoot'])
    if old_user != old_output / 'user-data':
        raise ValueError(f'Previous installation has a nonstandard save folder: {old_user}')
    profile = output / 'user-data'
    check_path(profile)
    if not profile.is_dir():
        raise ValueError(f'Previous save folder is missing: {profile}')
    return profile


def save_inventory(root):
    if not root.exists():
        return {}
    check_path(root)
    files = {}
    unique = set()
    def unreadable(error):
        raise error

    for directory, subdirs, names in os.walk(root, onerror=unreadable):
        directory = Path(directory)
        relative = directory.relative_to(root)
        check_path(directory)
        if relative == Path('.'):
            names = [name for name in names if name.casefold() != SAVE_RECEIPT]
            subdirs[:] = [name for name in subdirs if name.casefold() not in {'cache', 'venom-guest-cache'}]
        if relative in {PUBLIC_DLC, PUBLIC_DLC / 'Headers'}:
            subdirs[:] = [name for name in subdirs if name.casefold() != '00000002']
        for name in (*subdirs, *names):
            check_path(directory / name)
        for name in names:
            src = directory / name
            key = src.relative_to(root)
            if not src.is_file() or key.as_posix().casefold() in unique:
                raise ValueError(f'Unsupported or ambiguous save path: {src}')
            unique.add(key.as_posix().casefold())
            before = src.stat()
            checksum = file_hash(src)
            after = src.stat()
            if (before.st_size, before.st_mtime_ns) != (after.st_size, after.st_mtime_ns):
                raise ValueError(f'Save changed while being read: {src}')
            files[key] = checksum
    return files


def ancestry(output):
    path = output / 'user-data' / SAVE_RECEIPT
    if not path.is_file():
        return []
    check_path(path)
    return read_json(path).get('ancestors', [])


def choose_previous(output, selected):
    if selected is not None:
        check_path(selected.absolute())
        previous = Path(os.path.abspath(selected))
        if (previous == output or previous.is_relative_to(output) or output.is_relative_to(previous)):
            raise ValueError('Previous and new install outputs must be separate directories.')
        profile = previous_profile(previous)
        if profile is None:
            raise ValueError(f'No completed Venom install found in the previous output: {previous}')
        return previous, profile, save_inventory(profile)

    candidates = {}
    for sibling in output.parent.iterdir():
        if sibling == output or not (sibling / 'build-install-receipt.json').is_file():
            continue
        profile = previous_profile(sibling)
        if profile is not None:
            saves = save_inventory(profile)
            if saves:
                candidates[sibling] = (profile, saves)
    superseded = {name.casefold() for candidate in candidates for name in ancestry(candidate)}
    active = [candidate for candidate in candidates if str(candidate).casefold() not in superseded]
    if len(active) > 1 or (candidates and not active):
        choices = '\n'.join(f'  {path}' for path in sorted(candidates))
        raise ValueError('Multiple previous Venom saves were found. Select Previous Venom output '
                         f'or use --previous-output with one of these folders:\n{choices}')
    if active:
        previous = active[0]
        return previous, *candidates[previous]
    return None, None, {}


def write_upgrade_receipt(directory, record):
    path = directory / SAVE_RECEIPT
    with path.open('x', encoding='utf-8') as stream:
        json.dump(record, stream, indent=2)
        stream.write('\n')
        stream.flush()
        os.fsync(stream.fileno())


def prepare_saves(output, previous_output=None):
    """Copy and verify an earlier profile before the installer imports DLC."""
    target = output / 'user-data'
    check_path(target)
    receipt_path = target / SAVE_RECEIPT
    if receipt_path.is_file():
        check_path(receipt_path)
        receipt = read_json(receipt_path)
        if previous_output is not None and receipt.get('previousOutput') != os.path.abspath(previous_output):
            raise ValueError('This output already has a saved migration choice. Choose a fresh output to change it.')
        return receipt

    existing = save_inventory(target)
    if target.exists() and previous_output is None:
        # Legacy in-place resume. The output already owns this profile.
        return {'status': 'existing-profile', 'filesPreserved': len(existing), 'ancestors': []}

    previous, profile, files = choose_previous(output, previous_output)
    record = {'status': 'copied' if files else 'new-profile',
              'previousOutput': str(previous) if previous else None,
              'filesPreserved': len(files),
              'ancestors': list(dict.fromkeys([str(previous), *ancestry(previous)])) if previous else []}
    if target.exists():
        if existing != files:
            raise ValueError('The new output already has different saves. Both profiles were preserved. '
                             'Choose a fresh output for this upgrade.')
        return {'status': 'existing-profile', 'filesPreserved': len(existing), 'ancestors': []}

    with TemporaryDirectory(prefix='.venom-save-import-', dir=output) as directory:
        work = Path(directory) / 'user-data'
        work.mkdir()
        for relative, checksum in files.items():
            src, dst = profile / relative, work / relative
            check_path(src)
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, dst)
            if file_hash(dst) != checksum:
                raise ValueError(f'Save changed while being copied: {src}')
        if profile is not None and save_inventory(profile) != files:
            raise ValueError('Previous saves changed during the upgrade. Close the earlier game and retry.')
        write_upgrade_receipt(work, record)
        check_path(target)
        # The destination is a fresh Windows directory. Do not replace an existing profile.
        if target.exists():
            raise ValueError('Another process created the new save folder during the upgrade.')
        work.rename(target)
    return record
