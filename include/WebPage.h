// Version Component: WebPage.h v1.2.3
// Version Global System: v1.2.3
#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <pgmspace.h>

const char HTTP_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Dobson Push-To</title>
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #121212; color: #e0e0e0; margin: 0; padding: 15px; }
    .card { background: #1e1e1e; border-radius: 12px; padding: 15px; max-width: 650px; margin: 0 auto; box-shadow: 0 4px 12px rgba(0,0,0,0.5); position: relative; }
    
    .header-row { display: flex; justify-content: space-between; align-items: flex-start; margin-bottom: 15px; }
    h1 { color: #ff5252; margin: 0; font-size: 1.5em; text-align: left; }
    .version-tag { font-size: 0.75em; color: #ffb74d; font-weight: bold; background: #2a2a2a; padding: 3px 8px; border-radius: 4px; border: 1px solid #333; text-decoration: none; display: inline-block; }
    .version-tag:hover { background: #37474f; border-color: #ffb74d; }
    
    .grid { display: block; width: 100%; margin-bottom: 15px; }
    .metric { background: #2a2a2a; border-radius: 8px; padding: 10px; text-align: center; display: flex; flex-direction: column; justify-content: center; width: 100%; box-sizing: border-box; max-width: none; }
    .metric-title { font-size: 0.75em; color: #aaa; text-transform: uppercase; }
    .metric-value { font-size: 1.1em; font-weight: bold; color: #4caf50; margin-top: 4px; }
    .metric-deg { font-size: 0.9em; color: #ffb74d; margin-top: 2px; font-weight: normal; }
    
    /* Target & Guidance Box */
    .target-box { background: #263238; border: 1px solid #00897b; border-radius: 8px; padding: 12px; margin-bottom: 15px; text-align: center; }
    .target-title { font-size: 0.9em; color: #80cbc4; text-transform: uppercase; margin-bottom: 5px; }
    .target-name { font-size: 1.4em; font-weight: bold; color: #fff; }
    
    /* Détails Stellarium-like */
    .target-details { display: grid; grid-template-columns: 1fr 1fr; gap: 6px 12px; margin: 10px 0 5px 0; background: #1c272c; padding: 8px; border-radius: 6px; font-size: 0.8em; text-align: left; }
    .target-details div { color: #b0bec5; }
    .target-details span { color: #fff; font-weight: bold; }

    .delta-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-top: 10px; }
    .delta-val { font-size: 1.4em; font-weight: bold; color: #4caf50; }
    .delta-val-neg { color: #ff5252; font-weight: bold; }

    /* Canvas de guidage double (Compas AZ + Barre ALT) */
    .guidance-container { display: flex; justify-content: space-around; align-items: center; margin: 10px 0 5px 0; background: #1a2327; padding: 10px; border-radius: 8px; }
    .canvas-block { display: flex; flex-direction: column; align-items: center; }
    .canvas-label { font-size: 0.75em; color: #ffb74d; font-weight: bold; margin-bottom: 4px; text-transform: uppercase; }

    .btn { display: block; width: 100%; padding: 12px; margin: 8px 0; background: #2196f3; color: white; border: none; border-radius: 8px; font-size: 0.95em; font-weight: bold; cursor: pointer; text-align: center; text-decoration: none; box-sizing: border-box; transition: all 0.15s ease-in-out; }
    .btn:hover { filter: brightness(1.15); }
    .btn:active { transform: scale(0.97); filter: brightness(0.85); }
    .btn:disabled { background: #444; color: #888; cursor: not-allowed; filter: none; transform: none; }
    .btn-sm { padding: 6px 10px; font-size: 0.8em; margin: 0; display: inline-block; width: auto; }
    .btn-align { background: #4caf50; margin-top: 12px; }

    /* Menu de Navigation */
    .nav-menu { width: 100%; padding: 12px; background: #2a2a2a; color: #ffb74d; border: 1px solid #00897b; border-radius: 8px; font-size: 1em; font-weight: bold; margin: 8px 0; box-sizing: border-box; cursor: pointer; }
    .nav-menu option { background: #1e1e1e; color: #fff; }

    .cat-section { margin-top: 15px; border-top: 1px solid #333; padding-top: 15px; }
    .controls-row { display: flex; flex-wrap: wrap; gap: 8px; margin-bottom: 10px; }
    .search-bar { flex: 1; min-width: 130px; padding: 8px; border-radius: 6px; border: 1px solid #444; background: #2a2a2a; color: #fff; }
    .filter-select { padding: 8px; border-radius: 6px; border: 1px solid #444; background: #2a2a2a; color: #fff; font-size: 0.85em; cursor: pointer; }
    .filter-btn { background: #37474f; color: #fff; border: 1px solid #546e7a; border-radius: 6px; padding: 6px 10px; cursor: pointer; font-size: 0.85em; }
    .filter-btn.active { background: #00897b; border-color: #80cbc4; }

    /* Légende cliquable */
    .legend-row { font-size: 0.75em; color: #aaa; margin-bottom: 8px; display: flex; justify-content: space-between; background: #222; padding: 6px 4px; border-radius: 6px; flex-wrap: wrap; gap: 4px; }
    .legend-item { cursor: pointer; padding: 3px 5px; border-radius: 4px; user-select: none; transition: background 0.2s; }
    .legend-item:hover { background: #333; color: #fff; }
    .legend-item.active { background: #00897b; color: #fff; font-weight: bold; }

    .geo-row { display: flex; align-items: center; justify-content: space-between; background: #2a2a2a; padding: 8px 12px; border-radius: 6px; margin-bottom: 12px; font-size: 0.85em; }

    .table-container { max-height: 350px; overflow-y: auto; border-radius: 6px; border: 1px solid #333; }
    table { width: 100%; border-collapse: collapse; font-size: 0.85em; text-align: left; }
    th { background: #2a2a2a; color: #ff9800; padding: 8px; position: sticky; top: 0; }
    td { padding: 8px; border-bottom: 1px solid #282828; vertical-align: middle; }
    tr:nth-child(even) { background: #181818; }
    tr:hover { background: #2c2c2c; }
    
    .obj-title { font-weight: bold; color: #fff; display: block; }
    .obj-sub { font-size: 0.8em; color: #aaa; }
    .picto-type { font-size: 1.2em; text-align: center; cursor: help; }
    .picto-vis { font-size: 1.2em; text-align: center; cursor: help; }
  </style>
</head>
<body>
  <div class="card">
    <div class="header-row">
      <h1>Dobson Push-To</h1>
      <a href="/releasenotes" class="version-tag" title="Voir l'historique des versions">v1.2.3</a>
    </div>
    
    <!-- Position Télescope Pleine Largeur -->
    <div class="grid">
      <div class="metric">
        <div class="metric-title">Position Télescope</div>
        <div class="metric-value" id="pos">AZ: 0 | ALT: 0</div>
        <div class="metric-deg" id="posDeg">0.0° | 0.0°</div>
      </div>
    </div>

    <!-- Boîte de Cible & Deltas de Guidage -->
    <div class="target-box">
      <div class="target-title">Cible Actuelle</div>
      <div class="target-name" id="targetName">Aucune cible</div>
      
      <!-- Bloc d'informations style Stellarium -->
      <div class="target-details" id="targetDetails" style="display: none;">
        <div>Mag: <span id="targetMag">--</span></div>
        <div>Vis: <span id="targetVis">--</span></div>
        <div>Ra/Dec: <span id="targetRaDec">--</span></div>
        <div>Az/Alt: <span id="targetAzAlt">--</span></div>
      </div>

      <!-- Module de Guidage Visuel (Compas AZ + Barre ALT) -->
      <div class="guidance-container">
        <div class="canvas-block">
          <div class="canvas-label">Azimut (Compas)</div>
          <canvas id="azCanvas" width="60" height="60"></canvas>
        </div>
        <div class="canvas-block">
          <div class="canvas-label">Altitude (Niveau)</div>
          <canvas id="altCanvas" width="40" height="140"></canvas>
        </div>
      </div>

      <div class="delta-grid">
        <div>
          <div class="metric-title">Écart Azimut</div>
          <div class="delta-val" id="deltaAZ">+0.0°</div>
        </div>
        <div>
          <div class="metric-title">Écart Altitude</div>
          <div class="delta-val" id="deltaALT">+0.0°</div>
        </div>
      </div>
      <!-- Bouton d'alignement centralisé -->
      <button id="btnAlignActive" class="btn btn-align" onclick="alignOnCurrentTarget()" disabled>🎯 Aligner sur cette cible</button>
    </div>

    <!-- Menu déroulant de Navigation -->
    <select class="nav-menu" onchange="navigateTo(this.value)">
      <option value="" disabled selected>📑 Navigation / Pages...</option>
      <option value="/station">🎯 Mise en Station / Alignement</option>
      <option value="/config">⚙️ Paramètres ESP32</option>
      <option value="/calib_page">⚙️ Étalonnage des Encodeurs</option>
      <option value="/releasenotes">📋 Release Notes / Historique</option>
      <option value="/help">📖 Aide Connexion SkySafari</option>
      <option value="/test">🧪 Tests Automatisés</option>
      <option value="/simu">🎮 Mode Simulation</option>
    </select>

    <!-- Catalogue d'Objets -->
    <div class="cat-section">
      <h2 style="color:#ff9800; margin:0 0 10px 0; font-size:1.1em;">Catalogue d'Objets</h2>
      
      <div class="geo-row">
        <span>📍 Lieu d'observation :</span>
        <span id="displaySiteName" style="color: #ffb74d; font-weight: bold;">Chargement...</span>
      </div>

      <div class="controls-row">
        <input type="text" id="searchInput" class="search-bar" placeholder="Rechercher..." onkeyup="applyFilters()">
        
        <select id="magFilterSelect" class="filter-select" onchange="applyFilters()">
          <option value="99">Mag. Max : Toutes</option>
          <option value="6">Mag. ≤ 6 (Oeil nu)</option>
          <option value="8">Mag. ≤ 8 (Jumelles)</option>
          <option value="10">Mag. ≤ 10 (Petit télescope)</option>
          <option value="12">Mag. ≤ 12 (Grand télescope)</option>
        </select>

        <button id="timeFilterBtn" class="filter-btn" onclick="toggleTimeFilter()">🕒 Visibles</button>
      </div>

      <!-- Légende des Pictos Cliquables -->
      <div class="legend-row">
        <span class="legend-item active" onclick="filterByType('all', this)">✨ Tous</span>
        <span class="legend-item" onclick="filterByType('planete', this)">🪐 Planète</span>
        <span class="legend-item" onclick="filterByType('galaxie', this)">🌌 Galaxie</span>
        <span class="legend-item" onclick="filterByType('neb_planetaire', this)">🌀 Néb. Planétaire</span>
        <span class="legend-item" onclick="filterByType('nebuleuse', this)">☁️ Nébuleuse</span>
        <span class="legend-item" onclick="filterByType('amas', this)">🌟 Amas</span>
        <span class="legend-item" onclick="filterByType('etoile', this)">🌠 Étoile</span>
      </div>

      <div class="table-container">
        <table>
          <thead>
            <tr>
              <th>Objet / Nom</th>
              <th style="text-align:center;">Type</th>
              <th>Mag.</th>
              <th style="text-align:center;">Vis.</th>
              <th>Action</th>
            </tr>
          </thead>
          <tbody id="catalogBody">
            <tr><td colspan="5" style="text-align:center;">Chargement du catalogue...</td></tr>
          </tbody>
        </table>
      </div>
    </div>
  </div>

  <script>
    let fullCatalog = [];
    let currentAZ = 0, currentALT = 0;
    let ticksAZ = 10000, ticksALT = 10000;
    let dirAZ = 1, dirALT = 1;
    let selectedTarget = null;
    let filterVisibleOnly = false;
    let selectedTypeFilter = 'all';

    let userLat = 45.89;
    let userLon = 6.05;
    let isGpsActive = false;

    let alignRefAZ = null;
    let alignRefALT = null;

    function navigateTo(url) {
      if (url) window.location.href = url;
    }

    function initGeolocation() {
      if ("geolocation" in navigator) {
        navigator.geolocation.getCurrentPosition(
          (pos) => {
            userLat = pos.coords.latitude;
            userLon = pos.coords.longitude;
            isGpsActive = true;
            document.getElementById('displaySiteName').innerText = `GPS Mobile (${userLat.toFixed(2)}°N, ${userLon.toFixed(2)}°E)`;
            applyFilters();
          },
          (err) => {
            console.log("GPS non disponible ou refusé, utilisation des valeurs ESP32.");
          },
          { enableHighAccuracy: true, timeout: 5000, maximumAge: 0 }
        );
      }
    }

    function getPrecessedCoords(raJ2000, decJ2000) {
      const now = new Date();
      const currentYear = now.getUTCFullYear() + (now.getUTCMonth() / 12.0) + (now.getUTCDate() / 365.25);
      const T = (currentYear - 2000.0) / 100.0;
      
      const raRad = raJ2000 * 15.0 * (Math.PI / 180.0);
      const decRad = decJ2000 * (Math.PI / 180.0);

      const dRA_sec = (307.5 + 133.6 * Math.sin(raRad) * Math.tan(decRad)) * T;
      const dDEC_arcsec = (2004.3 * Math.cos(raRad)) * T;

      let raNow = (raJ2000 + (dRA_sec / 3600.0) + 24.0) % 24.0;
      let decNow = decJ2000 + (dDEC_arcsec / 3600.0);

      return { ra: raNow, dec: decNow };
    }

    function formatRA(raHours) {
      let h = Math.floor(raHours);
      let totalSec = (raHours - h) * 3600;
      let m = Math.floor(totalSec / 60);
      let s = Math.floor(totalSec % 60);
      return `${h < 10 ? '0' : ''}${h}h ${m < 10 ? '0' : ''}${m}m ${s < 10 ? '0' : ''}${s}s`;
    }

    function formatDEC(decDeg) {
      let sign = decDeg >= 0 ? '+' : '-';
      let absD = Math.abs(decDeg);
      let d = Math.floor(absD);
      let totalSec = (absD - d) * 3600;
      let m = Math.floor(totalSec / 60);
      let s = Math.floor(totalSec % 60);
      return `${sign}${d < 10 ? '0' : ''}${d}° ${m < 10 ? '0' : ''}${m}' ${s < 10 ? '0' : ''}${s}"`;
    }

    function formatDegMin(deg) {
      let absD = Math.abs((deg % 360 + 360) % 360);
      let d = Math.floor(absD);
      let m = Math.floor((absD - d) * 60);
      return `${d < 10 ? '0' : ''}${d}° ${m < 10 ? '0' : ''}${m}'`;
    }

    function updateStatus() {
      fetch('/status').then(r => r.json()).then(d => {
        currentAZ = d.rawAZ;
        currentALT = d.rawALT;
        ticksAZ = d.ticksAZ || 10000;
        ticksALT = d.ticksALT || 10000;
        dirAZ = d.dirAZ || 1;
        dirALT = d.dirALT || 1;

        if (!isGpsActive && d.lat !== undefined && d.lon !== undefined) {
          userLat = d.lat;
          userLon = d.lon;
          const name = d.siteName || "Lieu d'observation";
          document.getElementById('displaySiteName').innerText = `${name} (${d.lat.toFixed(2)}°N, ${d.lon.toFixed(2)}°E)`;
        }

        let degAZ = (((currentAZ * dirAZ) / ticksAZ) * 360.0) % 360.0;
        let degALT = (((currentALT * dirALT) / ticksALT) * 360.0) % 360.0;
        if (degAZ < 0) degAZ += 360.0;
        if (degALT < 0) degALT += 360.0;

        document.getElementById('pos').innerText = `AZ: ${d.rawAZ} | ALT: ${d.rawALT}`;
        document.getElementById('posDeg').innerText = `${degAZ.toFixed(1)}° | ${degALT.toFixed(1)}°`;

        updateGuidance(degAZ, degALT);
        applyFilters();
      });
    }

    function selectTarget(displayName, ra, dec, mag) {
      const p = getPrecessedCoords(ra, dec);
      selectedTarget = { name: displayName, ra: ra, dec: dec, raNow: p.ra, decNow: p.dec, mag: mag };
      alignRefAZ = null;
      alignRefALT = null;

      document.getElementById('targetName').innerText = displayName;
      
      let targetAzDeg = getAzimuth(p.ra, p.dec);
      let targetAltDeg = getAltitude(p.ra, p.dec);
      
      document.getElementById('targetMag').innerText = mag !== undefined ? mag.toFixed(1) : "--";
      document.getElementById('targetVis').innerText = targetAltDeg >= 10 ? "Oui (" + targetAltDeg.toFixed(0) + "°)" : "Non (" + targetAltDeg.toFixed(0) + "°)";
      document.getElementById('targetRaDec').innerText = `${formatRA(p.ra)} / ${formatDEC(p.dec)}`;
      document.getElementById('targetAzAlt').innerText = `${formatDegMin(targetAzDeg)} / ${formatDegMin(targetAltDeg)}`;
      document.getElementById('targetDetails').style.display = 'grid';

      document.getElementById('btnAlignActive').disabled = false;

      fetch(`/cmd?action=select_target&name=${encodeURIComponent(displayName)}&ra=${p.ra}&dec=${p.dec}`);
    }

    function alignOnCurrentTarget() {
      if (!selectedTarget) return;

      let targetAzDeg = getAzimuth(selectedTarget.raNow, selectedTarget.decNow);
      let targetAltDeg = getAltitude(selectedTarget.raNow, selectedTarget.decNow);

      alignRefAZ = targetAzDeg;
      alignRefALT = targetAltDeg;

      fetch(`/cmd?action=align_star&az=${targetAzDeg.toFixed(2)}&alt=${targetAltDeg.toFixed(2)}`)
        .then(r => r.text())
        .then(() => {
          document.getElementById('posDeg').innerText = `${targetAzDeg.toFixed(1)}° | ${targetAltDeg.toFixed(1)}°`;
          document.getElementById('deltaAZ').innerText = "+0.0°";
          document.getElementById('deltaALT').innerText = "+0.0°";
          document.getElementById('deltaAZ').className = "delta-val";
          document.getElementById('deltaALT').className = "delta-val";
          
          drawGuidanceGauges(0, 0);
        });
    }

    function drawGuidanceGauges(dAZ, dALT) {
      // 1. COMPAS AZIMUT (Cercle indicateur de direction)
      const azCvs = document.getElementById('azCanvas');
      if (azCvs) {
        const ctx = azCvs.getContext('2d');
        const w = azCvs.width, h = azCvs.height;
        const cx = w / 2, cy = h / 2, r = Math.min(w, h) / 2 - 4;

        ctx.clearRect(0, 0, w, h);

        // Fond du compas
        ctx.fillStyle = "#162024";
        ctx.beginPath();
        ctx.arc(cx, cy, r, 0, 2 * Math.PI);
        ctx.fill();
        ctx.strokeStyle = "#ffb74d";
        ctx.lineWidth = 1.5;
        ctx.stroke();

        // Repère Nord (N en haut)
        ctx.fillStyle = "#ffb74d";
        ctx.font = "9px sans-serif";
        ctx.textAlign = "center";
        ctx.textBaseline = "top";
        ctx.fillText("N", cx, cy - r + 1);

        if (selectedTarget) {
          const ok = Math.abs(dAZ) < 0.5;
          // Angle d'écart converti en radians (0 = tout droit, positif = tourner dans un sens)
          // On mappe l'écart dAZ sur un cercle complet (ex: -180 à +180 deg -> -PI à +PI)
          let angleRad = (dAZ * Math.PI / 180.0) - (Math.PI / 2); // -PI/2 pour orienter le 0 vers le haut (Nord)

          let needleX = cx + (r - 8) * Math.cos(angleRad);
          let needleY = cy + (r - 8) * Math.sin(angleRad);

          // Aiguille / Point indicateur de direction
          ctx.strokeStyle = ok ? "#4caf50" : "#ff5252";
          ctx.lineWidth = 2;
          ctx.beginPath();
          ctx.moveTo(cx, cy);
          ctx.lineTo(needleX, needleY);
          ctx.stroke();

          ctx.fillStyle = ok ? "#4caf50" : "#ff5252";
          ctx.beginPath();
          ctx.arc(needleX, needleY, 4, 0, 2 * Math.PI);
          ctx.fill();
        } else {
          // Point central neutre si pas de cible
          ctx.fillStyle = "#546e7a";
          ctx.beginPath();
          ctx.arc(cx, cy, 3, 0, 2 * Math.PI);
          ctx.fill();
        }
      }

      // 2. NIVEAU ALTITUDE (Barre horizontale mobile)
      const altCvs = document.getElementById('altCanvas');
      if (altCvs) {
        const ctx = altCvs.getContext('2d');
        const w = altCvs.width, h = altCvs.height;
        ctx.clearRect(0, 0, w, h);

        ctx.fillStyle = "#263238";
        ctx.fillRect(w / 2 - 4, 0, 8, h);

        ctx.strokeStyle = "#ffb74d";
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(4, h / 2); ctx.lineTo(w - 4, h / 2);
        ctx.stroke();

        if (selectedTarget) {
          const maxDeg = 15.0;
          let posY = (h / 2) - (dALT / maxDeg) * (h / 2 - 10);
          posY = Math.max(8, Math.min(h - 8, posY));

          const ok = Math.abs(dALT) < 0.5;
          ctx.strokeStyle = ok ? "#4caf50" : "#ff5252";
          ctx.lineWidth = 4;
          ctx.beginPath();
          ctx.moveTo(4, posY);
          ctx.lineTo(w - 4, posY);
          ctx.stroke();
        }
      }
    }

    function getTypePicto(typeStr) {
      let t = (typeStr || "").toLowerCase().trim();
      
      if (t.includes("planétaire") || t.includes("planetaire") || t.includes("pn")) {
        return { picto: "🌀", title: "Nébuleuse Planétaire (" + typeStr + ")" };
      }
      
      if (t === "planète" || t === "planete" || (t.startsWith("pla") && !t.includes("planét") && !t.includes("planet"))) {
        return { picto: "🪐", title: "Planète" };
      }
      
      if (t.includes("gal") || t.includes("gx")) {
        return { picto: "🌌", title: "Galaxie (" + typeStr + ")" };
      }
      
      if (t.includes("neb") || t.includes("dn")) {
        return { picto: "☁️", title: "Nébuleuse (" + typeStr + ")" };
      }
      
      if (t.includes("amas") || t.includes("oc") || t.includes("gc")) {
        return { picto: "🌟", title: "Amas d'étoiles (" + typeStr + ")" };
      }
      
      return { picto: "🌠", title: "Étoile / Autre (" + typeStr + ")" };
    }

    function filterByType(typeKey, el) {
      selectedTypeFilter = typeKey;
      document.querySelectorAll('.legend-item').forEach(i => i.classList.remove('active'));
      if (el) el.classList.add('active');
      applyFilters();
    }

    function updateGuidance(currentAZDeg, currentALTDeg) {
      if (!selectedTarget) {
        drawGuidanceGauges(0, 0);
        return;
      }

      let targetAzDeg, targetAltDeg;

      if (alignRefAZ !== null && alignRefALT !== null) {
        targetAzDeg = alignRefAZ;
        targetAltDeg = alignRefALT;
      } else {
        targetAzDeg = getAzimuth(selectedTarget.raNow, selectedTarget.decNow);
        targetAltDeg = getAltitude(selectedTarget.raNow, selectedTarget.decNow);
      }

      document.getElementById('targetAzAlt').innerText = `${formatDegMin(targetAzDeg)} / ${formatDegMin(targetAltDeg)}`;

      let dAZ = targetAzDeg - currentAZDeg;
      let dALT = targetAltDeg - currentALTDeg;

      while (dAZ > 180) dAZ -= 360;
      while (dAZ < -180) dAZ += 360;

      if (Math.abs(dAZ) < 0.1) dAZ = 0.0;
      if (Math.abs(dALT) < 0.1) dALT = 0.0;

      const elAZ = document.getElementById('deltaAZ');
      const elALT = document.getElementById('deltaALT');

      elAZ.innerText = (dAZ >= 0 ? "+" : "") + dAZ.toFixed(1) + "°";
      elALT.innerText = (dALT >= 0 ? "+" : "") + dALT.toFixed(1) + "°";

      elAZ.className = Math.abs(dAZ) < 0.5 ? "delta-val" : (dAZ < 0 ? "delta-val delta-val-neg" : "delta-val");
      elALT.className = Math.abs(dALT) < 0.5 ? "delta-val" : (dALT < 0 ? "delta-val delta-val-neg" : "delta-val");

      drawGuidanceGauges(dAZ, dALT);
    }

    function getPlanetsCoordinates() {
      const d = (new Date().getTime() / 86400000.0) - 10957.5;
      
      const planets = [
        { name: "Vénus", type: "Planète", mag: -4.0, N: 76.68, i: 3.39, w: 54.88, a: 0.7233, e: 0.0067, M0: 50.11, w0: 1.6021 },
        { name: "Mars", type: "Planète", mag: -1.5, N: 49.56, i: 1.85, w: 286.50, a: 1.5237, e: 0.0934, M0: 19.37, w0: 0.5240 },
        { name: "Jupiter", type: "Planète", mag: -2.5, N: 100.46, i: 1.30, w: 273.87, a: 5.2026, e: 0.0485, M0: 20.02, w0: 0.0831 },
        { name: "Saturne", type: "Planète", mag: 0.2, N: 113.67, i: 2.49, w: 339.39, a: 9.5549, e: 0.0555, M0: 317.02, w0: 0.0335 }
      ];

      return planets.map(p => {
        let M = (p.M0 + p.w0 * d) % 360;
        let radM = M * Math.PI / 180;
        let E = M + (180 / Math.PI) * p.e * Math.sin(radM);
        let radE = E * Math.PI / 180;

        let x = p.a * (Math.cos(radE) - p.e);
        let y = p.a * Math.sqrt(1 - p.e * p.e) * Math.sin(radE);

        let v = Math.atan2(y, x) * 180 / Math.PI;
        let lonecl = (v + p.w) % 360;
        let radLonecl = lonecl * Math.PI / 180;

        let ra = (lonecl / 15.0) % 24;
        if (ra < 0) ra += 24;
        let dec = 23.44 * Math.sin(radLonecl);

        return { name: p.name, type: p.type, mag: p.mag, ra: ra, dec: dec };
      });
    }

    function loadCatalog() {
      fetch('/catalog')
        .then(r => r.json())
        .then(data => {
          let planetData = getPlanetsCoordinates();
          fullCatalog = planetData.concat(data);
          applyFilters();
        });
    }

    function toggleTimeFilter() {
      filterVisibleOnly = !filterVisibleOnly;
      const btn = document.getElementById('timeFilterBtn');
      if (filterVisibleOnly) {
        btn.classList.add('active');
      } else {
        btn.classList.remove('active');
      }
      applyFilters();
    }

    function getLST() {
      const now = new Date();
      
      const Y = now.getUTCFullYear();
      const M = now.getUTCMonth() + 1;
      const D = now.getUTCDate();
      const H = now.getUTCHours() + now.getUTCMinutes() / 60.0 + now.getUTCSeconds() / 3600.0;

      let y = Y, m = M;
      if (m <= 2) { y -= 1; m += 12; }
      const A = Math.floor(y / 100);
      const B = 2 - A + Math.floor(A / 4);
      const JD0 = Math.floor(365.25 * (y + 4716)) + Math.floor(30.6001 * (m + 1)) + D + B - 1524.5;
      
      const S = (JD0 - 2451545.0) / 36525.0;
      let gmst0 = 6.697374558 + (2400.051336 * S) + (0.000025862 * S * S);
      let gmst = gmst0 + (H * 1.00273790935);
      let lst = gmst + (userLon / 15.0);

      return (lst % 24.0 + 24.0) % 24.0;
    }

    function getAltitude(ra, dec) {
      const lst = getLST();
      let ha = (lst - ra) * 15.0 * (Math.PI / 180.0);
      let latRad = userLat * (Math.PI / 180.0);
      let decRad = dec * (Math.PI / 180.0);

      let sinAlt = Math.sin(decRad) * Math.sin(latRad) + Math.cos(decRad) * Math.cos(latRad) * Math.cos(ha);
      let trueAltDeg = Math.asin(sinAlt) * (180.0 / Math.PI);

      if (trueAltDeg > -1.0) {
        let refrArcmin = 1.02 / Math.tan((trueAltDeg + 10.3 / (trueAltDeg + 5.11)) * (Math.PI / 180.0));
        return trueAltDeg + (refrArcmin / 60.0);
      }
      return trueAltDeg;
    }

    function getAzimuth(ra, dec) {
      const lst = getLST();
      let ha = (lst - ra) * 15.0 * (Math.PI / 180.0);
      let latRad = userLat * (Math.PI / 180.0);
      let decRad = dec * (Math.PI / 180.0);

      let sinAlt = Math.sin(decRad) * Math.sin(latRad) + Math.cos(decRad) * Math.cos(latRad) * Math.cos(ha);
      let alt = Math.asin(sinAlt);

      let cosAZ = (Math.sin(decRad) - Math.sin(latRad) * sinAlt) / (Math.cos(latRad) * Math.cos(alt));
      let az = Math.acos(Math.max(-1, Math.min(1, cosAZ))) * (180.0 / Math.PI);

      if (Math.sin(ha) > 0) az = 360.0 - az;
      return az;
    }

    function getTimeUntilVisible(ra, dec) {
      const alt = getAltitude(ra, dec);
      if (alt >= 10) return "Visible";

      const lst = getLST();
      let haCurrent = (lst - ra + 24) % 24;
      let hoursToWait = (18.0 - haCurrent + 24) % 24;

      if (hoursToWait > 12) return "Non visible cette nuit";

      let h = Math.floor(hoursToWait);
      let m = Math.round((hoursToWait - h) * 60);
      return `Dans ${h}h${m < 10 ? '0' : ''}${m}`;
    }

    function applyFilters() {
      const q = document.getElementById('searchInput').value.toLowerCase();
      const maxMag = parseFloat(document.getElementById('magFilterSelect').value);

      let currDegAZ = (((currentAZ * dirAZ) / ticksAZ) * 360.0) % 360.0;
      let currDegALT = (((currentALT * dirALT) / ticksALT) * 360.0) % 360.0;
      if (currDegAZ < 0) currDegAZ += 360.0;
      if (currDegALT < 0) currDegALT += 360.0;

      let filtered = fullCatalog.map(o => {
        let p = getPrecessedCoords(o.ra, o.dec);
        let alt = getAltitude(p.ra, p.dec);
        let az = getAzimuth(p.ra, p.dec);
        let timeStr = getTimeUntilVisible(p.ra, p.dec);

        let dAZ = az - currDegAZ;
        while (dAZ > 180) dAZ -= 360;
        while (dAZ < -180) dAZ += 360;
        let dALT = alt - currDegALT;
        let dist = Math.sqrt(dAZ * dAZ + dALT * dALT);

        return { ...o, raNow: p.ra, decNow: p.dec, altitude: alt, timeStr: timeStr, dist: dist };
      });

      filtered = filtered.filter(o => 
        (o.name.toLowerCase().includes(q) || (o.commonName && o.commonName.toLowerCase().includes(q)) || o.type.toLowerCase().includes(q))
      );

      if (!isNaN(maxMag)) {
        filtered = filtered.filter(o => o.mag <= maxMag);
      }

      if (selectedTypeFilter !== 'all') {
        filtered = filtered.filter(o => {
          let t = (o.type || "").toLowerCase();
          if (selectedTypeFilter === 'neb_planetaire') return t.includes('planétaire') || t.includes('planetaire') || t.includes('pn');
          if (selectedTypeFilter === 'planete') return (t === 'planète' || t === 'planete') && !t.includes('neb');
          if (selectedTypeFilter === 'galaxie') return t.includes('gal') || t.includes('gx');
          if (selectedTypeFilter === 'nebuleuse') return (t.includes('neb') || t.includes('dn')) && !t.includes('planét') && !t.includes('planet');
          if (selectedTypeFilter === 'amas') return t.includes('amas') || t.includes('oc') || t.includes('gc');
          if (selectedTypeFilter === 'etoile') return !t.includes('gal') && !t.includes('neb') && !t.includes('amas') && !t.includes('pla');
          return true;
        });
      }

      if (filterVisibleOnly) {
        filtered = filtered.filter(o => o.altitude >= 10);
      }

      filtered.sort((a, b) => a.dist - b.dist);

      renderCatalog(filtered);
    }

    function renderCatalog(items) {
      const tbody = document.getElementById('catalogBody');
      if (items.length === 0) {
        tbody.innerHTML = '<tr><td colspan="5" style="text-align:center;">Aucun objet trouvé</td></tr>';
        return;
      }
      tbody.innerHTML = items.map(o => {
        let typeInfo = getTypePicto(o.type);
        let displayName = o.commonName ? `${o.commonName} (${o.name})` : o.name;
        
        return `
        <tr>
          <td>
            <span class="obj-title">${o.commonName || o.name}</span>
            ${o.commonName ? `<span class="obj-sub">${o.name}</span>` : ''}
          </td>
          <td style="text-align:center;">
            <span class="picto-type" title="${typeInfo.title}">${typeInfo.picto}</span>
          </td>
          <td>${o.mag.toFixed(1)}</td>
          <td style="text-align:center;">
            ${o.altitude >= 10 
              ? `<span class="picto-vis" title="Visible (${o.altitude.toFixed(0)}°)">🟢</span>` 
              : `<span class="picto-vis" title="${o.timeStr}">🔴</span>`}
          </td>
          <td>
            <button class="btn btn-sm" onclick="selectTarget('${displayName.replace(/'/g, "\\'")}', ${o.ra}, ${o.dec}, ${o.mag})">Cible</button>
          </td>
        </tr>
      `;
      }).join('');
    }

    initGeolocation();
    setInterval(updateStatus, 250);
    setInterval(applyFilters, 60000);
    drawGuidanceGauges(0, 0);
    loadCatalog();
  </script>
</body>
</html>
)rawliteral";

#endif