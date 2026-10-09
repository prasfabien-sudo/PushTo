#ifndef CALIBPAGE_H
#define CALIBPAGE_H

#include <pgmspace.h>

const char HTTP_CALIB_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Étalonnage des Encodeurs</title>
  <style>
    .ui-icon { display: inline-block; width: 1em; height: 1em; vertical-align: -0.15em; fill: none; stroke: currentColor; stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; }
    .nav-icon .ui-icon { vertical-align: middle; }
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #121212; color: #fff; padding: 15px; margin: 0; }
    .card { background: #1e1e1e; border-radius: 12px; padding: 20px; max-width: 600px; margin: 0 auto; text-align: center; box-shadow: 0 4px 12px rgba(0,0,0,0.5); }
    h2 { color: #ff9800; margin-top: 0; }
    .step-box { background: #2a2a2a; border-radius: 8px; padding: 15px; margin: 15px 0; border-left: 4px solid #2196f3; text-align: left; }
    .step-title { font-weight: bold; color: #81d4fa; margin-bottom: 5px; }
    .btn { padding: 12px 20px; background: #4caf50; color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer; width: 100%; margin-top: 10px; transition: all 0.15s ease-in-out; }
    .btn:hover { filter: brightness(1.15); }
    .btn:active { transform: scale(0.97); filter: brightness(0.85); }
    .btn-sec { background: #424242; text-decoration: none; display: inline-block; margin-bottom: 15px; width: 100%; box-sizing: border-box; }.val-highlight { font-size: 1.3em; color: #ffb74d; font-weight: bold; }
    .deg-highlight { font-size: 1.1em; color: #4caf50; margin-top: 4px; font-weight: normal; }
    body { padding-bottom: 100px; }
    html[data-night-mode="true"]::after { content: ""; position: fixed; inset: 0; z-index: 2147483647; pointer-events: none; background: rgba(255, 0, 0, 0.82); mix-blend-mode: multiply; filter: brightness(0.45); }
    .bottom-nav { position: fixed; z-index: 20; left: 0; right: 0; bottom: 0; display: flex; justify-content: space-around; gap: 4px; padding: 8px 8px calc(8px + env(safe-area-inset-bottom)); background: rgba(30,30,30,0.97); border-top: 1px solid #37474f; box-shadow: 0 -4px 16px rgba(0,0,0,0.4); }
    .nav-item, .nav-more-button { flex: 1; min-width: 0; min-height: 48px; display: flex; align-items: center; justify-content: center; padding: 8px 2px; border: 0; border-radius: 8px; background: transparent; color: #b0bec5; font: inherit; text-decoration: none; cursor: pointer; }
    .nav-item[aria-current="page"], .nav-more-button[aria-expanded="true"], .nav-more-button[aria-current="page"] { color: #ffb74d; background: #263238; }
    .night-mode-toggle { background: #f1f3f4; color: #101418; border: 1px solid #fff; }
    .night-mode-toggle[aria-pressed="true"] { background: #ff5252; color: #fff; }
    .nav-icon { display: flex; font-size: 22px; line-height: 1; }
    .nav-more { position: relative; flex: 1; min-width: 0; display: flex; }
    .nav-more-button { width: 100%; }
    .nav-more-menu { position: absolute; right: 0; bottom: calc(100% + 12px); width: min(230px, calc(100vw - 24px)); padding: 6px; border: 1px solid #455a64; border-radius: 12px; background: #1e1e1e; box-shadow: 0 4px 18px rgba(0,0,0,0.55); }
    .nav-more-menu[hidden] { display: none; }
    .nav-more-link { display: block; padding: 11px 12px; border-radius: 7px; color: #e0e0e0; text-decoration: none; font-size: 0.9em; }
    .nav-more-link:hover, .nav-more-link[aria-current="page"] { background: #263238; color: #ffb74d; }
    @media (min-width: 700px) { .bottom-nav { left: 50%; right: auto; width: min(500px, calc(100% - 32px)); transform: translateX(-50%); border: 1px solid #37474f; border-bottom: 0; border-radius: 14px 14px 0 0; } }
  </style>
</head>
<body>
  <div class="card">
    <h2>Étalonnage des Encodeurs</h2>

    <div class="step-box">
      <div class="step-title">1. Démarrage</div>
      <p>Positionnez le télescope à l'horizontale (0° avec niveau à bulle) et sur un repère Azimut fixe.</p>
      <button class="btn" onclick="sendCalib('start')">Démarrer l'étalonnage</button>
    </div>

    <div class="step-box">
      <div class="step-title">2. Axe Azimut (Tour complet 360°)</div>
      <p>Faites effectuer un tour complet de <strong>360°</strong> à l'azimut jusqu'à revenir sur votre repère.</p>
      <div>Pas mesurés : <span class="val-highlight" id="rawAZ">0</span></div>
      <div>Angle équivalent : <span class="deg-highlight" id="degAZ">0.0°</span></div>
      <button class="btn" onclick="sendCalib('fin_az')">Valider Azimut (360°)</button>
    </div>

    <div class="step-box">
      <div class="step-title">3. Axe Altitude (Basculement 90°)</div>
      <p>Pointez à l'horizontale (0°), puis montez le tube à la verticale exacte (<strong>90°</strong> au zénith).</p>
      <div>Pas mesurés (sur 90°) : <span class="val-highlight" id="rawALT">0</span></div>
      <div>Angle calculé : <span class="deg-highlight" id="degALT">0.0°</span></div>
      <button class="btn" onclick="sendCalib('fin_alt_90')">Valider Altitude (90°)</button>
    </div>
  </div>

  <nav class="bottom-nav" aria-label="Navigation principale">
    <a class="nav-item" href="/" aria-label="Menu principal" title="Menu principal"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-telescope'></use></svg></span></a>
    <a class="nav-item" href="/station" aria-label="Mise en station" title="Mise en station"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-crosshair'></use></svg></span></a>
    <a class="nav-item" href="/beta_sky" aria-label="Vue 3D du ciel" title="Vue 3D du ciel"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-globe-2'></use></svg></span></a>
    <a class="nav-item" href="/help" aria-label="Aide SkySafari" title="Aide SkySafari"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-book-open'></use></svg></span></a>
    <button class="nav-more-button night-mode-toggle" id="nightModeToggle" type="button" aria-label="Activer le mode nuit" title="Activer le mode nuit" aria-pressed="false" onclick="toggleNightMode()"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-moon'></use></svg></span></button>
    <div class="nav-more">
      <button class="nav-more-button" type="button" aria-label="Plus" title="Plus" aria-current="page" aria-expanded="false" aria-controls="navMoreMenu" onclick="toggleMoreMenu()"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-more-horizontal'></use></svg></span></button>
      <div class="nav-more-menu" id="navMoreMenu" hidden>
        <a class="nav-more-link" href="/calib_page" aria-current="page"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-crosshair'></use></svg> Calibration</a>
        <a class="nav-more-link" href="/config"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-settings'></use></svg> Configuration</a>
        <a class="nav-more-link" href="/simu"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-gamepad-2'></use></svg> Simulation</a>
        <a class="nav-more-link" href="/test"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-flask-conical'></use></svg> Tests système</a>
        <a class="nav-more-link" href="/releasenotes"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-clipboard-list'></use></svg> Notes de version</a>
      </div>
    </div>
  </nav>
  <script>
    function setNightMode(enabled, persist = false) {
      document.documentElement.dataset.nightMode = String(enabled);
      const button = document.getElementById('nightModeToggle');
      const label = enabled ? 'D\u00e9sactiver le mode nuit' : 'Activer le mode nuit';
      button.setAttribute('aria-label', label);
      button.setAttribute('aria-pressed', String(enabled));
      button.title = label;
      if (persist) localStorage.setItem('dobson-night-mode', String(enabled));
    }

    function toggleNightMode() {
      setNightMode(document.documentElement.dataset.nightMode !== 'true', true);
    }

    setNightMode(localStorage.getItem('dobson-night-mode') === 'true');

    function toggleMoreMenu() {
      const menu = document.getElementById('navMoreMenu');
      const button = document.querySelector('.nav-more-button');
      menu.hidden = !menu.hidden;
      button.setAttribute('aria-expanded', String(!menu.hidden));
    }

    let ticksAZ = 10000;
    let ticksALT = 10000;

    function updateStatus() {
      fetch('/status').then(r => r.json()).then(d => {
        ticksAZ = d.ticksAZ || 10000;
        ticksALT = d.ticksALT || 10000;

        let rawAZ = d.rawAZ;
        let rawALT = d.rawALT;

        let angleAZ = ((rawAZ / ticksAZ) * 360.0) % 360.0;
        let angleALT = (rawALT / ticksALT) * 360.0;
        if (angleAZ < 0) angleAZ += 360.0;

        document.getElementById('rawAZ').innerText = rawAZ;
        document.getElementById('degAZ').innerText = `${angleAZ.toFixed(1)}°`;

        document.getElementById('rawALT').innerText = rawALT;
        document.getElementById('degALT').innerText = `${angleALT.toFixed(1)}° (Cible : 90°)`;
      });
    }

    function sendCalib(cmd) {
      fetch(`/calib?cmd=${cmd}`).then(() => updateStatus());
    }

    setInterval(updateStatus, 250);
  </script>
</body>
</html>
)rawliteral";

#endif