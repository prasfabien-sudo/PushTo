// Version Component: BetaSkyPage.h v5.2.0
// Version Global System: v1.2.7
#ifndef BETASKYPAGE_H
#define BETASKYPAGE_H

#include <pgmspace.h>

const char HTTP_BETA_SKY_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="theme-color" content="#08090c">
  <title>Vue 3D du ciel — Bêta</title>
  <style>
    * { box-sizing: border-box; }
    body { margin: 0; padding: 12px 12px 90px; min-height: 100vh; background: #08090c; color: #d8dce6; font: 14px -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; }
    .sky-card { width: min(100%, 760px); margin: 0 auto; }
    .sky-heading { display: flex; align-items: center; justify-content: space-between; gap: 10px; margin: 2px 2px 10px; }
    h1 { margin: 0; font-size: 1.15rem; color: #f1f3f8; }
    .beta { padding: 3px 7px; border: 1px solid #74531e; border-radius: 999px; color: #ffc66d; font-size: .7rem; letter-spacing: .08em; }
    .sky-status { min-height: 20px; margin: 0 2px 8px; color: #aab3c2; font-size: .82rem; }
    .sky-status.error { color: #ff7777; }
    .sky-status.align-ok { color: #69ff84; }
    .scene { position: relative; height: min(65vh, 540px); min-height: 320px; overflow: hidden; border: 1px solid #273142; border-radius: 14px; background: radial-gradient(ellipse at 50% 100%, #1b2638 0, #0d1420 47%, #05070b 100%); touch-action: none; }
    canvas { display: block; width: 100%; height: 100%; cursor: grab; }
    canvas:active { cursor: grabbing; }
    .scene-label { position: absolute; color: #acb8c8; font-size: .72rem; text-shadow: 0 1px 4px #000; pointer-events: none; }
    .scene-label.north { top: 10px; left: 50%; transform: translateX(-50%); }
    .scene-label.telescope { top: 10px; left: 10px; }
    .scene-label.telescope.error { color: #ff7777; }
    .scene-label.view { left: 10px; bottom: 10px; }
    .scene-label.horizon { right: 10px; bottom: 10px; }
    .scene-center { position: absolute; top: 50%; left: 50%; width: 8px; height: 8px; border: 1px solid rgba(255,255,255,.7); border-radius: 50%; transform: translate(-50%,-50%); pointer-events: none; }
    
    .object-label { position: absolute; z-index: 1; padding: 2px 5px; border-radius: 4px; color: #f4e6be; background: rgba(5,8,12,0.75); font-size: .68rem; white-space: nowrap; text-shadow: 0 1px 3px #000; transform: translate(-50%,-120%); pointer-events: none; transition: opacity 0.2s; }
    .object-label.polaris { color: #ffb74d; background: rgba(30,35,45,0.9); border: 1px solid rgba(255,183,77,0.5); font-weight: bold; z-index: 3; }
    .constellation-label { position: absolute; z-index: 1; color: #7cb9e8; font-size: .75rem; font-weight: bold; letter-spacing: 0.05em; text-transform: uppercase; text-shadow: 0 1px 5px #000; transform: translate(-50%,-50%); pointer-events: none; opacity: 0.85; }

    .controls { display: grid; grid-template-columns: repeat(3, minmax(0,1fr)); gap: 8px; margin-top: 10px; }
    button, select { min-height: 44px; border: 1px solid #344256; border-radius: 9px; background: #151b25; color: #e6eaf2; font: inherit; }
    button { padding: 8px 10px; cursor: pointer; }
    button:active { background: #253247; }
    select { width: 100%; padding: 8px 10px; }
    .scene-help { margin: 9px 2px; color: #929cac; font-size: .78rem; line-height: 1.45; }

    /* Modale d'information objet */
    .object-modal { position: fixed; inset: 0; z-index: 100; background: rgba(0,0,0,0.6); display: flex; align-items: center; justify-content: center; padding: 16px; }
    .object-modal[hidden] { display: none; }
    .modal-content { width: min(100%, 380px); background: #151b25; border: 1px solid #344256; border-radius: 14px; padding: 20px; box-shadow: 0 8px 32px rgba(0,0,0,0.7); }
    .modal-content h2 { margin: 0 0 4px; font-size: 1.1rem; color: #ffb74d; }
    .modal-sub { color: #8b94a2; font-size: .85rem; margin-bottom: 12px; }
    .modal-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 8px; margin-bottom: 16px; }
    .modal-stat { background: #10151d; padding: 8px 10px; border-radius: 8px; border: 1px solid #222d3b; }
    .modal-stat-label { font-size: .7rem; color: #8b94a2; display: block; }
    .modal-stat-value { font-size: .95rem; font-weight: bold; color: #f1f3f8; }
    .modal-close { width: 100%; background: #263238; border: 1px solid #455a64; color: #fff; font-weight: bold; }

    html[data-night-mode="true"]::after { content: ""; position: fixed; inset: 0; z-index: 2147483647; pointer-events: none; background: rgba(255, 0, 0, 0.82); mix-blend-mode: multiply; filter: brightness(0.45); }
    .bottom-nav { position: fixed; z-index: 20; left: 0; right: 0; bottom: 0; display: flex; justify-content: space-around; gap: 4px; padding: 8px 8px calc(8px + env(safe-area-inset-bottom)); background: rgba(18,20,25,.98); border-top: 1px solid #37474f; }
    .nav-item, .nav-more-button { flex: 1; min-width: 0; min-height: 48px; display: flex; align-items: center; justify-content: center; padding: 8px 2px; border: 0; border-radius: 8px; background: transparent; color: #b0bec5; text-decoration: none; cursor: pointer; }
    .nav-item[aria-current="page"], .nav-more-button[aria-expanded="true"], .nav-more-button[aria-current="page"] { color: #ffb74d; background: #263238; }
    .night-mode-toggle { background: #f1f3f4; color: #101418; border: 1px solid #fff; }
    .night-mode-toggle[aria-pressed="true"] { background: #ff5252; color: #fff; }
    .nav-icon { display: flex; font-size: 22px; line-height: 1; }
    .ui-icon { display: inline-block; width: 1em; height: 1em; vertical-align: -.15em; fill: none; stroke: currentColor; stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; }
    .nav-icon .ui-icon { vertical-align: middle; }
    .nav-more { position: relative; flex: 1; min-width: 0; display: flex; }
    .nav-more-button { width: 100%; }
    .nav-more-menu { position: absolute; right: 0; bottom: calc(100% + 12px); width: min(250px, calc(100vw - 24px)); padding: 6px; border: 1px solid #455a64; border-radius: 12px; background: #1e1e1e; box-shadow: 0 4px 18px rgba(0,0,0,.55); }
    .nav-more-menu[hidden] { display: none; }
    .nav-more-link { display: block; padding: 11px 12px; border-radius: 7px; color: #e0e0e0; text-decoration: none; }
    .nav-more-link:hover, .nav-more-link[aria-current="page"] { background: #263238; color: #ffb74d; }
    @media (min-width: 700px) { .bottom-nav { left: 50%; right: auto; width: min(500px, calc(100% - 32px)); transform: translateX(-50%); border: 1px solid #37474f; border-bottom: 0; border-radius: 14px 14px 0 0; } }
  </style>
</head>
<body>
  <main class="sky-card">
    <header class="sky-heading">
      <h1><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-globe-2"></use></svg> Vue 3D du ciel</h1>
      <span class="beta">BÊTA</span>
    </header>
    <p id="skyStatus" class="sky-status" role="status">Initialisation du rendu 3D...</p>
    <div class="scene" id="scene">
      <canvas id="skyCanvas" aria-label="Carte 3D interactive du ciel"></canvas>
      <span class="scene-label north">N</span>
      <span class="scene-label telescope" id="telescopeReadout">Télescope : connexion...</span>
      <span class="scene-label view" id="viewReadout">Vue AZ 0° · ALT 0°</span>
      <span class="scene-label horizon">Horizon</span>
      <span class="scene-center" aria-hidden="true"></span>
      <div id="objectLabels" aria-hidden="true"></div>
      <div id="constellationLabels" aria-hidden="true"></div>
    </div>
    <div class="controls">
      <select id="typeFilter" aria-label="Filtrer les objets">
        <option value="all">Tous les objets</option>
        <option value="galaxie">Galaxies</option>
        <option value="nebuleuse">Nébuleuses</option>
        <option value="amas">Amas d'étoiles</option>
        <option value="etoile">Étoiles</option>
      </select>
      <button id="toggleNamesBtn" type="button">Noms : ON</button>
      <button id="centerButton" type="button">Recentrer</button>
    </div>
    <p class="scene-help">Glissez pour tourner, pincez pour zoomer. Polaris est marquée en permanence.</p>
  </main>

  <!-- Modale d'information objet -->
  <div class="object-modal" id="objectModal" hidden>
    <div class="modal-content">
      <h2 id="modalTitle">Nom de l'objet</h2>
      <div class="modal-sub" id="modalSub">Constellation · Type</div>
      <div class="modal-grid">
        <div class="modal-stat">
          <span class="modal-stat-label">Magnitude</span>
          <span class="modal-stat-value" id="modalMag">-</span>
        </div>
        <div class="modal-stat">
          <span class="modal-stat-label">Distance</span>
          <span class="modal-stat-value" id="modalDist">-</span>
        </div>
        <div class="modal-stat">
          <span class="modal-stat-label">Taille apparente</span>
          <span class="modal-stat-value" id="modalSize">-</span>
        </div>
        <div class="modal-stat">
          <span class="modal-stat-label">Coordonnées AD/Dec</span>
          <span class="modal-stat-value" id="modalCoords" style="font-size:0.8rem">-</span>
        </div>
      </div>
      <button class="modal-close" type="button" onclick="closeModal()">Fermer</button>
    </div>
  </div>

  <nav class="bottom-nav" aria-label="Navigation principale">
    <a class="nav-item" href="/" aria-label="Menu principal" title="Menu principal"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-telescope"></use></svg></span></a>
    <a class="nav-item" href="/station" aria-label="Mise en station" title="Mise en station"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-crosshair"></use></svg></span></a>
    <a class="nav-item" href="/beta_sky" aria-label="Vue 3D du ciel" aria-current="page" title="Vue 3D du ciel"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-globe-2"></use></svg></span></a>
    <a class="nav-item" href="/help" aria-label="Aide SkySafari" title="Aide SkySafari"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-book-open"></use></svg></span></a>
    <button class="nav-more-button night-mode-toggle" id="nightModeToggle" type="button" aria-label="Activer le mode nuit" title="Activer le mode nuit" aria-pressed="false" onclick="toggleNightMode()"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-moon"></use></svg></span></button>
    <div class="nav-more">
      <button class="nav-more-button" type="button" aria-label="Plus" title="Plus" aria-expanded="false" aria-controls="navMoreMenu" onclick="toggleMoreMenu()"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-more-horizontal"></use></svg></span></button>
      <div class="nav-more-menu" id="navMoreMenu" hidden>
        <a class="nav-more-link" href="/calib_page"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-crosshair"></use></svg> Calibration</a>
        <a class="nav-more-link" href="/config"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-settings"></use></svg> Configuration</a>
        <a class="nav-more-link" href="/simu"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-gamepad-2"></use></svg> Simulation</a>
        <a class="nav-more-link" href="/test"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-flask-conical"></use></svg> Tests système</a>
        <a class="nav-more-link" href="/releasenotes"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-clipboard-list"></use></svg> Notes de version</a>
      </div>
    </div>
  </nav>

  <script>
    const canvas = document.getElementById('skyCanvas');
    const gl = canvas.getContext('webgl', { antialias: true, alpha: true });
    const statusElement = document.getElementById('skyStatus');
    const filterElement = document.getElementById('typeFilter');
    const labelsElement = document.getElementById('objectLabels');
    const constLabelsElement = document.getElementById('constellationLabels');
    const viewReadout = document.getElementById('viewReadout');
    const telescopeReadout = document.getElementById('telescopeReadout');
    const toggleNamesBtn = document.getElementById('toggleNamesBtn');
    
    let objects = [];
    let latitude = 45.89;
    let longitude = 6.05;
    let heading = 0;
    let elevation = 35;
    let touchHeading = 0;
    let touchElevation = 0;
    let animationFrame = 0;
    let lastRenderAt = 0;
    let fieldOfView = 34;
    let encoderRequestInFlight = false;
    let showNames = true;
    let lastScreenObjects = [];

    const CONSTELLATION_LINES = [
      ["Dubhe", "Merak"], ["Merak", "Phecda"], ["Phecda", "Megrez"], 
      ["Megrez", "Dubhe"], ["Megrez", "Alioth"], ["Alioth", "Mizar"], ["Mizar", "Alkaid"],
      ["Polaris", "Yildun"], ["Yildun", "Zeta UMi"], ["Zeta UMi", "Epsilon UMi"],
      ["Epsilon UMi", "Pherkad"], ["Pherkad", "Kochab"],
      ["Caph", "Shedar"], ["Shedar", "Navi"], ["Navi", "Ruchbah"], ["Ruchbah", "Segin"],
      ["Deneb", "Sadr"], ["Sadr", "Albireo"], ["Fawaris", "Sadr"], ["Sadr", "Gienah"],
      ["Vega", "Sheliak"], ["Sheliak", "Sulafat"], ["Sulafat", "γ Lyr"], ["γ Lyr", "Vega"],
      ["Rigel", "Betelgeuse"], ["Betelgeuse", "Bellatrix"], ["Bellatrix", "Rigel"],
      ["Alnitak", "Alnilam"], ["Alnilam", "Mintaka"], ["Saiph", "Rigel"], ["Betelgeuse", "Meissa"]
    ];

    const CONSTELLATION_CENTERS = [
      { name: "Grande Ourse", refStar: "Alioth" },
      { name: "Petite Ourse", refStar: "Polaris" },
      { name: "Cassiopée", refStar: "Navi" },
      { name: "Cygne", refScan: "Sadr", refStar: "Sadr" },
      { name: "Lyre", refStar: "Vega" },
      { name: "Orion", refStar: "Alnilam" }
    ];

    function setStatus(message, type = '') {
      statusElement.textContent = message;
      statusElement.className = 'sky-status ' + type;
    }

    function getEnrichedDetails(obj) {
      let dist = "Indéterminée";
      let size = "15'";
      const name = (obj.name || "").toUpperCase();
      const type = (obj.type || "").toLowerCase();

      if (name.includes("M31")) { dist = "2.5 millions al"; size = "3.0° × 1.0°"; }
      else if (name.includes("M45")) { dist = "440 al"; size = "110'"; }
      else if (name.includes("M42")) { dist = "1 340 al"; size = "65' × 60'"; }
      else if (name.includes("M13")) { dist = "22 200 al"; size = "20'"; }
      else if (name.includes("M57")) { dist = "2 300 al"; size = "1.4' × 1.0'"; }
      else if (type.includes("galaxie")) { dist = "15 - 50 millions al"; size = "8' × 5'"; }
      else if (type.includes("nebuleuse")) { dist = "2 000 - 5 000 al"; size = "30'"; }
      else if (type.includes("amas")) { dist = "5 000 - 15 000 al"; size = "15'"; }
      else { dist = "10 - 500 al (Étoile)"; size = "Ponctuel"; }

      return { dist, size };
    }

    function applyEncoderStatus(status) {
      const rawAZ = Number(status.rawAZ);
      const rawALT = Number(status.rawALT);
      const ticksAZ = Number(status.ticksAZ);
      const ticksALT = Number(status.ticksALT);
      const dirAZ = Number(status.dirAZ);
      const dirALT = Number(status.dirALT);
      if (![rawAZ, rawALT, ticksAZ, ticksALT, dirAZ, dirALT].every(Number.isFinite)
        || ticksAZ <= 0 || ticksALT <= 0 || ![1, -1].includes(dirAZ) || ![1, -1].includes(dirALT)) {
        throw new Error('Données encodeurs invalides.');
      }

      const azimuth = ((rawAZ * dirAZ / ticksAZ * 360) % 360 + 360) % 360;
      const altitudeCircle = ((rawALT * dirALT / ticksALT * 360) % 360 + 360) % 360;
      heading = azimuth;
      elevation = altitudeCircle > 180 ? altitudeCircle - 360 : altitudeCircle;
      
      telescopeReadout.textContent = `Télescope AZ ${azimuth.toFixed(1)}° · ALT ${elevation.toFixed(1)}°`;
      telescopeReadout.classList.remove('error');

      const diffAz = Math.abs(heading - ((heading + touchHeading + 360) % 360));
      if (diffAz < 1.5 && Math.abs(elevation - (elevation + touchElevation)) < 1.5) {
        setStatus("Alignement AR OK : La vue correspond exactement au pointage télescope.", "align-ok");
      } else {
        setStatus("Mode AR actif : Ajustez le cadrage ou utilisez Recentrer pour synchroniser.", "");
      }
    }

    async function pollEncoderStatus() {
      if (encoderRequestInFlight) return;
      encoderRequestInFlight = true;
      try {
        const response = await fetch('/status', { cache: 'no-store' });
        if (!response.ok) throw new Error(`/status HTTP ${response.status}`);
        applyEncoderStatus(await response.json());
      } catch (error) {
        telescopeReadout.textContent = `Encodeurs hors ligne`;
        telescopeReadout.classList.add('error');
      } finally {
        encoderRequestInFlight = false;
      }
    }

    function setNightMode(enabled, persist = false) {
      document.documentElement.dataset.nightMode = String(enabled);
      const button = document.getElementById('nightModeToggle');
      const label = enabled ? 'Désactiver mode nuit' : 'Activer mode nuit';
      button.setAttribute('aria-label', label);
      button.setAttribute('aria-pressed', String(enabled));
      button.title = label;
      if (persist) localStorage.setItem('dobson-night-mode', String(enabled));
    }

    function toggleNightMode() {
      setNightMode(document.documentElement.dataset.nightMode !== 'true', true);
    }

    function toggleMoreMenu() {
      const menu = document.getElementById('navMoreMenu');
      const button = document.querySelector('.nav-more > .nav-more-button');
      menu.hidden = !menu.hidden;
      button.setAttribute('aria-expanded', String(!menu.hidden));
    }

    setNightMode(localStorage.getItem('dobson-night-mode') === 'true');

    function compileShader(type, source) {
      const shader = gl.createShader(type);
      if (!shader) throw new Error('Création shader impossible.');
      gl.shaderSource(shader, source);
      gl.compileShader(shader);
      if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        const msg = gl.getShaderInfoLog(shader);
        gl.deleteShader(shader);
        throw new Error(`Erreur WebGL : ${msg}`);
      }
      return shader;
    }

    function createProgram() {
      const vertex = compileShader(gl.VERTEX_SHADER, `
        attribute vec3 aPosition;
        attribute vec3 aColor;
        attribute float aSize;
        uniform vec3 uRight;
        uniform vec3 uUp;
        uniform vec3 uForward;
        uniform float uTanHalfFov;
        uniform float uAspect;
        uniform float uGrid;
        varying vec3 vColor;
        void main() {
          float depth = dot(aPosition, uForward);
          if (depth <= 0.01) {
            gl_Position = vec4(3.0, 3.0, 3.0, 1.0);
          } else {
            float x = dot(aPosition, uRight) / (depth * uTanHalfFov * uAspect);
            float y = dot(aPosition, uUp) / (depth * uTanHalfFov);
            gl_Position = vec4(x, y, 0.0, 1.0);
          }
          gl_PointSize = aSize;
          vColor = aColor;
        }`);
      const fragment = compileShader(gl.FRAGMENT_SHADER, `
        precision mediump float;
        uniform float uGrid;
        varying vec3 vColor;
        void main() {
          if (uGrid > 0.5) {
            gl_FragColor = vec4(vColor, 0.85);
            return;
          }
          vec2 point = gl_PointCoord - vec2(0.5);
          float edge = 1.0 - smoothstep(0.3, 0.5, length(point));
          if (edge < 0.05) discard;
          gl_FragColor = vec4(vColor, edge);
        }`);
      const program = gl.createProgram();
      gl.attachShader(program, vertex);
      gl.attachShader(program, fragment);
      gl.linkProgram(program);
      return program;
    }

    // Gestion avancée des couleurs spectrales et types d'objets
    function objectColor(object) {
      const type = (object.type || '').toUpperCase();
      const name = (object.name || '').toUpperCase();

      if (name === 'POLARIS') return [1.0, 0.95, 0.75]; // Éclat particulier pour la Polaire
      if (type.includes('PLAN')) return [0.43, 0.78, 1.0];
      if (type.includes('GAL')) return [0.84, 0.67, 1.0];
      if (type.includes('NEB')) return [1.0, 0.57, 0.48];
      if (type.includes('AMAS') || type.includes('GLOB')) return [1.0, 0.82, 0.42];

      // Classification spectrale des étoiles (O, B, A, F, G, K, M)
      if (type.includes('O') || type.includes('B')) return [0.60, 0.78, 1.0]; // Bleu / Blanc chaud
      if (type.includes('A')) return [0.90, 0.94, 1.0]; // Blanc pur
      if (type.includes('F')) return [1.0, 1.0, 0.85]; // Jaune-blanc
      if (type.includes('G')) return [1.0, 0.95, 0.60]; // Jaune (solaire)
      if (type.includes('K')) return [1.0, 0.75, 0.40]; // Orange
      if (type.includes('M')) return [1.0, 0.50, 0.30]; // Orange-rouge / Rouge froid

      return [0.91, 0.94, 1.0];
    }

    function matchesFilter(object) {
      const type = (object.type || '').toLowerCase();
      const filter = filterElement.value;
      if (filter === 'all') return true;
      if (filter === 'galaxie') return type.includes('gal');
      if (filter === 'nebuleuse') return type.includes('neb');
      if (filter === 'amas') return type.includes('amas') || type.includes('glob');
      return !type.includes('gal') && !type.includes('neb') && !type.includes('amas') && !type.includes('glob');
    }

    function horizontalPosition(object, siderealDegrees, latRadians) {
      const dec = object.dec * Math.PI / 180;
      const hourAngle = (siderealDegrees - object.ra * 15) * Math.PI / 180;
      const sinAltitude = Math.sin(dec) * Math.sin(latRadians) + Math.cos(dec) * Math.cos(latRadians) * Math.cos(hourAngle);
      const altitude = Math.asin(Math.max(-1, Math.min(1, sinAltitude)));
      const azimuth = Math.atan2(
        -Math.sin(hourAngle) * Math.cos(dec),
        Math.sin(dec) * Math.cos(latRadians) - Math.cos(dec) * Math.sin(latRadians) * Math.cos(hourAngle),
      );
      const cosAltitude = Math.cos(altitude);
      return {
        altitude: altitude * 180 / Math.PI,
        position: [Math.sin(azimuth) * cosAltitude, Math.sin(altitude), Math.cos(azimuth) * cosAltitude],
      };
    }

    function siderealDegrees(date) {
      const julianDate = date.getTime() / 86400000 + 2440587.5;
      const centuries = (julianDate - 2451545.0) / 36525;
      const greenwich = 280.46061837 + 360.98564736629 * (julianDate - 2451545.0)
        + 0.000387933 * centuries * centuries - centuries * centuries * centuries / 38710000;
      return ((greenwich + longitude) % 360 + 360) % 360;
    }

    function makeGrid() {
      const vertices = [];
      const add = (azimuth, altitude) => {
        const az = azimuth * Math.PI / 180;
        const alt = altitude * Math.PI / 180;
        vertices.push(Math.sin(az) * Math.cos(alt), Math.sin(alt), Math.cos(az) * Math.cos(alt));
      };
      for (let az = 0; az < 360; az += 2) { add(az, 0); add(az + 2, 0); }
      for (const altitude of [30, 60]) {
        for (let az = 0; az < 360; az += 2) { add(az, altitude); add(az + 2, altitude); }
      }
      for (let azimuth = 0; azimuth < 360; azimuth += 45) {
        for (let altitude = 0; altitude < 88; altitude += 2) { add(azimuth, altitude); add(azimuth, altitude + 2); }
      }
      return new Float32Array(vertices);
    }

    function resizeCanvas() {
      const ratio = Math.min(window.devicePixelRatio || 1, 2);
      const width = Math.max(1, Math.round(canvas.clientWidth * ratio));
      const height = Math.max(1, Math.round(canvas.clientHeight * ratio));
      if (canvas.width !== width || canvas.height !== height) {
        canvas.width = width;
        canvas.height = height;
        gl.viewport(0, 0, width, height);
      }
    }

    function render() {
      animationFrame = requestAnimationFrame(render);
      if (!gl || objects.length === 0) return;
      const now = performance.now();
      if (now - lastRenderAt < 33) return;
      lastRenderAt = now;
      resizeCanvas();
      const width = canvas.width;
      const height = canvas.height;
      const aspect = width / height;
      const currentHeading = (heading + touchHeading + 360) % 360;
      const currentElevation = Math.max(-5, Math.min(88, elevation + touchElevation));
      const az = currentHeading * Math.PI / 180;
      const el = currentElevation * Math.PI / 180;
      const forward = [Math.sin(az) * Math.cos(el), Math.sin(el), Math.cos(az) * Math.cos(el)];
      const right = [Math.cos(az), 0, -Math.sin(az)];
      const up = [-Math.sin(az) * Math.sin(el), Math.cos(el), -Math.cos(az) * Math.sin(el)];
      const tanHalfFov = Math.tan(fieldOfView * Math.PI / 180);
      const sidereal = siderealDegrees(new Date());
      const latRadians = latitude * Math.PI / 180;

      const objectMap = new Map();
      for (const object of objects) {
        const horizontal = horizontalPosition(object, sidereal, latRadians);
        objectMap.set(object.name, horizontal.position);
      }

      const lineVertices = [];
      for (const [starA, starB] of CONSTELLATION_LINES) {
        const posA = objectMap.get(starA);
        const posB = objectMap.get(starB);
        if (posA && posB) lineVertices.push(...posA, ...posB);
      }

      const positions = [];
      const colors = [];
      const sizes = [];
      lastScreenObjects = [];

      for (const object of objects) {
        if (!matchesFilter(object)) continue;
        const horizontal = horizontalPosition(object, sidereal, latRadians);
        if (horizontal.altitude < 0) continue;
        const depth = horizontal.position[0] * forward[0] + horizontal.position[1] * forward[1] + horizontal.position[2] * forward[2];
        if (depth <= 0.01) continue;
        const screenX = (horizontal.position[0] * right[0] + horizontal.position[1] * right[1] + horizontal.position[2] * right[2]) / (depth * tanHalfFov * aspect);
        const screenY = (horizontal.position[0] * up[0] + horizontal.position[1] * up[1] + horizontal.position[2] * up[2]) / (depth * tanHalfFov);
        if (Math.abs(screenX) > 1 || Math.abs(screenY) > 1) continue;
        
        positions.push(...horizontal.position);
        colors.push(...objectColor(object));
        
        // Calcul dynamique de la taille selon la magnitude (avec mise en valeur de Polaris)
        let starSize = Math.max(4, Math.min(15, (6.5 - object.mag) * 2.0 + 3));
        if (object.name === "Polaris") starSize = 18;
        sizes.push(starSize);

        lastScreenObjects.push({ object, screenX, screenY });
      }

      gl.clearColor(0, 0, 0, 0);
      gl.clear(gl.COLOR_BUFFER_BIT);
      gl.useProgram(program);
      gl.uniform3fv(rightLocation, right);
      gl.uniform3fv(upLocation, up);
      gl.uniform3fv(forwardLocation, forward);
      gl.uniform1f(tanHalfFovLocation, tanHalfFov);
      gl.uniform1f(aspectLocation, aspect);

      // Grille
      gl.uniform1f(gridLocation, 1);
      gl.bindBuffer(gl.ARRAY_BUFFER, gridBuffer);
      gl.vertexAttribPointer(positionLocation, 3, gl.FLOAT, false, 0, 0);
      gl.enableVertexAttribArray(positionLocation);
      gl.disableVertexAttribArray(colorLocation);
      gl.disableVertexAttribArray(sizeLocation);
      gl.vertexAttrib3f(colorLocation, 0.23, 0.31, 0.42);
      gl.vertexAttrib1f(sizeLocation, 1);
      gl.drawArrays(gl.LINES, 0, gridVertices.length / 3);

      // Lignes de constellations
      if (lineVertices.length > 0) {
        gl.bindBuffer(gl.ARRAY_BUFFER, lineBuffer);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(lineVertices), gl.DYNAMIC_DRAW);
        gl.vertexAttribPointer(positionLocation, 3, gl.FLOAT, false, 0, 0);
        gl.vertexAttrib3f(colorLocation, 0.55, 0.82, 1.0); 
        gl.drawArrays(gl.LINES, 0, lineVertices.length / 3);
      }

      // Étoiles
      gl.uniform1f(gridLocation, 0);
      if (positions.length) {
        gl.bindBuffer(gl.ARRAY_BUFFER, objectBuffer);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(positions), gl.DYNAMIC_DRAW);
        gl.vertexAttribPointer(positionLocation, 3, gl.FLOAT, false, 0, 0);
        gl.bindBuffer(gl.ARRAY_BUFFER, colorBuffer);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.DYNAMIC_DRAW);
        gl.vertexAttribPointer(colorLocation, 3, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(colorLocation);
        gl.bindBuffer(gl.ARRAY_BUFFER, sizeBuffer);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(sizes), gl.DYNAMIC_DRAW);
        gl.vertexAttribPointer(sizeLocation, 1, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(sizeLocation);
        gl.drawArrays(gl.POINTS, 0, sizes.length);
      }

      viewReadout.textContent = `Vue AZ ${Math.round(currentHeading)}° · ALT ${Math.round(currentElevation)}°`;

      // Noms des objets et marquage spécial permanent de Polaris
      const labels = document.createDocumentFragment();
      for (const item of lastScreenObjects) {
        const isPolaris = (item.object.name === "Polaris");
        // Polaris s'affiche TOUJOURS, les autres dépendent de showNames et de la magnitude
        if (!isPolaris && (!showNames || item.object.mag > 7.5)) continue;
        if (isPolaris && !showNames) {
          // Si les noms sont masqués, on affiche quand même Polaris de manière spécifique
        } else if (!isPolaris && !showNames) {
          continue;
        }

        // Limiter le nombre de labels affichés simultanément pour garder de la fluidité
        if (!isPolaris && lastScreenObjects.filter(e => e.object.mag <= 7.5).indexOf(item) > 16) continue;

        const label = document.createElement('span');
        label.className = isPolaris ? 'object-label polaris' : 'object-label';
        label.style.left = `${(item.screenX + 1) * 50}%`;
        label.style.top = `${(1 - item.screenY) * 50}%`;
        label.textContent = isPolaris ? `★ ${item.object.commonName || item.object.name} (Polaire)` : (item.object.commonName || item.object.name);
        labels.appendChild(label);
      }
      labelsElement.replaceChildren(labels);

      // Noms des constellations
      const constLabels = document.createDocumentFragment();
      if (showNames) {
        for (const c of CONSTELLATION_CENTERS) {
          const pos = objectMap.get(c.refStar);
          if (pos) {
            const depth = pos[0] * forward[0] + pos[1] * forward[1] + pos[2] * forward[2];
            if (depth > 0.01) {
              const screenX = (pos[0] * right[0] + pos[1] * right[1] + pos[2] * right[2]) / (depth * tanHalfFov * aspect);
              const screenY = (pos[0] * up[0] + pos[1] * up[1] + pos[2] * up[2]) / (depth * tanHalfFov);
              if (Math.abs(screenX) <= 1 && Math.abs(screenY) <= 1) {
                const cl = document.createElement('span');
                cl.className = 'constellation-label';
                cl.style.left = `${(screenX + 1) * 50}%`;
                cl.style.top = `${(1 - screenY) * 50}%`;
                cl.textContent = c.name;
                constLabels.appendChild(cl);
              }
            }
          }
        }
      }
      constLabelsElement.replaceChildren(constLabels);
    }

    let program, gridBuffer, lineBuffer, objectBuffer, colorBuffer, sizeBuffer;
    let positionLocation, colorLocation, sizeLocation, rightLocation, upLocation, forwardLocation, tanHalfFovLocation, aspectLocation, gridLocation, gridVertices;

    async function initialize() {
      if (!gl) { setStatus('WebGL non supporté.', true); return; }
      try {
        program = createProgram();
        positionLocation = gl.getAttribLocation(program, 'aPosition');
        colorLocation = gl.getAttribLocation(program, 'aColor');
        sizeLocation = gl.getAttribLocation(program, 'aSize');
        rightLocation = gl.getUniformLocation(program, 'uRight');
        upLocation = gl.getUniformLocation(program, 'uUp');
        forwardLocation = gl.getUniformLocation(program, 'uForward');
        tanHalfFovLocation = gl.getUniformLocation(program, 'uTanHalfFov');
        aspectLocation = gl.getUniformLocation(program, 'uAspect');
        gridLocation = gl.getUniformLocation(program, 'uGrid');
        
        gridBuffer = gl.createBuffer();
        lineBuffer = gl.createBuffer();
        objectBuffer = gl.createBuffer();
        colorBuffer = gl.createBuffer();
        sizeBuffer = gl.createBuffer();
        
        gridVertices = makeGrid();
        gl.bindBuffer(gl.ARRAY_BUFFER, gridBuffer);
        gl.bufferData(gl.ARRAY_BUFFER, gridVertices, gl.STATIC_DRAW);
        gl.enable(gl.BLEND);
        gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);

        const [catalogRes, statusRes] = await Promise.all([fetch('/catalog'), fetch('/status')]);
        objects = await catalogRes.json();
        const status = await statusRes.json();
        applyEncoderStatus(status);
        latitude = Number(status.lat ?? latitude);
        longitude = Number(status.lon ?? longitude);
        
        setStatus(`${objects.length} objets chargés. Prêt.`, "align-ok");
        window.setInterval(pollEncoderStatus, 250);
        render();
      } catch (error) {
        setStatus(error.message, true);
      }
    }

    canvas.addEventListener('click', event => {
      const rect = canvas.getBoundingClientRect();
      const x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
      const y = -(((event.clientY - rect.top) / rect.height) * 2 - 1);

      let closestObj = null;
      let minDst = 0.08;

      for (const item of lastScreenObjects) {
        const dst = Math.hypot(item.screenX - x, item.screenY - y);
        if (dst < minDst) {
          minDst = dst;
          closestObj = item.object;
        }
      }

      if (closestObj) {
        const details = getEnrichedDetails(closestObj);
        document.getElementById('modalTitle').textContent = closestObj.commonName ? `${closestObj.name} (${closestObj.commonName})` : closestObj.name;
        document.getElementById('modalSub').textContent = `${closestObj.type || 'Objet céleste'}`;
        document.getElementById('modalMag').textContent = closestObj.mag.toFixed(1);
        document.getElementById('modalDist').textContent = details.dist;
        document.getElementById('modalSize').textContent = details.size;
        document.getElementById('modalCoords').textContent = `AD ${closestObj.ra}h · Dec ${closestObj.dec}°`;
        document.getElementById('objectModal').hidden = false;
      }
    });

    function closeModal() {
      document.getElementById('objectModal').hidden = true;
    }

    let dragPoint = null;
    const activePointers = new Map();
    let pinchDistance = null;
    const pointerDistance = () => {
      const [first, second] = [...activePointers.values()];
      return first && second ? Math.hypot(first.x - second.x, first.y - second.y) : null;
    };
    
    const scene = document.getElementById('scene');
    scene.addEventListener('pointerdown', event => {
      activePointers.set(event.pointerId, { x: event.clientX, y: event.clientY });
      dragPoint = { x: event.clientX, y: event.clientY };
      scene.setPointerCapture(event.pointerId);
      pinchDistance = activePointers.size === 2 ? pointerDistance() : null;
    });
    scene.addEventListener('pointermove', event => {
      if (activePointers.has(event.pointerId)) activePointers.set(event.pointerId, { x: event.clientX, y: event.clientY });
      if (activePointers.size >= 2) {
        const distance = pointerDistance();
        if (distance && pinchDistance) fieldOfView = Math.max(12, Math.min(60, fieldOfView * pinchDistance / distance));
        pinchDistance = distance;
        dragPoint = null;
        return;
      }
      if (!dragPoint) return;
      touchHeading = (touchHeading - (event.clientX - dragPoint.x) * 0.35 + 360) % 360;
      touchElevation = Math.max(-80, Math.min(80, touchElevation + (event.clientY - dragPoint.y) * 0.25));
      dragPoint = { x: event.clientX, y: event.clientY };
    });
    const releasePointer = event => {
      activePointers.delete(event.pointerId);
      pinchDistance = null;
      dragPoint = activePointers.values().next().value || null;
    };
    scene.addEventListener('pointerup', releasePointer);
    scene.addEventListener('pointercancel', releasePointer);
    scene.addEventListener('wheel', event => {
      event.preventDefault();
      fieldOfView = Math.max(12, Math.min(60, fieldOfView + Math.sign(event.deltaY) * 2));
    }, { passive: false });

    toggleNamesBtn.addEventListener('click', () => {
      showNames = !showNames;
      toggleNamesBtn.textContent = showNames ? "Noms : ON" : "Noms : OFF";
    });

    document.getElementById('centerButton').addEventListener('click', () => {
      touchHeading = 0;
      touchElevation = 0;
      fieldOfView = 34;
    });

    window.addEventListener('resize', resizeCanvas);
    initialize();
  </script>
</body>
</html>
)rawliteral";

#endif