const port = Number(process.env.PORT ?? 8787);
const sessions = new Map<string, Set<ServerWebSocket<{ role: string; session: string }>>>();

function sessionPeers(session: string) {
  let peers = sessions.get(session);
  if (!peers) {
    peers = new Set();
    sessions.set(session, peers);
  }
  return peers;
}

function relay(ws: ServerWebSocket<{ role: string; session: string }>, value: unknown) {
  const payload = JSON.stringify(value);
  for (const peer of sessionPeers(ws.data.session)) {
    if (peer !== ws && peer.data.role !== ws.data.role) peer.send(payload);
  }
}

const keyCodes: Record<string, number> = {
  Enter: 0x0d, Escape: 0x1b, ArrowUp: 0x26, ArrowDown: 0x28,
  ArrowLeft: 0x25, ArrowRight: 0x27, ";": 0xba, "'": 0xde,
  x: 0x58, X: 0x58, z: 0x5a, Z: 0x5a
};

function bridgeHostInput(value: any) {
  const key = typeof value?.key === "string" ? value.key : "";
  const code = keyCodes[key];
  if (!code || typeof value?.down !== "boolean") return;
  const script = decodeURIComponent(new URL("../../research/pivot/bridge_xenia_input.ps1", import.meta.url).pathname).replace(/^\/+/, "");
  Bun.spawn(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", script, "-Key", String(code), "-Down", String(value.down)], { stdout: "ignore", stderr: "ignore" });
}

const server = Bun.serve<{ role: string; session: string }>({
  port,
  fetch(request, server) {
    const url = new URL(request.url);
    if (url.pathname === "/health") {
      return Response.json({ ok: true, service: "fight-night-sbs-signaling", sessions: sessions.size });
    }
    if (url.pathname === "/ws") {
      const session = url.searchParams.get("session");
      const role = url.searchParams.get("role");
      if (!session || !/^[A-Za-z0-9_-]{6,64}$/.test(session) || !["browser", "host"].includes(role ?? "")) {
        return new Response("invalid session or role", { status: 400 });
      }
      if (server.upgrade(request, { data: { session, role: role! } })) return undefined;
      return new Response("websocket upgrade failed", { status: 500 });
    }
    if (url.pathname === "/host") return new Response(Bun.file(new URL("./host.html", import.meta.url)));
    return new Response(Bun.file(new URL("./index.html", import.meta.url)));
  },
  websocket: {
    open(ws) { sessionPeers(ws.data.session).add(ws); },
    message(ws, message) {
      try {
        const value = JSON.parse(String(message));
        if (!value || typeof value.type !== "string") throw new Error("message type required");
        if (!["offer", "answer", "ice", "host-ready", "host-stats", "host-input"].includes(value.type)) throw new Error("unsupported message type");
        if (value.type === "host-input" && ws.data.role !== "host") throw new Error("host-input requires host role");
        if (value.type === "host-input") bridgeHostInput(value);
        relay(ws, value);
      } catch (error) {
        ws.send(JSON.stringify({ type: "error", error: String(error instanceof Error ? error.message : error) }));
      }
    },
    close(ws) {
      const peers = sessions.get(ws.data.session);
      peers?.delete(ws);
      if (peers && peers.size === 0) sessions.delete(ws.data.session);
    }
  }
});

console.log(`Fight Night SBS signaling server listening on http://localhost:${server.port}`);
