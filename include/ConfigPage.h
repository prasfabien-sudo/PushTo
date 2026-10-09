#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H

#include <pgmspace.h>

const char HTTP_CONFIG_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Configuration ESP32</title>
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #121212; color: #fff; padding: 15px; margin: 0; }
    .card { background: #1e1e1e; border-radius: 12px; padding: 20px; max-width: 500px; margin: 0 auto; box-shadow: 0 4px 12px rgba(0,0,0,0.5); }
    h2 { color: #ff9800; margin-top: 10px; text-align: center; }
    .btn { padding: 12px 20px; background: #4caf50; color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer; width: 100%; margin-top: 15px; transition: all 0.15s ease-in-out; }
    .btn:hover { filter: brightness(1.15); }
    .btn:active { transform: scale(0.97); filter: brightness(0.85); }
    .btn-sec { background: #2196f3; text-decoration: none; display: block; text-align: center; margin-bottom: 15px; width: 100%; box-sizing: border-box; }
    .btn-add { background: #00897b; margin-top: 8px; }
    .btn-del { background: #e53935; margin-top: 8px; padding: 8px 12px; font-size: 0.85em; }
    
    .section-title { font-weight: bold; color: #80cbc4; border-bottom: 1px solid #333; padding-bottom: 5px; margin-top: 20px; margin-bottom: 10px; }
    .form-group { display: flex; justify-content: space-between; align-items: center; margin-bottom: 12px; }
    label { font-size: 0.9em; color: #ccc; }
    input[type="number"], input[type="text"], select { background: #121212; color: #ffb74d; border: 1px solid #444; padding: 8px; border-radius: 6px; font-size: 0.95em; box-sizing: border-box; }
    input[type="number"] { width: 110px; text-align: right; }
    input[type="text"] { width: 100%; margin-bottom: 8px; }
    select { width: 100%; }

    .switch { position: relative; display: inline-block; width: 46px; height: 24px; }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #424242; transition: .3s; border-radius: 24px; }
    .slider:before { position: absolute; content: ""; height: 18px; width: 18px; left: 3px; bottom: 3px; background-color: white; transition: .3s; border-radius: 50%; }
    input:checked + .slider { background-color: #ff9800; }
    input:checked + .slider:before { transform: translateX(22px); }
  </style>
</head>
<body>
  <div class="card">
    <a href="/" class="btn btn-sec">🏠 Retour à la page principale</a>
    <h2>⚙️ Paramètres ESP32</h2>

    <form id="configForm" onsubmit="saveConfig(event)">
      <!-- Section Encodeurs -->
      <div class="section-title">Encodeurs & Résolution</div>
      
      <div class="form-group">
        <label for="ticksAZ">Pas par tour AZ :</label>
        <input type="number" id="ticksAZ" name="ticksAZ" required>
      </div>

      <div class="form-group">
        <label for="ticksALT">Pas par tour ALT :</label>
        <input type="number" id="ticksALT" name="ticksALT" required>
      </div>

      <!-- Sens de Rotation -->
      <div class="section-title">Sens de Rotation</div>

      <div class="form-group">
        <label>Inverser le sens AZ :</label>
        <label class="switch">
          <input type="checkbox" id="revAZ">
          <span class="slider"></span>
        </label>
      </div>

      <div class="form-group">
        <label>Inverser le sens ALT :</label>
        <label class="switch">
          <input type="checkbox" id="revALT">
          <span class="slider"></span>
        </label>
      </div>

      <!-- Gestion des Lieux d'Observation -->
      <div class="section-title">Lieu d'Observation Actif</div>

      <div style="margin-bottom: 8px;">
        <label for="siteSelect">Sélectionner le lieu :</label>
        <select id="siteSelect" onchange="onSiteSelectChange()"></select>
      </div>

      <button type="button" id="deleteSiteBtn" class="btn btn-del" onclick="deleteCurrentSite()">🗑️ Supprimer ce lieu</button>

      <div class="form-group" style="margin-top: 12px;">
        <label for="lat">Latitude (°N) :</label>
        <input type="number" step="0.0001" id="lat" name="lat" required>
      </div>

      <div class="form-group">
        <label for="lon">Longitude (°E) :</label>
        <input type="number" step="0.0001" id="lon" name="lon" required>
      </div>

      <!-- Ajouter un Nouveau Lieu -->
      <div class="section-title">Ajouter un Nouveau Lieu</div>
      <div>
        <input type="text" id="newSiteName" placeholder="Nom du lieu (ex: Observatoire)">
        <div style="display: flex; gap: 8px;">
          <input type="number" step="0.0001" id="newSiteLat" placeholder="Lat (°N)">
          <input type="number" step="0.0001" id="newSiteLon" placeholder="Lon (°E)">
        </div>
        <button type="button" class="btn btn-add" onclick="addNewSite()">➕ Ajouter ce lieu</button>
      </div>

      <a href="/test" class="btn btn-sec">🧪 Tests Automatisés</a>

      <button type="submit" class="btn">💾 Enregistrer la configuration</button>
    </form>
  </div>

  <script>
    let sitesData = {};

    function loadSitesFromESP(activeLat, activeLon) {
      fetch('/get_sites')
        .then(r => r.json())
        .then(sites => {
          sitesData = sites;
          populateSiteSelect(activeLat, activeLon);
        });
    }

    function populateSiteSelect(selectedLat, selectedLon) {
      const select = document.getElementById('siteSelect');
      select.innerHTML = '';
      const delBtn = document.getElementById('deleteSiteBtn');

      const keys = Object.keys(sitesData);
      if (keys.length === 0) {
        const opt = document.createElement('option');
        opt.value = "";
        opt.textContent = "-- Aucun lieu enregistré --";
        select.appendChild(opt);
        delBtn.style.display = 'none';
        return;
      }

      delBtn.style.display = 'block';

      keys.forEach(key => {
        const s = sitesData[key];
        const opt = document.createElement('option');
        opt.value = key;
        opt.textContent = `${s.name} (${s.lat}°N, ${s.lon}°E)`;
        if (Math.abs(s.lat - selectedLat) < 0.001 && Math.abs(s.lon - selectedLon) < 0.001) {
          opt.selected = true;
        }
        select.appendChild(opt);
      });

      onSiteSelectChange();
    }

    function onSiteSelectChange() {
      const key = document.getElementById('siteSelect').value;
      if (sitesData[key]) {
        document.getElementById('lat').value = sitesData[key].lat;
        document.getElementById('lon').value = sitesData[key].lon;
      }
    }

    function syncSitesToESP() {
      return fetch(`/cmd?action=save_sites&json=${encodeURIComponent(JSON.stringify(sitesData))}`);
    }

    function deleteCurrentSite() {
      const select = document.getElementById('siteSelect');
      const key = select.value;
      if (!key || !sitesData[key]) return;

      const siteName = sitesData[key].name;

      if (confirm(`Voulez-vous vraiment supprimer le lieu "${siteName}" ?`)) {
        delete sitesData[key];
        syncSitesToESP().then(() => {
          const keys = Object.keys(sitesData);
          if (keys.length > 0) {
            document.getElementById('lat').value = sitesData[keys[0]].lat;
            document.getElementById('lon').value = sitesData[keys[0]].lon;
            populateSiteSelect(sitesData[keys[0]].lat, sitesData[keys[0]].lon);
          } else {
            populateSiteSelect(0, 0);
          }
          alert(`Le lieu "${siteName}" a été supprimé.`);
        });
      }
    }

    function addNewSite() {
      const name = document.getElementById('newSiteName').value.trim();
      const lat = parseFloat(document.getElementById('newSiteLat').value);
      const lon = parseFloat(document.getElementById('newSiteLon').value);

      if (!name || isNaN(lat) || isNaN(lon)) {
        alert("Veuillez renseigner un nom, une latitude et une longitude valides.");
        return;
      }

      const key = name.toLowerCase().replace(/\s+/g, '_');
      sitesData[key] = { name: name, lat: lat, lon: lon };

      syncSitesToESP().then(() => {
        document.getElementById('newSiteName').value = '';
        document.getElementById('newSiteLat').value = '';
        document.getElementById('newSiteLon').value = '';

        document.getElementById('lat').value = lat;
        document.getElementById('lon').value = lon;

        populateSiteSelect(lat, lon);
        alert(`Lieu "${name}" ajouté !`);
      });
    }

    function loadConfig() {
      fetch('/status')
        .then(r => r.json())
        .then(d => {
          document.getElementById('ticksAZ').value = d.ticksAZ || 10000;
          document.getElementById('ticksALT').value = d.ticksALT || 10000;
          document.getElementById('revAZ').checked = d.revAZ || false;
          document.getElementById('revALT').checked = d.revALT || false;
          
          const lat = d.lat || 45.89;
          const lon = d.lon || 6.05;
          document.getElementById('lat').value = lat;
          document.getElementById('lon').value = lon;

          loadSitesFromESP(lat, lon);
        });
    }

    function saveConfig(e) {
      e.preventDefault();

      const ticksAZ = document.getElementById('ticksAZ').value;
      const ticksALT = document.getElementById('ticksALT').value;
      const revAZ = document.getElementById('revAZ').checked ? 1 : 0;
      const revALT = document.getElementById('revALT').checked ? 1 : 0;
      const lat = document.getElementById('lat').value;
      const lon = document.getElementById('lon').value;

      const selectKey = document.getElementById('siteSelect').value;
      let siteName = "Lieu personnalisé";
      if (selectKey && sitesData[selectKey]) {
        siteName = sitesData[selectKey].name;
      }

      const url = `/cmd?action=set_config&ticksAZ=${ticksAZ}&ticksALT=${ticksALT}&revAZ=${revAZ}&revALT=${revALT}&lat=${lat}&lon=${lon}&siteName=${encodeURIComponent(siteName)}`;

      fetch(url)
        .then(r => r.json())
        .then(d => {
          if (d.status === "OK") {
            alert("Paramètres enregistrés avec succès dans l'ESP32 !");
          } else {
            alert("Erreur lors de l'enregistrement.");
          }
        })
        .catch(err => alert("Erreur de communication."));
    }

    loadConfig();
  </script>
</body>
</html>
)rawliteral";

#endif