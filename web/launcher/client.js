const $ = (id) => document.getElementById(id);
let socket;
let peer;
let controls;
const status = (text) => { $("status").textContent = text; };

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
  controls.onopen = () => status("Connected. Controller input is active; waiting for SBS video.");
  peer.ontrack = ({ streams: [stream] }) => { $("video").srcObject = stream; status("Connected. Receiving SBS video."); };
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
    else if (message.type === "error") status(`Signaling error: ${message.error}`);
  };
  peer.onicecandidate = ({ candidate }) => candidate && socket.readyState === WebSocket.OPEN && socket.send(JSON.stringify({ type: "ice", candidate }));
};

addEventListener("keydown", (event) => controls?.readyState === "open" && controls.send(JSON.stringify({ type: "key", key: event.key, down: true })));
addEventListener("keyup", (event) => controls?.readyState === "open" && controls.send(JSON.stringify({ type: "key", key: event.key, down: false })));
$("fullscreen").onclick = () => $("video").requestFullscreen();
