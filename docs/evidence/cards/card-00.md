# Card 00 — workspace and source lock

Status: PASS
Date: 10/02/2026 00:10:01

The single Xbox ISO beside the workspace was identified without modifying it. Its source path, size, last-write time, and SHA-256 are recorded in esearch/source/iso.metadata.txt, esearch/source/iso.sha256, and esearch/source/CARD_00_RESULT.json.

- Source: $(@{card=00; status=PASS; date=10/02/2026 00:10:01; workspace=C:\Users\samsa\Desktop\Workplace\Projects\Fight Night Champion Mods\xbox-full-game-sbs; parent_source_path=C:\Users\samsa\Desktop\Workplace\Projects\Fight Night Champion Mods\Fight Night Champion (USA, Europe).iso; sha256=75E9D3EAA8440A24BFE1052B6E051F2A29DF189DDBE854969C1406E31D2E8242; size_bytes=7838695424; last_write_time=12/24/1996 23:32:00; next_card=01}.parent_source_path)
- Size: $(@{card=00; status=PASS; date=10/02/2026 00:10:01; workspace=C:\Users\samsa\Desktop\Workplace\Projects\Fight Night Champion Mods\xbox-full-game-sbs; parent_source_path=C:\Users\samsa\Desktop\Workplace\Projects\Fight Night Champion Mods\Fight Night Champion (USA, Europe).iso; sha256=75E9D3EAA8440A24BFE1052B6E051F2A29DF189DDBE854969C1406E31D2E8242; size_bytes=7838695424; last_write_time=12/24/1996 23:32:00; next_card=01}.size_bytes) bytes
- SHA-256: $(@{card=00; status=PASS; date=10/02/2026 00:10:01; workspace=C:\Users\samsa\Desktop\Workplace\Projects\Fight Night Champion Mods\xbox-full-game-sbs; parent_source_path=C:\Users\samsa\Desktop\Workplace\Projects\Fight Night Champion Mods\Fight Night Champion (USA, Europe).iso; sha256=75E9D3EAA8440A24BFE1052B6E051F2A29DF189DDBE854969C1406E31D2E8242; size_bytes=7838695424; last_write_time=12/24/1996 23:32:00; next_card=01}.sha256)
- Independent critic: esearch/source/CARD_00_CRITIC.json
- Next card: Card 01 — tool discovery and exact-title feasibility

The critic independently found exactly one matching ISO and reproduced the recorded hash and size. No extraction, patching, or source mutation was performed.
