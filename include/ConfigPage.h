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
    .ui-icon { display: inline-block; width: 1em; height: 1em; vertical-align: -0.15em; fill: none; stroke: currentColor; stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; }
    .nav-icon .ui-icon { vertical-align: middle; }
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
    <h2><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-settings'></use></svg> Paramètres ESP32</h2>

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

      <button type="button" id="deleteSiteBtn" class="btn btn-del" onclick="deleteCurrentSite()"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-trash-2'></use></svg> Supprimer ce lieu</button>

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
        <button type="button" class="btn btn-add" onclick="addNewSite()"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-plus'></use></svg> Ajouter ce lieu</button>
      </div>

      <a href="/test" class="btn btn-sec"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-flask-conical'></use></svg> Tests Automatisés</a>

      <button type="submit" class="btn"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-save'></use></svg> Enregistrer la configuration</button>
    </form>
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
        <a class="nav-more-link" href="/config" aria-current="page"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-settings'></use></svg> Configuration</a>
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