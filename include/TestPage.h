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
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #121212; color: #fff; padding: 15px; margin: 0; }
    .card { background: #1e1e1e; border-radius: 12px; padding: 20px; max-width: 600px; margin: 0 auto; box-shadow: 0 4px 12px rgba(0,0,0,0.5); }
    h2 { color: #ff9800; margin-top: 10px; text-align: center; }
    .btn { padding: 12px 20px; background: #4caf50; color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer; width: 100%; margin-top: 12px; transition: all 0.15s ease-in-out; }
    .btn:hover { filter: brightness(1.15); }
    .btn:active { transform: scale(0.97); filter: brightness(0.85); }
    .btn-sec { background: #2196f3; text-decoration: none; display: block; text-align: center; margin-bottom: 15px; width: 100%; box-sizing: border-box; }
    
    .test-item { background: #2a2a2a; padding: 10px 12px; border-radius: 6px; margin-bottom: 8px; display: flex; justify-content: space-between; align-items: center; font-size: 0.9em; }
    .test-name { font-weight: bold; }
    .status-badge { padding: 4px 8px; border-radius: 4px; font-size: 0.8em; font-weight: bold; min-width: 70px; text-align: center; }
    .status-pending { background: #424242; color: #ccc; }
    .status-running { background: #ff9800; color: #000; }
    .status-pass { background: #4caf50; color: #fff; }
    .status-fail { background: #f44336; color: #fff; }
    
    #logOutput { background: #111; border: 1px solid #333; color: #80cbc4; font-family: monospace; padding: 10px; border-radius: 6px; height: 180px; overflow-y: auto; font-size: 0.8em; margin-top: 15px; white-space: pre-wrap; }
  </style>
</head>
<body>
  <div class="card">
    <a href="/" class="btn btn-sec">🏠 Retour à la page principale</a>
    <h2>🧪 Tests Automatisés du Système</h2>

    <div id="testList">
      <div class="test-item">
        <span class="test-name">1. Connexion Serveur HTTP (/status)</span>
        <span id="test-status" class="status-badge status-pending">EN ATTENTE</span>
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

  <script>
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

      // Test 1: Route /status
      setBadge('test-status', 'running', 'EN COURS');
      log("Test 1: Requête GET /status...");
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
      
      // Test 3: Route /catalog
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

      // Test 4: Route /get_sites
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

      // Test 5: Route /cmd (Reset ALT)
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