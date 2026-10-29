// Version Component: BetaSkyPage.h v5.60.0
// Version Global System: v1.3.0
#ifndef BETASKYPAGE_H
#define BETASKYPAGE_H

#include <pgmspace.h>

const char HTTP_BETA_SKY_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="theme-color" content="#08090c">
  <title>Vue 3D du ciel — Planétarium</title>
  <style>
    * { box-sizing: border-box; }
    body { margin: 0; padding: 12px 12px 90px; min-height: 100vh; background: #08090c; color: #d8dce6; font: 14px -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; transition: all 0.3s; }
    .sky-card { width: min(100%, 760px); margin: 0 auto; transition: all 0.3s; }
    .sky-heading { display: flex; align-items: center; justify-content: space-between; gap: 10px; margin: 2px 2px 10px; }
    h1 { margin: 0; font-size: 1.15rem; color: #f1f3f8; }
    .beta { padding: 3px 7px; border: 1px solid #74531e; border-radius: 999px; color: #ffc66d; font-size: .7rem; letter-spacing: .08em; }
    .sky-status { min-height: 20px; margin: 0 2px 8px; color: #aab3c2; font-size: .82rem; }
    .sky-status.error { color: #ff7777; }
    .sky-status.align-ok { color: #69ff84; }
    
    .scene { position: relative; height: min(60vh, 500px); min-height: 300px; overflow: hidden; border: 1px solid #273142; border-radius: 14px; background: radial-gradient(ellipse at 50% 100%, #1b2638 0, #0d1420 47%, #05070b 100%); touch-action: none; transition: height 0.3s, background 0.5s; }
    canvas { display: block; width: 100%; height: 100%; cursor: grab; position: relative; z-index: 1; background: transparent; }
    canvas:active { cursor: grabbing; }
    .scene-label { position: absolute; z-index: 2; color: #acb8c8; font-size: .72rem; text-shadow: 0 1px 4px #000; pointer-events: none; }
    .scene-label.cardinal { font-weight: bold; color: #ffb74d; background: rgba(5,8,12,0.6); padding: 2px 6px; border-radius: 4px; }
    .scene-label.telescope { top: 10px; left: 10px; }
    .scene-label.telescope.error { color: #ff7777; }
    .scene-label.view { left: 10px; bottom: 10px; }
    .scene-label.horizon { right: 10px; bottom: 10px; }
    .scene-center { position: absolute; top: 50%; left: 50%; width: 8px; height: 8px; border: 1px solid rgba(255,255,255,.7); border-radius: 50%; transform: translate(-50%,-50%); pointer-events: none; z-index: 2; }
    
    .scene-zoom-controls { position: absolute; top: 10px; right: 10px; z-index: 5; display: flex; flex-direction: column; gap: 4px; }
    .zoom-btn { width: 32px; height: 32px; background: rgba(21, 27, 37, 0.85); border: 1px solid #344256; border-radius: 6px; color: #ffb74d; font-size: 1.1rem; font-weight: bold; display: flex; align-items: center; justify-content: center; cursor: pointer; backdrop-filter: blur(4px); padding: 0; }
    .zoom-btn:active { background: rgba(37, 50, 71, 0.95); transform: scale(0.95); }

    .object-label { position: absolute; z-index: 2; padding: 2px 6px; border-radius: 4px; color: #f4e6be; background: rgba(5,8,12,0.8); font-size: .68rem; white-space: nowrap; text-shadow: 0 1px 3px #000; pointer-events: none; transition: transform 0.1s ease-out; }
    .object-label.polaris { color: #ffb74d; background: rgba(30,35,45,0.9); border: 1px solid rgba(255,183,77,0.5); font-weight: bold; z-index: 3; }
    .constellation-label { position: absolute; z-index: 2; color: #ffb366; font-size: .75rem; font-weight: bold; letter-spacing: 0.05em; text-transform: uppercase; text-shadow: 0 1px 5px #000; transform: translate(-50%,-50%); pointer-events: none; opacity: 0.9; }

    .controls { display: grid; grid-template-columns: repeat(4, minmax(0,1fr)); gap: 8px; margin-top: 10px; }
    
    .time-control-box { grid-column: span 4; display: flex; flex-wrap: wrap; gap: 6px; align-items: center; justify-content: space-between; background: #151b25; padding: 8px 10px; border: 1px solid #344256; border-radius: 9px; }
    .time-input-group { display: flex; align-items: center; gap: 6px; flex: 1; min-width: 200px; }
    .time-step-group { display: flex; align-items: center; gap: 3px; }
    .time-btn { min-height: 36px; padding: 2px 8px; font-size: 0.85rem; font-weight: bold; background: #1e293b; color: #ffb74d; border: 1px solid #344256; border-radius: 6px; cursor: pointer; display: flex; align-items: center; justify-content: center; }
    .time-btn:active { background: #253247; transform: scale(0.95); }
    
    .settings-btn { grid-column: span 2; background: #151b25; border: 1px solid #344256; border-radius: 9px; color: #ffb74d; font-weight: bold; display: flex; justify-content: center; align-items: center; gap: 6px; cursor: pointer; font: inherit; }
    
    button, select, input { min-height: 44px; border: 1px solid #344256; border-radius: 9px; background: #151b25; color: #e6eaf2; font: inherit; }
    button { padding: 8px 10px; cursor: pointer; }
    button:active { background: #253247; }
    select, input[type="datetime-local"] { width: 100%; padding: 8px 10px; background: #10151d; }
    .scene-help { margin: 9px 2px; color: #929cac; font-size: .78rem; line-height: 1.45; text-align: center; }

    .dpad-wrapper { grid-column: span 4; display: flex; align-items: center; justify-content: center; gap: 12px; background: #121822; padding: 8px; border: 1px solid #273142; border-radius: 12px; margin-bottom: 4px; }
    .dpad-container { display: grid; grid-template-columns: repeat(3, 1fr); gap: 4px; width: 180px; }
    .jog-btn { background: #1e293b !important; font-weight: bold; color: #ffb74d !important; font-size: 1rem; display: flex; align-items: center; justify-content: center; min-height: 38px; }
    .jog-up { grid-column: 2; grid-row: 1; }
    .jog-left { grid-column: 1; grid-row: 2; }
    .jog-center { grid-column: 2; grid-row: 2; background: #10151d !important; border: 1px dashed #344256; font-size: 0.6rem; color: #8b94a2 !important; cursor: default; display: flex; align-items: center; justify-content: center; }
    .jog-right { grid-column: 3; grid-row: 2; }
    .jog-down { grid-column: 2; grid-row: 3; }
    
    .sens-box { display: flex; flex-direction: column; gap: 4px; font-size: 0.75rem; color: #8b94a2; }
    .sens-box select { min-height: 36px; padding: 4px 8px; font-size: 0.8rem; background: #151b25; color: #ffb74d; font-weight: bold; }

    body.landscape-fullscreen { padding: 0 !important; background: #000; overflow: hidden; }
    body.landscape-fullscreen .sky-heading,
    body.landscape-fullscreen .sky-status,
    body.landscape-fullscreen .controls,
    body.landscape-fullscreen .scene-help,
    body.landscape-fullscreen .bottom-nav { display: none !important; }
    body.landscape-fullscreen .sky-card { width: 100% !important; max-width: none !important; margin: 0; }
    body.landscape-fullscreen .scene { height: 100vh !important; border: none; border-radius: 0; }

    #manualFullscreenBtn { display: inline-block; background: #263238; color: #ffb74d; border: 1px solid #455a64; font-weight: bold; }
    @media (orientation: landscape) {
      #manualFullscreenBtn { display: none !important; }
    }

    .object-modal { position: fixed; inset: 0; z-index: 100; background: rgba(0,0,0,0.7); display: flex; align-items: center; justify-content: center; padding: 16px; backdrop-filter: blur(2px); }
    .object-modal[hidden] { display: none; }
    .modal-content { width: min(100%, 390px); background: #151b25; border: 1px solid #344256; border-radius: 14px; padding: 20px; box-shadow: 0 8px 32px rgba(0,0,0,0.7); max-height: 85vh; overflow-y: auto; }
    .modal-content h2 { margin: 0 0 12px; font-size: 1.1rem; color: #ffb74d; border-bottom: 1px solid #273142; padding-bottom: 8px; }
    .modal-sub { color: #8b94a2; font-size: .85rem; margin-bottom: 12px; }
    .modal-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 8px; margin-bottom: 16px; }
    .modal-stat { background: #10151d; padding: 8px 10px; border-radius: 8px; border: 1px solid #222d3b; }
    .modal-stat-label { font-size: .7rem; color: #8b94a2; display: block; }
    .modal-stat-value { font-size: .95rem; font-weight: bold; color: #f1f3f8; }
    
    .settings-section-title { font-size: 0.72rem; color: #ffb74d; font-weight: bold; margin: 12px 0 6px; text-transform: uppercase; letter-spacing: 0.05em; border-bottom: 1px solid #273142; padding-bottom: 2px; }
    .settings-table { display: flex; flex-direction: column; gap: 6px; }
    .settings-row { display: grid; grid-template-columns: 1fr auto auto; align-items: center; gap: 10px; padding: 4px 6px; background: #10151d; border-radius: 6px; border: 1px solid #222d3b; font-size: 0.83rem; }
    .settings-row span { color: #e6eaf2; font-weight: 500; }
    .settings-headers { display: grid; grid-template-columns: 1fr auto auto; gap: 10px; padding: 0 6px 2px; font-size: 0.68rem; color: #8b94a2; text-transform: uppercase; font-weight: bold; text-align: center; }
    .settings-headers div:nth-child(2), .settings-headers div:nth-child(3) { width: 42px; }
    .settings-cell-check { display: flex; justify-content: center; width: 42px; }
    .settings-modal-body input[type="checkbox"] { width: 17px; height: 17px; accent-color: #ffb74d; cursor: pointer; }
    .settings-modal-body input[type="checkbox"]:disabled { opacity: 0.3; cursor: not-allowed; }
    .modal-close { width: 100%; background: #263238; border: 1px solid #455a64; color: #fff; font-weight: bold; margin-top: 14px; }

    html[data-night-mode="true"]::after { content: ""; position: fixed; inset: 0; z-index: 2147483647; pointer-events: none; background: rgba(255, 0, 0, 0.82); mix-blend-mode: multiply; filter: brightness(0.45); }
    
    .bottom-nav { position: fixed; z-index: 20; left: 0; right: 0; bottom: 0; display: flex; overflow-x: auto; scroll-snap-type: x mandatory; -webkit-overflow-scrolling: touch; gap: 6px; padding: 8px 12px calc(8px + env(safe-area-inset-bottom)); background: rgba(30,30,30,0.98); border-top: 1px solid #37474f; box-shadow: -4px 16px rgba(0,0,0,0.4); scrollbar-width: none; }
    .bottom-nav::-webkit-scrollbar { display: none; }
    .nav-item { flex: 0 0 56px; min-height: 48px; display: flex; align-items: center; justify-content: center; border: 0; border-radius: 8px; background: transparent; color: #b0bec5; font: inherit; text-decoration: none; cursor: pointer; scroll-snap-align: start; }
    .nav-item[aria-current="page"] { color: #ffb74d; background: #263238; }
    .night-mode-toggle { background: #f1f3f4; color: #101418; border: 1px solid #fff; }
    .night-mode-toggle[aria-pressed="true"] { background: #ff5252; color: #fff; }
    .nav-icon { display: flex; font-size: 24px; line-height: 1; }
    .ui-icon { display: inline-block; width: 1em; height: 1em; vertical-align: -.15em; fill: none; stroke: currentColor; stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; }
    .nav-icon .ui-icon { vertical-align: middle; }
    @media (min-width: 700px) { .bottom-nav { left: 50%; right: auto; width: min(650px, calc(100% - 32px)); transform: translateX(-50%); border: 1px solid #37474f; border-bottom: 0; border-radius: 14px 14px 0 0; justify-content: center; overflow-x: hidden; } .nav-item { flex: 1; } }
  </style>
</head>
<body>
  <main class="sky-card">
    <header class="sky-heading">
      <h1><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-compass"></use></svg> Vue 3D du ciel</h1>
      <span class="beta">PLANÉTARIUM v5.60.0</span>
    </header>
    <p id="skyStatus" class="sky-status" role="status">Initialisation du planétarium 3D...</p>
    
    <div class="scene" id="scene">
      <!-- Indicateur de guidage combiné (Azimut + Altitude simultanés) -->
      <div id="searchArrowOverlay" style="position: absolute; top: 10px; left: 50%; transform: translateX(-50%); z-index: 10; display: none; background: rgba(5,8,12,0.9); border: 1px solid #ff9800; padding: 5px 14px; border-radius: 20px; color: #ffb74d; font-weight: bold; font-size: 0.85rem; align-items: center; gap: 10px; pointer-events: none; backdrop-filter: blur(4px);">
        <span id="targetNameLabel" style="color: #fff;">M42</span>
        <div style="display: flex; align-items: center; gap: 6px; border-left: 1px solid #344256; padding-left: 8px;">
          <span id="azArrow" style="font-size: 1rem;" title="Azimut (Gauche / Droite)">⬅️</span>
          <span id="altArrow" style="font-size: 1rem;" title="Altitude (Haut / Bas)">⬆️</span>
          <span id="angularDistText" style="font-size: 0.78rem; color: #aab3c2;">19.8°</span>
        </div>
      </div>

      <div class="scene-zoom-controls">
        <button id="zoomInBtn" type="button" class="zoom-btn" title="Zoomer (+)">+</button>
        <button id="zoomOutBtn" type="button" class="zoom-btn" title="Dézoomer (-)">-</button>
      </div>

      <canvas id="skyCanvas" aria-label="Carte 3D interactive du ciel"></canvas>
      <span class="scene-label cardinal" id="labelN" style="display:none">N</span>
      <span class="scene-label cardinal" id="labelS" style="display:none">S</span>
      <span class="scene-label cardinal" id="labelE" style="display:none">E</span>
      <span class="scene-label cardinal" id="labelW" style="display:none">O</span>
      <span class="scene-label telescope" id="telescopeReadout">Télescope : connexion...</span>
      <span class="scene-label view" id="viewReadout">Vue AZ 0° · ALT 0°</span>
      <span class="scene-label horizon">Grille AltAz & Heure</span>
      <span class="scene-center" aria-hidden="true"></span>
      <div id="objectLabels" aria-hidden="true"></div>
      <div id="constellationLabels" aria-hidden="true"></div>
    </div>
    
    <div class="controls">
      <button id="toggleAllMaskBtn" type="button" style="grid-column: span 2; background: #263238; color: #ffb74d; font-weight: bold; font-size: 0.8rem;">Tout masquer</button>
      <button id="openSearchTargetModalBtn" type="button" style="grid-column: span 2; background: #263238; color: #ffb74d; font-weight: bold; font-size: 0.8rem;">Rechercher une cible 🔍</button>
      <button id="openSettingsModalBtn" type="button" class="settings-btn" style="grid-column: span 4;">Paramètres ⚙️</button>

      <div class="dpad-wrapper">
        <div class="dpad-container">
          <button id="jogUpBtn" type="button" class="jog-btn jog-up" title="Ajuster Altitude">▲</button>
          <button id="jogLeftBtn" type="button" class="jog-btn jog-left" title="Ajuster Azimut">◀</button>
          <div class="jog-center">Offset</div>
          <button id="jogRightBtn" type="button" class="jog-btn jog-right" title="Ajuster Azimut">▶</button>
          <button id="jogDownBtn" type="button" class="jog-btn jog-down" title="Ajuster Altitude">▼</button>
        </div>
        <div class="sens-box">
          <label for="stepSelect">Pas (sensibilité) :</label>
          <select id="stepSelect" aria-label="Sensibilité du pas d'offset">
            <option value="0.1">0.1° (Fin)</option>
            <option value="1" selected>1.0° (Normal)</option>
            <option value="5">5.0° (Large)</option>
          </select>
        </div>
      </div>

      <div class="time-control-box">
        <div class="time-input-group">
          <span style="font-size: 0.75rem; color: #8b94a2; white-space: nowrap;">Heure ciel :</span>
          <input type="datetime-local" id="customTimeInput" aria-label="Heure d'affichage du ciel">
        </div>
        
        <div class="time-step-group">
          <button type="button" class="time-btn" id="stepPrevHourBtn" title="-1 Heure">«</button>
          <button type="button" class="time-btn" id="stepPrev10mBtn" title="-10 Minutes">&lt;</button>
          <button type="button" id="resetTimeBtn" class="time-btn" style="min-height:36px; color:#fff; background:#263238; border-color:#455a64; font-size:0.75rem;" title="Revenir à l'heure réelle">Direct</button>
          <button type="button" class="time-btn" id="stepNext10mBtn" title="+10 Minutes">&gt;</button>
          <button type="button" class="time-btn" id="stepNextHourBtn" title="+1 Heure">»</button>
        </div>
      </div>

      <select id="magFilterSelect" aria-label="Mag. Max" style="grid-column: span 4;">
        <option value="99" selected>Mag. Max : Toutes</option>
        <option value="6">Mag. ≤ 6 (Œil nu)</option>
        <option value="8">Mag. ≤ 8 (Jumelles)</option>
        <option value="10">Mag. ≤ 10 (Petit télescope)</option>
        <option value="12">Mag. ≤ 12 (Grand télescope)</option>
      </select>

      <button id="centerButton" type="button" style="grid-column: span 4;">Recentrer</button>
      
      <button id="manualFullscreenBtn" type="button" style="grid-column: span 4;">Plein écran</button>
    </div>
    <p class="scene-help">Utilisez la croix pour appliquer un offset permanent. Le déplacement tactile est temporaire.</p>
  </main>

  <!-- POP-UP RECHERCHE DE CIBLE -->
  <div class="object-modal" id="searchTargetModal" hidden>
    <div class="modal-content">
      <h2>Rechercher un objet cible 🎯</h2>
      <div class="modal-sub">Sélectionnez un objet du catalogue pour activer le guidage</div>
      <div style="display: flex; flex-direction: column; gap: 10px; margin-bottom: 16px;">
        <input type="text" id="targetSearchInput" placeholder="Filtrer par nom (ex: M42, Andromède...)" style="width: 100%; padding: 8px 10px; background: #10151d; border: 1px solid #344256; border-radius: 8px; color: #fff;">
        <select id="targetSelectDropdown" size="6" style="width: 100%; background: #10151d; border: 1px solid #344256; border-radius: 8px; color: #fff; padding: 4px;">
          <!-- Les options sont générées dynamiquement -->
        </select>
      </div>
      <button type="button" class="btn" style="width: 100%; background: #ff9800; color: #000; font-weight: bold; margin-bottom: 8px;" onclick="confirmSelectedTarget()">Définir comme cible 🚀</button>
      <button class="modal-close" type="button" onclick="closeSearchTargetModal()">Fermer</button>
    </div>
  </div>

  <!-- POP-UP PARAMÈTRES D'AFFICHAGE -->
  <div class="object-modal" id="settingsModal" hidden>
    <div class="modal-content">
      <h2>Paramètres d'affichage</h2>
      <div class="settings-modal-body">
        
        <div class="settings-section-title">Repères & Grille</div>
        <div class="settings-table" style="margin-top:6px;">
          <div class="settings-row">
            <span>Grille & Horizon</span>
            <div class="settings-cell-check" style="grid-column: 3;"><input type="checkbox" id="showGrid" checked></div>
          </div>
        </div>

        <div class="settings-section-title">Catalogue céleste</div>
        <div class="settings-headers">
          <div style="text-align: left;">Type d'objet</div>
          <div>Objet</div>
          <div>Nom</div>
        </div>
        
        <div class="settings-table">
          <div class="settings-row">
            <span>Galaxies</span>
            <div class="settings-cell-check"><input type="checkbox" id="showGal" checked data-name-id="labelGal"></div>
            <div class="settings-cell-check"><input type="checkbox" id="labelGal" checked></div>
          </div>
          <div class="settings-row">
            <span>Nébuleuses</span>
            <div class="settings-cell-check"><input type="checkbox" id="showNeb" checked data-name-id="labelNeb"></div>
            <div class="settings-cell-check"><input type="checkbox" id="labelNeb" checked></div>
          </div>
          <div class="settings-row">
            <span>Amas d'étoiles</span>
            <div class="settings-cell-check"><input type="checkbox" id="showAmas" checked data-name-id="labelAmas"></div>
            <div class="settings-cell-check"><input type="checkbox" id="labelAmas" checked></div>
          </div>
          <div class="settings-row">
            <span>Étoiles</span>
            <div class="settings-cell-check"><input type="checkbox" id="showStars" checked data-name-id="labelStars"></div>
            <div class="settings-cell-check"><input type="checkbox" id="labelStars" checked></div>
          </div>
          <div class="settings-row">
            <span>Constellations</span>
            <div class="settings-cell-check"><input type="checkbox" id="showConstellations" checked data-name-id="labelConstellations"></div>
            <div class="settings-cell-check"><input type="checkbox" id="labelConstellations" checked></div>
          </div>
        </div>

      </div>
      <button class="modal-close" type="button" onclick="closeSettingsModal()">Fermer</button>
    </div>
  </div>

  <!-- POP-UP FICHE OBJET -->
  <div class="object-modal" id="objectModal" hidden>
    <div class="modal-content">
      <h2 id="modalTitle">Nom de l'objet</h2>
      <div class="modal-sub" id="modalSub">Constellation · Type</div>
      <div class="modal-grid">
        <div class="modal-stat">
          <span class="modal-stat-label">Magnitude</span>
          <span class="modal-stat-value" id="modalMag">-</span>
        </div>
        <div class="modal-stat">
          <span class="modal-stat-label">Distance</span>
          <span class="modal-stat-value" id="modalDist">-</span>
        </div>
        <div class="modal-stat">
          <span class="modal-stat-label">Taille apparente</span>
          <span class="modal-stat-value" id="modalSize">-</span>
        </div>
        <div class="modal-stat">
          <span class="modal-stat-label">Coordonnées AD/Dec</span>
          <span class="modal-stat-value" id="modalCoords" style="font-size:0.8rem">-</span>
        </div>
      </div>
      <button type="button" class="btn" id="targetBtn" style="width:100%; background:#ff9800; color:#000; font-weight:bold; margin-bottom:8px;" onclick="setSearchTarget()">Rechercher cet objet 🎯</button>
      <button class="modal-close" type="button" onclick="closeModal()">Fermer</button>
    </div>
  </div>

  <nav class="bottom-nav" aria-label="Navigation principale">
    <a class="nav-item" href="/beta_sky" aria-current="page" title="Vue 3D du ciel"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-compass"></use></svg></span></a>
    <a class="nav-item" href="/" title="Mode Encodeurs"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-telescope"></use></svg></span></a>
    <a class="nav-item" href="/station" title="Mise en station"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-crosshair"></use></svg></span></a>
    <a class="nav-item" href="/calib_page" title="Calibration"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-ruler"></use></svg></span></a>
    <a class="nav-item" href="/config" title="Configuration"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-settings"></use></svg></span></a>
    <a class="nav-item" href="/help" title="Aide SkySafari"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-book-open"></use></svg></span></a>
    <a class="nav-item" href="/simu" title="Simulation"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-gamepad-2"></use></svg></span></a>
    <a class="nav-item" href="/test" title="Tests système"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-flask-conical"></use></svg></span></a>
    <a class="nav-item" href="/releasenotes" title="Notes de version"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-clipboard-list"></use></svg></span></a>
    <button class="nav-item night-mode-toggle" id="nightModeToggle" type="button" aria-label="Activer le mode nuit" title="Activer le mode nuit" aria-pressed="false" onclick="toggleNightMode()"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-moon"></use></svg></span></a>
  </nav>

  <script>
    const canvas = document.getElementById('skyCanvas');
    const sceneDiv = document.getElementById('scene');
    const ctx2d = canvas.getContext('2d');
    const statusElement = document.getElementById('skyStatus');
    const magFilterElement = document.getElementById('magFilterSelect');
    const labelsElement = document.getElementById('objectLabels');
    const constLabelsElement = document.getElementById('constellationLabels');
    const viewReadout = document.getElementById('viewReadout');
    const telescopeReadout = document.getElementById('telescopeReadout');
    const customTimeInput = document.getElementById('customTimeInput');
    const stepSelect = document.getElementById('stepSelect');
    
    let objects = [];
    let latitude = 45.89;
    let longitude = 6.05;
    
    let targetHeading = 0;
    let targetElevation = 35;
    let heading = 0;
    let elevation = 35;

    let touchHeading = 0;
    let touchElevation = 0;
    
    let alignOffsetHeading = 0;
    let alignOffsetElevation = 0;

    let fieldOfView = 34;
    let encoderRequestInFlight = false;
    let lastScreenObjects = [];
    let useSystemTime = true;
    
    let lastRawAZ = null;
    let lastRawALT = null;

    let activeTargetObject = null;

    let wakeLock = null;
    async function requestWakeLock() {
      if ('wakeLock' in navigator) {
        try {
          wakeLock = await navigator.wakeLock.request('screen');
          wakeLock.addEventListener('release', () => {});
        } catch (err) {}
      }
    }

    document.addEventListener('visibilitychange', async () => {
      if (wakeLock !== null && document.visibilityState === 'visible') {
        await requestWakeLock();
      }
    });

    const settingsModal = document.getElementById('settingsModal');
    document.getElementById('openSettingsModalBtn').addEventListener('click', () => {
      settingsModal.hidden = false;
    });
    function closeSettingsModal() {
      settingsModal.hidden = true;
    }

    // --- LOGIQUE RECHERCHE GLOBALE DE CIBLE ---
    const searchTargetModal = document.getElementById('searchTargetModal');
    const targetSearchInput = document.getElementById('targetSearchInput');
    const targetSelectDropdown = document.getElementById('targetSelectDropdown');

    document.getElementById('openSearchTargetModalBtn').addEventListener('click', () => {
      populateTargetDropdown('');
      searchTargetModal.hidden = false;
    });

    function closeSearchTargetModal() {
      searchTargetModal.hidden = true;
    }

    function populateTargetDropdown(filterText) {
      targetSelectDropdown.innerHTML = '';
      const query = filterText.toLowerCase();
      
      const filtered = objects.filter(o => 
        (o.name && o.name.toLowerCase().includes(query)) || 
        (o.commonName && o.commonName.toLowerCase().includes(query)) ||
        (o.type && o.type.toLowerCase().includes(query))
      );

      filtered.slice(0, 100).forEach(o => {
        const opt = document.createElement('option');
        opt.value = o.name;
        opt.textContent = `${o.name}${o.commonName ? ' (' + o.commonName + ')' : ''} - Mag: ${o.mag}`;
        targetSelectDropdown.appendChild(opt);
      });
    }

    targetSearchInput.addEventListener('input', (e) => {
      populateTargetDropdown(e.target.value);
    });

    function confirmSelectedTarget() {
      const selectedName = targetSelectDropdown.value;
      if (!selectedName) return;

      activeTargetObject = objects.find(o => o.name === selectedName);
      closeSearchTargetModal();

      const overlay = document.getElementById('searchArrowOverlay');
      if (activeTargetObject) {
        overlay.style.display = 'flex';
      }
    }

    function setSearchTarget() {
      const titleText = document.getElementById('modalTitle').textContent;
      const objName = titleText.split(' (')[0].trim();
      
      activeTargetObject = objects.find(o => o.name === objName);
      closeModal();
      
      const overlay = document.getElementById('searchArrowOverlay');
      if (activeTargetObject) {
        overlay.style.display = 'flex';
      } else {
        overlay.style.display = 'none';
      }
    }

    async function loadSavedSkyPrefs() {
      try {
        const res = await fetch('/get_sky_prefs');
        if (res.ok) {
          const p = await res.json();
          if (p.mag !== undefined) magFilterElement.value = p.mag;
          if (p.showGrid !== undefined) document.getElementById('showGrid').checked = p.showGrid;
          if (p.showGal !== undefined) document.getElementById('showGal').checked = p.showGal;
          if (p.showNeb !== undefined) document.getElementById('showNeb').checked = p.showNeb;
          if (p.showAmas !== undefined) document.getElementById('showAmas').checked = p.showAmas;
          if (p.showStars !== undefined) document.getElementById('showStars').checked = p.showStars;
          if (p.showConst !== undefined) document.getElementById('showConstellations').checked = p.showConst;
          
          if (p.labelGal !== undefined) document.getElementById('labelGal').checked = p.labelGal;
          if (p.labelNeb !== undefined) document.getElementById('labelNeb').checked = p.labelNeb;
          if (p.labelAmas !== undefined) document.getElementById('labelAmas').checked = p.labelAmas;
          if (p.labelStars !== undefined) document.getElementById('labelStars').checked = p.labelStars;
          if (p.labelConst !== undefined) document.getElementById('labelConstellations').checked = p.labelConst;
        }
      } catch (e) {
        console.warn("Préférences ESP32 non disponibles:", e);
      }
    }

    async function saveSkyPrefs() {
      try {
        const p = {
          mag: magFilterElement.value,
          showGrid: document.getElementById('showGrid').checked ? 1 : 0,
          showGal: document.getElementById('showGal').checked ? 1 : 0,
          showNeb: document.getElementById('showNeb').checked ? 1 : 0,
          showAmas: document.getElementById('showAmas').checked ? 1 : 0,
          showStars: document.getElementById('showStars').checked ? 1 : 0,
          showConst: document.getElementById('showConstellations').checked ? 1 : 0,
          labelGal: document.getElementById('labelGal').checked ? 1 : 0,
          labelNeb: document.getElementById('labelNeb').checked ? 1 : 0,
          labelAmas: document.getElementById('labelAmas').checked ? 1 : 0,
          labelStars: document.getElementById('labelStars').checked ? 1 : 0,
          labelConst: document.getElementById('labelConstellations').checked ? 1 : 0
        };
        const query = new URLSearchParams(p).toString();
        await fetch('/save_sky_prefs?' + query, { method: 'POST' });
      } catch (e) {
        console.warn("Erreur sauvegarde ESP32:", e);
      }
    }

    magFilterElement.addEventListener('change', saveSkyPrefs);
    document.querySelectorAll('#settingsModal input[type="checkbox"]').forEach(chk => {
      chk.addEventListener('change', saveSkyPrefs);
    });

    document.querySelectorAll('.settings-row input[type="checkbox"][data-name-id]').forEach(objChk => {
      const nameId = objChk.dataset.nameId;
      const nameChk = document.getElementById(nameId);
      
      const updateLinkState = () => {
        if (!objChk.checked) {
          nameChk.checked = false;
          nameChk.disabled = true;
        } else {
          nameChk.disabled = false;
        }
      };

      updateLinkState();
      objChk.addEventListener('change', updateLinkState);
    });

    document.getElementById('jogLeftBtn').addEventListener('click', () => {
      const step = parseFloat(stepSelect.value) || 1;
      alignOffsetHeading = (alignOffsetHeading - step + 360) % 360;
    });
    document.getElementById('jogRightBtn').addEventListener('click', () => {
      const step = parseFloat(stepSelect.value) || 1;
      alignOffsetHeading = (alignOffsetHeading + step + 360) % 360;
    });
    document.getElementById('jogUpBtn').addEventListener('click', () => {
      const step = parseFloat(stepSelect.value) || 1;
      alignOffsetElevation = Math.max(-45, Math.min(45, alignOffsetElevation + step));
    });
    document.getElementById('jogDownBtn').addEventListener('click', () => {
      const step = parseFloat(stepSelect.value) || 1;
      alignOffsetElevation = Math.max(-45, Math.min(45, alignOffsetElevation - step));
    });

    document.getElementById('zoomInBtn').addEventListener('click', () => {
      fieldOfView = Math.max(12, fieldOfView - 4);
    });
    document.getElementById('zoomOutBtn').addEventListener('click', () => {
      fieldOfView = Math.min(60, fieldOfView + 4);
    });

    function getSunAltitude(date, lat, lon) {
      const start = new Date(date.getFullYear(), 0, 0);
      const diff = (date - start) + ((start.getTimezoneOffset() - date.getTimezoneOffset()) * 60 * 1000);
      const n = Math.floor(diff / (1000 * 60 * 60 * 24));
      
      const declination = 23.45 * Math.sin((2 * Math.PI / 365) * (n - 81)) * Math.PI / 180;
      const b = (2 * Math.PI / 365) * (n - 81);
      const eot = 9.87 * Math.sin(2 * b) - 7.53 * Math.cos(b) - 1.5 * Math.sin(b);
      
      const hours = date.getHours() + date.getMinutes() / 60 + date.getSeconds() / 3600;
      const solarTime = hours + (4 * lon + eot) / 60;
      const hourAngle = (solarTime - 12) * 15 * Math.PI / 180;
      const latRad = lat * Math.PI / 180;
      
      const sinAlt = Math.sin(latRad) * Math.sin(declination) + Math.cos(latRad) * Math.cos(declination) * Math.cos(hourAngle);
      return Math.asin(Math.max(-1, Math.min(1, sinAlt))) * 180 / Math.PI;
    }

    function updateSkyBackground(date) {
      const sunAlt = getSunAltitude(date, latitude, longitude);

      const hexToRgb = (hex) => {
        const num = parseInt(hex.slice(1), 16);
        return [(num >> 16) & 255, (num >> 8) & 255, num & 255];
      };
      const rgbToHex = (r, g, b) => `#${((1 << 24) + (Math.round(r) << 16) + (Math.round(g) << 8) + Math.round(b)).toString(16).slice(1)}`;
      const lerp = (a, b, t) => a + (b - a) * Math.max(0, Math.min(1, t));
      const lerpRgb = (c1, c2, t) => [lerp(c1[0], c2[0], t), lerp(c1[1], c2[1], t), lerp(c1[2], c2[2], t)];

      const keyframes = [
        { alt: -18, top: '#020305', mid: '#05080f', bot: '#080c14' },
        { alt: -12, top: '#05080f', mid: '#0f1423', bot: '#1b1b3a' },
        { alt: -6,  top: '#090d16', mid: '#241b3b', bot: '#4a154b' },
        { alt: 0,   top: '#0f172a', mid: '#4a223f', bot: '#b23a1e' },
        { alt: 6,   top: '#1b365d', mid: '#2a6f97', bot: '#e07a5f' },
        { alt: 15,  top: '#0d47a1', mid: '#1565c0', bot: '#2979ff' }
      ];

      let k1 = keyframes[0], k2 = keyframes[keyframes.length - 1];
      for (let i = 0; i < keyframes.length - 1; i++) {
        if (sunAlt >= keyframes[i].alt && sunAlt <= keyframes[i+1].alt) {
          k1 = keyframes[i];
          k2 = keyframes[i+1];
          break;
        } else if (sunAlt < keyframes[0].alt) {
          k1 = keyframes[0]; k2 = keyframes[0];
          break;
        } else if (sunAlt > keyframes[keyframes.length - 1].alt) {
          k1 = keyframes[keyframes.length - 1]; k2 = keyframes[keyframes.length - 1];
          break;
        }
      }

      const t = k2.alt === k1.alt ? 0 : (sunAlt - k1.alt) / (k2.alt - k1.alt);
      const topColor = rgbToHex(...lerpRgb(hexToRgb(k1.top), hexToRgb(k2.top), t));
      const midColor = rgbToHex(...lerpRgb(hexToRgb(k1.mid), hexToRgb(k2.mid), t));
      const botColor = rgbToHex(...lerpRgb(hexToRgb(k1.bot), hexToRgb(k2.bot), t));

      sceneDiv.style.background = `radial-gradient(ellipse at 50% 100%, ${botColor} 0, ${midColor} 50%, ${topColor} 100%)`;
    }

    function checkOrientation() {
      if (window.innerHeight < window.innerWidth) {
        document.body.classList.add('landscape-fullscreen');
        if (!document.fullscreenElement) {
          document.documentElement.requestFullscreen().catch(() => {});
        }
      } else {
        document.body.classList.remove('landscape-fullscreen');
        if (document.fullscreenElement) {
          document.exitFullscreen().catch(() => {});
        }
      }
      resizeCanvas();
    }
    window.addEventListener('resize', checkOrientation);
    window.addEventListener('orientationchange', checkOrientation);

    document.getElementById('manualFullscreenBtn').addEventListener('click', () => {
      if (!document.fullscreenElement) {
        document.documentElement.requestFullscreen().catch(() => {});
      } else {
        document.exitFullscreen().catch(() => {});
      }
    });

    function setDateTimeInputValue(d) {
      const offsetMs = d.getTimezoneOffset() * 60000;
      const localIso = new Date(d.getTime() - offsetMs).toISOString().slice(0, 16);
      if (customTimeInput) customTimeInput.value = localIso;
    }

    function initDateTimeInput() {
      setDateTimeInputValue(new Date());
    }
    initDateTimeInput();

    customTimeInput.addEventListener('change', () => {
      useSystemTime = false;
      updateSkyBackground(getSkyDate());
    });

    document.getElementById('resetTimeBtn').addEventListener('click', () => {
      useSystemTime = true;
      initDateTimeInput();
      updateSkyBackground(getSkyDate());
    });

    function adjustTime(minutesDelta) {
      useSystemTime = false;
      const currentDate = getSkyDate();
      currentDate.setMinutes(currentDate.getMinutes() + minutesDelta);
      setDateTimeInputValue(currentDate);
      updateSkyBackground(currentDate);
    }

    document.getElementById('stepPrevHourBtn').addEventListener('click', () => adjustTime(-60));
    document.getElementById('stepPrev10mBtn').addEventListener('click', () => adjustTime(-10));
    document.getElementById('stepNext10mBtn').addEventListener('click', () => adjustTime(10));
    document.getElementById('stepNextHourBtn').addEventListener('click', () => adjustTime(60));

    function getSkyDate() {
      if (useSystemTime) return new Date();
      const val = customTimeInput.value;
      return val ? new Date(val) : new Date();
    }

    const CONSTELLATION_LINES = [
      ["Tarazed", "Altair"], ["Altair", "Alshain"], ["Altair", "Deneb el Okab"], ["Altair", "Eta Aql"], ["Eta Aql", "Theta Aql"],
      ["Alpheratz", "Delta And"], ["Delta And", "Mirach"], ["Mirach", "Almach"], ["Alpheratz", "Adhil"], ["Mirach", "51 And"],
      ["Zubenelgenubi", "Zubeneschamali"], ["Zubeneschamali", "Zubenelhakrabi"], ["Zubenelhakrabi", "Zubenelgenubi"],
      ["Hamal", "Sheratan"], ["Sheratan", "Mesarthim"], ["Hamal", "Botein"],
      ["Arcturus", "Muphrid"], ["Arcturus", "Izar"], ["Izar", "Seginus"], ["Seginus", "Nekkar"], ["Nekkar", "Hozian"], ["Hozian", "Izar"], ["Seginus", "Alkalurops"],
      ["Tarf", "Asellus Australis"], ["Asellus Australis", "Asellus Borealis"], ["Asellus Australis", "Acubens"], ["Asellus Borealis", "Iota Cnc"],
      ["Sadalmelik", "Sadalsuud"], ["Sadalmelik", "Sadachbia"], ["Sadachbia", "Skat"], ["Skat", "Ancha"],
      ["Caph", "Shedar"], ["Shedar", "Achird"], ["Shedar", "Navi"], ["Navi", "Ruchbah"], ["Ruchbah", "Segin"],
      ["Alderamin", "Alfirk"], ["Alfirk", "Errai"], ["Alderamin", "Zeta Cep"], ["Zeta Cep", "Delta Cep"], ["Delta Cep", "Iota Cep"], ["Iota Cep", "Errai"], ["Alderamin", "Kurhah"],
      ["Cor Caroli", "Chara"],
      ["Diadem", "Beta Com"], ["Beta Com", "Gamma Com"],
      ["Capella", "Menkalinan"], ["Menkalinan", "Mahasim"], ["Mahasim", "Hassaleh"], ["Capella", "Almaaz"], ["Almaaz", "Haedus"], ["Haedus", "Hassaleh"],
      ["Nusakan", "Alphecca"], ["Alphecca", "Gamma CrB"], ["Gamma CrB", "Epsilon CrB"],
      ["Deneb", "Sadr"], ["Sadr", "Albireo"], ["Fawaris", "Sadr"], ["Sadr", "Gienah"], ["Gienah", "Zeta Cyg"], ["Fawaris", "Iota Cyg"],
      ["Rotanev", "Sualocin"], ["Sualocin", "Gamma Del"], ["Gamma Del", "Delta Del"], ["Delta Del", "Rotanev"],
      ["Rastaban", "Eltanin"], ["Eltanin", "Grumium"], ["Grumium", "Kuma"], ["Kuma", "Rastaban"],
      ["Grumium", "Altais"], ["Altais", "Alsafi"], ["Alsafi", "Chi Dra"], ["Chi Dra", "Aldhibah"],
      ["Aldhibah", "Edasich"], ["Edasich", "Thuban"],
      ["Sham", "Beta Sge"], ["Beta Sge", "Gamma Sge"],
      ["Castor", "Pollux"], ["Castor", "Propus"], ["Propus", "Tejat"], ["Tejat", "Mebsuta"], ["Mebsuta", "Castor"],
      ["Pollux", "Wasat"], ["Wasat", "Mekbuda"], ["Mekbuda", "Alhena"], ["Wasat", "Alhena"],
      ["Beta Cam", "Alpha Cam"], ["Alpha Cam", "Gamma Cam"],
      ["Dubhe", "Merak"], ["Merak", "Phecda"], ["Phecda", "Megrez"], ["Megrez", "Dubhe"],
      ["Megrez", "Alioth"], ["Alioth", "Mizar"], ["Mizar", "Alkaid"],
      ["Kornephoros", "Rutilicus"], ["Rutilicus", "Eta Her"], ["Eta Her", "Pi Her"], ["Pi Her", "Epsilon Her"], ["Epsilon Her", "Rutilicus"],
      ["Epsilon Her", "Sujata"], ["Sujata", "Marfik"], ["Kornephoros", "Rasalgethi"],
      ["Alpha Lacertae", "Beta Lacertae"],
      ["Regulus", "Algieba"], ["Algieba", "Adhafera"], ["Adhafera", "Ras Elased Australis"], ["Ras Elased Australis", "Regulus"],
      ["Algieba", "Zosma"], ["Zosma", "Denebola"], ["Denebola", "Chertan"], ["Chertan", "Regulus"],
      ["Praecipua", "Beta LMi"],
      ["Alpha Lyncis", "38 Lyncis"],
      ["Vega", "Epsilon Lyr"], ["Vega", "Zeta Lyr"], ["Zeta Lyr", "Sheliak"], ["Sheliak", "Sulafat"], ["Sulafat", "Zeta Lyr"],
      ["Rasalhague", "Cebalrai"], ["Rasalhague", "Yed Prior"], ["Yed Prior", "Yed Posterior"], ["Yed Posterior", "Sabik"],
      ["Betelgeuse", "Bellatrix"], ["Betelgeuse", "Meissa"], ["Bellatrix", "Meissa"],
      ["Betelgeuse", "Mintaka"], ["Bellatrix", "Alnitak"],
      ["Mintaka", "Alnilam"], ["Alnilam", "Alnitak"],
      ["Alnitak", "Saiph"], ["Mintaka", "Rigel"], ["Rigel", "Saiph"], ["Alnilam", "Hatysa"],
      ["Betelgeuse", "μ Ori"], ["μ Ori", "ξ Ori"], ["ξ Ori", "ν Ori"], ["μ Ori", "χ² Ori"],
      ["Bellatrix", "π³ Ori"], ["π¹ Ori", "π² Ori"], ["π² Ori", "π³ Ori"], ["π³ Ori", "π⁴ Ori"], ["π⁴ Ori", "π⁵ Ori"], ["π⁵ Ori", "π⁶ Ori"],
      ["Markab", "Scheat"], ["Scheat", "Alpheratz"], ["Alpheratz", "Algenib"], ["Algenib", "Markab"],
      ["Markab", "Homam"], ["Homam", "Enif"], ["Scheat", "Matar"], ["Matar", "Sadalbari"],
      ["Mirfak", "Algol"], ["Algol", "Atik"], ["Atik", "Menkib"], ["Mirfak", "Delta Per"], ["Delta Per", "Epsilon Per"], ["Mirfak", "Gamma Per"],
      ["Kitalpha", "Beta Equ"],
      ["Polaris", "Yildun"], ["Yildun", "Zeta UMi"], ["Zeta UMi", "Epsilon UMi"],
      ["Epsilon UMi", "Pherkad"], ["Pherkad", "Kochab"], ["Kochab", "Zeta UMi"], ["Kochab", "Eta UMi"],
      ["Alpherg", "Gamma Piscium"], ["Gamma Piscium", "Iota Psc"], ["Iota Psc", "Omega Psc"], ["Omega Psc", "Alrescha"],
      ["Unukalhai", "Beta Ser"], ["Unukalhai", "Alya"],
      ["Aldebaran", "Ain"], ["Ain", "Elnath"], ["Aldebaran", "Gamma Tau"], ["Aldebaran", "Tianguis"], ["Aldebaran", "Alcyone"],
      ["Beta Trianguli", "Mothallah"], ["Mothallah", "Gamma Tri"], ["Gamma Tri", "Beta Trianguli"],
      ["Spica", "Heze"], ["Heze", "Vindemiatrix"], ["Vindemiatrix", "Porrima"], ["Porrima", "Zavijava"], ["Porrima", "Spica"],
      ["Sirius", "Mirzam"], ["Sirius", "Adhara"], ["Adhara", "Wezen"], ["Wezen", "Sirius"]
    ];

    const CONSTELLATION_CENTERS = [
      { name: "Aigle", refStar: "Altair" },
      { name: "Andromède", refStar: "Mirach" },
      { name: "Balance", refStar: "Zubenelgenubi" },
      { name: "Bélier", refStar: "Hamal" },
      { name: "Bouvier", refStar: "Arcturus" },
      { name: "Cancer", refStar: "Acubens" },
      { name: "Capricorne", refStar: "Sadalmelik" },
      { name: "Cassiopée", refStar: "Navi" },
      { name: "Céphée", refStar: "Alderamin" },
      { name: "Chevelure de Bérénice", refStar: "Diadem" },
      { name: "Chiens de chasse", refStar: "Cor Caroli" },
      { name: "Cocher", refStar: "Capella" },
      { name: "Couronne boréale", refStar: "Alphecca" },
      { name: "Cygne", refStar: "Sadr" },
      { name: "Dauphin", refStar: "Rotanev" },
      { name: "Dragon", refStar: "Eltanin" },
      { name: "Flèche", refStar: "Sham" },
      { name: "Gémeaux", refStar: "Pollux" },
      { name: "Girafe", refStar: "Beta Cam" },
      { name: "Grande Ourse", refStar: "Alioth" },
      { name: "Hercule", refStar: "Kornephoros" },
      { name: "Lézard", refStar: "Alpha Lacertae" },
      { name: "Lion", refStar: "Regulus" },
      { name: "Petit Lion", refStar: "Praecipua" },
      { name: "Lyre", refStar: "Vega" },
      { name: "Lynx", refStar: "Alpha Lyncis" },
      { name: "Ophiuchus", refStar: "Rasalhague" },
      { name: "Orion", refStar: "Betelgeuse" },
      { name: "Pégase", refStar: "Markab" },
      { name: "Persée", refStar: "Mirfak" },
      { name: "Petit Cheval", refStar: "Kitalpha" },
      { name: "Petite Ourse", refStar: "Polaris" },
      { name: "Poissons", refStar: "Alpherg" },
      { name: "Serpent", refStar: "Unukalhai" },
      { name: "Taureau", refStar: "Aldebaran" },
      { name: "Triangle", refStar: "Mothallah" },
      { name: "Vierge", refStar: "Spica" },
      { name: "Petit Chien", refStar: "Procyon" },
      { name: "Grand Chien", refStar: "Sirius" }
    ];

    function setStatus(message, type = '') {
      statusElement.textContent = message;
      statusElement.className = 'sky-status ' + type;
    }

    function getEnrichedDetails(obj) {
      let dist = "Indéterminée";
      let size = "15'";
      const name = (obj.name || "").toUpperCase();
      const type = (obj.type || "").toLowerCase();

      if (name.includes("M31")) { dist = "2.5 millions al"; size = "3.0° × 1.0°"; }
      else if (name.includes("M45")) { dist = "440 al"; size = "110'"; }
      else if (name.includes("M42")) { dist = "1 340 al"; size = "65' × 60'"; }
      else if (name.includes("M13")) { dist = "22 200 al"; size = "20'"; }
      else if (name.includes("M57")) { dist = "2 300 al"; size = "1.4' × 1.0'"; }
      else if (type.includes("galaxie")) { dist = "15 - 50 millions al"; size = "8' × 5'"; }
      else if (type.includes("nebuleuse")) { dist = "2 000 - 5 000 al"; size = "30'"; }
      else if (type.includes("amas")) { dist = "5 000 - 15 000 al"; size = "15'"; }
      else { dist = "10 - 500 al (Étoile)"; size = "Ponctuel"; }

      return { dist, size };
    }

    function applyEncoderStatus(status) {
      const rawAZ = Number(status.rawAZ);
      const rawALT = Number(status.rawALT);
      const ticksAZ = Number(status.ticksAZ);
      const ticksALT = Number(status.ticksALT);
      const dirAZ = Number(status.dirAZ);
      const dirALT = Number(status.dirALT);
      if (![rawAZ, rawALT, ticksAZ, ticksALT, dirAZ, dirALT].every(Number.isFinite)
        || ticksAZ <= 0 || ticksALT <= 0 || ![1, -1].includes(dirAZ) || ![1, -1].includes(dirALT)) {
        throw new Error('Données encodeurs invalides.');
      }

      if (lastRawAZ !== null && lastRawALT !== null) {
        if (rawAZ !== lastRawAZ || rawALT !== lastRawALT) {
          touchHeading = 0;
          touchElevation = 0;
        }
      }
      lastRawAZ = rawAZ;
      lastRawALT = rawALT;

      const azimuth = ((rawAZ * dirAZ / ticksAZ * 360) % 360 + 360) % 360;
      const altitudeCircle = ((rawALT * dirALT / ticksALT * 360) % 360 + 360) % 360;
      
      targetHeading = azimuth;
      targetElevation = altitudeCircle > 180 ? altitudeCircle - 360 : altitudeCircle;
      
      telescopeReadout.textContent = `Télescope AZ ${azimuth.toFixed(1)}° · ALT ${targetElevation.toFixed(1)}°`;
      telescopeReadout.classList.remove('error');
    }

    async function pollEncoderStatus() {
      if (encoderRequestInFlight) return;
      encoderRequestInFlight = true;
      try {
        const response = await fetch('/status', { cache: 'no-store' });
        if (!response.ok) throw new Error(`/status HTTP ${response.status}`);
        applyEncoderStatus(await response.json());
      } catch (error) {
        telescopeReadout.textContent = `Encodeurs hors ligne`;
        telescopeReadout.classList.add('error');
      } finally {
        encoderRequestInFlight = false;
      }
    }

    function setNightMode(enabled, persist = false) {
      document.documentElement.dataset.nightMode = String(enabled);
      const button = document.getElementById('nightModeToggle');
      const label = enabled ? 'Désactiver mode nuit' : 'Activer mode nuit';
      button.setAttribute('aria-label', label);
      button.setAttribute('aria-pressed', String(enabled));
      button.title = label;
      if (persist) localStorage.setItem('dobson-night-mode', String(enabled));
    }

    function toggleNightMode() {
      setNightMode(document.documentElement.dataset.nightMode !== 'true', true);
    }

    setNightMode(localStorage.getItem('dobson-night-mode') === 'true');

    function objectColorHex(object) {
      const type = (object.type || '').toUpperCase();
      const name = (object.name || '').toUpperCase();

      if (name === 'POLARIS') return '#ffedcc';
      if (type.includes('PLAN')) return '#80d8ff';
      if (type.includes('GAL')) return '#e0b8ff';
      if (type.includes('NEB')) return '#ff9e84';
      if (type.includes('AMAS') || type.includes('GLOB')) return '#ffe066';

      if (type.includes('O') || type.includes('B')) return '#9ec5ff';
      if (type.includes('A')) return '#d8eaff';
      if (type.includes('F')) return '#fffde6';
      if (type.includes('G')) return '#ffee66';
      if (type.includes('K')) return '#ffad42';
      if (type.includes('M')) return '#ff6b42';

      if (object.mag < 1.0) return '#ffffff';
      if (object.mag < 2.5) return '#e0f0ff';
      return '#c2dcff';
    }

    function getObjectType(object) {
      const type = (object.type || '').toLowerCase();
      const isGal = type.includes('gal');
      const isNeb = type.includes('neb');
      const isAmas = type.includes('amas') || type.includes('glob');
      const isStar = !isGal && !isNeb && !isAmas;
      return { isGal, isNeb, isAmas, isStar };
    }

    function matchesFilter(object) {
      const maxMag = parseFloat(magFilterElement.value);
      if (object.mag > maxMag) return false;

      const { isGal, isNeb, isAmas, isStar } = getObjectType(object);
      
      const showGal = document.getElementById('showGal').checked;
      const showNeb = document.getElementById('showNeb').checked;
      const showAmas = document.getElementById('showAmas').checked;
      const showStars = document.getElementById('showStars').checked;

      if (isGal && showGal) return true;
      if (isNeb && showNeb) return true;
      if (isAmas && showAmas) return true;
      if (isStar && showStars) return true;

      return false;
    }

    function horizontalPosition(object, siderealDegrees, latRadians) {
      const dec = object.dec * Math.PI / 180;
      const hourAngle = (siderealDegrees - object.ra * 15) * Math.PI / 180;
      const sinAltitude = Math.sin(dec) * Math.sin(latRadians) + Math.cos(dec) * Math.cos(latRadians) * Math.cos(hourAngle);
      const altitude = Math.asin(Math.max(-1, Math.min(1, sinAltitude)));
      const azimuth = Math.atan2(
        -Math.sin(hourAngle) * Math.cos(dec),
        Math.sin(dec) * Math.cos(latRadians) - Math.cos(dec) * Math.sin(latRadians) * Math.cos(hourAngle),
      );
      const cosAltitude = Math.cos(altitude);
      return {
        altitude: altitude * 180 / Math.PI,
        azimuth: (azimuth * 180 / Math.PI + 360) % 360,
        position: [Math.sin(azimuth) * cosAltitude, Math.sin(altitude), Math.cos(azimuth) * cosAltitude],
      };
    }

    function siderealDegrees(date) {
      const julianDate = date.getTime() / 86400000 + 2440587.5;
      const centuries = (julianDate - 2451545.0) / 36525;
      const greenwich = 280.46061837 + 360.98564736629 * (julianDate - 2451545.0)
        + 0.000387933 * centuries * centuries - centuries * centuries * centuries / 38710000;
      return ((greenwich + longitude) % 360 + 360) % 360;
    }

    function resizeCanvas() {
      const ratio = Math.min(window.devicePixelRatio || 1, 2);
      const width = Math.max(1, Math.round(canvas.clientWidth * ratio));
      const height = Math.max(1, Math.round(canvas.clientHeight * ratio));
      if (canvas.width !== width || canvas.height !== height) {
        canvas.width = width;
        canvas.height = height;
        ctx2d.setTransform(ratio, 0, 0, ratio, 0, 0);
      }
    }

    function render() {
      requestAnimationFrame(render);
      if (objects.length === 0) return;
      resizeCanvas();
      
      const width = canvas.clientWidth;
      const height = canvas.clientHeight;
      const aspect = width / height;
      
      let diffAZ = targetHeading - heading;
      if (diffAZ > 180) diffAZ -= 360;
      if (diffAZ < -180) diffAZ += 360;
      heading = (heading + diffAZ * 0.4 + 360) % 360;
      elevation += (targetElevation - elevation) * 0.4;

      const currentHeading = (heading + touchHeading + alignOffsetHeading + 360) % 360;
      const currentElevation = Math.max(-5, Math.min(88, elevation + touchElevation + alignOffsetElevation));
      
      const az = currentHeading * Math.PI / 180;
      const el = currentElevation * Math.PI / 180;
      
      const forward = [Math.sin(az) * Math.cos(el), Math.sin(el), Math.cos(az) * Math.cos(el)];
      
      const worldUp = [0, 1, 0];
      let rx = worldUp[1] * forward[2] - worldUp[2] * forward[1];
      let ry = worldUp[2] * forward[0] - worldUp[0] * forward[2];
      let rz = worldUp[0] * forward[1] - worldUp[1] * forward[0];
      const rLen = Math.hypot(rx, ry, rz) || 1;
      const right = [rx / rLen, ry / rLen, rz / rLen];
      
      const up = [
        forward[1] * right[2] - forward[2] * right[1],
        forward[2] * right[0] - forward[0] * right[2],
        forward[0] * right[1] - forward[1] * right[0]
      ];
      
      const tanHalfFov = Math.tan(fieldOfView * Math.PI / 180);
      
      const currentSkyDate = getSkyDate();
      const sidereal = siderealDegrees(currentSkyDate);
      const latRadians = latitude * Math.PI / 180;

      // --- GUIDAGE CIBLE 2 ÉTAPES SIMULTANÉES (AZIMUT + ALTITUDE) ---
      const arrowOverlay = document.getElementById('searchArrowOverlay');
      if (activeTargetObject) {
        const targetHoriz = horizontalPosition(activeTargetObject, sidereal, latRadians);
        
        let dAz = targetHoriz.azimuth - currentHeading;
        if (dAz > 180) dAz -= 360;
        if (dAz < -180) dAz += 360;
        
        let dEl = targetHoriz.altitude - currentElevation;
        const angularDistance = Math.hypot(dAz * Math.cos(currentElevation * Math.PI / 180), dEl);

        const targetNameLabel = document.getElementById('targetNameLabel');
        const azArrow = document.getElementById('azArrow');
        const altArrow = document.getElementById('altArrow');
        const angularDistText = document.getElementById('angularDistText');

        targetNameLabel.textContent = activeTargetObject.name;
        angularDistText.textContent = `${angularDistance.toFixed(1)}°`;

        if (angularDistance < 1.0) {
          azArrow.textContent = '🎯';
          altArrow.textContent = '✨';
          angularDistText.style.color = '#69ff84';
          targetNameLabel.style.color = '#69ff84';
        } else {
          angularDistText.style.color = '#aab3c2';
          targetNameLabel.style.color = '#fff';

          // Flèche Azimut (Gauche / Droite)
          if (Math.abs(dAz) > 0.5) {
            // Si dAz > 0, l'objet est à l'est, il faut tourner à droite (ou inversement selon le sens des encodeurs)
            azArrow.textContent = dAz > 0 ? '➡️' : '⬅️';
            azArrow.style.opacity = '1';
          } else {
            azArrow.textContent = '✔️';
            azArrow.style.opacity = '0.4';
          }

          // Flèche Altitude (Haut / Bas)
          if (Math.abs(dEl) > 0.5) {
            altArrow.textContent = dEl > 0 ? '⬆️' : '⬇️';
            altArrow.style.opacity = '1';
          } else {
            altArrow.textContent = '✔️';
            altArrow.style.opacity = '0.4';
          }
        }
      }

      const objectMap = new Map();
      for (const object of objects) {
        const horizontal = horizontalPosition(object, sidereal, latRadians);
        objectMap.set(object.name, horizontal.position);
      }

      const cardinalPoints = [
        { id: 'labelN', az: 0 }, { id: 'labelE', az: 90 },
        { id: 'labelS', az: 180 }, { id: 'labelW', az: 270 }
      ];
      cardinalPoints.forEach(cp => {
        const azRad = cp.az * Math.PI / 180;
        const pos = [Math.sin(azRad), 0, Math.cos(azRad)];
        const depth = pos[0] * forward[0] + pos[1] * forward[1] + pos[2] * forward[2];
        const elEl = document.getElementById(cp.id);
        if (depth > 0.05) {
          const sx = (pos[0] * right[0] + pos[1] * right[1] + pos[2] * right[2]) / (depth * tanHalfFov * aspect);
          const sy = (pos[0] * up[0] + pos[1] * up[1] + pos[2] * up[2]) / (depth * tanHalfFov);
          if (Math.abs(sx) <= 1.1 && Math.abs(sy) <= 1.1) {
            elEl.style.left = `${(sx + 1) * 50}%`;
            elEl.style.top = `${(1 - sy) * 50}%`;
            elEl.style.display = 'block';
          } else {
            elEl.style.display = 'none';
          }
        } else {
          elEl.style.display = 'none';
        }
      });

      ctx2d.clearRect(0, 0, width, height);

      const showGrid = document.getElementById('showGrid').checked;
      if (showGrid) {
        for (let altDeg = 0; altDeg <= 80; altDeg += 20) {
          const altRad = altDeg * Math.PI / 180;
          if (altDeg === 0) {
            ctx2d.strokeStyle = "rgba(0, 220, 255, 0.9)";
            ctx2d.lineWidth = 2.2;
          } else {
            ctx2d.strokeStyle = "rgba(0, 160, 220, 0.35)";
            ctx2d.lineWidth = 1.2;
          }
          
          let ptsDrawn = 0;
          ctx2d.beginPath();
          for (let aDeg = 0; aDeg <= 360; aDeg += 5) {
            const aRad = aDeg * Math.PI / 180;
            const pos = [Math.sin(aRad) * Math.cos(altRad), Math.sin(altRad), Math.cos(aRad) * Math.cos(altRad)];
            const depth = pos[0] * forward[0] + pos[1] * forward[1] + pos[2] * forward[2];
            if (depth > 0.02) {
              const sx = width / 2 + ((pos[0] * right[0] + pos[1] * right[1] + pos[2] * right[2]) / (depth * tanHalfFov * aspect)) * (width / 2);
              const sy = height / 2 - ((pos[0] * up[0] + pos[1] * up[1] + pos[2] * up[2]) / (depth * tanHalfFov)) * (height / 2);
              if (ptsDrawn === 0) ctx2d.moveTo(sx, sy);
              else ctx2d.lineTo(sx, sy);
              ptsDrawn++;
            } else {
              ptsDrawn = 0;
            }
          }
          if (ptsDrawn > 1) ctx2d.stroke();
        }

        for (let azDeg = 0; azDeg < 360; azDeg += 45) {
          const azRad = azDeg * Math.PI / 180;
          ctx2d.strokeStyle = "rgba(0, 160, 220, 0.35)";
          ctx2d.lineWidth = 1.2;
          let ptsDrawn = 0;
          ctx2d.beginPath();
          for (let altDeg = 0; altDeg <= 90; altDeg += 5) {
            const altRad = altDeg * Math.PI / 180;
            const pos = [Math.sin(azRad) * Math.cos(altRad), Math.sin(altRad), Math.cos(azRad) * Math.cos(altRad)];
            const depth = pos[0] * forward[0] + pos[1] * forward[1] + pos[2] * forward[2];
            if (depth > 0.02) {
              const sx = width / 2 + ((pos[0] * right[0] + pos[1] * right[1] + pos[2] * right[2]) / (depth * tanHalfFov * aspect)) * (width / 2);
              const sy = height / 2 - ((pos[0] * up[0] + pos[1] * up[1] + pos[2] * up[2]) / (depth * tanHalfFov)) * (height / 2);
              if (ptsDrawn === 0) ctx2d.moveTo(sx, sy);
              else ctx2d.lineTo(sx, sy);
              ptsDrawn++;
            } else {
              ptsDrawn = 0;
            }
          }
          if (ptsDrawn > 1) ctx2d.stroke();
        }
      }

      const showConstellations = document.getElementById('showConstellations').checked;
      if (showConstellations) {
        ctx2d.strokeStyle = "rgba(255, 165, 80, 0.45)";
        ctx2d.lineWidth = 1.0;
        for (const [starA, starB] of CONSTELLATION_LINES) {
          const posA = objectMap.get(starA);
          const posB = objectMap.get(starB);
          if (posA && posB) {
            const depthA = posA[0] * forward[0] + posA[1] * forward[1] + posA[2] * forward[2];
            const depthB = posB[0] * forward[0] + posB[1] * forward[1] + posB[2] * forward[2];
            if (depthA > 0 && depthB > 0) {
              const sxA = width / 2 + ((posA[0] * right[0] + posA[1] * right[1] + posA[2] * right[2]) / (depthA * tanHalfFov * aspect)) * (width / 2);
              const syA = height / 2 - ((posA[0] * up[0] + posA[1] * up[1] + posA[2] * up[2]) / (depthA * tanHalfFov)) * (height / 2);
              const sxB = width / 2 + ((posB[0] * right[0] + posB[1] * right[1] + posB[2] * right[2]) / (depthB * tanHalfFov * aspect)) * (width / 2);
              const syB = height / 2 - ((posB[0] * up[0] + posB[1] * up[1] + posB[2] * up[2]) / (depthB * tanHalfFov)) * (height / 2);
              
              ctx2d.beginPath();
              ctx2d.moveTo(sxA, syA);
              ctx2d.lineTo(sxB, syB);
              ctx2d.stroke();
            }
          }
        }
      }

      lastScreenObjects = [];

      for (const object of objects) {
        if (!matchesFilter(object)) continue;
        const horizontal = horizontalPosition(object, sidereal, latRadians);
        if (horizontal.altitude < 0) continue;
        
        const depth = horizontal.position[0] * forward[0] + horizontal.position[1] * forward[1] + horizontal.position[2] * forward[2];
        if (depth <= 0.0) continue;
        
        const screenXNorm = (horizontal.position[0] * right[0] + horizontal.position[1] * right[1] + horizontal.position[2] * right[2]) / (depth * tanHalfFov * aspect);
        const screenYNorm = (horizontal.position[0] * up[0] + horizontal.position[1] * up[1] + horizontal.position[2] * up[2]) / (depth * tanHalfFov);
        
        if (Math.abs(screenXNorm) > 1.2 || Math.abs(screenYNorm) > 1.2) continue;
        
        const sx = width / 2 + screenXNorm * (width / 2);
        const sy = height / 2 - screenYNorm * (height / 2);
        
        const type = (object.type || '').toLowerCase();
        const colorHex = objectColorHex(object);

        ctx2d.save();
        
        if (type.includes('gal')) {
          ctx2d.strokeStyle = colorHex;
          ctx2d.fillStyle = colorHex;
          ctx2d.lineWidth = 1.2;
          ctx2d.beginPath();
          ctx2d.ellipse(sx, sy, 7, 3.5, Math.PI / 4, 0, 2 * Math.PI);
          ctx2d.stroke();
          ctx2d.globalAlpha = 0.3;
          ctx2d.fill();
        } else if (type.includes('neb')) {
          ctx2d.strokeStyle = colorHex;
          ctx2d.fillStyle = colorHex;
          ctx2d.lineWidth = 1.2;
          ctx2d.beginPath();
          ctx2d.moveTo(sx, sy - 6);
          ctx2d.lineTo(sx + 6, sy);
          ctx2d.lineTo(sx, sy + 6);
          ctx2d.lineTo(sx - 6, sy);
          ctx2d.closePath();
          ctx2d.stroke();
          ctx2d.globalAlpha = 0.25;
          ctx2d.fill();
        } else if (type.includes('amas') || type.includes('glob')) {
          ctx2d.strokeStyle = colorHex;
          ctx2d.lineWidth = 1.2;
          ctx2d.setLineDash([2, 2]);
          ctx2d.beginPath();
          ctx2d.arc(sx, sy, 5.5, 0, 2 * Math.PI);
          ctx2d.stroke();
          ctx2d.setLineDash([]);
        } else {
          let starSize = Math.max(0.7, Math.min(4.5, (8.5 - object.mag) * 0.5 + 0.8));
          if (object.name === "Polaris") starSize = 2.8;
          const zoomFactor = 34 / fieldOfView;
          starSize *= Math.min(zoomFactor, 2.0);

          ctx2d.fillStyle = colorHex;
          ctx2d.shadowColor = colorHex;
          ctx2d.shadowBlur = object.mag < 2.0 ? starSize * 2.5 : starSize * 1.2;
          ctx2d.beginPath();
          ctx2d.arc(sx, sy, starSize, 0, 2 * Math.PI);
          ctx2d.fill();
        }

        ctx2d.restore();
        lastScreenObjects.push({ object, screenX: screenXNorm, screenY: screenYNorm });
      }

      viewReadout.textContent = `Vue AZ ${Math.round(currentHeading)}° · ALT ${Math.round(currentElevation)}°`;

      const labels = document.createDocumentFragment();
      const labelGal = document.getElementById('labelGal').checked;
      const labelNeb = document.getElementById('labelNeb').checked;
      const labelAmas = document.getElementById('labelAmas').checked;
      const labelStars = document.getElementById('labelStars').checked;

      for (const item of lastScreenObjects) {
        const isPolaris = (item.object.name === "Polaris");
        const { isGal, isNeb, isAmas, isStar } = getObjectType(item.object);

        let showThisLabel = false;
        if (isPolaris) showThisLabel = true;
        else if (isGal && labelGal) showThisLabel = true;
        else if (isNeb && labelNeb) showThisLabel = true;
        else if (isAmas && labelAmas) showThisLabel = true;
        else if (isStar && labelStars && item.object.mag <= 8.0) showThisLabel = true;

        if (!showThisLabel) continue;

        const label = document.createElement('span');
        label.className = isPolaris ? 'object-label polaris' : 'object-label';
        
        let leftPct = (item.screenX + 1) * 50;
        let topPct = (1 - item.screenY) * 50;
        
        label.style.left = `${leftPct}%`;
        label.style.top = `${topPct}%`;
        label.style.transform = `translate(0px, -16px)`;
        
        if (isPolaris) {
          label.textContent = `★ Polaris (Pôle Nord)`;
        } else if (item.object.name.startsWith("M") || item.object.name.startsWith("NGC")) {
          label.textContent = `${item.object.name} (${item.object.commonName})`;
        } else {
          label.textContent = `${item.object.name} (${item.object.commonName})`;
        }
        
        labels.appendChild(label);
      }
      labelsElement.replaceChildren(labels);

      const constLabels = document.createDocumentFragment();
      const labelConstellations = document.getElementById('labelConstellations').checked;
      if (labelConstellations) {
        for (const c of CONSTELLATION_CENTERS) {
          const pos = objectMap.get(c.refStar);
          if (pos) {
            const depth = pos[0] * forward[0] + pos[1] * forward[1] + pos[2] * forward[2];
            if (depth > 0.01) {
              const screenX = (pos[0] * right[0] + pos[1] * right[1] + pos[2] * right[2]) / (depth * tanHalfFov * aspect);
              const screenY = (pos[0] * up[0] + pos[1] * up[1] + pos[2] * up[2]) / (depth * tanHalfFov);
              if (Math.abs(screenX) <= 1 && Math.abs(screenY) <= 1) {
                const cl = document.createElement('span');
                cl.className = 'constellation-label';
                cl.style.left = `${(screenX + 1) * 50}%`;
                cl.style.top = `${(1 - screenY) * 50}%`;
                cl.textContent = c.name;
                constLabels.appendChild(cl);
              }
            }
          }
        }
      }
      constLabelsElement.replaceChildren(constLabels);
    }

    async function initialize() {
      try {
        await requestWakeLock();
        await loadSavedSkyPrefs();
        resizeCanvas();

        const [catalogRes, statusRes] = await Promise.all([fetch('/catalog'), fetch('/status')]);
        objects = await catalogRes.json();
        const status = await statusRes.json();
        
        applyEncoderStatus(status);
        heading = targetHeading;
        elevation = targetElevation;

        latitude = Number(status.lat ?? latitude);
        longitude = Number(status.lon ?? longitude);
        
        updateSkyBackground(getSkyDate());
        setStatus(`${objects.length} objets chargés. Planétarium prêt.`, "align-ok");
        
        window.setInterval(pollEncoderStatus, 100);
        render();
      } catch (error) {
        setStatus(error.message, true);
      }
    }

    canvas.addEventListener('click', event => {
      const rect = canvas.getBoundingClientRect();
      const x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
      const y = -(((event.clientY - rect.top) / rect.height) * 2 - 1);

      let closestObj = null;
      let minDst = 0.08;

      for (const item of lastScreenObjects) {
        const dst = Math.hypot(item.screenX - x, item.screenY - y);
        if (dst < minDst) {
          minDst = dst;
          closestObj = item.object;
        }
      }

      if (closestObj) {
        const details = getEnrichedDetails(closestObj);
        document.getElementById('modalTitle').textContent = `${closestObj.name} (${closestObj.commonName})`;
        document.getElementById('modalSub').textContent = `${closestObj.type || 'Objet céleste'}`;
        document.getElementById('modalMag').textContent = closestObj.mag.toFixed(1);
        document.getElementById('modalDist').textContent = details.dist;
        document.getElementById('modalSize').textContent = details.size;
        document.getElementById('modalCoords').textContent = `AD ${closestObj.ra}h · Dec ${closestObj.dec}°`;
        document.getElementById('objectModal').hidden = false;
      }
    });

    function closeModal() {
      document.getElementById('objectModal').hidden = true;
    }

    let dragPoint = null;
    const activePointers = new Map();
    let pinchDistance = null;
    const pointerDistance = () => {
      const [first, second] = [...activePointers.values()];
      return first && second ? Math.hypot(first.x - second.x, first.y - second.y) : null;
    };
    
    const scene = document.getElementById('scene');
    scene.addEventListener('pointerdown', event => {
      activePointers.set(event.pointerId, { x: event.clientX, y: event.clientY });
      dragPoint = { x: event.clientX, y: event.clientY };
      scene.setPointerCapture(event.pointerId);
      pinchDistance = activePointers.size === 2 ? pointerDistance() : null;
    });
    scene.addEventListener('pointermove', event => {
      if (activePointers.has(event.pointerId)) activePointers.set(event.pointerId, { x: event.clientX, y: event.clientY });
      if (activePointers.size >= 2) {
        const distance = pointerDistance();
        if (distance && pinchDistance) fieldOfView = Math.max(12, Math.min(60, fieldOfView * pinchDistance / distance));
        pinchDistance = distance;
        dragPoint = null;
        return;
      }
      if (!dragPoint) return;
      touchHeading = (touchHeading - (event.clientX - dragPoint.x) * 0.35 + 360) % 360;
      touchElevation = Math.max(-80, Math.min(80, touchElevation + (event.clientY - dragPoint.y) * 0.25));
      dragPoint = { x: event.clientX, y: event.clientY };
    });
    const releasePointer = event => {
      activePointers.delete(event.pointerId);
      pinchDistance = null;
      dragPoint = activePointers.values().next().value || null;
    };
    scene.addEventListener('pointerup', releasePointer);
    scene.addEventListener('pointercancel', releasePointer);
    scene.addEventListener('wheel', event => {
      event.preventDefault();
      fieldOfView = Math.max(12, Math.min(60, fieldOfView + Math.sign(event.deltaY) * 2));
    }, { passive: false });

    const toggleAllMaskBtn = document.getElementById('toggleAllMaskBtn');
    let allMaskedState = false;

    toggleAllMaskBtn.addEventListener('click', () => {
      allMaskedState = !allMaskedState;
      
      document.getElementById('showGrid').checked = !allMaskedState;
      document.getElementById('showGal').checked = false;
      document.getElementById('showNeb').checked = false;
      document.getElementById('showAmas').checked = false;
      document.getElementById('showStars').checked = true;

      document.getElementById('labelGal').checked = false;
      document.getElementById('labelNeb').checked = false;
      document.getElementById('labelAmas').checked = false;
      document.getElementById('labelStars').checked = false;

      document.getElementById('showConstellations').checked = false;
      document.getElementById('labelConstellations').checked = false;

      document.querySelectorAll('.settings-row input[type="checkbox"][data-name-id]').forEach(objChk => {
        const nameChk = document.getElementById(objChk.dataset.nameId);
        nameChk.disabled = !objChk.checked;
      });

      toggleAllMaskBtn.textContent = allMaskedState ? "Restaurer" : "Tout masquer";
      saveSkyPrefs();
    });

    document.getElementById('centerButton').addEventListener('click', () => {
      touchHeading = 0;
      touchElevation = 0;
      alignOffsetHeading = 0;
      alignOffsetElevation = 0;
      fieldOfView = 34;
    });

    window.addEventListener('resize', resizeCanvas);
    checkOrientation();
    initialize();
  </script>
</body>
</html>
)rawliteral";

#endif