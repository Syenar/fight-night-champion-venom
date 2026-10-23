# Host adapter contract

The host page uses the browser's WebRTC implementation, so no third-party Bun
WebRTC package is required. Start `/host?session=...` on the same machine as
Xenia, confirm that the selected source is packed left|right output, and select
the Xenia window when `getDisplayMedia()` prompts. The host sends the selected
track as received and reports its dimensions and frame rate through signaling.
The server validates and retains that format for a browser that joins later.

The host peer publishes one video track and accepts one ordered `controller`
data channel. The browser draws every received frame into an SBS canvas by
copying the track's left half to the left viewport and right half to the right
viewport. A dimension mismatch between the host report and decoded track is
surfaced. Controller messages are validated by the Bun server and sent to a
local PowerShell bridge that targets only a visible `xenia_canary` window
whose title contains `FIGHT NIGHT` (or the optional `XENIA_TARGET_PATH`).

This is a local capture/input adapter, not a renderer: eye separation must
already exist in the selected native frame. The checkbox and source dimensions
are declarations, not proof of distinct eye geometry. Browser playback is
still not accepted until a fresh critic verifies a live session, input
round-trip, reconnect, and long-session behavior with a real native SBS pair.

