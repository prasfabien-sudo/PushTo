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
    select { width: 100%; margin: 4px 0 10px; padding: 9px; border: 1px solid #444; border-radius: 6px; background: #121212; color: #fff; }
    .star-list-label { display: block; margin: 10px 0 4px; color: #ccc; font-size: .85em; }
    .star-list-help { margin: 0 0 10px; color: #aaa; font-size: .8em; }
    #star2Candidates option.recommended { color: #80cbc4; }
    .position-grid { display: grid; grid-template-columns: repeat(2, minmax(0, 1fr)); gap: 8px; margin-bottom: 8px; }
    .position-value { padding: 10px; background: #191919; border-radius: 6px; }
    .position-value span { display: block; color: #aaa; font-size: .78em; }
    .position-value strong { display: block; margin-top: 4px; color: #ffb74d; font-size: 1.1em; }
    .position-state { min-height: 1.2em; margin: 0; color: #aaa; font-size: .8em; }
    .page-message { padding: 10px; margin: 12px 0; border-radius: 6px; background: #263238; color: #b2dfdb; font-size: .9em; }
    .page-message:empty { display: none; }
    .page-message.error { background: #3b2020; color: #ffcdd2; }
    
    .step-box { background: #2a2a2a; border-left: 4px solid #ff9800; padding: 10px; border-radius: 4px; margin-bottom: 12px; font-size: 0.85em; color: #ddd; }
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
    <div id="pageMessage" class="page-message" role="status" aria-live="polite"></div>

    <div class="section-title">Position actuelle du télescope</div>
    <div class="position-grid" aria-live="off">
      <div class="position-value"><span>Azimut</span><strong id="telescopeAzimuth">--</strong></div>
      <div class="position-value"><span>Altitude</span><strong id="telescopeAltitude">--</strong></div>
      <div class="position-value"><span>Compteur AZ (pas)</span><strong id="encoderStepsAZ">--</strong></div>
      <div class="position-value"><span>Compteur ALT (pas)</span><strong id="encoderStepsALT">--</strong></div>
      <div class="position-value"><span>Calibration AZ (pas/tour)</span><strong id="encoderResolutionAZ">--</strong></div>
      <div class="position-value"><span>Calibration ALT (pas/tour)</span><strong id="encoderResolutionALT">--</strong></div>
    </div>
    <p id="positionState" class="position-state" aria-live="off">Chargement de la position…</p>

    <div class="section-title">1. Alignement avec la polaire</div>
    <button class="btn" onclick="sendCmd('set_polaris')"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-star'></use></svg> Aligner sur la polaire</button>

    <!-- 2 Stars Alignment Step by Step -->
    <div class="section-title">2. Alignement Guidé 2 Étoiles</div>
    
    <div class="step-box" id="stepGuide">
      <strong>Procédure :</strong><br>
      1. Choisissez une étoile visible, pointez-la puis validez.<br>
      2. Choisissez une seconde étoile éloignée, pointez-la puis validez.
    </div>

    <div id="star1Group">
      <div style="font-weight:bold; color:#ffb74d; margin-bottom:5px;">Étape 1 : Première Étoile</div>
      <label class="star-list-label" for="star1Candidates">Étoiles visibles (les plus brillantes d'abord)</label>
      <select id="star1Candidates" aria-label="Choisir la première étoile"><option value="">Chargement des étoiles visibles...</option></select>
      <div class="form-group">
        <label>Azimut Théorique (°)</label>
        <input type="number" step="0.1" id="az1" placeholder="ex: 120.5" readonly>
      </div>
      <div class="form-group">
        <label>Altitude Théorique (°)</label>
        <input type="number" step="0.1" id="alt1" placeholder="ex: 45.0" readonly>
      </div>
      <button class="btn btn-step1" onclick="validateStar1()"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-star'></use></svg> Valider Étoile 1</button>
    </div>

    <div id="star2Group" style="margin-top: 15px; opacity: 0.5;">
      <div style="font-weight:bold; color:#e91e63; margin-bottom:5px;">Étape 2 : Seconde Étoile</div>
      <label class="star-list-label" for="star2Candidates">Étoiles visibles (les plus éloignées d'abord)</label>
      <p class="star-list-help">Privilégiez un écart total supérieur à 30°, avec au moins 5° en azimut et en altitude.</p>
      <select id="star2Candidates" aria-label="Choisir la seconde étoile" disabled><option value="">Validez d'abord l'étoile 1</option></select>
      <div class="form-group">
        <label>Azimut Théorique (°)</label>
        <input type="number" step="0.1" id="az2" placeholder="ex: 215.0" readonly disabled>
      </div>
      <div class="form-group">
        <label>Altitude Théorique (°)</label>
        <input type="number" step="0.1" id="alt2" placeholder="ex: 30.0" readonly disabled>
      </div>
      <button class="btn btn-step2" id="btnStar2" onclick="validateStar2()" disabled><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-star'></use></svg> Valider Étoile 2 & Terminer</button>
    </div>

  </div>

  <nav class="bottom-nav" aria-label="Navigation principale">
    <a class="nav-item" href="/beta_sky" title="Vue 3D du ciel"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-globe-2'></use></svg></span></a>
    <a class="nav-item" href="/" title="Mode Encodeurs"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-telescope'></use></svg></span></a>
    <a class="nav-item" href="/station" aria-current="page" title="Mise en station"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-compass'></use></svg></span></a>
    <a class="nav-item" href="/config" title="Configuration"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-settings'></use></svg></span></a>
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

    function showMessage(message, isError = false) {
      const element = document.getElementById('pageMessage');
      element.textContent = message;
      element.classList.toggle('error', isError);
    }

    function formatEncoderSteps(value) {
      return Math.round(value).toLocaleString('fr-FR');
    }

    async function updateTelescopeStatus() {
      const stateLabel = document.getElementById('positionState');
      try {
        const response = await fetch('/status');
        if (!response.ok) throw new Error('Impossible de lire la position du télescope.');
        const status = await response.json();
        const rawAZ = Number(status.rawAZ);
        const rawALT = Number(status.rawALT);
        const ticksAZ = Number(status.ticksAZ);
        const ticksALT = Number(status.ticksALT);
        const dirAZ = Number(status.dirAZ);
        const dirALT = Number(status.dirALT);
        if (![rawAZ, rawALT, ticksAZ, ticksALT, dirAZ, dirALT].every(Number.isFinite)
            || ticksAZ <= 0 || ticksALT <= 0) {
          throw new Error('Données de position invalides.');
        }

        const azimuth = (((rawAZ * dirAZ / ticksAZ * 360) % 360) + 360) % 360;
        const altitude = rawALT * dirALT / ticksALT * 360;
        document.getElementById('telescopeAzimuth').textContent = `${azimuth.toFixed(1)}°`;
        document.getElementById('telescopeAltitude').textContent = `${altitude.toFixed(1)}°`;
        document.getElementById('encoderStepsAZ').textContent = formatEncoderSteps(rawAZ);
        document.getElementById('encoderStepsALT').textContent = formatEncoderSteps(rawALT);
        document.getElementById('encoderResolutionAZ').textContent = formatEncoderSteps(ticksAZ);
        document.getElementById('encoderResolutionALT').textContent = formatEncoderSteps(ticksALT);
        stateLabel.textContent = status.siteName ? `Lieu : ${status.siteName}` : '';
      } catch (error) {
        console.error(error);
        [
          'telescopeAzimuth',
          'telescopeAltitude',
          'encoderStepsAZ',
          'encoderStepsALT',
          'encoderResolutionAZ',
          'encoderResolutionALT'
        ].forEach(id => document.getElementById(id).textContent = '--');
        stateLabel.textContent = error.message;
      }
    }

    updateTelescopeStatus();
    window.setInterval(updateTelescopeStatus, 500);

    let visibleStars = [];
    let selectedStar1 = null;
    let selectedStar2 = null;

    function getLocalSiderealHours(date, longitude) {
      const julianDate = date.getTime() / 86400000 + 2440587.5;
      const centuries = (julianDate - 2451545.0) / 36525;
      const greenwich = 280.46061837 + 360.98564736629 * (julianDate - 2451545.0)
        + 0.000387933 * centuries * centuries - centuries * centuries * centuries / 38710000;
      return ((greenwich + longitude) % 360 + 360) % 360 / 15;
    }

    function getStarHorizontalPosition(star, latitude, longitude, date) {
      const hourAngle = (getLocalSiderealHours(date, longitude) - star.ra) * Math.PI / 12;
      const dec = star.dec * Math.PI / 180;
      const lat = latitude * Math.PI / 180;
      const sinAltitude = Math.sin(dec) * Math.sin(lat) + Math.cos(dec) * Math.cos(lat) * Math.cos(hourAngle);
      const altitude = Math.asin(Math.max(-1, Math.min(1, sinAltitude)));
      const azimuth = Math.atan2(
        -Math.sin(hourAngle) * Math.cos(dec),
        Math.sin(dec) * Math.cos(lat) - Math.cos(dec) * Math.sin(lat) * Math.cos(hourAngle)
      );
      return { azimuth: (azimuth * 180 / Math.PI + 360) % 360, altitude: altitude * 180 / Math.PI };
    }

    function getAngularSeparation(starA, starB) {
      const altA = starA.altitude * Math.PI / 180;
      const altB = starB.altitude * Math.PI / 180;
      const deltaAz = (starA.azimuth - starB.azimuth) * Math.PI / 180;
      const cosSeparation = Math.sin(altA) * Math.sin(altB)
        + Math.cos(altA) * Math.cos(altB) * Math.cos(deltaAz);
      return Math.acos(Math.max(-1, Math.min(1, cosSeparation))) * 180 / Math.PI;
    }

    function getAzimuthDifference(azimuthA, azimuthB) {
      let difference = azimuthA - azimuthB;
      if (difference > 180) difference -= 360;
      if (difference < -180) difference += 360;
      return difference;
    }

    function formatStarOption(star, separation) {
      const details = `${star.name} — mag. ${star.mag.toFixed(1)}, alt. ${star.altitude.toFixed(0)}°, az. ${star.azimuth.toFixed(0)}°`;
      if (separation === undefined) return details;
      const azimuthDifference = Math.abs(getAzimuthDifference(star.azimuth, selectedStar1.azimuth));
      const altitudeDifference = Math.abs(star.altitude - selectedStar1.altitude);
      const recommended = separation >= 30 && azimuthDifference >= 5 && altitudeDifference >= 5;
      return `${details}, écart ${separation.toFixed(0)}° (ΔAZ ${azimuthDifference.toFixed(0)}°, ΔALT ${altitudeDifference.toFixed(0)}°)${recommended ? ' — recommandée' : ''}`;
    }

    function populateStarCandidates(select, stars, separationFromStar) {
      select.replaceChildren();
      if (stars.length === 0) {
        select.add(new Option('Aucune étoile visible à plus de 10°', ''));
        return;
      }

      const sortedStars = stars.map(star => ({
        star,
        separation: separationFromStar ? getAngularSeparation(star, separationFromStar) : undefined
      })).sort((a, b) => separationFromStar
        ? b.separation - a.separation || a.star.mag - b.star.mag
        : a.star.mag - b.star.mag);

      sortedStars.forEach(({ star, separation }) => {
        const option = new Option(formatStarOption(star, separation), star.name);
        if (separation !== undefined
            && separation >= 30
            && Math.abs(getAzimuthDifference(star.azimuth, separationFromStar.azimuth)) >= 5
            && Math.abs(star.altitude - separationFromStar.altitude) >= 5) option.className = 'recommended';
        select.add(option);
      });
      select.selectedIndex = 0;
      select.dispatchEvent(new Event('change'));
    }

    async function loadVisibleStars() {
      const firstSelect = document.getElementById('star1Candidates');
      try {
        const [catalogResponse, statusResponse] = await Promise.all([fetch('/catalog'), fetch('/status')]);
        if (!catalogResponse.ok || !statusResponse.ok) throw new Error('Impossible de charger le catalogue ou le lieu d’observation.');
        const [catalog, status] = await Promise.all([catalogResponse.json(), statusResponse.json()]);
        const latitude = Number(status.lat);
        const longitude = Number(status.lon);
        if (!Number.isFinite(latitude) || !Number.isFinite(longitude)) throw new Error('Coordonnées du lieu d’observation invalides.');

        const now = new Date();
        visibleStars = catalog
          .filter(object => object.type.toLowerCase().startsWith('étoile'))
          .map(star => ({ ...star, ...getStarHorizontalPosition(star, latitude, longitude, now) }))
          .filter(star => star.altitude >= 10)
          .sort((a, b) => a.mag - b.mag);
        populateStarCandidates(firstSelect, visibleStars);
      } catch (error) {
        console.error(error);
        firstSelect.replaceChildren(new Option('Erreur de chargement des étoiles visibles', ''));
        document.getElementById('stepGuide').textContent = error.message;
      }
    }

    function selectStar(selectId, step) {
      const select = document.getElementById(selectId);
      const star = visibleStars.find(candidate => candidate.name === select.value);
      if (!star) return;
      if (step === 1) {
        selectedStar1 = star;
        document.getElementById('az1').value = star.azimuth.toFixed(2);
        document.getElementById('alt1').value = star.altitude.toFixed(2);
      } else {
        selectedStar2 = star;
        document.getElementById('az2').value = star.azimuth.toFixed(2);
        document.getElementById('alt2').value = star.altitude.toFixed(2);
      }
    }

    document.getElementById('star1Candidates').addEventListener('change', () => selectStar('star1Candidates', 1));
    document.getElementById('star2Candidates').addEventListener('change', () => selectStar('star2Candidates', 2));
    loadVisibleStars();

    async function sendCmd(action) {
      try {
        const response = await fetch(`/cmd?action=${encodeURIComponent(action)}`);
        if (!response.ok) throw new Error(await response.text());
        showMessage('Alignement avec la polaire effectué.');
        await updateTelescopeStatus();
      } catch (error) {
        console.error(error);
        showMessage(`Impossible d'aligner le télescope : ${error.message}`, true);
      }
    }

    async function validateStar1() {
      if (!selectedStar1) {
        showMessage('Choisissez une étoile visible dans la liste.', true);
        return;
      }

      try {
        const response = await fetch(`/cmd?action=align_star1&az=${selectedStar1.azimuth}&alt=${selectedStar1.altitude}`);
        if (!response.ok) throw new Error(await response.text());
        const result = await response.json();
        if (result.status !== "OK") throw new Error("L'étoile 1 n'a pas pu être enregistrée.");

        document.getElementById('stepGuide').textContent = 'Choisissez une seconde étoile éloignée dans la liste, pointez-la puis validez.';
        const secondSelect = document.getElementById('star2Candidates');
        secondSelect.disabled = false;
        document.getElementById('az2').disabled = false;
        document.getElementById('alt2').disabled = false;
        document.getElementById('btnStar2').disabled = false;
        document.getElementById('star2Group').style.opacity = '1.0';
        populateStarCandidates(secondSelect, visibleStars.filter(star => star.name !== selectedStar1.name), selectedStar1);
        showMessage('Étoile 1 enregistrée. Choisissez puis pointez une seconde étoile éloignée.');
      } catch (error) {
        console.error(error);
        showMessage(`Impossible d'enregistrer l'étoile 1 : ${error.message}`, true);
      }
    }

    async function validateStar2() {
      if (!selectedStar2) {
        showMessage('Choisissez une seconde étoile visible dans la liste.', true);
        return;
      }

      try {
        const response = await fetch(`/cmd?action=align_star2&az=${selectedStar2.azimuth}&alt=${selectedStar2.altitude}`);
        if (!response.ok) throw new Error(await response.text());
        const result = await response.json();
        if (result.status !== "OK") throw new Error("L'alignement n'a pas pu être terminé.");

        const message = `Alignement terminé. Nouvelle calibration : AZ ${result.ticksAZ} pas/tour, ALT ${result.ticksALT} pas/tour.`;
        document.getElementById('stepGuide').textContent = 'Alignement terminé.';
        showMessage(message);
        await updateTelescopeStatus();
      } catch (error) {
        console.error(error);
        showMessage(`Impossible de terminer l'alignement : ${error.message}`, true);
      }
    }
  </script>
</body>
</html>
)rawliteral";

#endif