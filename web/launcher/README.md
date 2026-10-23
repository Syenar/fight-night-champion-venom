# Local browser launcher

This package is the browser transport prototype for the local Fight Night Champion SBS host. It contains no game files and cannot install or download the commercial game.

## Prerequisites

- Bun
- A separately prepared local game host that is already producing SBS video
- A browser with screen capture permission when using the host page

## Start

```powershell
bun install
bun run start
```

Open `/host?session=localtest01` on the host machine and choose the game window when prompted. Then open `/?session=localtest01` in the client browser. Both pages must use the same session value.

This launcher only handles local signaling, display capture, and controller messages. It does not provide a standalone browser game build. The current public launch path is temporarily unplayable while the native startup path is repaired; earlier private validation reached playable boxing, but complete gameplay, saves, and SBS acceptance remain unfinished.
