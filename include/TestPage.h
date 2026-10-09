#ifndef TESTPAGE_H
#define TESTPAGE_H

#include <pgmspace.h>

const char HTTP_TEST_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Tests Automatisés ESP32</title>
  <style>
    .ui-icon { display: inline-block; width: 1em; height: 1em; vertical-align: -0.15em; fill: none; stroke: currentColor; stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; }
    .nav-icon .ui-icon { vertical-align: middle; }
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #121212; color: #fff; padding: 15px; margin: 0; }
    .card { background: #1e1e1e; border-radius: 12px; padding: 20px; max-width: 600px; margin: 0 auto; box-shadow: 0 4px 12px rgba(0,0,0,0.5); }
    h2 { color: #ff9800; margin-top: 10px; text-align: center; }
    .btn { padding: 12px 20px; background: #4caf50; color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer; width: 100%; margin-top: 12px; transition: all 0.15s ease-in-out; }
    .btn:hover { filter: brightness(1.15); }
    .btn:active { transform: scale(0.97); filter: brightness(0.85); }
    .btn-sec { background: #2196f3; text-decoration: none; display: block; text-align: center; margin-bottom: 15px; width: 100%; box-sizing: border-box; }
    
    .test-item { background: #2a2a2a; padding: 10px 12px; border-radius: 6px; margin-bottom: 8px; display: flex; justify-content: space-between; align-items: center; gap: 8px; font-size: 0.9em; }
    .test-name { font-weight: bold; }
    .status-badge { padding: 4px 8px; border-radius: 4px; font-size: 0.8em; font-weight: bold; min-width: 70px; text-align: center; }
    .status-pending { background: #424242; color: #ccc; }
    .status-running { background: #ff9800; color: #000; }
    .status-pass { background: #4caf50; color: #fff; }
    .status-fail { background: #f44336; color: #fff; }
    
    #logOutput { background: #111; border: 1px solid #333; color: #80cbc4; font-family: monospace; padding: 10px; border-radius: 6px; height: 180px; overflow-y: auto; font-size: 0.8em; margin-top: 15px; white-space: pre-wrap; }
    .test-note { color: #b0bec5; font-size: 0.85em; line-height: 1.5; }
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
    <h2><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-flask-conical'></use></svg> Tests Automatisés du Système</h2>

    <div id="testList">
      <div class="test-item">
        <span class="test-name">1. État du système et format JSON (/status)</span>
        <span id="test-status" class="status-badge status-pending">EN ATTENTE</span>
      </div>
      <div class="test-item">
        <span class="test-name">2. Résolution et sens des encodeurs</span>
        <span id="test-encoders" class="status-badge status-pending">EN ATTENTE</span>
      </div>
      <div class="test-item">
        <span class="test-name">3. Catalogue astronomique (/catalog)</span>
        <span id="test-catalog" class="status-badge status-pending">EN ATTENTE</span>
      </div>
      <div class="test-item">
        <span class="test-name">4. Lieux et coordonnées (/get_sites)</span>
        <span id="test-sites" class="status-badge status-pending">EN ATTENTE</span>
      </div>
      <div class="test-item">
        <span class="test-name">5. Pages de l'interface</span>
        <span id="test-pages" class="status-badge status-pending">EN ATTENTE</span>
      </div>
      <div class="test-item">
        <span class="test-name">6. Gestion des commandes invalides (/cmd)</span>
        <span id="test-cmd" class="status-badge status-pending">EN ATTENTE</span>
      </div>
    </div>

    <button class="btn" onclick="runAllTests()"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-play'></use></svg> Lancer la Suite de Tests</button>

    <p class="test-note">Ces vérifications sont en lecture seule : elles ne déplacent pas les axes, ne remettent pas les compteurs à zéro et ne changent pas la configuration.</p>
    <div id="logOutput">Cliquez sur "Lancer la Suite de Tests" pour démarrer...</div>
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
        <a class="nav-more-link" href="/calib_page"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-crosshair'></use></svg> Calibration</a>
        <a class="nav-more-link" href="/config"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-settings'></use></svg> Configuration</a>
        <a class="nav-more-link" href="/simu"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-gamepad-2'></use></svg> Simulation</a>
        <a class="nav-more-link" href="/test" aria-current="page"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-flask-conical'></use></svg> Tests système</a>
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

    function log(msg) {
      const logEl = document.getElementById('logOutput');
      logEl.innerText += '\n' + `[${new Date().toLocaleTimeString('fr-FR', { hour12: false })}] ` + msg;
      logEl.scrollTop = logEl.scrollHeight;
    }

    function setBadge(id, status, text) {
      const el = document.getElementById(id);
      el.className = `status-badge status-${status}`;
      el.innerText = text;
    }

    async function runTest(id, label, check) {
      setBadge(id, 'running', 'EN COURS');
      log(`${label}...`);
      try {
        const result = await check();
        setBadge(id, 'pass', 'SUCCÈS');
        log(`OK : ${result}`);
      } catch (error) {
        setBadge(id, 'fail', 'ÉCHEC');
        log(`ERREUR : ${error.message}`);
      }
    }

    async function getJson(url) {
      const response = await fetch(url);
      if (!response.ok) throw new Error(`${url} répond HTTP ${response.status}`);
      return response.json();
    }

    async function runAllTests() {
      document.getElementById('logOutput').innerText = "=== DÉMARRAGE DES TESTS AUTOMATISÉS ===";
      const tests = ['test-status', 'test-encoders', 'test-catalog', 'test-sites', 'test-pages', 'test-cmd'];
      tests.forEach(id => setBadge(id, 'pending', 'EN ATTENTE'));

      await runTest('test-status', 'Test 1 : lecture et validation de /status', async () => {
        const data = await getJson('/status');
        const numericFields = ['rawAZ', 'rawALT', 'ticksAZ', 'ticksALT', 'dirAZ', 'dirALT', 'lat', 'lon', 'profile'];
        if (!numericFields.every(key => Number.isFinite(data[key]))) throw new Error('Un ou plusieurs champs numériques sont absents ou invalides');
        if (typeof data.siteName !== 'string' || data.siteName.length === 0) throw new Error('Nom du lieu absent');
        if (data.lat < -90 || data.lat > 90 || data.lon < -180 || data.lon > 180) throw new Error('Latitude ou longitude hors limites');
        return `axes : AZ=${data.rawAZ} ticks, ALT=${data.rawALT} ticks ; lieu : ${data.siteName} — latitude ${data.lat.toFixed(4)}°, longitude ${data.lon.toFixed(4)}°`;
      });

      await runTest('test-encoders', 'Test 2 : vérification de la configuration des encodeurs', async () => {
        const data = await getJson('/status');
        if (data.ticksAZ <= 0 || data.ticksALT <= 0) throw new Error('La résolution des encodeurs doit être supérieure à zéro');
        if (![1, -1].includes(data.dirAZ) || ![1, -1].includes(data.dirALT)) throw new Error('Le sens des axes doit valoir 1 ou -1');
        if (data.revAZ !== (data.dirAZ === -1) || data.revALT !== (data.dirALT === -1)) throw new Error('Les indicateurs d’inversion ne correspondent pas au sens des axes');
        return `AZ=${data.ticksAZ} pas/tour, ALT=${data.ticksALT} pas/tour`;
      });

      await runTest('test-catalog', 'Test 3 : lecture et validation du catalogue /catalog', async () => {
        const catalog = await getJson('/catalog');
        if (!Array.isArray(catalog) || catalog.length === 0) throw new Error('Le catalogue est vide ou son format est invalide');
        const valid = catalog.every(object =>
          typeof object.name === 'string' &&
          typeof object.commonName === 'string' &&
          typeof object.type === 'string' &&
          Number.isFinite(object.ra) && object.ra >= 0 && object.ra < 24 &&
          Number.isFinite(object.dec) && object.dec >= -90 && object.dec <= 90 &&
          Number.isFinite(object.mag)
        );
        if (!valid) throw new Error('Au moins un objet contient des champs invalides');
        return `${catalog.length} objets valides`;
      });

      await runTest('test-sites', 'Test 4 : lecture et validation des lieux /get_sites', async () => {
        const sites = await getJson('/get_sites');
        const entries = Object.values(sites);
        if (entries.length === 0) throw new Error('Aucun lieu enregistré');
        const valid = entries.every(site =>
          typeof site.name === 'string' &&
          Number.isFinite(site.lat) && site.lat >= -90 && site.lat <= 90 &&
          Number.isFinite(site.lon) && site.lon >= -180 && site.lon <= 180
        );
        if (!valid) throw new Error('Au moins un lieu contient des coordonnées invalides');
        return `${entries.length} lieux valides`;
      });

      await runTest('test-pages', 'Test 5 : disponibilité des pages web', async () => {
        const routes = ['/', '/station', '/beta_sky', '/calib_page', '/config', '/simu', '/help', '/test', '/releasenotes'];
        for (const route of routes) {
          const response = await fetch(route);
          if (!response.ok) throw new Error(`${route} répond HTTP ${response.status}`);
          const html = await response.text();
          if (!html.includes('<html')) throw new Error(`${route} ne retourne pas une page HTML`);
        }
        return `${routes.length} pages accessibles`;
      });

      await runTest('test-cmd', 'Test 6 : rejet des commandes invalides', async () => {
        const missingAction = await fetch('/cmd');
        if (missingAction.status !== 400) throw new Error(`/cmd sans action devrait répondre HTTP 400 (reçu ${missingAction.status})`);
        const unknownAction = await fetch('/cmd?action=automated_test_invalid');
        if (unknownAction.status !== 400) throw new Error(`Une commande inconnue devrait répondre HTTP 400 (reçu ${unknownAction.status})`);
        return 'les requêtes invalides sont rejetées sans action sur le télescope';
      });

      const failures = tests.filter(id => document.getElementById(id).classList.contains('status-fail')).length;
      log(failures === 0 ? "=== SUITE TERMINÉE : TOUS LES TESTS SONT RÉUSSIS ===" : `=== SUITE TERMINÉE : ${failures} TEST(S) EN ÉCHEC ===`);
    }
  </script>
</body>
</html>
)rawliteral";

#endif