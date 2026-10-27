#ifndef SIMUPAGE_H
#define SIMUPAGE_H

#include <pgmspace.h>

const char HTTP_SIMU_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Simulateur Push-To</title>
  <style>
    .ui-icon { display: inline-block; width: 1em; height: 1em; vertical-align: -0.15em; fill: none; stroke: currentColor; stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; }
    .nav-icon .ui-icon { vertical-align: middle; }
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #121212; color: #fff; padding: 15px; margin: 0; }
    .card { background: #1e1e1e; border-radius: 12px; padding: 20px; max-width: 500px; margin: 0 auto; text-align: center; box-shadow: 0 4px 12px rgba(0,0,0,0.5); }
    h2 { color: #ff9800; margin-top: 0; }
    .axis-title { font-weight: bold; color: #80cbc4; margin-top: 15px; margin-bottom: 5px; }
    .info-res { font-size: 0.85em; color: #aaa; margin-bottom: 8px; }
    .btn { padding: 10px 14px; background: #2196f3; color: white; border: none; border-radius: 8px; font-weight: bold; cursor: pointer; margin: 4px; transition: all 0.15s ease-in-out; }
    .btn:hover { filter: brightness(1.15); }
    .btn:active { transform: scale(0.95); filter: brightness(0.85); }
    .btn-group { display: flex; flex-wrap: wrap; justify-content: center; gap: 4px; margin-bottom: 15px; }
    .val { font-size: 1.2em; color: #ffb74d; margin: 10px 0; font-weight: bold; }
    .val-neg { color: #ff5252 !important; }
    .sim-status { min-height: 1.2em; color: #ff5252; font-size: 0.85em; margin-top: 10px; }
    
    body { padding-bottom: 100px; }
    html[data-night-mode="true"]::after { content: ""; position: fixed; inset: 0; z-index: 2147483647; pointer-events: none; background: rgba(255, 0, 0, 0.82); mix-blend-mode: multiply; filter: brightness(0.45); }
    
    .bottom-nav { position: fixed; z-index: 20; left: 0; right: 0; bottom: 0; display: flex; overflow-x: auto; scroll-snap-type: x mandatory; -webkit-overflow-scrolling: touch; gap: 6px; padding: 8px 12px calc(8px + env(safe-area-inset-bottom)); background: rgba(30,30,30,0.98); border-top: 1px solid #37474f; box-shadow: 0 -4px 16px rgba(0,0,0,0.4); scrollbar-width: none; }
    .bottom-nav::-webkit-scrollbar { display: none; }
    .nav-item { flex: 0 0 56px; min-height: 48px; display: flex; align-items: center; justify-content: center; border: 0; border-radius: 8px; background: transparent; color: #b0bec5; font: inherit; text-decoration: none; cursor: pointer; scroll-snap-align: start; }
    .nav-item[aria-current="page"] { color: #ffb74d; background: #263238; }
    .night-mode-toggle { background: #f1f3f4; color: #101418; border: 1px solid #fff; }
    .night-mode-toggle[aria-pressed="true"] { background: #ff5252; color: #fff; }
    .nav-icon { display: flex; font-size: 24px; line-height: 1; }
    @media (min-width: 700px) { .bottom-nav { left: 50%; right: auto; width: min(650px, calc(100% - 32px)); transform: translateX(-50%); border: 1px solid #37474f; border-bottom: 0; border-radius: 14px 14px 0 0; justify-content: center; overflow-x: hidden; } .nav-item { flex: 1; } }
  </style>
</head>
<body>
  <div class="card">
    <h2>Simulateur en Degrés</h2>

    <div class="axis-title">Axe Azimut (AZ)</div>
    <div class="info-res" id="infoAZ">Résolution AZ: -- pas/rev</div>
    <div class="val" id="valAZ">0.0° (0 pas)</div>
    <div class="btn-group">
      <button class="btn" onclick="stepDeg('az', -90)">-90°</button>
      <button class="btn" onclick="stepDeg('az', -45)">-45°</button>
      <button class="btn" onclick="stepDeg('az', -10)">-10°</button>
      <button class="btn" onclick="stepDeg('az', -1)">-1°</button>
      <button class="btn" onclick="stepDeg('az', -0.1)">-0.1°</button>
      <button class="btn" onclick="stepDeg('az', 0.1)">+0.1°</button>
      <button class="btn" onclick="stepDeg('az', 1)">+1°</button>
      <button class="btn" onclick="stepDeg('az', 10)">+10°</button>
      <button class="btn" onclick="stepDeg('az', 45)">+45°</button>
      <button class="btn" onclick="stepDeg('az', 90)">+90°</button>
    </div>

    <div class="axis-title">Axe Altitude (ALT)</div>
    <div class="info-res" id="infoALT">Résolution ALT: -- pas/rev</div>
    <div class="val" id="valALT">0.0° (0 pas)</div>
    <div class="btn-group">
      <button class="btn" onclick="stepDeg('alt', -90)">-90°</button>
      <button class="btn" onclick="stepDeg('alt', -45)">-45°</button>
      <button class="btn" onclick="stepDeg('alt', -10)">-10°</button>
      <button class="btn" onclick="stepDeg('alt', -1)">-1°</button>
      <button class="btn" onclick="stepDeg('alt', -0.1)">-0.1°</button>
      <button class="btn" onclick="stepDeg('alt', 0.1)">+0.1°</button>
      <button class="btn" onclick="stepDeg('alt', 1)">+1°</button>
      <button class="btn" onclick="stepDeg('alt', 10)">+10°</button>
      <button class="btn" onclick="stepDeg('alt', 45)">+45°</button>
      <button class="btn" onclick="stepDeg('alt', 90)">+90°</button>
    </div>
    
    <button class="btn" style="background: #e53935; width: 100%; margin-top: 10px;" onclick="resetSimulation()">Remettre les deux axes à zéro</button>
    <div class="sim-status" id="simStatus" role="status" aria-live="polite"></div>
  </div>

  <nav class="bottom-nav" aria-label="Navigation principale">
    <a class="nav-item" href="/beta_sky" title="Vue 3D du ciel"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-globe-2'></use></svg></span></a>
    <a class="nav-item" href="/" title="Mode Encodeurs"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-telescope'></use></svg></span></a>
    <a class="nav-item" href="/station" title="Mise en station"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-compass'></use></svg></span></a>
    <a class="nav-item" href="/calib_page" title="Calibration"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-ruler'></use></svg></span></a>
    <a class="nav-item" href="/config" title="Configuration"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-settings'></use></svg></span></a>
    <a class="nav-item" href="/help" title="Aide SkySafari"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-book-open'></use></svg></span></a>
    <a class="nav-item" href="/simu" aria-current="page" title="Simulation"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-gamepad-2'></use></svg></span></a>
    <a class="nav-item" href="/test" title="Tests système"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-flask-conical'></use></svg></span></a>
    <a class="nav-item" href="/releasenotes" title="Notes de version"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-clipboard-list'></use></svg></span></a>
    <button class="nav-item night-mode-toggle" id="nightModeToggle" type="button" aria-label="Activer le mode nuit" title="Activer le mode nuit" aria-pressed="false" onclick="toggleNightMode()"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-moon'></use></svg></span></a>
  </nav>

  <script>
    function setNightMode(enabled, persist = false) {
      document.documentElement.dataset.nightMode = String(enabled);
      const button = document.getElementById('nightModeToggle');
      const label = enabled ? 'Désactiver le mode nuit' : 'Activer le mode nuit';
      button.setAttribute('aria-label', label);
      button.setAttribute('aria-pressed', String(enabled));
      button.title = label;
      if (persist) localStorage.setItem('dobson-night-mode', String(enabled));
    }

    function toggleNightMode() {
      setNightMode(document.documentElement.dataset.nightMode !== 'true', true);
    }

    setNightMode(localStorage.getItem('dobson-night-mode') === 'true');

    let ticksAZ = 10000;
    let ticksALT = 10000;

    function updateVal() {
      fetch('/status').then(r => {
        if (!r.ok) throw new Error(`HTTP ${r.status}`);
        return r.json();
      }).then(d => {
        ticksAZ = d.ticksAZ || 10000;
        ticksALT = d.ticksALT || 10000;
        document.getElementById('infoAZ').innerText = `Résolution AZ: ${ticksAZ} pas/rev`;
        document.getElementById('infoALT').innerText = `Résolution ALT: ${ticksALT} pas/rev`;
        let degAZ = ((d.rawAZ / (d.ticksAZ || 10000)) * 360.0) % 360.0;
        let degALT = ((d.rawALT / (d.ticksALT || 10000)) * 360.0) % 360.0;
        if (degAZ < 0) degAZ += 360.0;
        if (degALT < 0) degALT += 360.0;

        const elAZ = document.getElementById('valAZ');
        const elALT = document.getElementById('valALT');

        elAZ.innerText = `${degAZ.toFixed(1)}° (${d.rawAZ} pas)`;
        elALT.innerText = `${degALT.toFixed(1)}° (${d.rawALT} pas)`;

        if (d.rawAZ < 0) elAZ.classList.add('val-neg');
        else elAZ.classList.remove('val-neg');

        if (d.rawALT < 0) elALT.classList.add('val-neg');
        else elALT.classList.remove('val-neg');
        document.getElementById('simStatus').innerText = '';
      }).catch(error => {
        document.getElementById('simStatus').innerText = `Erreur de communication : ${error.message}`;
      });
    }

    function stepDeg(axis, deg) {
      let ticks = (axis === 'az') ? ticksAZ : ticksALT;
      let deltaTicks = Math.round((deg / 360.0) * ticks);
      fetch(`/simstep?axis=${axis}&delta=${deltaTicks}`)
        .then(r => {
          if (!r.ok) throw new Error(`HTTP ${r.status}`);
          updateVal();
        })
        .catch(error => {
          document.getElementById('simStatus').innerText = `Erreur de simulation : ${error.message}`;
        });
    }

    function resetSimulation() {
      fetch('/simstep?axis=reset&delta=0')
        .then(r => {
          if (!r.ok) throw new Error(`HTTP ${r.status}`);
          updateVal();
        })
        .catch(error => {
          document.getElementById('simStatus').innerText = `Erreur de réinitialisation : ${error.message}`;
        });
    }

    updateVal();
    setInterval(updateVal, 1000);
  </script>
</body>
</html>
)rawliteral";

#endif