# Evidence records

Each completed runbook card writes one record under `docs/evidence/cards/` using a stable name such as `card-00-source-lock.md`. A record contains:

- date and agent
- input paths and SHA-256 hashes
- exact commands and tool versions
- output paths, logs, captures, and replay identifiers
- status: `PASS`, `PARTIAL`, or `BLOCKED`
- the next card and any invalidated evidence

Large captures, extracted game data, Ghidra projects, generated code, and save data remain under local `research/` paths. Evidence records may link to those paths but must not copy or embed proprietary data.
