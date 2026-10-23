const $ = (id) => document.getElementById(id);
let socket;
let peer;
let controls;
const status = (text) => { $("status").textContent = text; };
const sourceVideo = $("source-video");
const stereoView = $("stereo-view");
const stereoContext = stereoView.getContext("2d", { alpha: false });
let packedFormat = null;
let paintLoopStarted = false;
let dimensionWarningShown = false;

const querySession = new URL(location.href).searchParams.get("session");
if (querySession) $("session").value = querySession;

$("connect").onclick = async () => {
  const session = $("session").value.trim();
  if (!/^[A-Za-z0-9_-]{6,64}$/.test(session)) return status("Session must be 6–64 URL-safe characters.");
  peer = new RTCPeerConnection();
  // Request the native host's already-packed SBS video track. Without an
  // explicit recvonly transceiver Chrome creates a data-only offer and the
  // host has no media section to answer.
  peer.addTransceiver("video", { direction: "recvonly" });
  controls = peer.createDataChannel("controller", { ordered: true });
  controls.onopen = () => status("Connected. Controller input is active; waiting for packed left|right frames.");
  peer.ontrack = ({ streams: [stream] }) => {
    sourceVideo.srcObject = stream;
    sourceVideo.onloadedmetadata = () => {
      sourceVideo.play().catch(() => status("Video arrived; press play in the browser if playback is paused."));
      paintPackedFrame();
    };
    status("Video track negotiated; waiting for the host's packed-frame format.");
  };
  socket = new WebSocket(`${location.protocol === "https:" ? "wss" : "ws"}://${location.host}/ws?session=${encodeURIComponent(session)}&role=browser`);
  socket.onopen = async () => {
    const offer = await peer.createOffer();
    await peer.setLocalDescription(offer);
    socket.send(JSON.stringify({ type: "offer", sdp: peer.localDescription }));
    status("Signaling connected; waiting for native host.");
  };
  socket.onmessage = async ({ data }) => {
    const message = JSON.parse(data);
    if (message.type === "answer") await peer.setRemoteDescription(message.sdp);
    else if (message.type === "ice" && message.candidate) await peer.addIceCandidate(message.candidate);
    else if (message.type === "host-ready") status("Native host ready; negotiating video.");
    else if (message.type === "host-video-format") {
      packedFormat = message;
      status(`Host reports ${message.layout} frames ${message.width}×${message.height} at ${message.frameRate.toFixed(1)} fps. Browser will split each transmitted frame into its left and right halves.`);
      paintPackedFrame();
    }
    else if (message.type === "host-ended") {
      packedFormat = null;
      sourceVideo.srcObject = null;
      stereoContext.clearRect(0, 0, stereoView.width, stereoView.height);
      status("Host capture ended; waiting for a new packed-frame source.");
    }
    else if (message.type === "error") status(`Signaling error: ${message.error}`);
  };
  peer.onicecandidate = ({ candidate }) => candidate && socket.readyState === WebSocket.OPEN && socket.send(JSON.stringify({ type: "ice", candidate }));
};

function paintPackedFrame() {
  if (paintLoopStarted) return;
  paintLoopStarted = true;
  const draw = () => {
    if (sourceVideo.readyState >= HTMLMediaElement.HAVE_CURRENT_DATA && packedFormat?.layout === "side-by-side") {
      const width = sourceVideo.videoWidth;
      const height = sourceVideo.videoHeight;
      const leftWidth = Math.floor(width / 2);
      const rightWidth = width - leftWidth;
      if (width > 1 && height > 0) {
        if ((width !== packedFormat.width || height !== packedFormat.height) && !dimensionWarningShown) {
          dimensionWarningShown = true;
          status(`Host reported ${packedFormat.width}×${packedFormat.height}, but the received track is ${width}×${height}; check the selected source.`);
        }
        if (stereoView.width !== width || stereoView.height !== height) {
          stereoView.width = width;
          stereoView.height = height;
        }
        stereoContext.imageSmoothingEnabled = false;
        stereoContext.drawImage(sourceVideo, 0, 0, leftWidth, height, 0, 0, leftWidth, height);
        stereoContext.drawImage(sourceVideo, leftWidth, 0, rightWidth, height, leftWidth, 0, rightWidth, height);
      }
    }
    requestAnimationFrame(draw);
  };
  requestAnimationFrame(draw);
}

addEventListener("keydown", (event) => controls?.readyState === "open" && controls.send(JSON.stringify({ type: "key", key: event.key, down: true })));
addEventListener("keyup", (event) => controls?.readyState === "open" && controls.send(JSON.stringify({ type: "key", key: event.key, down: false })));
$("fullscreen").onclick = () => stereoView.requestFullscreen();

