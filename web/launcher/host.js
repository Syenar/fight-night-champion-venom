const $ = (id) => document.getElementById(id);
let socket, peer, stream;
const status = (text) => { $("status").textContent = text; };
let videoFormatSent = false;

function announceVideoFormat() {
  if (videoFormatSent || socket?.readyState !== WebSocket.OPEN || !stream) return;
  const preview = $("preview");
  const width = preview.videoWidth;
  const height = preview.videoHeight;
  if (!width || !height) return;
  const frameRate = stream.getVideoTracks()[0]?.getSettings().frameRate ?? 0;
  socket.send(JSON.stringify({ type: "host-video-format", width, height, frameRate, layout: "side-by-side" }));
  videoFormatSent = true;
  status(`Host reports packed L|R source: ${width}×${height} at ${frameRate.toFixed(1)} fps. Waiting for browser connection.`);
}

const querySession = new URL(location.href).searchParams.get("session");
if (querySession) $("session").value = querySession;

$("start").onclick = async () => {
  const session = $("session").value.trim();
  if (!/^[A-Za-z0-9_-]{6,64}$/.test(session)) return status("Session must be 6–64 URL-safe characters.");
  if (!$('packed-sbs').checked) return status("Confirm that the selected Xenia window is already showing packed left|right SBS output.");
  if (!navigator.mediaDevices?.getDisplayMedia) return status("This browser does not expose screen capture. Use a secure context or localhost in a desktop browser.");
  try {
    videoFormatSent = false;
    stream = await navigator.mediaDevices.getDisplayMedia({ video: { frameRate: { ideal: 60, max: 60 } }, audio: false });
    const preview = $("preview");
    preview.srcObject = stream;
    preview.onloadedmetadata = announceVideoFormat;
    await preview.play();
    peer = new RTCPeerConnection();
    for (const track of stream.getVideoTracks()) peer.addTrack(track, stream);
    peer.onicecandidate = ({ candidate }) => candidate && socket?.readyState === WebSocket.OPEN && socket.send(JSON.stringify({ type: "ice", candidate }));
    peer.ondatachannel = ({ channel }) => {
      if (channel.label !== "controller") return;
      channel.onmessage = ({ data }) => {
        try { socket?.send(JSON.stringify({ type: "host-input", ...JSON.parse(data) })); } catch { /* malformed input is ignored */ }
      };
    };
    socket = new WebSocket(`${location.protocol === "https:" ? "wss" : "ws"}://${location.host}/ws?session=${encodeURIComponent(session)}&role=host`);
    socket.onopen = () => { announceVideoFormat(); status("Host ready; waiting for browser offer."); };
    socket.onmessage = async ({ data }) => {
      const message = JSON.parse(data);
      if (message.type === "offer") {
        await peer.setRemoteDescription(message.sdp);
        const answer = await peer.createAnswer();
        await peer.setLocalDescription(answer);
        socket.send(JSON.stringify({ type: "answer", sdp: peer.localDescription }));
        socket.send(JSON.stringify({ type: "host-ready" }));
        status("Connected. The selected Xenia frame track is being sent to the browser client.");
      } else if (message.type === "ice" && message.candidate) await peer.addIceCandidate(message.candidate);
      else if (message.type === "error") status(`Signaling error: ${message.error}`);
    };
    stream.getVideoTracks()[0].onended = () => status("Capture ended.");
  } catch (error) { status(`Capture failed: ${error?.message ?? error}`); }
};

