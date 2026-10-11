#ifndef STATIONPAGE_H
#define STATIONPAGE_H

#include <pgmspace.h>

const char HTTP_STATION_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Mise en Station & Alignement</title>
  <style>
    .ui-icon { display: inline-block; width: 1em; height: 1em; vertical-align: -0.15em; fill: none; stroke: currentColor; stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; }
    .nav-icon .ui-icon { vertical-align: middle; }
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #121212; color: #fff; padding: 15px; margin: 0; }
    .card { background: #1e1e1e; border-radius: 12px; padding: 20px; max-width: 500px; margin: 0 auto; box-shadow: 0 4px 12px rgba(0,0,0,0.5); }
    h2 { color: #ff9800; margin-top: 10px; text-align: center; }
    .btn { padding: 12px 20px; background: #4caf50; color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer; width: 100%; margin-top: 10px; transition: all 0.15s ease-in-out; }
    .btn:hover { filter: brightness(1.15); }
    .btn:active { transform: scale(0.97); filter: brightness(0.85); }
    .btn-sec { background: #2196f3; text-decoration: none; display: block; text-align: center; margin-bottom: 15px; width: 100%; box-sizing: border-box; }
    .btn-step1 { background: #ff9800; }
    .btn-step2 { background: #e91e63; }
    
    .section-title { font-weight: bold; color: #80cbc4; border-bottom: 1px solid #333; padding-bottom: 5px; margin-top: 20px; margin-bottom: 10px; }
    .form-group { display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px; }
    label { font-size: 0.85em; color: #ccc; }
    input[type="number"] { background: #121212; color: #ffb74d; border: 1px solid #444; padding: 6px; border-radius: 6px; font-size: 0.9em; width: 90px; text-align: right; }
    
    .step-box { background: #2a2a2a; border-left: 4px solid #ff9800; padding: 10px; border-radius: 4px; margin-bottom: 12px; font-size: 0.85em; color: #ddd; }
    .status-ok { color: #4caf50; font-weight: bold; }
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
    <h2><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-compass'></use></svg> Mise en Station & Recalage</h2>

    <!-- Quick Zero Section -->
    <div class="section-title">1. Calage Rapide sur Repères</div>
    <button class="btn" onclick="sendCmd('set_zero_alt')"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-ruler'></use></svg> Mettre à zéro l'Altitude (Tube à l'horizontale)</button>
    <button class="btn" onclick="sendCmd('set_zero_az')"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-compass'></use></svg> Mettre à zéro l'Azimut (Pointe au Nord)</button>
    <button class="btn" onclick="sendCmd('set_polaris')"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-star'></use></svg> Recalage rapide sur la Polaire</button>

    <!-- 2 Stars Alignment Step by Step -->
    <div class="section-title">2. Alignement Guidé 2 Étoiles</div>
    
    <div class="step-box" id="stepGuide">
      <strong>Procédure :</strong><br>
      1. Vise la 1ère étoile dans le télescope, entre ses coordonnées puis valide.<br>
      2. Vise la 2ème étoile, entre ses coordonnées et valide pour terminer.
    </div>

    <div id="star1Group">
      <div style="font-weight:bold; color:#ffb74d; margin-bottom:5px;">Étape 1 : Première Étoile</div>
      <div class="form-group">
        <label>Azimut Théorique (°)</label>
        <input type="number" step="0.1" id="az1" placeholder="ex: 120.5">
      </div>
      <div class="form-group">
        <label>Altitude Théorique (°)</label>
        <input type="number" step="0.1" id="alt1" placeholder="ex: 45.0">
      </div>
      <button class="btn btn-step1" onclick="validateStar1()"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-star'></use></svg> Valider Étoile 1</button>
    </div>

    <div id="star2Group" style="margin-top: 15px; opacity: 0.5;">
      <div style="font-weight:bold; color:#e91e63; margin-bottom:5px;">Étape 2 : Seconde Étoile</div>
      <div class="form-group">
        <label>Azimut Théorique (°)</label>
        <input type="number" step="0.1" id="az2" placeholder="ex: 215.0" disabled>
      </div>
      <div class="form-group">
        <label>Altitude Théorique (°)</label>
        <input type="number" step="0.1" id="alt2" placeholder="ex: 30.0" disabled>
      </div>
      <button class="btn btn-step2" id="btnStar2" onclick="validateStar2()" disabled><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-star'></use></svg> Valider Étoile 2 & Terminer</button>
    </div>

  </div>

  <nav class="bottom-nav" aria-label="Navigation principale">
    <a class="nav-item" href="/beta_sky" title="Vue 3D du ciel"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-globe-2'></use></svg></span></a>
    <a class="nav-item" href="/" title="Mode Encodeurs"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-telescope'></use></svg></span></a>
    <a class="nav-item" href="/station" aria-current="page" title="Mise en station"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-compass'></use></svg></span></a>
    <a class="nav-item" href="/calib_page" title="Calibration"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-ruler'></use></svg></span></a>
    <a class="nav-item" href="/config" title="Configuration"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-settings'></use></svg></span></a>
    <a class="nav-item" href="/help" title="Aide SkySafari"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-book-open'></use></svg></span></a>
    <a class="nav-item" href="/simu" title="Simulation"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-gamepad-2'></use></svg></span></a>
    <a class="nav-item" href="/test" title="Tests système"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-flask-conical'></use></svg></span></a>
    <a class="nav-item" href="/releasenotes" title="Notes de version"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-clipboard-list'></use></svg></span></a>
    <button class="nav-item night-mode-toggle" id="nightModeToggle" type="button" aria-label="Activer le mode nuit" title="Activer le mode nuit" aria-pressed="false" onclick="toggleNightMode()"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-moon'></use></svg></span></button>
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

    function sendCmd(action) {
      fetch(`/cmd?action=${action}`)
        .then(r => r.text())
        .then(res => alert("Action exécutée : " + action));
    }

    function validateStar1() {
      const az = document.getElementById('az1').value;
      const alt = document.getElementById('alt1').value;

      if (!az || !alt) {
        alert("Veuillez saisir l'Azimut et l'Altitude de la première étoile.");
        return;
      }

      fetch(`/cmd?action=align_star1&az=${az}&alt=${alt}`)
        .then(r => r.json())
        .then(d => {
          if (d.status === "OK") {
            document.getElementById('stepGuide').innerHTML = '<span class="status-ok"><svg class="ui-icon" aria-hidden="true" focusable="false"><use href="/icons.svg#icon-circle-check"></use></svg> Étoile 1 enregistrée !</span><br>Déplace le télescope vers la 2ème étoile, saisis ses coordonnées et valide.';
            
            document.getElementById('az2').disabled = false;
            document.getElementById('alt2').disabled = false;
            document.getElementById('btnStar2').disabled = false;
            document.getElementById('star2Group').style.opacity = '1.0';
          }
        });
    }

    function validateStar2() {
      const az = document.getElementById('az2').value;
      const alt = document.getElementById('alt2').value;

      if (!az || !alt) {
        alert("Veuillez saisir l'Azimut et l'Altitude de la seconde étoile.");
        return;
      }

      fetch(`/cmd?action=align_star2&az=${az}&alt=${alt}`)
        .then(r => r.json())
        .then(d => {
          if (d.status === "OK") {
            alert("Alignement sur 2 étoiles réussi ! Les compteurs du télescope sont recalés.");
            location.reload();
          }
        });
    }
  </script>
</body>
</html>
)rawliteral";

#endif