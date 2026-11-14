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
    .step-picker { display: flex; align-items: center; justify-content: center; gap: 10px; margin: 12px 0 18px; }
    .step-picker label { color: #ccc; font-size: .9em; }
    .step-picker select { min-height: 42px; min-width: 90px; padding: 6px 10px; border: 1px solid #455a64; border-radius: 8px; background: #121212; color: #ffb74d; font: inherit; }
    .direction-pad { display: grid; grid-template-columns: repeat(3, 64px); grid-template-rows: repeat(3, 56px); justify-content: center; gap: 6px; margin-bottom: 15px; }
    .direction-btn { display: flex; align-items: center; justify-content: center; width: 100%; height: 100%; padding: 8px; margin: 0; }
    .direction-btn .ui-icon { width: 28px; height: 28px; }
    .direction-up { grid-column: 2; grid-row: 1; }
    .direction-left { grid-column: 1; grid-row: 2; }
    .direction-down { grid-column: 2; grid-row: 3; }
    .direction-right { grid-column: 3; grid-row: 2; }
    .direction-center { grid-column: 2; grid-row: 2; display: flex; align-items: center; justify-content: center; color: #607d8b; font-size: .75em; }
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
    <div class="step-picker">
      <label for="simulationStep">Pas de déplacement :</label>
      <select id="simulationStep">
        <option value="0.1">0,1°</option>
        <option value="0.5">0,5°</option>
        <option value="1" selected>1°</option>
        <option value="5">5°</option>
        <option value="10">10°</option>
      </select>
    </div>

    <div class="axis-title">Axe Azimut (AZ)</div>
    <div class="info-res" id="infoAZ">Résolution AZ: -- pas/rev</div>
    <div class="val" id="valAZ">0.0° (0 pas)</div>
    <div class="direction-pad" role="group" aria-label="Déplacer l'axe azimut">
      <button class="btn direction-btn direction-left" type="button" aria-label="Tourner l'azimut vers la gauche" title="Azimut gauche" onclick="stepSelected('az', -1)"><svg class="ui-icon" aria-hidden="true" focusable="false"><use href="/icons.svg#icon-arrow-left"></use></svg></button>
      <span class="direction-center" aria-hidden="true">AZ</span>
      <button class="btn direction-btn direction-right" type="button" aria-label="Tourner l'azimut vers la droite" title="Azimut droite" onclick="stepSelected('az', 1)"><svg class="ui-icon" aria-hidden="true" focusable="false"><use href="/icons.svg#icon-arrow-right"></use></svg></button>
    </div>

    <div class="axis-title">Axe Altitude (ALT)</div>
    <div class="info-res" id="infoALT">Résolution ALT: -- pas/rev</div>
    <div class="val" id="valALT">0.0° (0 pas)</div>
    <div class="direction-pad" role="group" aria-label="Déplacer l'axe altitude">
      <button class="btn direction-btn direction-up" type="button" aria-label="Monter l'altitude" title="Altitude haut" onclick="stepSelected('alt', 1)"><svg class="ui-icon" aria-hidden="true" focusable="false"><use href="/icons.svg#icon-arrow-up"></use></svg></button>
      <span class="direction-center" aria-hidden="true">ALT</span>
      <button class="btn direction-btn direction-down" type="button" aria-label="Descendre l'altitude" title="Altitude bas" onclick="stepSelected('alt', -1)"><svg class="ui-icon" aria-hidden="true" focusable="false"><use href="/icons.svg#icon-arrow-down"></use></svg></button>
    </div>
    
    <button class="btn" style="background: #e53935; width: 100%; margin-top: 10px;" onclick="resetSimulation()">Remettre les deux axes à zéro</button>
    <div class="sim-status" id="simStatus" role="status" aria-live="polite"></div>
  </div>

  <nav class="bottom-nav" aria-label="Navigation principale">
    <a class="nav-item" href="/beta_sky" title="Vue 3D du ciel"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-globe-2'></use></svg></span></a>
    <a class="nav-item" href="/" title="Mode Encodeurs"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-telescope'></use></svg></span></a>
    <a class="nav-item" href="/station" title="Mise en station"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-compass'></use></svg></span></a>
    <a class="nav-item" href="/config" title="Configuration"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-settings'></use></svg></span></a>
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

    function stepSelected(axis, direction) {
      const step = Number(document.getElementById('simulationStep').value);
      if (![0.1, 0.5, 1, 5, 10].includes(step) || ![-1, 1].includes(direction)) {
        document.getElementById('simStatus').innerText = 'Pas de déplacement invalide.';
        return;
      }

      stepDeg(axis, step * direction);
    }

    function stepDeg(axis, deg) {
      let ticks = (axis === 'az') ? ticksAZ : ticksALT;
      let deltaTicks = Math.sign(deg) * Math.max(1, Math.round((Math.abs(deg) / 360.0) * ticks));
      fetch(`/simstep?axis=${axis}&delta=${deltaTicks}`)
        .then(async r => {
          if (!r.ok) {
            const result = await r.json();
            throw new Error(result.message || `HTTP ${r.status}`);
          }
          updateVal();
        })
        .catch(error => {
          document.getElementById('simStatus').innerText = `Erreur de simulation : ${error.message}`;
        });
    }

    function resetSimulation() {
      fetch('/simstep?axis=reset&delta=0')
        .then(async r => {
          if (!r.ok) {
            const result = await r.json();
            throw new Error(result.message || `HTTP ${r.status}`);
          }
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