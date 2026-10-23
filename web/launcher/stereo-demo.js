const canvas = document.querySelector("#scene");
const metric = document.querySelector("#metric");
const ipdInput = document.querySelector("#ipd");
const ipdValue = document.querySelector("#ipd-value");
const gl = canvas.getContext("webgl", { antialias: true, alpha: false, preserveDrawingBuffer: true });

if (!gl) {
  metric.textContent = "WebGL is unavailable in this browser.";
  metric.classList.add("error");
  throw new Error("WebGL context creation failed");
}

const vertexSource = `
attribute vec3 a_position;
attribute vec3 a_color;
uniform mat4 u_mvp;
varying vec3 v_color;
void main() { gl_Position = u_mvp * vec4(a_position, 1.0); v_color = a_color; }
`;
const fragmentSource = `
precision mediump float;
varying vec3 v_color;
void main() { gl_FragColor = vec4(v_color, 1.0); }
`;

function compile(type, source) {
  const shader = gl.createShader(type);
  gl.shaderSource(shader, source);
  gl.compileShader(shader);
  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) throw new Error(gl.getShaderInfoLog(shader));
  return shader;
}

const program = gl.createProgram();
gl.attachShader(program, compile(gl.VERTEX_SHADER, vertexSource));
gl.attachShader(program, compile(gl.FRAGMENT_SHADER, fragmentSource));
gl.linkProgram(program);
if (!gl.getProgramParameter(program, gl.LINK_STATUS)) throw new Error(gl.getProgramInfoLog(program));
gl.useProgram(program);

const cubeFaces = [
  { n: [0, 0, 1], c: [0.88, 0.90, 0.93], p: [[-1,-1,1],[1,-1,1],[1,1,1],[-1,1,1]] },
  { n: [0, 0,-1], c: [0.47, 0.52, 0.60], p: [[1,-1,-1],[-1,-1,-1],[-1,1,-1],[1,1,-1]] },
  { n: [0, 1, 0], c: [0.76, 0.80, 0.86], p: [[-1,1,1],[1,1,1],[1,1,-1],[-1,1,-1]] },
  { n: [0,-1, 0], c: [0.35, 0.40, 0.47], p: [[-1,-1,-1],[1,-1,-1],[1,-1,1],[-1,-1,1]] },
  { n: [1, 0, 0], c: [0.63, 0.68, 0.74], p: [[1,-1,1],[1,-1,-1],[1,1,-1],[1,1,1]] },
  { n: [-1,0, 0], c: [0.54, 0.59, 0.66], p: [[-1,-1,-1],[-1,-1,1],[-1,1,1],[-1,1,-1]] }
];
const corners = [0, 1, 2, 0, 2, 3];
const vertices = [];
for (const face of cubeFaces) for (const corner of corners) vertices.push(...face.p[corner], ...face.c);
const vertexBuffer = gl.createBuffer();
gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
const stride = 6 * Float32Array.BYTES_PER_ELEMENT;
const positionLoc = gl.getAttribLocation(program, "a_position");
const colorLoc = gl.getAttribLocation(program, "a_color");
gl.enableVertexAttribArray(positionLoc);
gl.vertexAttribPointer(positionLoc, 3, gl.FLOAT, false, stride, 0);
gl.enableVertexAttribArray(colorLoc);
gl.vertexAttribPointer(colorLoc, 3, gl.FLOAT, false, stride, 3 * Float32Array.BYTES_PER_ELEMENT);
const mvpLoc = gl.getUniformLocation(program, "u_mvp");

function multiply(a, b) {
  const out = new Float32Array(16);
  for (let col = 0; col < 4; col++) for (let row = 0; row < 4; row++) {
    out[col * 4 + row] = a[row] * b[col * 4] + a[4 + row] * b[col * 4 + 1] + a[8 + row] * b[col * 4 + 2] + a[12 + row] * b[col * 4 + 3];
  }
  return out;
}

function model(x, y, z, sx, sy, sz) {
  return new Float32Array([sx,0,0,0, 0,sy,0,0, 0,0,sz,0, x,y,z,1]);
}

function viewFromEye(eyeX) {
  return new Float32Array([1,0,0,0, 0,1,0,0, 0,0,1,0, -eyeX,-1.35,-7.2,1]);
}

function perspectiveOffAxis(aspect, eyeX) {
  const near = 0.1, far = 40, fov = Math.PI / 3;
  const f = 1 / Math.tan(fov / 2);
  const out = new Float32Array(16);
  out[0] = f / aspect;
  out[5] = f;
  // Parallel eye cameras converge at the center of the ring (z=0).
  out[8] = -f * eyeX / (aspect * 7.2);
  out[10] = (far + near) / (near - far);
  out[11] = -1;
  out[14] = (2 * far * near) / (near - far);
  return out;
}

function boxerParts(x, z, shirt, shorts, bob, guard) {
  const arm = guard * 0.22;
  return [
    [x, 1.22 + bob, z, 0.42, 0.58, 0.24, shirt],
    [x, 1.95 + bob, z, 0.25, 0.28, 0.25, [0.72,0.49,0.34]],
    [x - 0.48, 1.42 + bob + arm, z, 0.13, 0.48, 0.14, shirt],
    [x + 0.48, 1.42 + bob - arm, z, 0.13, 0.48, 0.14, shirt],
    [x - 0.51, 0.92 + bob + arm, z, 0.19, 0.17, 0.20, shorts],
    [x + 0.51, 0.92 + bob - arm, z, 0.19, 0.17, 0.20, shorts],
    [x - 0.17, 0.45 + bob, z, 0.15, 0.50, 0.17, [0.76,0.53,0.37]],
    [x + 0.17, 0.45 + bob, z, 0.15, 0.50, 0.17, [0.76,0.53,0.37]]
  ];
}

function makeScene(time) {
  const bob = Math.sin(time * 3) * 0.035;
  const guard = Math.sin(time * 2.1);
  return [
    [0, -0.2, 0, 4.6, 0.18, 3.0, [0.28,0.34,0.40]],
    [-3.3, 1.7, 0, 0.11, 1.8, 0.11, [0.72,0.75,0.79]], [3.3, 1.7, 0, 0.11, 1.8, 0.11, [0.72,0.75,0.79]],
    [0, 1.7, -2.8, 3.3, 0.11, 0.11, [0.78,0.80,0.84]], [0, 1.7, 2.8, 3.3, 0.11, 0.11, [0.78,0.80,0.84]],
    [0, 0.55, -2.8, 3.3, 0.07, 0.07, [0.88,0.18,0.16]], [0, 1.0, -2.8, 3.3, 0.07, 0.07, [0.90,0.90,0.90]],
    [0, 1.45, -2.8, 3.3, 0.07, 0.07, [0.15,0.43,0.86]], [0, 0.55, 2.8, 3.3, 0.07, 0.07, [0.88,0.18,0.16]],
    [0, 1.0, 2.8, 3.3, 0.07, 0.07, [0.90,0.90,0.90]], [0, 1.45, 2.8, 3.3, 0.07, 0.07, [0.15,0.43,0.86]],
    ...boxerParts(-1.0, 0.1, [0.62,0.12,0.11], [0.30,0.08,0.08], bob, guard),
    ...boxerParts(1.0, -0.45, [0.10,0.31,0.66], [0.08,0.17,0.38], -bob, -guard),
    [0, 0.45, 1.35, 0.10, 0.45, 0.10, [0.98,0.75,0.10]],
    [0, 0.08, 1.35, 0.40, 0.06, 0.08, [0.98,0.75,0.10]]
  ];
}

function resize() {
  const rect = canvas.getBoundingClientRect();
  const dpr = Math.min(devicePixelRatio || 1, 2);
  const width = Math.max(2, Math.round(rect.width * dpr));
  const height = Math.max(2, Math.round(rect.height * dpr));
  if (canvas.width !== width || canvas.height !== height) { canvas.width = width; canvas.height = height; }
}

let paused = false;
let started = performance.now();
let pausedAt = 0;
let pausedAccum = 0;
let fpsWindowStart = started;
let frameCount = 0;
let fps = 0;
function draw(now) {
  resize();
  const width = canvas.width;
  const height = canvas.height;
  const eyeWidth = Math.floor(width / 2);
  const sceneTime = ((paused ? pausedAt : now) - started - pausedAccum) / 1000;
  const ipd = Number(ipdInput.value);
  ipdValue.value = ipd.toFixed(3);
  const objects = makeScene(sceneTime);
  gl.enable(gl.DEPTH_TEST);
  gl.clearColor(0.025, 0.045, 0.075, 1);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

  // Both eye passes use one scene snapshot/time value and distinct camera transforms.
  for (const eye of [-1, 1]) {
    const eyeX = eye * ipd * 0.5;
    const x = eye < 0 ? 0 : eyeWidth;
    const viewportWidth = eye < 0 ? eyeWidth : width - eyeWidth;
    const aspect = viewportWidth / height;
    const projection = perspectiveOffAxis(aspect, eyeX);
    const view = viewFromEye(eyeX);
    gl.viewport(x, 0, viewportWidth, height);
    for (const [px, py, pz, sx, sy, sz, color] of objects) {
      const mvp = multiply(projection, multiply(view, model(px, py, pz, sx, sy, sz)));
      gl.uniformMatrix4fv(mvpLoc, false, mvp);
      gl.drawArrays(gl.TRIANGLES, 0, vertices.length / 6);
    }
  }

  frameCount++;
  if (now - fpsWindowStart >= 1000) { fps = frameCount * 1000 / (now - fpsWindowStart); frameCount = 0; fpsWindowStart = now; }
  metric.textContent = `Frame ${Math.floor(now)} · ${fps.toFixed(0)} fps · 2 eye passes`;
  requestAnimationFrame(draw);
}

document.querySelector("#pause").addEventListener("click", (event) => {
  paused = !paused;
  if (paused) pausedAt = performance.now();
  else pausedAccum += performance.now() - pausedAt;
  event.currentTarget.textContent = paused ? "Resume scene" : "Pause scene";
});
document.querySelector("#fullscreen").addEventListener("click", () => canvas.parentElement.requestFullscreen());
new ResizeObserver(resize).observe(canvas);
requestAnimationFrame(draw);

