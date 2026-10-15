# Bun/WebRTC browser client

This package is the browser transport boundary for the local SBS host. It contains no game data. The Bun server relays session-scoped WebRTC signaling. `host.html` is the local host adapter: it captures the already-packed Xenia SBS window with `getDisplayMedia`, publishes that real video track, and forwards controller events through the constrained local PowerShell bridge.

Run `bun run start`, open `/host?session=localtest01` on the machine running Xenia, choose the Xenia window, then open `/?session=localtest01` in the browser client. The host must already be using the Vulkan SBS build. `bun test` covers health, host-route delivery, and role-isolated signaling.
