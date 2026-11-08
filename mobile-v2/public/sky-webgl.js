(function () {
  "use strict";

  function compileShader(gl, type, source) {
    const shader = gl.createShader(type);
    if (!shader) throw new Error("Impossible de créer un shader WebGL.");
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
      const message = gl.getShaderInfoLog(shader);
      gl.deleteShader(shader);
      throw new Error(`Compilation du shader WebGL impossible : ${message}`);
    }
    return shader;
  }

  function createRenderer(canvas) {
    const gl = canvas.getContext("webgl", { alpha: true, antialias: true, depth: false });
    if (!gl) throw new Error("WebGL n'est pas disponible.");
    const vertexShader = compileShader(gl, gl.VERTEX_SHADER, `
      attribute vec2 position;
      attribute vec4 color;
      varying vec4 vertexColor;
      void main() {
        gl_Position = vec4(position, 0.0, 1.0);
        vertexColor = color;
      }`);
    const fragmentShader = compileShader(gl, gl.FRAGMENT_SHADER, `
      precision mediump float;
      varying vec4 vertexColor;
      void main() { gl_FragColor = vertexColor; }`);
    const program = gl.createProgram();
    if (!program) throw new Error("Impossible de créer le programme WebGL.");
    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragmentShader);
    gl.linkProgram(program);
    gl.deleteShader(vertexShader);
    gl.deleteShader(fragmentShader);
    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
      throw new Error(`Initialisation du programme WebGL impossible : ${gl.getProgramInfoLog(program)}`);
    }
    const buffer = gl.createBuffer();
    if (!buffer) throw new Error("Impossible de créer le tampon WebGL.");
    const position = gl.getAttribLocation(program, "position");
    const color = gl.getAttribLocation(program, "color");
    gl.useProgram(program);
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.enableVertexAttribArray(position);
    gl.enableVertexAttribArray(color);
    gl.vertexAttribPointer(position, 2, gl.FLOAT, false, 24, 0);
    gl.vertexAttribPointer(color, 4, gl.FLOAT, false, 24, 8);
    gl.enable(gl.BLEND);
    gl.blendFuncSeparate(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA, gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
    gl.clearColor(0, 0, 0, 0);

    let vertices = [];
    let width = 1;
    let height = 1;
    let path = [];
    let currentPath;
    let state = { fillStyle: "#000000", strokeStyle: "#000000", lineWidth: 1, globalAlpha: 1, dash: [], shadowColor: "#000000", shadowBlur: 0 };
    const stack = [];

    function rgba(value) {
      if (value[0] === "#" && value.length === 7) {
        return [parseInt(value.slice(1, 3), 16) / 255, parseInt(value.slice(3, 5), 16) / 255, parseInt(value.slice(5, 7), 16) / 255, state.globalAlpha];
      }
      const match = /^rgba?\(\s*([\d.]+)\s*,\s*([\d.]+)\s*,\s*([\d.]+)(?:\s*,\s*([\d.]+))?\s*\)$/.exec(value);
      if (!match) throw new Error(`Couleur WebGL non prise en charge : ${value}`);
      return [Number(match[1]) / 255, Number(match[2]) / 255, Number(match[3]) / 255, (match[4] === undefined ? 1 : Number(match[4])) * state.globalAlpha];
    }

    function vertex(x, y, colorValue) {
      vertices.push(2 * x / width - 1, 1 - 2 * y / height, ...colorValue);
    }

    function triangle(a, b, c, colorValue) {
      vertex(a[0], a[1], colorValue);
      vertex(b[0], b[1], colorValue);
      vertex(c[0], c[1], colorValue);
    }

    function line(a, b, colorValue) {
      let [x1, y1] = a;
      let [x2, y2] = b;
      if (![x1, y1, x2, y2].every(Number.isFinite)) return;
      const dx = x2 - x1;
      const dy = y2 - y1;
      let start = 0;
      let end = 1;
      const margin = state.lineWidth + 2;
      for (const [p, q] of [[-dx, x1 + margin], [dx, width + margin - x1], [-dy, y1 + margin], [dy, height + margin - y1]]) {
        if (p === 0) {
          if (q < 0) return;
        } else {
          const t = q / p;
          if (p < 0) start = Math.max(start, t);
          else end = Math.min(end, t);
        }
      }
      if (start > end) return;
      x2 = x1 + dx * end;
      y2 = y1 + dy * end;
      x1 += dx * start;
      y1 += dy * start;
      const length = Math.hypot(x2 - x1, y2 - y1);
      if (!length) return;
      const nx = -(y2 - y1) / length * state.lineWidth / 2;
      const ny = (x2 - x1) / length * state.lineWidth / 2;
      const p1 = [x1 + nx, y1 + ny], p2 = [x1 - nx, y1 - ny];
      const p3 = [x2 + nx, y2 + ny], p4 = [x2 - nx, y2 - ny];
      triangle(p1, p2, p3, colorValue);
      triangle(p3, p2, p4, colorValue);
    }

    function curve(x, y, radiusX, radiusY, rotation, startAngle, endAngle) {
      const span = Math.min(2 * Math.PI, Math.max(0, endAngle - startAngle));
      const count = Math.max(16, Math.ceil(span * Math.max(radiusX, radiusY) / 3));
      const cos = Math.cos(rotation), sin = Math.sin(rotation);
      const points = [];
      for (let i = 0; i <= count; i++) {
        const angle = startAngle + span * i / count;
        const dx = radiusX * Math.cos(angle), dy = radiusY * Math.sin(angle);
        points.push([x + dx * cos - dy * sin, y + dx * sin + dy * cos]);
      }
      path.push(points);
      currentPath = points;
    }

    const renderer = {
      begin(widthValue, heightValue, pixelWidth, pixelHeight) {
        width = widthValue;
        height = heightValue;
        if (canvas.width !== pixelWidth) canvas.width = pixelWidth;
        if (canvas.height !== pixelHeight) canvas.height = pixelHeight;
        gl.viewport(0, 0, canvas.width, canvas.height);
        gl.clear(gl.COLOR_BUFFER_BIT);
        vertices = [];
      },
      flush() {
        if (!vertices.length) return;
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.DYNAMIC_DRAW);
        gl.drawArrays(gl.TRIANGLES, 0, vertices.length / 6);
      },
      beginPath() { path = []; currentPath = undefined; },
      moveTo(x, y) { currentPath = [[x, y]]; path.push(currentPath); },
      lineTo(x, y) {
        if (!currentPath) this.moveTo(x, y);
        else currentPath.push([x, y]);
      },
      closePath() {
        if (currentPath && currentPath.length > 1) currentPath.push(currentPath[0]);
      },
      arc(x, y, radius, start, end) { curve(x, y, radius, radius, 0, start, end); },
      ellipse(x, y, rx, ry, rotation, start, end) { curve(x, y, rx, ry, rotation, start, end); },
      stroke() {
        const colorValue = rgba(state.strokeStyle);
        for (const points of path) {
          let dashDistance = 0;
          const dash = state.dash;
          const dashLength = dash.reduce((sum, value) => sum + value, 0);
          for (let i = 1; i < points.length; i++) {
            const a = points[i - 1], b = points[i];
            const length = Math.hypot(b[0] - a[0], b[1] - a[1]);
            if (!dashLength || !Number.isFinite(length)) {
              line(a, b, colorValue);
              continue;
            }
            let travelled = 0;
            while (travelled < length) {
              const phase = dashDistance % dashLength;
              let index = 0, offset = phase;
              while (index < dash.length - 1 && offset >= dash[index]) offset -= dash[index++];
              const step = Math.min(length - travelled, dash[index] - offset);
              if (step <= 0) break;
              if (index % 2 === 0) {
                const start = travelled / length, end = (travelled + step) / length;
                line([a[0] + (b[0] - a[0]) * start, a[1] + (b[1] - a[1]) * start],
                  [a[0] + (b[0] - a[0]) * end, a[1] + (b[1] - a[1]) * end], colorValue);
              }
              travelled += step;
              dashDistance += step;
            }
          }
        }
      },
      fill() {
        const colorValue = rgba(state.fillStyle);
        for (const points of path) {
          if (state.shadowBlur > 0 && points.length > 3) {
            const count = points.length - 1;
            const center = points.slice(0, count).reduce((sum, point) => [sum[0] + point[0] / count, sum[1] + point[1] / count], [0, 0]);
            const radius = Math.hypot(points[0][0] - center[0], points[0][1] - center[1]);
            if (radius > 0) {
              const glow = rgba(state.shadowColor);
              glow[3] *= 0.3;
              const transparent = [glow[0], glow[1], glow[2], 0];
              const scale = 1 + state.shadowBlur / radius;
              for (let i = 0; i < count; i++) {
                const a = points[i], b = points[(i + 1) % count];
                vertex(center[0], center[1], glow);
                vertex(center[0] + (a[0] - center[0]) * scale, center[1] + (a[1] - center[1]) * scale, transparent);
                vertex(center[0] + (b[0] - center[0]) * scale, center[1] + (b[1] - center[1]) * scale, transparent);
              }
            }
          }
          for (let i = 1; i + 1 < points.length; i++) triangle(points[0], points[i], points[i + 1], colorValue);
        }
      },
      save() { stack.push({ ...state, dash: [...state.dash] }); },
      restore() { if (stack.length) state = stack.pop(); },
      setLineDash(dash) { state.dash = [...dash]; },
      set fillStyle(value) { state.fillStyle = value; },
      set strokeStyle(value) { state.strokeStyle = value; },
      set lineWidth(value) { state.lineWidth = value; },
      set globalAlpha(value) { state.globalAlpha = value; },
      set shadowColor(value) { state.shadowColor = value; },
      set shadowBlur(value) { state.shadowBlur = value; }
    };
    return renderer;
  }

  window.createDobsonSkyContext = function (canvas) {
    const fallback = canvas.getContext("2d");
    const gpuCanvas = document.createElement("canvas");
    gpuCanvas.className = "dobson-webgl-canvas";
    gpuCanvas.setAttribute("aria-hidden", "true");
    canvas.parentElement.insertBefore(gpuCanvas, canvas);
    let renderer;
    let active = false;
    try {
      renderer = createRenderer(gpuCanvas);
      active = true;
    } catch (error) {
      console.warn("Rendu WebGL indisponible, utilisation de Canvas 2D.", error);
      gpuCanvas.remove();
      return fallback;
    }
    gpuCanvas.addEventListener("webglcontextlost", (event) => {
      event.preventDefault();
      active = false;
      gpuCanvas.hidden = true;
      console.warn("Contexte WebGL perdu, utilisation de Canvas 2D.");
    });

    const methods = ["beginPath", "moveTo", "lineTo", "closePath", "arc", "ellipse", "stroke", "fill", "save", "restore", "setLineDash"];
    const wrapper = {
      setTransform(...args) { fallback.setTransform(...args); },
      clearRect(x, y, width, height) {
        fallback.clearRect(x, y, width, height);
        if (active) renderer.begin(width, height, canvas.width, canvas.height);
      },
      flush() { if (active) renderer.flush(); }
    };
    for (const method of methods) {
      wrapper[method] = (...args) => {
        if (!active || (method !== "fill" && method !== "stroke")) fallback[method](...args);
        if (active) renderer[method](...args);
      };
    }
    for (const property of ["fillStyle", "strokeStyle", "lineWidth", "globalAlpha", "shadowColor", "shadowBlur"]) {
      Object.defineProperty(wrapper, property, {
        set(value) {
          fallback[property] = value;
          if (active) renderer[property] = value;
        }
      });
    }
    return wrapper;
  };
})();
