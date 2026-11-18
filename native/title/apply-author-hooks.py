from hashlib import sha256
import json
from pathlib import Path
import sys

plan = Path(__file__).with_name('author-hook-transforms.json')
generated = Path(sys.argv[1])
entries = json.loads(plan.read_text(encoding='utf-8'))

prepared = []
for entry in entries:
    target = generated / entry['file']
    before = target.read_bytes()
    if sha256(before).hexdigest() != entry['beforeSha256']:
        raise ValueError(f'Unexpected generated source identity: {target}')
    lines = before.decode('utf-8').replace('\r\n','\n').splitlines(keepends=True)
    for edit in reversed(entry['edits']):
        start = edit['startLineZeroBased']
        lines[start:start + edit['removeLineCount']] = edit['insertLines']
    after = ''.join(lines).encode('utf-8')
    if sha256(after).hexdigest() != entry['afterSha256']:
        raise ValueError(f'Unexpected authored-transform output: {target}')
    prepared.append((target, after))
for target, after in prepared:
    target.write_bytes(after)
print(f'Applied authored hooks to {len(prepared)} generated files')

