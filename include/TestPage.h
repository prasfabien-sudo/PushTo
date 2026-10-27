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
    
    .test-item { background: #2a2a2a; padding: 10px 12px; border-radius: 6px; margin-bottom: 8px; display: flex; justify-content: space-between; align-items: center; font-size: 0.9em; }
    .test-name { font-weight: bold; }
    .status-badge { padding: 4px 8px; border-radius: 4px; font-size: 0.8em; font-weight: bold; min-width: 70px; text-align: center; }
    .status-pending { background: #424242; color: #ccc; }
    .status-running { background: #ff9800; color: #000; }
    .status-pass { background: #4caf50; color: #fff; }
    .status-fail { background: #f44336; color: #fff; }
    
    #logOutput { background: #111; border: 1px solid #333; color: #80cbc4; font-family: monospace; padding: 10px; border-radius: 6px; height: 180px; overflow-y: auto; font-size: 0.8em; margin-top: 15px; white-space: pre-wrap; }

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
    <h2>🧪 Tests Automatisés du Système</h2>

    <div id="testList">
      <div class="test-item">
        <span class="test-name">1. Connexion Serveur HTTP (/status)</span>
        <span id="test-status" class="status-badge status-pending">EN ATTENTE</span>
      </div>
      <div class="test-item">
        <span class="test-name">2. Vérification Tension Batterie & Ticks</span>
        <span id="test-battery" class="status-badge status-pending">EN ATTENTE</span>
      </div>
      <div class="test-item">
        <span class="test-name">3. Chargement Catalogue DSOs (/catalog)</span>
        <span id="test-catalog" class="status-badge status-pending">EN ATTENTE</span>
      </div>
      <div class="test-item">
        <span class="test-name">4. Lecture Base de Lieux (/get_sites)</span>
        <span id="test-sites" class="status-badge status-pending">EN ATTENTE</span>
      </div>
      <div class="test-item">
        <span class="test-name">5. Test Commande Remise à Zéro (/cmd)</span>
        <span id="test-cmd" class="status-badge status-pending">EN ATTENTE</span>
      </div>
    </div>

    <button class="btn" onclick="runAllTests()">▶ Lancer la Suite de Tests</button>

    <div id="logOutput">Cliquez sur "Lancer la Suite de Tests" pour démarrer...</div>
  </div>

  <nav class="bottom-nav" aria-label="Navigation principale">
    <a class="nav-item" href="/beta_sky" title="Vue 3D du ciel"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-globe-2'></use></svg></span></a>
    <a class="nav-item" href="/" title="Mode Encodeurs"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-telescope'></use></svg></span></a>
    <a class="nav-item" href="/station" title="Mise en station"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-compass'></use></svg></span></a>
    <a class="nav-item" href="/calib_page" title="Calibration"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-ruler'></use></svg></span></a>
    <a class="nav-item" href="/config" title="Configuration"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-settings'></use></svg></span></a>
    <a class="nav-item" href="/help" title="Aide SkySafari"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-book-open'></use></svg></span></a>
    <a class="nav-item" href="/simu" title="Simulation"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-gamepad-2'></use></svg></span></a>
    <a class="nav-item" href="/test" aria-current="page" title="Tests système"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-flask-conical'></use></svg></span></a>
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

    function log(msg) {
      const logEl = document.getElementById('logOutput');
      logEl.innerText += '\n' + `[${new Date().toLocaleTimeString()}] ` + msg;
      logEl.scrollTop = logEl.scrollHeight;
    }

    function setBadge(id, status, text) {
      const el = document.getElementById(id);
      el.className = `status-badge status-${status}`;
      el.innerText = text;
    }

    async function runAllTests() {
      document.getElementById('logOutput').innerText = "=== DÉMARRAGE DES TESTS AUTOMATISÉS ===";
      
      const tests = ['test-status', 'test-battery', 'test-catalog', 'test-sites', 'test-cmd'];
      tests.forEach(id => setBadge(id, 'pending', 'EN ATTENTE'));

      let statusData = null;
      try {
        const res = await fetch('/status');
        if (res.ok) {
          statusData = await res.json();
          log(`Status reçu : Lat=${statusData.lat}, Lon=${statusData.lon}, Site="${statusData.siteName}"`);
          setBadge('test-status', 'pass', 'SUCCÈS');
        } else {
          throw new Error(`HTTP ${res.status}`);
        }
      } catch (e) {
        log(`Échec Test 1: ${e.message}`);
        setBadge('test-status', 'fail', 'ÉCHEC');
      }

      setBadge('test-battery', 'running', 'EN COURS');
      log("Test 2: Validation des paramètres système...");
      if (statusData && statusData.ticksAZ > 0 && statusData.ticksALT > 0 && parseFloat(statusData.vBat) >= 0) {
        log(`Batterie: ${statusData.vBat}V (${statusData.pBat}%), Ticks AZ:${statusData.ticksAZ}, ALT:${statusData.ticksALT}`);
        setBadge('test-battery', 'pass', 'SUCCÈS');
      } else {
        log("Échec Test 2: Données de batterie ou de ticks invalides");
        setBadge('test-battery', 'fail', 'ÉCHEC');
      }

      setBadge('test-catalog', 'running', 'EN COURS');
      log("Test 3: Requête GET /catalog...");
      try {
        const res = await fetch('/catalog');
        if (res.ok) {
          const catalog = await res.json();
          log(`Catalogue chargé : ${catalog.length} objets célestes trouvés`);
          if (catalog.length > 0) setBadge('test-catalog', 'pass', 'SUCCÈS');
          else setBadge('test-catalog', 'fail', 'VIDE');
        } else {
          throw new Error(`HTTP ${res.status}`);
        }
      } catch (e) {
        log(`Échec Test 3: ${e.message}`);
        setBadge('test-catalog', 'fail', 'ÉCHEC');
      }

      setBadge('test-sites', 'running', 'EN COURS');
      log("Test 4: Requête GET /get_sites...");
      try {
        const res = await fetch('/get_sites');
        if (res.ok) {
          const sites = await res.json();
          const count = Object.keys(sites).length;
          log(`Lieux enregistrés dans l'ESP32 : ${count} site(s) répertorié(s)`);
          setBadge('test-sites', 'pass', 'SUCCÈS');
        } else {
          throw new Error(`HTTP ${res.status}`);
        }
      } catch (e) {
        log(`Échec Test 4: ${e.message}`);
        setBadge('test-sites', 'fail', 'ÉCHEC');
      }

      setBadge('test-cmd', 'running', 'EN COURS');
      log("Test 5: Envoi commande test /cmd?action=set_zero_alt...");
      try {
        const res = await fetch('/cmd?action=set_zero_alt');
        if (res.ok) {
          const text = await res.text();
          log(`Réponse de l'ESP32 : ${text}`);
          setBadge('test-cmd', 'pass', 'SUCCÈS');
        } else {
          throw new Error(`HTTP ${res.status}`);
        }
      } catch (e) {
        log(`Échec Test 5: ${e.message}`);
        setBadge('test-cmd', 'fail', 'ÉCHEC');
      }

      log("=== FIN DE LA SUITE DE TESTS ===");
    }
  </script>
</body>
</html>
)rawliteral";

#endif