// Version Component: BetaSkyPage.h v5.36.0
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
  <title>Vue 3D du ciel — Planétarium</title>
  <style>
    * { box-sizing: border-box; }
    body { margin: 0; padding: 12px 12px 90px; min-height: 100vh; background: #08090c; color: #d8dce6; font: 14px -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; transition: all 0.3s; }
    .sky-card { width: min(100%, 760px); margin: 0 auto; transition: all 0.3s; }
    .sky-heading { display: flex; align-items: center; justify-content: space-between; gap: 10px; margin: 2px 2px 10px; }
    h1 { margin: 0; font-size: 1.15rem; color: #f1f3f8; }
    .beta { padding: 3px 7px; border: 1px solid #74531e; border-radius: 999px; color: #ffc66d; font-size: .7rem; letter-spacing: .08em; }
    .sky-status { min-height: 20px; margin: 0 2px 8px; color: #aab3c2; font-size: .82rem; }
    .sky-status.error { color: #ff7777; }
    .sky-status.align-ok { color: #69ff84; }
    
    .scene { position: relative; height: min(60vh, 500px); min-height: 300px; overflow: hidden; border: 1px solid #273142; border-radius: 14px; background: radial-gradient(ellipse at 50% 100%, #1b2638 0, #0d1420 47%, #05070b 100%); touch-action: none; transition: height 0.3s, background 0.5s; }
    canvas { display: block; width: 100%; height: 100%; cursor: grab; position: relative; z-index: 1; background: transparent; }
    canvas:active { cursor: grabbing; }
    .scene-label { position: absolute; z-index: 2; color: #acb8c8; font-size: .72rem; text-shadow: 0 1px 4px #000; pointer-events: none; }
    .scene-label.cardinal { font-weight: bold; color: #ffb74d; background: rgba(5,8,12,0.6); padding: 2px 6px; border-radius: 4px; }
    .scene-label.telescope { top: 10px; left: 10px; }
    .scene-label.telescope.error { color: #ff7777; }
    .scene-label.view { left: 10px; bottom: 10px; }
    .scene-label.horizon { right: 10px; bottom: 10px; }
    .scene-center { position: absolute; top: 50%; left: 50%; width: 8px; height: 8px; border: 1px solid rgba(255,255,255,.7); border-radius: 50%; transform: translate(-50%,-50%); pointer-events: none; z-index: 2; }
    
    /* Boutons de Zoom intégrés dans le planétarium (coin supérieur droit) */
    .scene-zoom-controls { position: absolute; top: 10px; right: 10px; z-index: 5; display: flex; flex-direction: column; gap: 4px; }
    .zoom-btn { width: 32px; height: 32px; background: rgba(21, 27, 37, 0.85); border: 1px solid #344256; border-radius: 6px; color: #ffb74d; font-size: 1.1rem; font-weight: bold; display: flex; align-items: center; justify-content: center; cursor: pointer; backdrop-filter: blur(4px); padding: 0; }
    .zoom-btn:active { background: rgba(37, 50, 71, 0.95); transform: scale(0.95); }

    .object-label { position: absolute; z-index: 2; padding: 2px 5px; border-radius: 4px; color: #f4e6be; background: rgba(5,8,12,0.75); font-size: .68rem; white-space: nowrap; text-shadow: 0 1px 3px #000; transform: translate(-50%,-120%); pointer-events: none; transition: opacity 0.2s; }
    .object-label.polaris { color: #ffb74d; background: rgba(30,35,45,0.9); border: 1px solid rgba(255,183,77,0.5); font-weight: bold; z-index: 3; }
    .constellation-label { position: absolute; z-index: 2; color: #7cb9e8; font-size: .75rem; font-weight: bold; letter-spacing: 0.05em; text-transform: uppercase; text-shadow: 0 1px 5px #000; transform: translate(-50%,-50%); pointer-events: none; opacity: 0.85; }

    .controls { display: grid; grid-template-columns: repeat(4, minmax(0,1fr)); gap: 8px; margin-top: 10px; }
    .time-control-box { grid-column: span 4; display: flex; gap: 6px; align-items: center; background: #151b25; padding: 6px 10px; border: 1px solid #344256; border-radius: 9px; }
    
    .filter-dropdown { position: relative; grid-column: span 2; }
    .filter-drop-btn { width: 100%; min-height: 44px; background: #151b25; border: 1px solid #344256; border-radius: 9px; color: #e6eaf2; text-align: left; padding: 8px 10px; cursor: pointer; display: flex; justify-content: space-between; align-items: center; font: inherit; }
    .filter-drop-menu { position: absolute; bottom: calc(100% + 4px); left: 0; right: 0; background: #151b25; border: 1px solid #344256; border-radius: 9px; padding: 8px 12px; z-index: 50; box-shadow: 0 4px 16px rgba(0,0,0,0.6); display: none; }
    .filter-drop-menu.open { display: block; }
    .filter-drop-menu label { display: flex; align-items: center; gap: 8px; padding: 4px 0; font-size: 0.85rem; cursor: pointer; color: #e6eaf2; }

    button, select, input { min-height: 44px; border: 1px solid #344256; border-radius: 9px; background: #151b25; color: #e6eaf2; font: inherit; }
    button { padding: 8px 10px; cursor: pointer; }
    button:active { background: #253247; }
    select, input[type="datetime-local"] { width: 100%; padding: 8px 10px; background: #10151d; }
    .scene-help { margin: 9px 2px; color: #929cac; font-size: .78rem; line-height: 1.45; text-align: center; }

    /* Croix directionnelle (D-Pad) et réglage de sensibilité combinés en haut */
    .dpad-wrapper { grid-column: span 4; display: flex; align-items: center; justify-content: center; gap: 12px; background: #121822; padding: 8px; border: 1px solid #273142; border-radius: 12px; margin-bottom: 4px; }
    .dpad-container { display: grid; grid-template-columns: repeat(3, 1fr); gap: 4px; width: 180px; }
    .jog-btn { background: #1e293b !important; font-weight: bold; color: #ffb74d !important; font-size: 1rem; display: flex; align-items: center; justify-content: center; min-height: 38px; }
    .jog-up { grid-column: 2; grid-row: 1; }
    .jog-left { grid-column: 1; grid-row: 2; }
    .jog-center { grid-column: 2; grid-row: 2; background: #10151d !important; border: 1px dashed #344256; font-size: 0.6rem; color: #8b94a2 !important; cursor: default; display: flex; align-items: center; justify-content: center; }
    .jog-right { grid-column: 3; grid-row: 2; }
    .jog-down { grid-column: 2; grid-row: 3; }
    
    .sens-box { display: flex; flex-direction: column; gap: 4px; font-size: 0.75rem; color: #8b94a2; }
    .sens-box select { min-height: 36px; padding: 4px 8px; font-size: 0.8rem; background: #151b25; color: #ffb74d; font-weight: bold; }

    body.landscape-fullscreen { padding: 0 !important; background: #000; overflow: hidden; }
    body.landscape-fullscreen .sky-heading,
    body.landscape-fullscreen .sky-status,
    body.landscape-fullscreen .controls,
    body.landscape-fullscreen .scene-help,
    body.landscape-fullscreen .bottom-nav { display: none !important; }
    body.landscape-fullscreen .sky-card { width: 100% !important; max-width: none !important; margin: 0; }
    body.landscape-fullscreen .scene { height: 100vh !important; border: none; border-radius: 0; }

    #manualFullscreenBtn { display: inline-block; background: #263238; color: #ffb74d; border: 1px solid #455a64; font-weight: bold; }
    @media (orientation: landscape) {
      #manualFullscreenBtn { display: none !important; }
    }

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
    
    .bottom-nav { position: fixed; z-index: 20; left: 0; right: 0; bottom: 0; display: flex; overflow-x: auto; scroll-snap-type: x mandatory; -webkit-overflow-scrolling: touch; gap: 6px; padding: 8px 12px calc(8px + env(safe-area-inset-bottom)); background: rgba(30,30,30,0.98); border-top: 1px solid #37474f; box-shadow: 0 -4px 16px rgba(0,0,0,0.4); scrollbar-width: none; }
    .bottom-nav::-webkit-scrollbar { display: none; }
    .nav-item { flex: 0 0 56px; min-height: 48px; display: flex; align-items: center; justify-content: center; border: 0; border-radius: 8px; background: transparent; color: #b0bec5; font: inherit; text-decoration: none; cursor: pointer; scroll-snap-align: start; }
    .nav-item[aria-current="page"] { color: #ffb74d; background: #263238; }
    .night-mode-toggle { background: #f1f3f4; color: #101418; border: 1px solid #fff; }
    .night-mode-toggle[aria-pressed="true"] { background: #ff5252; color: #fff; }
    .nav-icon { display: flex; font-size: 24px; line-height: 1; }
    .ui-icon { display: inline-block; width: 1em; height: 1em; vertical-align: -.15em; fill: none; stroke: currentColor; stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; }
    .nav-icon .ui-icon { vertical-align: middle; }
    @media (min-width: 700px) { .bottom-nav { left: 50%; right: auto; width: min(650px, calc(100% - 32px)); transform: translateX(-50%); border: 1px solid #37474f; border-bottom: 0; border-radius: 14px 14px 0 0; justify-content: center; overflow-x: hidden; } .nav-item { flex: 1; } }
  </style>
</head>
<body>
  <main class="sky-card">
    <header class="sky-heading">
      <h1><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-compass"></use></svg> Vue 3D du ciel</h1>
      <span class="beta">PLANÉTARIUM v5.36</span>
    </header>
    <p id="skyStatus" class="sky-status" role="status">Initialisation du planétarium 3D...</p>
    
    <div class="scene" id="scene">
      <!-- Boutons de zoom intégrés dans le planétarium (coin supérieur droit) -->
      <div class="scene-zoom-controls">
        <button id="zoomInBtn" type="button" class="zoom-btn" title="Zoomer (+)">+</button>
        <button id="zoomOutBtn" type="button" class="zoom-btn" title="Dézoomer (-)">-</button>
      </div>

      <canvas id="skyCanvas" aria-label="Carte 3D interactive du ciel"></canvas>
      <span class="scene-label cardinal" id="labelN" style="display:none">N</span>
      <span class="scene-label cardinal" id="labelS" style="display:none">S</span>
      <span class="scene-label cardinal" id="labelE" style="display:none">E</span>
      <span class="scene-label cardinal" id="labelW" style="display:none">O</span>
      <span class="scene-label telescope" id="telescopeReadout">Télescope : connexion...</span>
      <span class="scene-label view" id="viewReadout">Vue AZ 0° · ALT 0°</span>
      <span class="scene-label horizon">Grille AltAz & Heure</span>
      <span class="scene-center" aria-hidden="true"></span>
      <div id="objectLabels" aria-hidden="true"></div>
      <div id="constellationLabels" aria-hidden="true"></div>
    </div>
    
    <div class="controls">
      <!-- Croix directionnelle (D-Pad) et réglage de sensibilité combinés en haut -->
      <div class="dpad-wrapper">
        <div class="dpad-container">
          <button id="jogUpBtn" type="button" class="jog-btn jog-up" title="Ajuster Altitude">▲</button>
          <button id="jogLeftBtn" type="button" class="jog-btn jog-left" title="Ajuster Azimut">◀</button>
          <div class="jog-center">Offset</div>
          <button id="jogRightBtn" type="button" class="jog-btn jog-right" title="Ajuster Azimut">▶</button>
          <button id="jogDownBtn" type="button" class="jog-btn jog-down" title="Ajuster Altitude">▼</button>
        </div>
        <div class="sens-box">
          <label for="stepSelect">Pas (sensibilité) :</label>
          <select id="stepSelect" aria-label="Sensibilité du pas d'offset">
            <option value="0.1">0.1° (Fin)</option>
            <option value="1" selected>1.0° (Normal)</option>
            <option value="5">5.0° (Large)</option>
          </select>
        </div>
      </div>

      <div class="time-control-box">
        <span style="font-size: 0.75rem; color: #8b94a2; white-space: nowrap;">Heure ciel :</span>
        <input type="datetime-local" id="customTimeInput" aria-label="Heure d'affichage du ciel">
        <button type="button" id="resetTimeBtn" style="min-height: 36px; padding: 4px 10px; font-size: 0.75rem; white-space: nowrap;">Direct</button>
      </div>

      <div class="filter-dropdown">
        <button type="button" class="filter-drop-btn" id="filterDropBtn">Types d'objets <span style="font-size:0.7em;">▼</span></button>
        <div class="filter-drop-menu" id="filterDropMenu">
          <label><input type="checkbox" value="galaxie" checked> Galaxies</label>
          <label><input type="checkbox" value="nebuleuse" checked> Nébuleuses</label>
          <label><input type="checkbox" value="amas" checked> Amas d'étoiles</label>
          <label><input type="checkbox" value="etoile" checked> Étoiles</label>
        </div>
      </div>

      <select id="magFilterSelect" aria-label="Mag. Max">
        <option value="99" selected>Mag. Max : Toutes</option>
        <option value="6">Mag. ≤ 6 (Œil nu)</option>
        <option value="8">Mag. ≤ 8 (Jumelles)</option>
        <option value="10">Mag. ≤ 10 (Petit télescope)</option>
        <option value="12">Mag. ≤ 12 (Grand télescope)</option>
      </select>

      <button id="toggleNamesBtn" type="button">Noms : ON</button>
      <button id="centerButton" type="button">Recentrer</button>
      
      <button id="manualFullscreenBtn" type="button" style="grid-column: span 4;">Plein écran</button>
    </div>
    <p class="scene-help">Utilisez la croix pour appliquer un offset permanent. Le déplacement tactile est temporaire.</p>
  </main>

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
    <a class="nav-item" href="/beta_sky" aria-current="page" title="Vue 3D du ciel"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-compass"></use></svg></span></a>
    <a class="nav-item" href="/" title="Mode Encodeurs"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-telescope"></use></svg></span></a>
    <a class="nav-item" href="/station" title="Mise en station"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-crosshair"></use></svg></span></a>
    <a class="nav-item" href="/calib_page" title="Calibration"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-ruler"></use></svg></span></a>
    <a class="nav-item" href="/config" title="Configuration"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-settings"></use></svg></span></a>
    <a class="nav-item" href="/help" title="Aide SkySafari"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-book-open"></use></svg></span></a>
    <a class="nav-item" href="/simu" title="Simulation"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-gamepad-2"></use></svg></span></a>
    <a class="nav-item" href="/test" title="Tests système"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-flask-conical"></use></svg></span></a>
    <a class="nav-item" href="/releasenotes" title="Notes de version"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-clipboard-list"></use></svg></span></a>
    <button class="nav-item night-mode-toggle" id="nightModeToggle" type="button" aria-label="Activer le mode nuit" title="Activer le mode nuit" aria-pressed="false" onclick="toggleNightMode()"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-moon"></use></svg></span></a>
  </nav>

  <script>
    const canvas = document.getElementById('skyCanvas');
    const sceneDiv = document.getElementById('scene');
    const gl = canvas.getContext('webgl', { antialias: true, alpha: true, powerPreference: 'high-performance' });
    const statusElement = document.getElementById('skyStatus');
    const magFilterElement = document.getElementById('magFilterSelect');
    const labelsElement = document.getElementById('objectLabels');
    const constLabelsElement = document.getElementById('constellationLabels');
    const viewReadout = document.getElementById('viewReadout');
    const telescopeReadout = document.getElementById('telescopeReadout');
    const toggleNamesBtn = document.getElementById('toggleNamesBtn');
    const customTimeInput = document.getElementById('customTimeInput');
    const filterDropBtn = document.getElementById('filterDropBtn');
    const filterDropMenu = document.getElementById('filterDropMenu');
    const stepSelect = document.getElementById('stepSelect');
    
    let objects = [];
    let latitude = 45.89;
    let longitude = 6.05;
    
    let targetHeading = 0;
    let targetElevation = 35;
    let heading = 0;
    let elevation = 35;

    let touchHeading = 0;
    let touchElevation = 0;
    
    let alignOffsetHeading = 0;
    let alignOffsetElevation = 0;

    let fieldOfView = 34;
    let encoderRequestInFlight = false;
    let showNames = true;
    let lastScreenObjects = [];
    let useSystemTime = true;
    
    let lastRawAZ = null;
    let lastRawALT = null;

    filterDropBtn.addEventListener('click', (e) => {
      e.stopPropagation();
      filterDropMenu.classList.toggle('open');
    });
    window.addEventListener('click', () => filterDropMenu.classList.remove('open'));
    filterDropMenu.addEventListener('click', (e) => e.stopPropagation());

    document.getElementById('jogLeftBtn').addEventListener('click', () => {
      const step = parseFloat(stepSelect.value) || 1;
      alignOffsetHeading = (alignOffsetHeading - step + 360) % 360;
    });
    document.getElementById('jogRightBtn').addEventListener('click', () => {
      const step = parseFloat(stepSelect.value) || 1;
      alignOffsetHeading = (alignOffsetHeading + step + 360) % 360;
    });
    document.getElementById('jogUpBtn').addEventListener('click', () => {
      const step = parseFloat(stepSelect.value) || 1;
      alignOffsetElevation = Math.max(-45, Math.min(45, alignOffsetElevation + step));
    });
    document.getElementById('jogDownBtn').addEventListener('click', () => {
      const step = parseFloat(stepSelect.value) || 1;
      alignOffsetElevation = Math.max(-45, Math.min(45, alignOffsetElevation - step));
    });

    document.getElementById('zoomInBtn').addEventListener('click', () => {
      fieldOfView = Math.max(12, fieldOfView - 4);
    });
    document.getElementById('zoomOutBtn').addEventListener('click', () => {
      fieldOfView = Math.min(60, fieldOfView + 4);
    });

    function updateSkyBackground(date) {
      const hours = date.getHours() + date.getMinutes() / 60;
      let bg = '';
      if (hours >= 6 && hours < 8) {
        bg = 'radial-gradient(ellipse at 50% 100%, #3a2e39 0, #1c2331 50%, #08090c 100%)';
      } else if (hours >= 8 && hours < 18) {
        bg = 'radial-gradient(ellipse at 50% 100%, #284b63 0, #153243 50%, #08090c 100%)';
      } else if (hours >= 18 && hours < 20) {
        bg = 'radial-gradient(ellipse at 50% 100%, #4a2832 0, #22152b 50%, #08090c 100%)';
      } else {
        bg = 'radial-gradient(ellipse at 50% 100%, #1b2638 0, #0d1420 47%, #05070b 100%)';
      }
      sceneDiv.style.background = bg;
    }

    function checkOrientation() {
      if (window.innerHeight < window.innerWidth) {
        document.body.classList.add('landscape-fullscreen');
        if (!document.fullscreenElement) {
          document.documentElement.requestFullscreen().catch(() => {});
        }
      } else {
        document.body.classList.remove('landscape-fullscreen');
        if (document.fullscreenElement) {
          document.exitFullscreen().catch(() => {});
        }
      }
      resizeCanvas();
    }
    window.addEventListener('resize', checkOrientation);
    window.addEventListener('orientationchange', checkOrientation);

    document.getElementById('manualFullscreenBtn').addEventListener('click', () => {
      if (!document.fullscreenElement) {
        document.documentElement.requestFullscreen().catch(() => {});
      } else {
        document.exitFullscreen().catch(() => {});
      }
    });

    function initDateTimeInput() {
      const now = new Date();
      const offsetMs = now.getTimezoneOffset() * 60000;
      const localIso = new Date(now.getTime() - offsetMs).toISOString().slice(0, 16);
      if (customTimeInput && !customTimeInput.value) {
        customTimeInput.value = localIso;
      }
    }
    initDateTimeInput();

    customTimeInput.addEventListener('change', () => {
      useSystemTime = false;
      updateSkyBackground(getSkyDate());
    });

    document.getElementById('resetTimeBtn').addEventListener('click', () => {
      useSystemTime = true;
      initDateTimeInput();
      updateSkyBackground(getSkyDate());
    });

    function getSkyDate() {
      if (useSystemTime) return new Date();
      const val = customTimeInput.value;
      return val ? new Date(val) : new Date();
    }

    const CONSTELLATION_LINES = [
      ["Tarazed", "Altair"], ["Altair", "Alshain"], ["Altair", "Deneb el Okab"],
      ["Alpheratz", "Mirach"], ["Mirach", "Almach"], ["Alpheratz", "Adhil"],
      ["Zubenelgenubi", "Zubeneschamali"],
      ["Hamal", "Sheratan"], ["Sheratan", "Mesarthim"],
      ["Arcturus", "Nekkar"], ["Nekkar", "Seginus"], ["Seginus", "Alkalurops"], ["Arcturus", "Muphrid"], ["Arcturus", "Hozian"],
      ["Tarf", "Acubens"],
      ["Sadalmelik", "Sadalsuud"],
      ["Caph", "Shedar"], ["Shedar", "Navi"], ["Navi", "Ruchbah"], ["Ruchbah", "Segin"],
      ["Alderamin", "Alrai"], ["Alrai", "Errai"], ["Alderamin", "Kurhah"],
      ["Cor Caroli", "Chara"], ["Diadem", "Cor Caroli"],
      ["Capella", "Menkalinan"], ["Menkalinan", "Mahasim"], ["Capella", "Hassaleh"],
      ["Alphecca", "Nusakan"],
      ["Deneb", "Sadr"], ["Sadr", "Albireo"], ["Fawaris", "Sadr"], ["Sadr", "Gienah"],
      ["Rotanev", "Sualocin"],
      ["Eltanin", "Rastaban"], ["Rastaban", "Thuban"],
      ["Sham", "Deneb"],
      ["Castor", "Pollux"], ["Pollux", "Alhena"], ["Castor", "Tejat"], ["Mebsuta", "Pollux"],
      ["Beta Camelopardalis", "Alpha Lyncis"],
      ["Dubhe", "Merak"], ["Merak", "Phecda"], ["Phecda", "Megrez"], 
      ["Megrez", "Dubhe"], ["Megrez", "Alioth"], ["Alioth", "Mizar"], ["Mizar", "Alkaid"],
      ["Kornephoros", "Rasalgethi"], ["Kornephoros", "Sujata"], ["Sujata", "Marfik"],
      ["Regulus", "Algieba"], ["Algieba", "Zosma"], ["Zosma", "Denebola"], ["Regulus", "Adhafera"], ["Ras Elased Australis", "Regulus"], ["Praecipua", "Algieba"],
      ["Vega", "Sheliak"], ["Sheliak", "Sulafat"], ["Sulafat", "γ Lyr"], ["γ Lyr", "Vega"],
      ["Alpha Lyncis", "Beta Camelopardalis"],
      ["Rasalhague", "Unukalhai"],
      ["Markab", "Scheat"], ["Scheat", "Alpheratz"], ["Alpheratz", "Algenib"], ["Algenib", "Markab"], ["Markab", "Enif"],
      ["Mirfak", "Algol"], ["Algol", "Menkib"], ["Mirfak", "Atik"],
      ["Kitalpha", "Alpha Equ"],
      ["Polaris", "Yildun"], ["Yildun", "Zeta UMi"], ["Zeta UMi", "Epsilon UMi"],
      ["Epsilon UMi", "Pherkad"], ["Pherkad", "Kochab"], ["Kochab", "Polaris"],
      ["Alpherg", "Gamma Piscium"],
      ["Aldebaran", "Elnath"], ["Aldebaran", "Alcyone"], ["Aldebaran", "Tianguis"],
      ["Beta Trianguli", "Mothallah"],
      ["Vindemiatrix", "Porrima"],
      ["Procyon", "Gomeisa"], ["Sirius", "Mirzam"], ["Sirius", "Adhara"], ["Mirzam", "Adhara"]
    ];

    const CONSTELLATION_CENTERS = [
      { name: "Aigle", refStar: "Altair" },
      { name: "Andromède", refStar: "Mirach" },
      { name: "Balance", refStar: "Zubenelgenubi" },
      { name: "Bélier", refStar: "Hamal" },
      { name: "Bouvier", refStar: "Arcturus" },
      { name: "Cancer", refStar: "Acubens" },
      { name: "Capricorne", refStar: "Sadalmelik" },
      { name: "Cassiopée", refStar: "Navi" },
      { name: "Céphée", refStar: "Alderamin" },
      { name: "Chevelure de Bérénice", refStar: "Diadem" },
      { name: "Chiens de chasse", refStar: "Cor Caroli" },
      { name: "Cocher", refStar: "Capella" },
      { name: "Couronne boréale", refStar: "Alphecca" },
      { name: "Cygne", refStar: "Sadr" },
      { name: "Dauphin", refStar: "Rotanev" },
      { name: "Dragon", refStar: "Eltanin" },
      { name: "Flèche", refStar: "Sham" },
      { name: "Gémeaux", refStar: "Pollux" },
      { name: "Girafe", refStar: "Beta Camelopardalis" },
      { name: "Grande Ourse", refStar: "Alioth" },
      { name: "Hercule", refStar: "Kornephoros" },
      { name: "Lézard", refStar: "Alpha Lacertae" },
      { name: "Lion", refStar: "Regulus" },
      { name: "Petit Lion", refStar: "Praecipua" },
      { name: "Lyre", refStar: "Vega" },
      { name: "Lynx", refStar: "Alpha Lyncis" },
      { name: "Ophiuchus", refStar: "Rasalhague" },
      { name: "Pégase", refStar: "Markab" },
      { name: "Persée", refStar: "Mirfak" },
      { name: "Petit Cheval", refStar: "Kitalpha" },
      { name: "Petite Ourse", refStar: "Polaris" },
      { name: "Poissons", refStar: "Alpherg" },
      { name: "Taureau", refStar: "Aldebaran" },
      { name: "Triangle", refStar: "Mothallah" },
      { name: "Vierge", refStar: "Vindemiatrix" },
      { name: "Petit Chien", refStar: "Procyon" },
      { name: "Grand Chien", refStar: "Sirius" }
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

      if (lastRawAZ !== null && lastRawALT !== null) {
        if (rawAZ !== lastRawAZ || rawALT !== lastRawALT) {
          touchHeading = 0;
          touchElevation = 0;
        }
      }
      lastRawAZ = rawAZ;
      lastRawALT = rawALT;

      const azimuth = ((rawAZ * dirAZ / ticksAZ * 360) % 360 + 360) % 360;
      const altitudeCircle = ((rawALT * dirALT / ticksALT * 360) % 360 + 360) % 360;
      
      targetHeading = azimuth;
      targetElevation = altitudeCircle > 180 ? altitudeCircle - 360 : altitudeCircle;
      
      telescopeReadout.textContent = `Télescope AZ ${azimuth.toFixed(1)}° · ALT ${targetElevation.toFixed(1)}°`;
      telescopeReadout.classList.remove('error');
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
            gl_FragColor = vec4(vColor, 0.75);
            return;
          }
          vec2 point = gl_PointCoord - vec2(0.5);
          float dist = length(point);
          float edge = 1.0 - smoothstep(0.2, 0.5, dist);
          if (edge < 0.05) discard;
          vec3 finalColor = mix(vColor, vec3(1.0), (0.5 - dist) * 0.6);
          gl_FragColor = vec4(finalColor, edge);
        }`);
      const program = gl.createProgram();
      gl.attachShader(program, vertex);
      gl.attachShader(program, fragment);
      gl.linkProgram(program);
      return program;
    }

    function objectColor(object) {
      const type = (object.type || '').toUpperCase();
      const name = (object.name || '').toUpperCase();

      if (name === 'POLARIS') return [1.0, 0.95, 0.8];
      if (type.includes('PLAN')) return [0.5, 0.85, 1.0];
      if (type.includes('GAL')) return [0.88, 0.72, 1.0];
      if (type.includes('NEB')) return [1.0, 0.62, 0.52];
      if (type.includes('AMAS') || type.includes('GLOB')) return [1.0, 0.88, 0.5];

      if (type.includes('O') || type.includes('B')) return [0.65, 0.82, 1.0];
      if (type.includes('A')) return [0.92, 0.96, 1.0];
      if (type.includes('F')) return [1.0, 1.0, 0.88];
      if (type.includes('G')) return [1.0, 0.96, 0.65];
      if (type.includes('K')) return [1.0, 0.78, 0.45];
      if (type.includes('M')) return [1.0, 0.55, 0.35];

      if (object.mag < 1.5) return [1.0, 0.98, 0.88];
      return [0.92, 0.95, 1.0];
    }

    function matchesFilter(object) {
      const type = (object.type || '').toLowerCase();
      const maxMag = parseFloat(magFilterElement.value);
      if (object.mag > maxMag) return false;

      const checkboxes = filterDropMenu.querySelectorAll('input[type="checkbox"]');
      let allowed = false;
      checkboxes.forEach(cb => {
        if (cb.checked) {
          const val = cb.value;
          if (val === 'galaxie' && type.includes('gal')) allowed = true;
          if (val === 'nebuleuse' && type.includes('neb')) allowed = true;
          if (val === 'amas' && (type.includes('amas') || type.includes('glob'))) allowed = true;
          if (val === 'etoile' && !type.includes('gal') && !type.includes('neb') && !type.includes('amas') && !type.includes('glob')) allowed = true;
        }
      });
      return allowed;
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
        azimuth: (azimuth * 180 / Math.PI + 360) % 360,
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

      for (let alt = 0; alt <= 80; alt += 20) {
        for (let az = 0; az < 360; az += 2) {
          add(az, alt); add(az + 2, alt);
        }
      }

      for (let az = 0; az < 360; az += 30) {
        for (let alt = 0; alt < 90; alt += 2) {
          add(az, alt); add(az, alt + 2);
        }
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
      resizeCanvas();
      const width = canvas.width;
      const height = canvas.height;
      const aspect = width / height;
      
      // Interpolation plus rapide (0.4) pour coller au plus près des mouvements du télescope
      let diffAZ = targetHeading - heading;
      if (diffAZ > 180) diffAZ -= 360;
      if (diffAZ < -180) diffAZ += 360;
      heading = (heading + diffAZ * 0.4 + 360) % 360;
      
      elevation += (targetElevation - elevation) * 0.4;

      const currentHeading = (heading + touchHeading + alignOffsetHeading + 360) % 360;
      const currentElevation = Math.max(-5, Math.min(88, elevation + touchElevation + alignOffsetElevation));
      
      const az = currentHeading * Math.PI / 180;
      const el = currentElevation * Math.PI / 180;
      const forward = [Math.sin(az) * Math.cos(el), Math.sin(el), Math.cos(az) * Math.cos(el)];
      const right = [Math.cos(az), 0, -Math.sin(az)];
      const up = [-Math.sin(az) * Math.sin(el), Math.cos(el), -Math.cos(az) * Math.sin(el)];
      const tanHalfFov = Math.tan(fieldOfView * Math.PI / 180);
      
      const currentSkyDate = getSkyDate();
      const sidereal = siderealDegrees(currentSkyDate);
      const latRadians = latitude * Math.PI / 180;

      const objectMap = new Map();
      for (const object of objects) {
        const horizontal = horizontalPosition(object, sidereal, latRadians);
        objectMap.set(object.name, horizontal.position);
      }

      const cardinalPoints = [
        { id: 'labelN', az: 0 }, { id: 'labelE', az: 90 },
        { id: 'labelS', az: 180 }, { id: 'labelW', az: 270 }
      ];
      cardinalPoints.forEach(cp => {
        const azRad = cp.az * Math.PI / 180;
        const pos = [Math.sin(azRad), 0, Math.cos(azRad)];
        const depth = pos[0] * forward[0] + pos[1] * forward[1] + pos[2] * forward[2];
        const elEl = document.getElementById(cp.id);
        if (depth > 0.05) {
          const sx = (pos[0] * right[0] + pos[1] * right[1] + pos[2] * right[2]) / (depth * tanHalfFov * aspect);
          const sy = (pos[0] * up[0] + pos[1] * up[1] + pos[2] * up[2]) / (depth * tanHalfFov);
          if (Math.abs(sx) <= 1.1 && Math.abs(sy) <= 1.1) {
            elEl.style.left = `${(sx + 1) * 50}%`;
            elEl.style.top = `${(1 - sy) * 50}%`;
            elEl.style.display = 'block';
          } else {
            elEl.style.display = 'none';
          }
        } else {
          elEl.style.display = 'none';
        }
      });

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
        
        let starSize = Math.max(3.5, Math.min(24, (7.5 - object.mag) * 2.8 + 4));
        if (object.name === "Polaris") starSize = 22;
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

      gl.uniform1f(gridLocation, 1);
      gl.bindBuffer(gl.ARRAY_BUFFER, gridBuffer);
      gl.vertexAttribPointer(positionLocation, 3, gl.FLOAT, false, 0, 0);
      gl.enableVertexAttribArray(positionLocation);
      gl.disableVertexAttribArray(colorLocation);
      gl.disableVertexAttribArray(sizeLocation);
      gl.vertexAttrib3f(colorLocation, 0.32, 0.45, 0.62);
      gl.vertexAttrib1f(sizeLocation, 1);
      gl.drawArrays(gl.LINES, 0, gridVertices.length / 3);

      if (lineVertices.length > 0) {
        gl.bindBuffer(gl.ARRAY_BUFFER, lineBuffer);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(lineVertices), gl.DYNAMIC_DRAW);
        gl.vertexAttribPointer(positionLocation, 3, gl.FLOAT, false, 0, 0);
        gl.vertexAttrib3f(colorLocation, 0.55, 0.82, 1.0); 
        gl.drawArrays(gl.LINES, 0, lineVertices.length / 3);
      }

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
        gl.vertexAttribPointer(sizeBuffer, 1, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(sizeBuffer);
        gl.drawArrays(gl.POINTS, 0, sizes.length);
      }

      viewReadout.textContent = `Vue AZ ${Math.round(currentHeading)}° · ALT ${Math.round(currentElevation)}°`;

      const labels = document.createDocumentFragment();
      for (const item of lastScreenObjects) {
        const isPolaris = (item.object.name === "Polaris");
        if (!isPolaris && (!showNames || item.object.mag > 8.0)) continue;
        if (!isPolaris && !showNames) continue;
        if (!isPolaris && lastScreenObjects.filter(e => e.object.mag <= 8.0).indexOf(item) > 22) continue;

        const label = document.createElement('span');
        label.className = isPolaris ? 'object-label polaris' : 'object-label';
        label.style.left = `${(item.screenX + 1) * 50}%`;
        label.style.top = `${(1 - item.screenY) * 50}%`;
        label.textContent = isPolaris ? `★ Polaris (Pôle Nord)` : (item.object.commonName || item.object.name);
        labels.appendChild(label);
      }
      labelsElement.replaceChildren(labels);

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

        resizeCanvas();

        const [catalogRes, statusRes] = await Promise.all([fetch('/catalog'), fetch('/status')]);
        objects = await catalogRes.json();
        const status = await statusRes.json();
        
        applyEncoderStatus(status);
        heading = targetHeading;
        elevation = targetElevation;

        latitude = Number(status.lat ?? latitude);
        longitude = Number(status.lon ?? longitude);
        
        updateSkyBackground(getSkyDate());
        setStatus(`${objects.length} objets chargés. Planétarium prêt.`, "align-ok");
        
        // Polling des encodeurs augmenté à 100 ms (10 fois par seconde) pour plus de réactivité
        window.setInterval(pollEncoderStatus, 100);
        
        resizeCanvas();
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
      alignOffsetHeading = 0;
      alignOffsetElevation = 0;
      fieldOfView = 34;
    });

    window.addEventListener('resize', resizeCanvas);
    checkOrientation();
    initialize();
  </script>
</body>
</html>
)rawliteral";

#endif