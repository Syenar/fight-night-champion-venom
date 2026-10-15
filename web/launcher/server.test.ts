import { expect, test } from "bun:test";

test("health endpoint reports the signaling service", async () => {
  const port = 18787;
  const proc = Bun.spawn(["bun", "run", "server.ts"], { cwd: import.meta.dir, env: { ...process.env, PORT: String(port) }, stdout: "pipe", stderr: "pipe" });
  try {
    for (let i = 0; i < 30; i++) {
      try {
        const response = await fetch(`http://127.0.0.1:${port}/health`);
        expect(response.status).toBe(200);
        expect(await response.json()).toMatchObject({ ok: true, service: "fight-night-sbs-signaling" });
        return;
      } catch { await Bun.sleep(100); }
    }
    throw new Error("server did not start");
  } finally { proc.kill(); }
});

test("host route serves the local capture adapter", async () => {
  const port = 18789;
  const proc = Bun.spawn(["bun", "run", "server.ts"], { cwd: import.meta.dir, env: { ...process.env, PORT: String(port) }, stdout: "pipe", stderr: "pipe" });
  try {
    for (let i = 0; i < 30; i++) {
      try { const response = await fetch(`http://127.0.0.1:${port}/host`); expect(response.status).toBe(200); expect(await response.text()).toContain("Fight Night SBS local host"); return; } catch { await Bun.sleep(100); }
    }
    throw new Error("host route did not start");
  } finally { proc.kill(); }
});

test("offer is relayed only between roles in the same session", async () => {
  const port = 18788;
  const proc = Bun.spawn(["bun", "run", "server.ts"], { cwd: import.meta.dir, env: { ...process.env, PORT: String(port) }, stdout: "pipe", stderr: "pipe" });
  try {
    for (let i = 0; i < 30; i++) {
      try { await fetch(`http://127.0.0.1:${port}/health`); break; } catch { await Bun.sleep(100); }
    }
    const browser = new WebSocket(`ws://127.0.0.1:${port}/ws?session=relay01&role=browser`);
    const host = new WebSocket(`ws://127.0.0.1:${port}/ws?session=relay01&role=host`);
    const received = new Promise<any>((resolve, reject) => {
      const timer = setTimeout(() => reject(new Error("relay timeout")), 2000);
      host.onmessage = (event) => { clearTimeout(timer); resolve(JSON.parse(String(event.data))); };
    });
    await new Promise<void>((resolve, reject) => { browser.onopen = () => resolve(); browser.onerror = () => reject(new Error("browser socket failed")); });
    await new Promise<void>((resolve, reject) => { host.onopen = () => resolve(); host.onerror = () => reject(new Error("host socket failed")); });
    browser.send(JSON.stringify({ type: "offer", sdp: { type: "offer", sdp: "test" } }));
    expect(await received).toMatchObject({ type: "offer", sdp: { type: "offer" } });
    browser.close(); host.close();
  } finally { proc.kill(); }
});
