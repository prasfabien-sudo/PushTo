#ifndef HELPPAGE_H
#define HELPPAGE_H

#include <pgmspace.h>

const char HTTP_HELP_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Aide & Connexion SkySafari</title>
  <style>
    .ui-icon { display: inline-block; width: 1em; height: 1em; vertical-align: -0.15em; fill: none; stroke: currentColor; stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; }
    .nav-icon .ui-icon { vertical-align: middle; }
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #121212; color: #fff; padding: 15px; margin: 0; }
    .card { background: #1e1e1e; border-radius: 12px; padding: 20px; max-width: 600px; margin: 0 auto; box-shadow: 0 4px 12px rgba(0,0,0,0.5); }
    h2 { color: #ff9800; margin-top: 10px; text-align: center; }
    h3 { color: #80cbc4; border-bottom: 1px solid #333; padding-bottom: 5px; margin-top: 20px; }
    .btn { padding: 12px 20px; background: #4caf50; color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer; width: 100%; margin-top: 15px; transition: all 0.15s ease-in-out; }
    .btn:hover { filter: brightness(1.15); }
    .btn:active { transform: scale(0.97); filter: brightness(0.85); }
    .btn-sec { background: #2196f3; text-decoration: none; display: block; text-align: center; margin-bottom: 15px; width: 100%; box-sizing: border-box; }
    
    ol { padding-left: 20px; line-height: 1.6; }
    li { margin-bottom: 10px; color: #e0e0e0; }
    .highlight { color: #ffb74d; font-weight: bold; }
    .code-box { background: #2a2a2a; padding: 8px 12px; border-radius: 6px; font-family: monospace; color: #4caf50; display: inline-block; margin: 4px 0; }
    .note { background: #263238; border-left: 4px solid #00897b; padding: 10px; border-radius: 4px; font-size: 0.9em; margin-top: 15px; color: #b0bec5; }
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
    <h2><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-book-open'></use></svg> Guide de Connexion SkySafari</h2>

    <h3>1. Connexion au réseau Wi-Fi</h3>
    <ol>
      <li>Coupe tes données mobile.</li>
      <li>Ouvre les paramètres Wi-Fi de ton smartphone ou de ta tablette.</li>
      <li>Connecte-toi au réseau Wi-Fi émis par le télescope : <br><span class="code-box">Dobson_PushTo</span></li>
      <li>Aucun mot de passe n'est requis. Veille à maintenir la connexion Wi-Fi même si ton téléphone indique "Pas d'accès Internet".</li>
    </ol>

    <h3>2. Configuration dans SkySafari</h3>
    <p style="font-size:0.9em; color:#ccc;">Configuration valable pour SkySafari Plus ou Pro (iOS / Android) :</p>
    <ol>
      <li>Ouvre l'application <strong>SkySafari</strong>.</li>
      <li>Rends-toi dans <strong>Settings</strong> (Paramètres) ➔ <strong>Telescope</strong> ➔ <strong>Setup</strong>.</li>
      <li>Renseigne les paramètres suivants :
        <ul>
          <li><strong>Equipment Type :</strong> <span class="highlight">Basic Encoder System</span></li>
          <li><strong>Mount Type :</strong> <span class="highlight">Alt-Az Push-To</span></li>
          <li><strong>Communication Type :</strong> <span class="highlight">Wi-Fi or Ethernet</span></li>
          <li><strong>IP Address :</strong> <span class="code-box">192.168.4.1</span></li>
          <li><strong>Port Number :</strong> <span class="code-box">4030</span></li>
        </ul>
      </li>
      <li>Dans la section <strong>Encoder Steps Per Revolution</strong> (Résolution des encodeurs) :
        <ul>
          <li>Indique le nombre de pas réglé sur ton ESP32 pour l'Azimut et l'Altitude (ex: <span class="highlight">10000</span> pas par tour).</li>
        </ul>
      </li>
    </ol>

    <h3>3. Connexion & Utilisation</h3>
    <ol>
      <li>Reviens à la carte du ciel principale dans SkySafari.</li>
      <li>Appuie sur le bouton <strong>Scope</strong> ➔ <strong>Connect</strong> dans le menu inférieur.</li>
      <li>Une reticule représentant la position de ton télescope apparaît à l'écran. Il se déplace en temps réel lorsque tu bouges le Dobson.</li>
    </ol>

    <div class="note">
      <svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-lightbulb'></use></svg> <strong>Conseil :</strong> Pense à effectuer la mise en station (recalage au Nord et à l'horizontale) depuis la page <strong><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-crosshair'></use></svg> Mise en Station / Alignement</strong> de l'interface Web avant de pointer tes premiers objets dans SkySafari.
    </div>
  </div>
  <nav class="bottom-nav" aria-label="Navigation principale">
    <a class="nav-item" href="/" aria-label="Menu principal" title="Menu principal"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-telescope'></use></svg></span></a>
    <a class="nav-item" href="/station" aria-label="Mise en station" title="Mise en station"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-crosshair'></use></svg></span></a>
    <a class="nav-item" href="/beta_sky" aria-label="Vue 3D du ciel" title="Vue 3D du ciel"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-globe-2'></use></svg></span></a>
    <a class="nav-item" href="/help" aria-label="Aide SkySafari" aria-current="page" title="Aide SkySafari"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-book-open'></use></svg></span></a>
    <button class="nav-more-button night-mode-toggle" id="nightModeToggle" type="button" aria-label="Activer le mode nuit" title="Activer le mode nuit" aria-pressed="false" onclick="toggleNightMode()"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-moon'></use></svg></span></button>
    <div class="nav-more">
      <button class="nav-more-button" type="button" aria-label="Plus" title="Plus" aria-expanded="false" aria-controls="navMoreMenu" onclick="toggleMoreMenu()"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-more-horizontal'></use></svg></span></button>
      <div class="nav-more-menu" id="navMoreMenu" hidden>
        <a class="nav-more-link" href="/calib_page"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-crosshair'></use></svg> Calibration</a>
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
  </script>
</body>
</html>
)rawliteral";

#endif