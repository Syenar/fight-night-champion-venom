# Host adapter contract

The host page uses the browser's WebRTC implementation, so no third-party Bun
WebRTC package is required. Start `/host?session=...` on the same machine as
Xenia and select the Xenia window when `getDisplayMedia()` prompts. The
selected window must already show the real left|right SBS output.

The host peer publishes one video track and accepts one ordered `controller`
data channel. Controller messages are validated by the Bun server and sent to
a local PowerShell bridge that targets only a visible `xenia_canary` window
whose title contains `FIGHT NIGHT` (or the optional `XENIA_TARGET_PATH`).

This is a local capture/input adapter, not a fake renderer. Browser playback
is still not accepted until a fresh critic verifies a live session, input
round-trip, reconnect, and long-session behavior.
