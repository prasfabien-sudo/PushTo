// Version Component: BetaSkyPage.h v5.73.0
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
  <title>Vue du ciel — Planétarium</title>
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
    
    .scene { position: relative; height: min(55vh, 450px); min-height: 280px; overflow: hidden; border: 1px solid #273142; border-radius: 14px; background: radial-gradient(ellipse at 50% 100%, #1b2638 0, #0d1420 47%, #05070b 100%); touch-action: none; transition: height 0.3s, background 0.5s; }
    canvas { display: block; width: 100%; height: 100%; cursor: grab; position: relative; z-index: 1; background: transparent; }
    canvas:active { cursor: grabbing; }
    .scene-label { position: absolute; z-index: 2; color: #acb8c8; font-size: .72rem; text-shadow: 0 1px 4px #000; pointer-events: none; }
    .scene-label.cardinal { font-weight: bold; color: #ffb74d; background: rgba(5,8,12,0.6); padding: 2px 6px; border-radius: 4px; }
    .scene-label.telescope { top: 10px; left: 10px; }
    .scene-label.telescope.error { color: #ff7777; }
    .scene-label.view { left: 10px; bottom: 10px; }
    .scene-search-btn { position: absolute; right: 10px; bottom: 10px; z-index: 5; width: 40px; height: 40px; display: flex; align-items: center; justify-content: center; padding: 0; background: rgba(21, 27, 37, 0.85); border: 1px solid #344256; border-radius: 8px; color: #ffb74d; cursor: pointer; backdrop-filter: blur(4px); }
    .scene-search-btn .ui-icon { width: 22px; height: 22px; }
    .scene-search-btn[aria-pressed="true"] { background: rgba(105, 255, 132, 0.18); border-color: #69ff84; }
    .scene-center { position: absolute; top: 50%; left: 50%; width: 8px; height: 8px; border: 1px solid rgba(255,255,255,.7); border-radius: 50%; transform: translate(-50%,-50%); pointer-events: none; z-index: 2; }
    
    .scene-zoom-controls { position: absolute; top: 10px; right: 10px; z-index: 5; display: flex; flex-direction: column; gap: 4px; }
    .zoom-btn { width: 32px; height: 32px; background: rgba(21, 27, 37, 0.85); border: 1px solid #344256; border-radius: 6px; color: #ffb74d; font-size: 1.1rem; font-weight: bold; display: flex; align-items: center; justify-content: center; cursor: pointer; backdrop-filter: blur(4px); padding: 0; }
    .zoom-btn:active { background: rgba(37, 50, 71, 0.95); transform: scale(0.95); }

    .object-label { position: absolute; left: 0; top: 0; z-index: 2; padding: 1px 3px; border-radius: 4px; background: transparent; font-size: 0.65rem; white-space: nowrap; text-shadow: 0 1px 3px #000, 0 0 3px #000, 0 0 5px #000; pointer-events: none; transition: opacity 0.15s ease-out; }
    .object-label.polaris { color: #ffb74d !important; font-weight: bold; z-index: 3; }
    .constellation-label { position: absolute; left: 0; top: 0; z-index: 2; color: #ffb366; font-size: .75rem; font-weight: bold; letter-spacing: 0.05em; text-transform: uppercase; text-shadow: 0 1px 5px #000; pointer-events: none; opacity: 0; transition: opacity 0.15s ease-out; }

    .controls { display: grid; grid-template-columns: repeat(4, minmax(0,1fr)); gap: 6px; margin-top: 8px; }
    
    .scene-action-toolbar { grid-column: span 4; display: grid; grid-template-columns: repeat(2, minmax(0,1fr)); gap: 6px; }
    .scene-action-btn { min-height: 48px; display: flex; align-items: center; justify-content: center; padding: 6px; background: #263238; color: #ffb74d; border: 1px solid #455a64; }
    .scene-action-btn .ui-icon { width: 26px; height: 26px; }

    .magnitude-filter { grid-column: span 4; padding: 8px 12px; background: #151b25; border: 1px solid #344256; border-radius: 9px; }
    .magnitude-filter-label, .magnitude-filter-scale { display: flex; align-items: center; justify-content: space-between; }
    .magnitude-filter-label { color: #e6eaf2; font-weight: bold; }
    .magnitude-filter-label output { color: #ffb74d; font-variant-numeric: tabular-nums; }
    .magnitude-filter input[type="range"] { display: block; width: 100%; min-height: 28px; margin: 4px 0 0; padding: 0; border: 0; border-radius: 0; background: transparent; accent-color: #ffb74d; cursor: pointer; }
    .magnitude-filter-scale { position: relative; height: 1em; color: #8b94a2; font-size: .72rem; }
    .magnitude-filter-scale span { position: absolute; transform: translateX(-50%); }
    .magnitude-filter-scale span:first-child { left: 0; transform: none; }
    .magnitude-filter-scale span:nth-child(2) { left: 70%; }
    .magnitude-filter-scale span:nth-child(3) { left: 85%; }
    .magnitude-filter-scale span:last-child { right: 0; transform: none; }

    /* --- BARRE UNIQUE UNIFIÉE --- */
    .unified-toolbar { grid-column: span 4; display: flex; flex-wrap: wrap; gap: 6px; align-items: center; justify-content: space-between; background: #151b25; padding: 8px 10px; border: 1px solid #344256; border-radius: 9px; }
    
    .time-picker-wrapper { display: flex; align-items: center; gap: 6px; flex-wrap: wrap; }
    
    .date-btn-container { position: relative; display: inline-flex; align-items: center; }
    #nativeDatePicker { position: absolute; opacity: 0; inset: 0; width: 100%; height: 100%; cursor: pointer; z-index: 2; }
    .date-display-btn { background: #10151d; border: 1px solid #344256; color: #ffb74d; padding: 5px 8px; border-radius: 8px; font-weight: bold; font-size: 0.8rem; display: flex; align-items: center; gap: 4px; cursor: pointer; min-height: 38px; }

    /* Sélecteur vertical d'heure / minute avec flèches au-dessus et en dessous */
    .time-col-group { display: flex; flex-direction: column; align-items: center; background: #10151d; border: 1px solid #344256; border-radius: 8px; padding: 2px 4px; gap: 1px; }
    .time-val { font-size: 0.9rem; font-weight: bold; color: #ffb74d; text-align: center; font-variant-numeric: tabular-nums; min-width: 20px; }
    .vert-arrow { background: #1e293b; border: 1px solid #344256; color: #ffb74d; font-size: 0.55rem; width: 100%; height: 13px; display: flex; align-items: center; justify-content: center; cursor: pointer; padding: 0; border-radius: 3px; }
    .vert-arrow:active { background: #344256; }

    /* Groupe de contrôle du temps (chevrons fins, direct, chevrons larges) */
    .time-step-group { display: flex; align-items: center; gap: 2px; }
    .time-btn { min-height: 38px; padding: 2px 7px; font-size: 0.85rem; font-weight: bold; background: #1e293b; color: #ffb74d; border: 1px solid #344256; border-radius: 6px; cursor: pointer; display: flex; align-items: center; justify-content: center; }
    .time-btn:active { background: #253247; transform: scale(0.95); }

    /* Contrôles d'offset intégrés */
    .offset-inline-group { display: flex; align-items: center; gap: 4px; }
    .dpad-inline { display: grid; grid-template-columns: repeat(3, 26px); grid-template-rows: repeat(3, 26px); gap: 2px; align-items: center; justify-content: center; background: #10151d; padding: 3px; border: 1px solid #344256; border-radius: 8px; }
    .jog-btn-inline { background: #1e293b !important; color: #ffb74d !important; font-size: 0.75rem; font-weight: bold; display: flex; align-items: center; justify-content: center; min-height: 26px !important; min-width: 26px !important; padding: 0 !important; border-radius: 4px; border: 1px solid #344256; }
    .jog-up-inline { grid-column: 2; grid-row: 1; }
    .jog-left-inline { grid-column: 1; grid-row: 2; }
    .jog-center-inline { grid-column: 2; grid-row: 2; background: #151b25 !important; font-size: 0.45rem; color: #8b94a2 !important; display: flex; align-items: center; justify-content: center; border: 1px dashed #344256; }
    .jog-right-inline { grid-column: 3; grid-row: 2; }
    .jog-down-inline { grid-column: 2; grid-row: 3; }

    .sens-inline-select { min-height: 38px !important; padding: 2px 6px !important; font-size: 0.78rem !important; background: #10151d !important; color: #ffb74d !important; font-weight: bold; width: auto !important; }

    button, select, input { min-height: 40px; border: 1px solid #344256; border-radius: 8px; background: #151b25; color: #e6eaf2; font: inherit; }
    button { padding: 6px 10px; cursor: pointer; }
    button:active { background: #253247; }
    select { width: 100%; padding: 6px 10px; background: #10151d; }
    body.landscape-fullscreen { padding: 0 !important; background: #000; overflow: hidden; }
    body.landscape-fullscreen .sky-heading,
    body.landscape-fullscreen .sky-status,
    body.landscape-fullscreen .controls,
    body.landscape-fullscreen .bottom-nav { display: none !important; }
    body.landscape-fullscreen .sky-card { width: 100% !important; max-width: none !important; margin: 0; }
    body.landscape-fullscreen .scene { height: 100vh !important; border: none; border-radius: 0; }

    .object-modal { position: fixed; inset: 0; z-index: 100; background: rgba(0,0,0,0.7); display: flex; align-items: center; justify-content: center; padding: 16px; backdrop-filter: blur(2px); }
    .object-modal[hidden] { display: none; }
    .modal-content { width: min(100%, 390px); background: #151b25; border: 1px solid #344256; border-radius: 14px; padding: 20px; box-shadow: 0 8px 32px rgba(0,0,0,0.7); max-height: 85vh; overflow-y: auto; }
    .modal-content h2 { margin: 0 0 12px; font-size: 1.1rem; color: #ffb74d; border-bottom: 1px solid #273142; padding-bottom: 8px; }
    .modal-sub { color: #8b94a2; font-size: .85rem; margin-bottom: 12px; }
    .modal-object-preview { height: 126px; margin: 0 0 14px; overflow: hidden; border: 1px solid #273142; border-radius: 9px; background: #0b1026; }
    .modal-object-preview svg { display: block; width: 100%; height: 100%; }
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
    #objectModal { inset: auto 0 0; display: flex; justify-content: center; padding: 8px; background: transparent; backdrop-filter: none; pointer-events: none; }
    #objectModal[hidden] { display: none; }
    #objectModal .modal-content { width: min(100%, 620px); max-height: 45vh; padding: 14px; border-radius: 14px 14px 0 0; pointer-events: auto; }
    #objectModal .modal-object-preview { height: 72px; }
    .sky-quick-controls { position: absolute; z-index: 6; top: 10px; left: 50%; display: flex; gap: 6px; align-items: center; transform: translateX(-50%); }
    .sky-quick-controls button, .sky-quick-controls output { padding: 7px 9px; border: 1px solid #344256; border-radius: 8px; background: rgba(8,20,38,.9); color: #f2c877; font: inherit; white-space: nowrap; }

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
      <h1><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-compass"></use></svg> Vue du ciel</h1>
      <span class="beta">PLANÉTARIUM v5.73.0</span>
    </header>
    <p id="skyStatus" class="sky-status" role="status">Initialisation du planétarium...</p>
    
    <div class="scene" id="scene">
      <!-- Indicateur de guidage centré flottant à côté du viseur -->
      <div id="searchArrowOverlay" style="position: absolute; top: calc(50% + 14px); left: calc(50% + 14px); z-index: 10; display: none; background: rgba(5,8,12,0.85); border: 1px solid #ff9800; padding: 4px 10px; border-radius: 12px; color: #ffb74d; font-weight: bold; font-size: 0.78rem; align-items: center; gap: 6px; pointer-events: none; backdrop-filter: blur(4px); box-shadow: 0 4px 12px rgba(0,0,0,0.5);">
        <!-- Flèche rotative boussole -->
        <div id="compassArrowContainer" style="display: inline-flex; transition: transform 0.1s ease-out;">
          <svg id="compassSvgIcon" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="#ffb74d" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round">
            <line x1="12" y1="19" x2="12" y2="5"></line>
            <polyline points="5 12 12 5 19 12"></polyline>
          </svg>
        </div>
        <span id="targetNameLabel" style="max-width: 150px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; color: #fff;"></span>
        <!-- Écarts détaillés signés -->
        <span id="azVal" style="font-size: 0.72rem;">AZ: +0.0°</span>
        <span id="altVal" style="font-size: 0.72rem;">ALT: +0.0°</span>
      </div>

      <div class="scene-zoom-controls">
        <button id="zoomInBtn" type="button" class="zoom-btn" title="Zoomer (+)">+</button>
        <button id="zoomOutBtn" type="button" class="zoom-btn" title="Dézoomer (-)">-</button>
      </div>

      <canvas id="skyCanvas" aria-label="Carte interactive du ciel"></canvas>
      <div class="sky-quick-controls">
        <button id="toggleSkyNames" type="button" aria-pressed="true">Noms : oui</button>
        <output id="skyDirection" aria-label="Direction de la vue">N · 0°</output>
      </div>
      <span class="scene-label cardinal" id="labelN" style="display:none">N</span>
      <span class="scene-label cardinal" id="labelS" style="display:none">S</span>
      <span class="scene-label cardinal" id="labelE" style="display:none">E</span>
      <span class="scene-label cardinal" id="labelW" style="display:none">O</span>
      <span class="scene-label telescope" id="telescopeReadout">Télescope : connexion...</span>
      <span class="scene-label view" id="viewReadout">Offset AZ 0° · ALT 0°</span>
      <button id="openSearchTargetModalBtn" type="button" class="scene-search-btn" aria-label="Rechercher une cible" title="Rechercher une cible" aria-pressed="false">
        <svg class="ui-icon" aria-hidden="true" focusable="false"><use href="/icons.svg#icon-search"></use></svg>
      </button>
      <span class="scene-center" aria-hidden="true"></span>
      <div id="objectLabels" aria-hidden="true"></div>
      <div id="constellationLabels" aria-hidden="true"></div>
    </div>
    
    <div class="controls">
      <div class="scene-action-toolbar" role="group" aria-label="Commandes de la vue du ciel">
        <button id="openSettingsModalBtn" type="button" class="scene-action-btn" aria-label="Paramètres" title="Paramètres">
          <svg class="ui-icon" aria-hidden="true" focusable="false"><use href="/icons.svg#icon-settings"></use></svg>
        </button>
        <button id="centerButton" type="button" class="scene-action-btn" aria-label="Réaligner la vue" title="Réaligner la vue">
          <svg class="ui-icon" aria-hidden="true" focusable="false"><use href="/icons.svg#icon-crosshair"></use></svg>
        </button>
      </div>
      
      <!-- LIGNE UNIQUE UNIFIÉE V5.73.0 -->
      <div class="unified-toolbar">
        <div class="time-picker-wrapper">
          <!-- Calendrier Natif -->
          <div class="date-btn-container">
            <input type="date" id="nativeDatePicker" onchange="onNativeDateChange(this.value)" title="Choisir la date">
            <button type="button" class="date-display-btn">
              <span id="dateDisplayLabel">JJ/MM/AAAA</span>
            </button>
          </div>

          <!-- Heure (Flèches au-dessus et en dessous) -->
          <div class="time-col-group">
            <button type="button" class="vert-arrow" onclick="adjustTimeUnit('hour', 1)">▲</button>
            <span id="dtHour" class="time-val">12</span>
            <button type="button" class="vert-arrow" onclick="adjustTimeUnit('hour', -1)">▼</button>
          </div>
          <span style="color:#8b94a2; font-weight:bold;">:</span>
          <!-- Minutes (Flèches au-dessus et en dessous) -->
          <div class="time-col-group">
            <button type="button" class="vert-arrow" onclick="adjustTimeUnit('minute', 5)">▲</button>
            <span id="dtMinute" class="time-val">00</span>
            <button type="button" class="vert-arrow" onclick="adjustTimeUnit('minute', -5)">▼</button>
          </div>
        </div>
        
        <!-- Contrôles temporels avec chevrons fins (-10 min / +10 min) et larges (-1H / +1H) autour du Direct -->
        <div class="time-step-group">
          <button type="button" class="time-btn" id="stepPrevHourBtn" title="-1 Heure">«</button>
          <button type="button" class="time-btn" id="stepPrevMinBtn" title="-10 Minutes">‹</button>
          <button type="button" id="resetTimeBtn" class="time-btn" style="color:#fff; background:#263238; border-color:#455a64; font-size:0.75rem;" title="Revenir à l'heure réelle">Direct</button>
          <button type="button" class="time-btn" id="stepNextMinBtn" title="+10 Minutes">›</button>
          <button type="button" class="time-btn" id="stepNextHourBtn" title="+1 Heure">»</button>
        </div>

        <!-- Contrôles d'offset intégrés -->
        <div class="offset-inline-group">
          <div class="dpad-inline">
            <button id="jogUpBtn" type="button" class="jog-btn-inline jog-up-inline" title="Ajuster Altitude">▲</button>
            <button id="jogLeftBtn" type="button" class="jog-btn-inline jog-left-inline" title="Ajuster Azimut">◀</button>
            <div class="jog-center-inline">Off</div>
            <button id="jogRightBtn" type="button" class="jog-btn-inline jog-right-inline" title="Ajuster Azimut">▶</button>
            <button id="jogDownBtn" type="button" class="jog-btn-inline jog-down-inline" title="Ajuster Altitude">▼</button>
          </div>
          <select id="stepSelect" class="sens-inline-select" aria-label="Sensibilité du pas d'offset">
            <option value="0.1">0.1°</option>
            <option value="1" selected>1.0°</option>
            <option value="5">5.0°</option>
          </select>
        </div>
      </div>

      <div class="magnitude-filter">
        <label class="magnitude-filter-label" for="magFilterSelect">
          <span>Magnitude maximale</span>
          <output id="magFilterValue" for="magFilterSelect">Toutes</output>
        </label>
        <input id="magFilterSelect" type="range" min="0" max="1000" step="1" value="1000" aria-valuetext="Toutes les magnitudes">
        <div class="magnitude-filter-scale" aria-hidden="true"><span>0</span><span>6</span><span>8</span><span>Toutes</span></div>
      </div>
    </div>
  </main>

  <!-- POP-UP RECHERCHE DE CIBLE -->
  <div class="object-modal" id="searchTargetModal" hidden>
    <div class="modal-content">
      <h2>Rechercher un objet cible</h2>
      <div class="modal-sub">Sélectionnez un objet du catalogue pour activer le guidage</div>
      <div style="display: flex; flex-direction: column; gap: 10px; margin-bottom: 16px;">
        <input type="text" id="targetSearchInput" placeholder="Filtrer par nom (ex: M42, Andromède...)" style="width: 100%; padding: 8px 10px; background: #10151d; border: 1px solid #344256; border-radius: 8px; color: #fff;">
        <select id="targetSelectDropdown" size="6" style="width: 100%; background: #10151d; border: 1px solid #344256; border-radius: 8px; color: #fff; padding: 4px;">
        </select>
      </div>
      <button type="button" class="btn" style="width: 100%; background: #ff9800; color: #000; font-weight: bold; margin-bottom: 8px;" onclick="confirmSelectedTarget()">Définir comme cible</button>
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
        <label><input type="checkbox" id="showMessierAll"> Afficher les objets Messier indépendamment de la magnitude</label>
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
  <div class="object-modal" id="objectModal" role="region" aria-labelledby="modalTitle" hidden>
    <div class="modal-content">
      <h2 id="modalTitle">Nom de l'objet</h2>
      <div class="modal-sub" id="modalSub">Constellation · Type</div>
      <div class="modal-object-preview" id="objectPreview" aria-live="polite"></div>
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
      <button type="button" class="btn" id="targetBtn" style="width:100%; background:#ff9800; color:#000; font-weight:bold; margin-bottom:8px;" onclick="setSearchTarget()">Rechercher cet objet</button>
      <button class="modal-close" type="button" onclick="closeModal()">Fermer</button>
    </div>
  </div>

  <nav class="bottom-nav" aria-label="Navigation principale">
    <a class="nav-item" href="/beta_sky" aria-current="page" title="Vue du ciel"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-compass"></use></svg></span></a>
    <a class="nav-item" href="/" title="Mode Encodeurs"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-telescope"></use></svg></span></a>
    <a class="nav-item" href="/station" title="Mise en station"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-crosshair"></use></svg></span></a>
    <a class="nav-item" href="/config" title="Configuration"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-settings"></use></svg></span></a>
    <button class="nav-item night-mode-toggle" id="nightModeToggle" type="button" aria-label="Activer le mode nuit" title="Activer le mode nuit" aria-pressed="false" onclick="toggleNightMode()"><span class="nav-icon"><svg class="ui-icon" aria-hidden="true"><use href="/icons.svg#icon-moon"></use></svg></span></button>
  </nav>

  <script>
    const canvas = document.getElementById('skyCanvas');
    const sceneDiv = document.getElementById('scene');
    const ctx2d = canvas.getContext('2d');
    const statusElement = document.getElementById('skyStatus');
    const magFilterElement = document.getElementById('magFilterSelect');
    const magFilterValueElement = document.getElementById('magFilterValue');
    const labelsElement = document.getElementById('objectLabels');
    const constLabelsElement = document.getElementById('constellationLabels');
    const viewReadout = document.getElementById('viewReadout');
    const telescopeReadout = document.getElementById('telescopeReadout');
    const stepSelect = document.getElementById('stepSelect');
    
    let baseObjects = [];
    let objects = [];
    const objectsByName = new Map();
    let constellationLinesValidated = false;
    function setObjects(nextObjects) {
      objects = nextObjects;
      objectsByName.clear();
      for (const object of objects) {
        if (!objectsByName.has(object.name)) objectsByName.set(object.name, object);
      }
      if (!constellationLinesValidated) {
        const missing = new Set();
        for (const [starA, starB] of CONSTELLATION_LINES) {
          if (!objectsByName.has(starA)) missing.add(starA);
          if (!objectsByName.has(starB)) missing.add(starB);
        }
        if (missing.size) {
          throw new Error(`Étoiles de constellation absentes du catalogue : ${[...missing].join(', ')}`);
        }
        constellationLinesValidated = true;
      }
    }
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
    let cameraCalibrationHeading = 0;
    let cameraCalibrationElevation = 0;
    let telescopeCameraDirection = [0, 0, 1];

    let fieldOfView = 34;
    let encoderRequestInFlight = false;
    let lastScreenObjects = [];
    let useSystemTime = true;
    let currentSkyDate = new Date();
    
    let activeTargetObject = null;
    let showSkyNames = localStorage.getItem('dobson-sky-names') !== 'false';
    let projectionMode = 'actuel'; // V5.73.0 Mode par défaut

    const namesButton = document.getElementById('toggleSkyNames');
    function updateNamesButton() {
      namesButton.textContent = showSkyNames ? 'Noms : oui' : 'Noms : non';
      namesButton.setAttribute('aria-pressed', String(showSkyNames));
    }
    namesButton.addEventListener('click', () => {
      showSkyNames = !showSkyNames;
      localStorage.setItem('dobson-sky-names', String(showSkyNames));
      updateNamesButton();
    });
    updateNamesButton();

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

    // --- CALCUL DES ÉPHÉMÉRIDES DU SYSTÈME SOLAIRE ---
    function getSolarSystemObjects(date) {
      const jd = date.getTime() / 86400000 + 2440587.5;
      const d = jd - 2451545.0;

      const sunMeanLong = (280.460 + 0.9856474 * d) % 360;
      const sunMeanAnom = (357.528 + 0.9856003 * d) % 360;
      const sunEclLong = sunMeanLong + 1.915 * Math.sin(sunMeanAnom * Math.PI / 180) + 0.020 * Math.sin(2 * sunMeanAnom * Math.PI / 180);
      
      const eps = 23.439 - 0.0000004 * d;
      const epsRad = eps * Math.PI / 180;
      const sunEclLongRad = sunEclLong * Math.PI / 180;

      const sinDecSun = Math.sin(epsRad) * Math.sin(sunEclLongRad);
      const decSun = Math.asin(sinDecSun) * 180 / Math.PI;
      const ySun = Math.cos(epsRad) * Math.sin(sunEclLongRad);
      const xSun = Math.cos(sunEclLongRad);
      let raSun = Math.atan2(ySun, xSun) * 180 / Math.PI / 15;
      raSun = (raSun + 24) % 24;

      const moonMeanLong = (218.316 + 13.176396 * d) % 360;
      const moonMeanAnom = (134.963 + 13.064993 * d) % 360;
      const moonAscNode = (125.228 - 0.052954 * d) % 360;
      const moonEclLong = moonMeanLong + 6.289 * Math.sin(moonMeanAnom * Math.PI / 180);
      const moonEclLat = 5.128 * Math.sin((moonMeanLong - moonAscNode) * Math.PI / 180);
      
      const lRad = moonEclLong * Math.PI / 180;
      const bRad = moonEclLat * Math.PI / 180;
      
      const sinDecMoon = Math.sin(bRad) * Math.cos(epsRad) + Math.cos(bRad) * Math.sin(epsRad) * Math.sin(lRad);
      const decMoon = Math.asin(sinDecMoon) * 180 / Math.PI;
      const yMoon = Math.sin(lRad) * Math.cos(epsRad) - Math.tan(bRad) * Math.sin(epsRad);
      const xMoon = Math.cos(lRad);
      let raMoon = Math.atan2(yMoon, xMoon) * 180 / Math.PI / 15;
      raMoon = (raMoon + 24) % 24;

      const planetDefs = [
        { name: "Mercure", period: 87.97, dist: 0.387, long0: 252.25, mag: -0.4, type: "Planète" },
        { name: "Vénus", period: 224.70, dist: 0.723, long0: 181.98, mag: -4.4, type: "Planète" },
        { name: "Mars", period: 686.98, dist: 1.524, long0: 355.45, mag: 1.0, type: "Planète" },
        { name: "Jupiter", period: 4332.59, dist: 5.204, long0: 34.40, mag: -2.2, type: "Planète" },
        { name: "Saturne", period: 10759.22, dist: 9.582, long0: 49.94, mag: 0.7, type: "Planète" },
        { name: "Uranus", period: 30685.4, dist: 19.218, long0: 313.23, mag: 5.5, type: "Planète" },
        { name: "Neptune", period: 60189.0, dist: 30.11, long0: 304.88, mag: 7.8, type: "Planète" },
        { name: "Pluton", period: 90560.0, dist: 39.48, long0: 238.95, mag: 14.0, type: "Planète" }
      ];

      const planets = planetDefs.map(p => {
        const meanLong = (p.long0 + (360 / p.period) * d) % 360;
        const eclLong = (sunEclLong + (meanLong - sunMeanLong) * (0.8 / p.dist) + 360) % 360;
        const eclLat = 0;
        
        const plRad = eclLong * Math.PI / 180;
        const pbRad = eclLat * Math.PI / 180;
        
        const sinDec = Math.sin(pbRad) * Math.cos(epsRad) + Math.cos(pbRad) * Math.sin(epsRad) * Math.sin(plRad);
        const dec = Math.asin(sinDec) * 180 / Math.PI;
        const y = Math.sin(plRad) * Math.cos(epsRad) - Math.tan(pbRad) * Math.sin(epsRad);
        const x = Math.cos(plRad);
        let ra = Math.atan2(y, x) * 180 / Math.PI / 15;
        ra = (ra + 24) % 24;

        return { name: p.name, commonName: p.name, type: p.type, ra: ra, dec: dec, mag: p.mag };
      });

      return [
        { name: "Soleil", commonName: "Soleil", type: "Étoile", ra: raSun, dec: decSun, mag: -26.7 },
        { name: "Lune", commonName: "Lune", type: "Satellite", ra: raMoon, dec: decMoon, mag: -12.5 },
        ...planets
      ];
    }

    // --- GESTION DU TEMPS ET DU CALENDRIER NATIF ---
    function updateDateTimeDisplay() {
      const year = currentSkyDate.getFullYear();
      const month = String(currentSkyDate.getMonth() + 1).padStart(2, '0');
      const day = String(currentSkyDate.getDate()).padStart(2, '0');
      
      document.getElementById('nativeDatePicker').value = `${year}-${month}-${day}`;
      document.getElementById('dateDisplayLabel').textContent = `${day}/${month}/${year}`;

      document.getElementById('dtHour').textContent = String(currentSkyDate.getHours()).padStart(2, '0');
      document.getElementById('dtMinute').textContent = String(currentSkyDate.getMinutes()).padStart(2, '0');
    }

    function onNativeDateChange(dateString) {
      if (!dateString) return;
      useSystemTime = false;
      const parts = dateString.split('-');
      currentSkyDate.setFullYear(parseInt(parts[0], 10));
      currentSkyDate.setMonth(parseInt(parts[1], 10) - 1);
      currentSkyDate.setDate(parseInt(parts[2], 10));

      updateDateTimeDisplay();
      setObjects([...baseObjects, ...getSolarSystemObjects(currentSkyDate)]);
      updateSkyBackground(currentSkyDate);
    }

    function adjustTimeUnit(unit, amount) {
      useSystemTime = false;
      if (unit === 'hour') {
        currentSkyDate.setHours(currentSkyDate.getHours() + amount);
      } else if (unit === 'minute') {
        currentSkyDate.setMinutes(currentSkyDate.getMinutes() + amount);
      }
      updateDateTimeDisplay();
      setObjects([...baseObjects, ...getSolarSystemObjects(currentSkyDate)]);
      updateSkyBackground(currentSkyDate);
    }

    function initDateTimeInput() {
      currentSkyDate = new Date();
      updateDateTimeDisplay();
    }
    initDateTimeInput();

    document.getElementById('resetTimeBtn').addEventListener('click', () => {
      useSystemTime = true;
      initDateTimeInput();
      setObjects([...baseObjects, ...getSolarSystemObjects(currentSkyDate)]);
      updateSkyBackground(currentSkyDate);
    });

    document.getElementById('stepPrevHourBtn').addEventListener('click', () => adjustTimeUnit('hour', -1));
    document.getElementById('stepNextHourBtn').addEventListener('click', () => adjustTimeUnit('hour', 1));
    document.getElementById('stepPrevMinBtn').addEventListener('click', () => adjustTimeUnit('minute', -10));
    document.getElementById('stepNextMinBtn').addEventListener('click', () => adjustTimeUnit('minute', 10));

    function getSkyDate() {
      if (useSystemTime) return new Date();
      return currentSkyDate;
    }

    // --- LOGIQUE RECHERCHE GLOBALE DE CIBLE ---
    const searchTargetModal = document.getElementById('searchTargetModal');
    const targetSearchInput = document.getElementById('targetSearchInput');
    const targetSelectDropdown = document.getElementById('targetSelectDropdown');
    const searchTargetButton = document.getElementById('openSearchTargetModalBtn');
    const dismissedTargetStorageKey = 'dobson-dismissed-sky-target';

    function targetIdentity(target) {
      return JSON.stringify([target.name, Number(target.ra), Number(target.dec)]);
    }

    function updateTargetSearchButton(isActive) {
      const icon = searchTargetButton.querySelector('use');
      icon.setAttribute('href', isActive ? '/icons.svg#icon-circle-x' : '/icons.svg#icon-search');
      const label = isActive ? 'Arrêter la recherche de cible' : 'Rechercher une cible';
      searchTargetButton.setAttribute('aria-label', label);
      searchTargetButton.setAttribute('title', label);
      searchTargetButton.setAttribute('aria-pressed', String(isActive));
    }

    function activateTargetSearch(target) {
      activeTargetObject = target;
      document.getElementById('searchArrowOverlay').style.display = target ? 'flex' : 'none';
      updateTargetSearchButton(Boolean(target));
      if (target) {
        try {
          localStorage.removeItem(dismissedTargetStorageKey);
        } catch (error) {
          console.warn('Impossible de mettre à jour l’état de recherche de la cible.', error);
        }
      }
    }

    function stopTargetSearch() {
      if (activeTargetObject) {
        try {
          localStorage.setItem(dismissedTargetStorageKey, targetIdentity(activeTargetObject));
        } catch (error) {
          console.warn('Impossible de mémoriser l’arrêt de la recherche de cible.', error);
        }
      }
      activateTargetSearch(null);
    }

    searchTargetButton.addEventListener('click', () => {
      if (activeTargetObject) {
        stopTargetSearch();
        return;
      }
      populateTargetDropdown('');
      targetSearchInput.value = '';
      searchTargetModal.hidden = false;
      targetSearchInput.focus();
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

      const target = objectsByName.get(selectedName);
      closeSearchTargetModal();
      activateTargetSearch(target);
    }

    function setSearchTarget() {
      const titleText = document.getElementById('modalTitle').textContent;
      const objName = titleText.split(' (')[0].trim();
      
      const target = objectsByName.get(objName);
      closeModal();
      activateTargetSearch(target);
    }

    async function loadSavedSkyPrefs() {
      try {
        const res = await fetch('/get_sky_prefs');
        if (res.ok) {
          const p = await res.json();
          if (p.mag !== undefined) {
            magFilterElement.value = getSliderPositionForMagnitude(Number(p.mag));
            updateMagnitudeFilterLabel();
          }
          if (p.showGrid !== undefined) document.getElementById('showGrid').checked = p.showGrid;
          if (p.showGal !== undefined) document.getElementById('showGal').checked = p.showGal;
          if (p.showNeb !== undefined) document.getElementById('showNeb').checked = p.showNeb;
          if (p.showAmas !== undefined) document.getElementById('showAmas').checked = p.showAmas;
          if (p.showStars !== undefined) document.getElementById('showStars').checked = p.showStars;
          if (p.showConst !== undefined) document.getElementById('showConstellations').checked = p.showConst;
          if (p.showMessierAll !== undefined) document.getElementById('showMessierAll').checked = Boolean(Number(p.showMessierAll));
          
          if (p.labelGal !== undefined) document.getElementById('labelGal').checked = p.labelGal;
          if (p.labelNeb !== undefined) document.getElementById('labelNeb').checked = p.labelNeb;
          if (p.labelAmas !== undefined) document.getElementById('labelAmas').checked = p.labelAmas;
          if (p.labelStars !== undefined) document.getElementById('labelStars').checked = p.labelStars;
          if (p.labelConst !== undefined) document.getElementById('labelConstellations').checked = p.labelConst;
          
          // V5.73.0 Restauration du mode de projection
          if (p.projMode) setProjectionMode(p.projMode);
        }
      } catch (e) {
        console.warn("Préférences ESP32 non disponibles:", e);
      }
    }

    async function saveSkyPrefs() {
      try {
        const p = {
          mag: getSelectedMagnitude(),
          showGrid: document.getElementById('showGrid').checked ? 1 : 0,
          showGal: document.getElementById('showGal').checked ? 1 : 0,
          showNeb: document.getElementById('showNeb').checked ? 1 : 0,
          showAmas: document.getElementById('showAmas').checked ? 1 : 0,
          showStars: document.getElementById('showStars').checked ? 1 : 0,
          showConst: document.getElementById('showConstellations').checked ? 1 : 0,
          showMessierAll: document.getElementById('showMessierAll').checked ? 1 : 0,
          labelGal: document.getElementById('labelGal').checked ? 1 : 0,
          labelNeb: document.getElementById('labelNeb').checked ? 1 : 0,
          labelAmas: document.getElementById('labelAmas').checked ? 1 : 0,
          labelStars: document.getElementById('labelStars').checked ? 1 : 0,
          labelConst: document.getElementById('labelConstellations').checked ? 1 : 0,
          projMode: projectionMode // V5.73.0 Sauvegarde du mode
        };
        const query = new URLSearchParams(p).toString();
        await fetch('/save_sky_prefs?' + query);
      } catch (e) {
        console.warn("Erreur sauvegarde ESP32:", e);
      }
    }

    function getSelectedMagnitude() {
      const position = Number(magFilterElement.value);
      if (position >= 1000) return 99;
      if (position <= 700) return position * 6 / 700;
      if (position <= 850) return 6 + (position - 700) * 2 / 150;
      return 8 + (position - 850) * 4 / 149;
    }

    function getSliderPositionForMagnitude(magnitude) {
      if (!Number.isFinite(magnitude) || magnitude >= 13) return 1000;
      const boundedMagnitude = Math.max(0, Math.min(12, magnitude));
      if (boundedMagnitude <= 6) return Math.round(boundedMagnitude * 700 / 6);
      if (boundedMagnitude <= 8) return Math.round(700 + (boundedMagnitude - 6) * 150 / 2);
      return Math.round(850 + (boundedMagnitude - 8) * 149 / 4);
    }

    function updateMagnitudeFilterLabel() {
      const position = Number(magFilterElement.value);
      const showAllMagnitudes = position >= 1000;
      const selectedMagnitude = getSelectedMagnitude();
      magFilterValueElement.textContent = showAllMagnitudes ? 'Toutes' : `≤ ${selectedMagnitude.toFixed(2)}`;
      magFilterElement.setAttribute(
        'aria-valuetext',
        showAllMagnitudes ? 'Toutes les magnitudes' : `Magnitude maximale ${selectedMagnitude.toFixed(2)}`
      );
    }

    updateMagnitudeFilterLabel();
    magFilterElement.addEventListener('input', updateMagnitudeFilterLabel);
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
      alignOffsetElevation = Math.max(-90, Math.min(90, alignOffsetElevation + step));
    });
    document.getElementById('jogDownBtn').addEventListener('click', () => {
      const step = parseFloat(stepSelect.value) || 1;
      alignOffsetElevation = Math.max(-90, Math.min(90, alignOffsetElevation - step));
    });

    document.getElementById('zoomInBtn').addEventListener('click', () => {
      fieldOfView = Math.max(2, fieldOfView - 4);
    });
    document.getElementById('zoomOutBtn').addEventListener('click', () => {
      fieldOfView = Math.min(75, fieldOfView + 4);
    });

    // --- GESTION DU MODE DE PROJECTION V5.73.0 ---
    function setProjectionMode(mode) {
      if (!['actuel', 'stéréo', 'gnomon'].includes(mode)) return;
      projectionMode = mode;
      
      // Mise à jour du FOV par défaut selon le mode
      if (mode === 'stéréo') fieldOfView = 60; // Plus large pour la carte
      else if (mode === 'gnomon') fieldOfView = 45; // Compromis
      else fieldOfView = 34; // Actuel / Perspective
    }

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

    // Adapted from ConstellationLines by Marc van der Sluys, CC BY 4.0.
    const CONSTELLATION_LINES = [
      ["Udkadua","Kaffalmusalsala"],
      ["Kaffalmusalsala","Rasalnaqa"],
      ["Rasalnaqa","Alfarasalkamil"],
      ["Alfarasalkamil","Rasalnaqa"],
      ["Rasalnaqa","HIP 1473"],
      ["HIP 1473","Delta And"],
      ["Delta And","Alpheratz"],
      ["Alpheratz","Delta And"],
      ["Delta And","HIP 3031"],
      ["HIP 3031","Shimu"],
      ["Shimu","HIP 3031"],
      ["HIP 3031","Delta And"],
      ["Delta And","Mirach"],
      ["Mirach","Almach"],
      ["Almach","Mirach"],
      ["Mirach","51 And"],
      ["51 And","Mirach"],
      ["Mirach","HIP 4436"],
      ["HIP 4436","HIP 3881"],
      ["HIP 53502","HIP 51172"],
      ["HIP 51172","HIP 47758"],
      ["HIP 47758","HIP 46515"],
      ["Paradys","HIP 80047"],
      ["HIP 80047","HIP 81065"],
      ["HIP 81065","HIP 81852"],
      ["Alshain","Altair"],
      ["Altair","Tarazed"],
      ["Tarazed","HIP 95501"],
      ["HIP 95501","Deneb el Okab"],
      ["Deneb el Okab","HIP 95501"],
      ["HIP 95501","Eta Aql"],
      ["Eta Aql","Theta Aql"],
      ["Theta Aql","Eta Aql"],
      ["Eta Aql","HIP 95501"],
      ["HIP 95501","HIP 93805"],
      ["HIP 93805","HIP 93429"],
      ["Safina","HIP 114855"],
      ["HIP 114855","HIP 114724"],
      ["HIP 114724","HIP 111497"],
      ["HIP 111497","Sadachbia"],
      ["Sadachbia","Shatabhisha"],
      ["Shatabhisha","Skat"],
      ["Skat","Shatabhisha"],
      ["Shatabhisha","Sadachbia"],
      ["Sadachbia","Ancha"],
      ["Ancha","HIP 109139"],
      ["HIP 109139","Ancha"],
      ["Ancha","Sadachbia"],
      ["Sadachbia","Sadalmelik"],
      ["Sadalmelik","Sadalsuud"],
      ["HIP 88714","HIP 85792"],
      ["HIP 85792","HIP 85258"],
      ["HIP 85258","HIP 85267"],
      ["HIP 85267","HIP 85727"],
      ["HIP 85727","HIP 85267"],
      ["HIP 85267","HIP 83081"],
      ["HIP 83081","HIP 82363"],
      ["HIP 82363","HIP 83081"],
      ["HIP 83081","HIP 83153"],
      ["HIP 83153","HIP 85792"],
      ["Botein","HR 888"],
      ["HR 888","Bharani"],
      ["Bharani","Hamal"],
      ["Hamal","Sheratan"],
      ["Sheratan","Mesarthim"],
      ["Capella","Menkalinan"],
      ["Menkalinan","Mahasim"],
      ["Mahasim","Elnath"],
      ["Elnath","Hassaleh"],
      ["Hassaleh","Saclateni"],
      ["Saclateni","Haedus"],
      ["Haedus","Almaaz"],
      ["Almaaz","Capella"],
      ["Seginus","Kalasungsang"],
      ["Kalasungsang","Arcturus"],
      ["Arcturus","Izar"],
      ["Izar","Hozian"],
      ["Hozian","Nekkar"],
      ["Nekkar","Seginus"],
      ["Seginus","Xuange"],
      ["Xuange","HIP 70497"],
      ["HIP 70497","HIP 69483"],
      ["HIP 69483","Xuange"],
      ["HIP 33347","HIP 33160"],
      ["HIP 33160","Muliphein"],
      ["Muliphein","HIP 33347"],
      ["HIP 33347","Sirius"],
      ["Sirius","Mirzam"],
      ["Mirzam","HIP 31592"],
      ["HIP 31592","HIP 33152"],
      ["HIP 33152","Adhara"],
      ["Adhara","Nganurganity"],
      ["Nganurganity","Wezen"],
      ["Wezen","Aludra"],
      ["Aludra","Wezen"],
      ["Wezen","HIP 33977"],
      ["HIP 33977","Sirius"],
      ["Procyon","Gomeisa"],
      ["Cor Caroli","Chara"],
      ["HIP 21060","HIP 21770"],
      ["HIP 21770","HIP 21861"],
      ["HIP 16281","HIP 16228"],
      ["HIP 16228","Custos"],
      ["Custos","Gamma Cam"],
      ["Gamma Cam","Alpha Cam"],
      ["Alpha Cam","Beta Cam"],
      ["Beta Cam","HIP 23040"],
      ["Algedi","Dabih"],
      ["Dabih","HIP 102485"],
      ["HIP 102485","HIP 102978"],
      ["HIP 102978","HIP 105881"],
      ["HIP 105881","Nashira"],
      ["Nashira","Deneb Algedi"],
      ["Deneb Algedi","Nashira"],
      ["Nashira","HIP 105515"],
      ["HIP 105515","Udang"],
      ["Udang","Dabih"],
      ["Canopus","HIP 32761"],
      ["HIP 32761","HIP 38827"],
      ["HIP 38827","Avior"],
      ["Avior","Aspidiske"],
      ["Aspidiske","HIP 50371"],
      ["HIP 50371","HIP 51576"],
      ["HIP 51576","HIP 52419"],
      ["HIP 52419","HIP 50099"],
      ["HIP 50099","Miaplacidus"],
      ["Miaplacidus","HIP 48002"],
      ["Segin","Ruchbah"],
      ["Ruchbah","Navi"],
      ["Navi","Shedar"],
      ["Shedar","Caph"],
      ["HIP 56561","HIP 57439"],
      ["HIP 57439","HIP 55425"],
      ["HIP 55425","HIP 59449"],
      ["HIP 59449","HIP 59196"],
      ["HIP 59196","HIP 60823"],
      ["HIP 60823","HIP 61622"],
      ["HIP 61622","HIP 61932"],
      ["HIP 61932","HIP 66657"],
      ["HIP 66657","Hadar"],
      ["Hadar","HIP 66657"],
      ["HIP 66657","Rigil Kentaurus"],
      ["Rigil Kentaurus","HIP 66657"],
      ["HIP 66657","Leepwal"],
      ["Leepwal","HIP 67472"],
      ["HIP 67472","Heng"],
      ["Heng","Kulou"],
      ["Kulou","Heng"],
      ["Heng","Menkent"],
      ["Menkent","Heng"],
      ["Heng","HIP 68245"],
      ["HIP 68245","HIP 71352"],
      ["HIP 71352","HIP 73334"],
      ["Errai","Alfirk"],
      ["Alfirk","Alderamin"],
      ["Alderamin","Zeta Cep"],
      ["Zeta Cep","Iota Cep"],
      ["Iota Cep","Errai"],
      ["Kaffaljidhma","HIP 11484"],
      ["HIP 11484","HIP 12828"],
      ["HIP 12828","HIP 13954"],
      ["HIP 13954","Menkar"],
      ["Menkar","Kaffaljidhma"],
      ["Kaffaljidhma","HIP 12387"],
      ["HIP 12387","Mira"],
      ["Mira","Baten Kaitos"],
      ["Baten Kaitos","HIP 6537"],
      ["HIP 6537","HIP 5364"],
      ["HIP 5364","Diphda"],
      ["Diphda","HIP 6960"],
      ["HIP 6960","HIP 9347"],
      ["HIP 9347","Baten Kaitos"],
      ["HIP 40702","HIP 40888"],
      ["HIP 40888","HIP 52595"],
      ["HIP 52595","HIP 60000"],
      ["HIP 60000","HIP 51839"],
      ["HIP 51839","HIP 40702"],
      ["HIP 74824","Xami"],
      ["Xami","HIP 75323"],
      ["Acubens","Asellus Australis"],
      ["Asellus Australis","Tarf"],
      ["Tarf","Asellus Australis"],
      ["Asellus Australis","Asellus Borealis"],
      ["Asellus Borealis","Iota Cnc"],
      ["HIP 30277","HIP 29807"],
      ["HIP 29807","HIP 28199"],
      ["HIP 28199","Wazn"],
      ["Wazn","HIP 28328"],
      ["HIP 28328","Wazn"],
      ["Wazn","Phact"],
      ["Phact","HIP 24659"],
      ["Diadem","Beta Com"],
      ["Beta Com","Gamma Com"],
      ["HIP 93174","HIP 93825"],
      ["HIP 93825","Meridiana"],
      ["Meridiana","HIP 94160"],
      ["HIP 94160","HIP 94005"],
      ["HIP 94005","HIP 93542"],
      ["Aurwandilsta","Epsilon CrB"],
      ["Epsilon CrB","Matrchakre"],
      ["Matrchakre","Gamma CrB"],
      ["Gamma CrB","Alphecca"],
      ["Alphecca","Nusakan"],
      ["Nusakan","Guansuo"],
      ["HIP 58188","HIP 57283"],
      ["HIP 57283","HIP 55705"],
      ["HIP 55705","HIP 54682"],
      ["HIP 54682","Alkes"],
      ["Alkes","HIP 55282"],
      ["HIP 55282","HIP 55705"],
      ["HIP 55705","HIP 55282"],
      ["HIP 55282","HIP 55687"],
      ["HIP 55687","HIP 56633"],
      ["Mimosa","Imai"],
      ["Acrux","Gacrux"],
      ["Alchiba","HIP 59316"],
      ["HIP 59316","HIP 59803"],
      ["HIP 59803","Algorab"],
      ["Algorab","Kraz"],
      ["Kraz","HIP 59316"],
      ["Albireo","HIP 98110"],
      ["HIP 98110","Sadr"],
      ["Sadr","Gienah"],
      ["Gienah","Zeta Cyg"],
      ["Zeta Cyg","Gienah"],
      ["Gienah","Sadr"],
      ["Sadr","Deneb"],
      ["Deneb","Sadr"],
      ["Sadr","Fawaris"],
      ["Fawaris","Iota Cyg"],
      ["Aldulfin","Rotanev"],
      ["Rotanev","Sualocin"],
      ["Sualocin","Gamma Del"],
      ["Gamma Del","Delta Del"],
      ["Delta Del","Rotanev"],
      ["HIP 19893","HIP 21281"],
      ["HIP 21281","HIP 23693"],
      ["HIP 23693","HIP 26069"],
      ["HIP 26069","HIP 27100"],
      ["HIP 27100","HIP 26069"],
      ["HIP 26069","HIP 27890"],
      ["Grumium","Eltanin"],
      ["Eltanin","Rastaban"],
      ["Rastaban","Kuma"],
      ["Kuma","Grumium"],
      ["Grumium","Altais"],
      ["Altais","HIP 97433"],
      ["HIP 97433","Chi Dra"],
      ["Chi Dra","Dziban"],
      ["Dziban","Aldhibah"],
      ["Aldhibah","Athebyne"],
      ["Athebyne","HIP 78527"],
      ["HIP 78527","Edasich"],
      ["Edasich","Thuban"],
      ["Thuban","HIP 61281"],
      ["HIP 61281","Giausar"],
      ["Giausar","HIP 47193"],
      ["Kitalpha","Beta Equ"],
      ["Beta Equ","HIP 104858"],
      ["HIP 104858","HIP 104521"],
      ["Cursa","HIP 22701"],
      ["HIP 22701","HIP 22109"],
      ["HIP 22109","HIP 21444"],
      ["HIP 21444","Beid"],
      ["Beid","Zaurak"],
      ["Zaurak","Rana"],
      ["Rana","Ran"],
      ["Ran","Azha"],
      ["Azha","HIP 12770"],
      ["HIP 12770","HIP 12843"],
      ["HIP 12843","HIP 14146"],
      ["HIP 14146","HIP 15474"],
      ["HIP 15474","HIP 16611"],
      ["HIP 16611","HIP 17651"],
      ["HIP 17651","HIP 18216"],
      ["HIP 18216","HIP 18673"],
      ["HIP 18673","Theemin"],
      ["Theemin","Beemim"],
      ["Beemim","HIP 20042"],
      ["HIP 20042","HIP 17874"],
      ["HIP 17874","HR 1190"],
      ["HR 1190","HIP 16870"],
      ["HIP 16870","82 G. Eri"],
      ["82 G. Eri","HR 898"],
      ["HR 898","HIP 12486"],
      ["HIP 12486","HIP 12413"],
      ["HIP 12413","HIP 11407"],
      ["HIP 11407","HIP 10602"],
      ["HIP 10602","HIP 9007"],
      ["HIP 9007","Achernar"],
      ["Dalim","HIP 13147"],
      ["HIP 13147","HIP 9677"],
      ["HIP 28734","Propus"],
      ["Propus","Tejat"],
      ["Tejat","Mebsuta"],
      ["Mebsuta","HIP 30883"],
      ["HIP 30883","Mebsuta"],
      ["Mebsuta","HIP 34693"],
      ["HIP 34693","HIP 36366"],
      ["HIP 36366","Castor"],
      ["Castor","HIP 36366"],
      ["HIP 36366","HIP 34693"],
      ["HIP 34693","HIP 33018"],
      ["HIP 33018","HIP 34693"],
      ["HIP 34693","HIP 36046"],
      ["HIP 36046","HIP 36962"],
      ["HIP 36962","Pollux"],
      ["Pollux","HIP 36962"],
      ["HIP 36962","HIP 37740"],
      ["HIP 37740","HIP 36962"],
      ["HIP 36962","Wasat"],
      ["Wasat","Mekbuda"],
      ["Mekbuda","Alhena"],
      ["Alhena","Mekbuda"],
      ["Mekbuda","Wasat"],
      ["Wasat","HIP 35350"],
      ["HIP 35350","Alzirr"],
      ["Aldhanab","HIP 109111"],
      ["HIP 109111","HIP 110997"],
      ["HIP 110997","Tiaki"],
      ["Tiaki","Alnair"],
      ["Alnair","Tiaki"],
      ["Tiaki","HIP 112623"],
      ["HIP 112623","HIP 113638"],
      ["HIP 113638","HIP 112623"],
      ["HIP 112623","HIP 114421"],
      ["HIP 114421","HIP 114131"],
      ["Epsilon Her","Pi Her"],
      ["Pi Her","HIP 87808"],
      ["HIP 87808","HIP 86414"],
      ["HIP 86414","HIP 87808"],
      ["HIP 87808","Pi Her"],
      ["Pi Her","Eta Her"],
      ["Eta Her","HIP 81126"],
      ["HIP 81126","Asuusiha"],
      ["Asuusiha","HIP 81126"],
      ["HIP 81126","Eta Her"],
      ["Eta Her","Rutilicus"],
      ["Rutilicus","Kornephoros"],
      ["Kornephoros","Rutilicus"],
      ["Rutilicus","Epsilon Her"],
      ["Epsilon Her","Sujata"],
      ["Sujata","HIP 86974"],
      ["HIP 86974","HIP 87933"],
      ["HIP 87933","HIP 88794"],
      ["HIP 13884","HIP 14240"],
      ["HIP 14240","HIP 12484"],
      ["HIP 12484","HIP 12225"],
      ["HIP 12225","HIP 12653"],
      ["HIP 12653","HIP 13502"],
      ["HIP 13502","HIP 19747"],
      ["HIP 19747","HIP 19515"],
      ["HIP 43234","Ashlesha"],
      ["Ashlesha","HIP 42313"],
      ["HIP 42313","Minchir"],
      ["Minchir","HIP 42799"],
      ["HIP 42799","HIP 43234"],
      ["HIP 43234","HIP 43813"],
      ["HIP 43813","HIP 45336"],
      ["HIP 45336","Ukdah"],
      ["Ukdah","Alphard"],
      ["Alphard","Zhang"],
      ["Zhang","HIP 49841"],
      ["HIP 49841","HIP 51069"],
      ["HIP 51069","HIP 52943"],
      ["HIP 52943","HIP 56343"],
      ["HIP 56343","HIP 57936"],
      ["HIP 57936","Naja"],
      ["Naja","HIP 68895"],
      ["HIP 9236","HIP 12394"],
      ["HIP 12394","HIP 17678"],
      ["HIP 17678","HIP 2021"],
      ["HIP 2021","HIP 9236"],
      ["HIP 101772","HIP 105319"],
      ["HIP 105319","HIP 108431"],
      ["HIP 108431","HIP 108870"],
      ["HIP 108870","HIP 108431"],
      ["HIP 108431","HIP 105319"],
      ["HIP 105319","HIP 104085"],
      ["HIP 104085","HIP 103227"],
      ["Praecipua","Beta LMi"],
      ["Beta LMi","HIP 49593"],
      ["HIP 109937","HIP 109754"],
      ["HIP 109754","HIP 111104"],
      ["HIP 111104","HIP 111944"],
      ["HIP 111944","HIP 111104"],
      ["HIP 111104","HIP 110351"],
      ["HIP 110351","HIP 111104"],
      ["HIP 111104","HIP 111022"],
      ["HIP 111022","HIP 110609"],
      ["HIP 110609","Beta Lacertae"],
      ["Beta Lacertae","Alpha Lacertae"],
      ["Alpha Lacertae","HIP 111022"],
      ["Ras Elased Australis","Rasalas"],
      ["Rasalas","Adhafera"],
      ["Adhafera","Algieba"],
      ["Algieba","HIP 49583"],
      ["HIP 49583","Regulus"],
      ["Regulus","Subra"],
      ["Subra","Regulus"],
      ["Regulus","Chertan"],
      ["Chertan","HIP 55642"],
      ["HIP 55642","HIP 55434"],
      ["HIP 55434","HIP 55642"],
      ["HIP 55642","Chertan"],
      ["Chertan","Denebola"],
      ["Denebola","Zosma"],
      ["Zosma","Algieba"],
      ["Ping","Nihal"],
      ["Nihal","Arneb"],
      ["Arneb","Bade"],
      ["Bade","Arneb"],
      ["Arneb","Darlugal"],
      ["Darlugal","HIP 28103"],
      ["HIP 77853","Zubenelhakrabi"],
      ["Zubenelhakrabi","Zubeneschamali"],
      ["Zubeneschamali","Zubenelgenubi"],
      ["Zubenelgenubi","Brachium"],
      ["Brachium","Zubeneschamali"],
      ["Zubeneschamali","Brachium"],
      ["Brachium","HIP 76470"],
      ["HIP 76470","HIP 76600"],
      ["HIP 78918","HIP 78384"],
      ["HIP 78384","HIP 76297"],
      ["HIP 76297","HIP 75264"],
      ["HIP 75264","HIP 74376"],
      ["HIP 74376","HIP 74395"],
      ["HIP 74395","Uridim"],
      ["Uridim","HIP 73273"],
      ["HIP 73273","HIP 75141"],
      ["HIP 75141","HIP 76297"],
      ["HIP 76297","HIP 75141"],
      ["HIP 75141","HIP 75177"],
      ["HIP 75177","HIP 77634"],
      ["Alpha Lyncis","38 Lyncis"],
      ["38 Lyncis","HIP 44700"],
      ["HIP 44700","HIP 44248"],
      ["HIP 44248","Alsciaukat"],
      ["Alsciaukat","HIP 36145"],
      ["HIP 36145","HIP 33449"],
      ["HIP 33449","HIP 30060"],
      ["Vega","Zeta Lyr"],
      ["Zeta Lyr","Sheliak"],
      ["Sheliak","Sulafat"],
      ["Sulafat","HIP 92791"],
      ["HIP 92791","Zeta Lyr"],
      ["Hoerikwaggo","HIP 25918"],
      ["HIP 25918","HIP 22871"],
      ["HIP 22871","HIP 23467"],
      ["HIP 105382","HIP 105140"],
      ["HIP 105140","HIP 103738"],
      ["HIP 103738","HIP 102831"],
      ["HIP 31216","HIP 30419"],
      ["HIP 30419","HIP 32578"],
      ["HIP 32578","HIP 34769"],
      ["HIP 34769","HIP 30867"],
      ["HIP 30867","HIP 29651"],
      ["HIP 29651","HIP 30867"],
      ["HIP 30867","HIP 34769"],
      ["HIP 34769","HIP 37447"],
      ["HIP 37447","HIP 39863"],
      ["HIP 62322","HIP 61585"],
      ["HIP 61585","HIP 63613"],
      ["HIP 63613","HIP 61585"],
      ["HIP 61585","HIP 61199"],
      ["HIP 61199","HIP 61585"],
      ["HIP 61585","HIP 59929"],
      ["HIP 59929","HIP 57363"],
      ["HIP 80582","HIP 80000"],
      ["HIP 80000","HIP 78639"],
      ["HIP 112405","HIP 70638"],
      ["HIP 70638","HIP 107089"],
      ["HIP 107089","HIP 112405"],
      ["Rasalhague","HIP 83000"],
      ["HIP 83000","HIP 80883"],
      ["HIP 80883","Yed Prior"],
      ["Yed Prior","Yed Posterior"],
      ["Yed Posterior","HIP 81377"],
      ["HIP 81377","Sabik"],
      ["Sabik","Cebalrai"],
      ["Cebalrai","Rasalhague"],
      ["Alnitak","Saiph"],
      ["Saiph","Rigel"],
      ["Rigel","Mintaka"],
      ["Mintaka","Bellatrix"],
      ["Bellatrix","Meissa"],
      ["Meissa","Betelgeuse"],
      ["Betelgeuse","μ Ori"],
      ["μ Ori","ξ Ori"],
      ["ξ Ori","ν Ori"],
      ["ν Ori","HIP 27913"],
      ["HIP 27913","χ² Ori"],
      ["χ² Ori","ξ Ori"],
      ["ξ Ori","μ Ori"],
      ["μ Ori","Betelgeuse"],
      ["Betelgeuse","Alnitak"],
      ["Alnitak","Alnilam"],
      ["Alnilam","Mintaka"],
      ["Mintaka","Bellatrix"],
      ["Bellatrix","π³ Ori"],
      ["π³ Ori","π⁴ Ori"],
      ["π⁴ Ori","π⁵ Ori"],
      ["π⁵ Ori","π⁶ Ori"],
      ["π⁶ Ori","π⁵ Ori"],
      ["π⁵ Ori","π⁴ Ori"],
      ["π⁴ Ori","π³ Ori"],
      ["π³ Ori","π² Ori"],
      ["π² Ori","HIP 22957"],
      ["HIP 22957","HIP 23607"],
      ["HIP 23607","HIP 24010"],
      ["Peacock","HIP 102395"],
      ["HIP 102395","HIP 105858"],
      ["HIP 105858","HIP 102395"],
      ["HIP 102395","HIP 98495"],
      ["HIP 98495","HIP 91792"],
      ["HIP 91792","HIP 86929"],
      ["HIP 86929","HIP 88866"],
      ["HIP 88866","HIP 90098"],
      ["HIP 90098","HIP 92609"],
      ["HIP 92609","HIP 99240"],
      ["HIP 99240","HIP 102395"],
      ["HIP 109410","Matar"],
      ["Matar","Scheat"],
      ["Scheat","Alpheratz"],
      ["Alpheratz","Scheat"],
      ["Scheat","Markab"],
      ["Markab","Algenib"],
      ["Algenib","Alpheratz"],
      ["Alpheratz","Algenib"],
      ["Algenib","Markab"],
      ["Markab","Homam"],
      ["Homam","Biham"],
      ["Biham","Enif"],
      ["Algol","Misam"],
      ["Misam","Mirfak"],
      ["Mirfak","Gamma Per"],
      ["Gamma Per","Mirfak"],
      ["Mirfak","Delta Per"],
      ["Delta Per","Epsilon Per"],
      ["Epsilon Per","HIP 18614"],
      ["HIP 18614","Menkib"],
      ["Ankaa","HIP 5165"],
      ["HIP 5165","HIP 6867"],
      ["HIP 6867","HIP 7083"],
      ["HIP 7083","HIP 6867"],
      ["HIP 6867","HIP 5165"],
      ["HIP 5165","Wurren"],
      ["Wurren","HIP 3405"],
      ["HIP 3405","HIP 765"],
      ["HIP 765","Ankaa"],
      ["HIP 32607","HIP 27530"],
      ["HIP 27530","HIP 27321"],
      ["Fomalhaut","HIP 111954"],
      ["HIP 111954","HIP 109789"],
      ["HIP 109789","HIP 108661"],
      ["HIP 108661","HIP 107608"],
      ["HIP 107608","HIP 107380"],
      ["HIP 107380","HIP 109285"],
      ["HIP 109285","HIP 111188"],
      ["HIP 111188","HIP 112948"],
      ["HIP 112948","HIP 113246"],
      ["HIP 113246","Fomalhaut"],
      ["HIP 5742","HIP 6193"],
      ["HIP 6193","HIP 5586"],
      ["HIP 5586","HIP 5742"],
      ["HIP 5742","Alpherg"],
      ["Alpherg","Torcular"],
      ["Torcular","Alrescha"],
      ["Alrescha","HIP 7884"],
      ["HIP 7884","HIP 4906"],
      ["HIP 4906","HIP 3786"],
      ["HIP 3786","Omega Psc"],
      ["Omega Psc","Iota Psc"],
      ["Iota Psc","HIP 115830"],
      ["HIP 115830","HIP 115227"],
      ["HIP 115227","Gamma Piscium"],
      ["Gamma Piscium","HIP 115738"],
      ["HIP 115738","HIP 116928"],
      ["HIP 116928","HIP 117245"],
      ["HIP 117245","Iota Psc"],
      ["Tureis","Azmidi"],
      ["Azmidi","HIP 37229"],
      ["HIP 37229","HIP 36917"],
      ["HIP 36917","HIP 35264"],
      ["HIP 35264","Pipit"],
      ["Pipit","HIP 32768"],
      ["HIP 32768","HIP 36377"],
      ["HIP 36377","Naos"],
      ["Naos","Tureis"],
      ["HIP 42515","HIP 42828"],
      ["HIP 42828","HIP 43409"],
      ["Rhombus","HIP 19921"],
      ["HIP 19921","HIP 18597"],
      ["HIP 18597","HIP 18744"],
      ["HIP 18744","HIP 17440"],
      ["HIP 17440","Rhombus"],
      ["HIP 4577","HIP 117452"],
      ["HIP 117452","HIP 115102"],
      ["HIP 115102","HIP 116231"],
      ["Acrab B","Alniyat"],
      ["Alniyat","Dschubba"],
      ["Dschubba","Alniyat"],
      ["Alniyat","Fang"],
      ["Fang","Alniyat"],
      ["Alniyat","Antares"],
      ["Antares","Paikauhale"],
      ["Paikauhale","Larawag"],
      ["Larawag","Xamidimura"],
      ["Xamidimura","HIP 82729"],
      ["HIP 82729","HIP 84143"],
      ["HIP 84143","Sargas"],
      ["Sargas","HIP 87073"],
      ["HIP 87073","HIP 86670"],
      ["HIP 86670","Shaula"],
      ["Shaula","Lesath"],
      ["HIP 92175","HIP 91117"],
      ["HIP 91117","HIP 90595"],
      ["HIP 90595","HIP 91117"],
      ["HIP 91117","HIP 90135"],
      ["HIP 84880","HIP 86263"],
      ["HIP 86263","HIP 86565"],
      ["HIP 86565","HIP 89962"],
      ["HIP 89962","Alya"],
      ["Chow","Gudja"],
      ["Gudja","HIP 78072"],
      ["HIP 78072","Chow"],
      ["Chow","HIP 76276"],
      ["HIP 76276","Unukalhai"],
      ["Unukalhai","HIP 77622"],
      ["HIP 77622","HIP 77516"],
      ["HIP 77516","HIP 84880"],
      ["HIP 51437","HIP 49641"],
      ["HIP 49641","HIP 48437"],
      ["Gamma Sge","HIP 97365"],
      ["HIP 97365","Sham"],
      ["Sham","HIP 97365"],
      ["HIP 97365","Beta Sge"],
      ["Rukbat","HIP 98032"],
      ["HIP 98032","Arkab Prior"],
      ["Arkab Prior","HIP 98032"],
      ["HIP 98032","HIP 98412"],
      ["HIP 98412","HIP 98688"],
      ["HIP 98688","HIP 98162"],
      ["HIP 98162","HIP 96465"],
      ["HIP 96465","HIP 93864"],
      ["HIP 93864","Nunki"],
      ["Nunki","HIP 93683"],
      ["HIP 93683","Albaldah"],
      ["Albaldah","HIP 95168"],
      ["HIP 95168","HIP 95176"],
      ["HIP 95176","HIP 95168"],
      ["HIP 95168","Albaldah"],
      ["Albaldah","HIP 93683"],
      ["HIP 93683","HIP 93085"],
      ["HIP 93085","HIP 93683"],
      ["HIP 93683","Nunki"],
      ["Nunki","HIP 92041"],
      ["HIP 92041","Ascella"],
      ["Ascella","HIP 93864"],
      ["HIP 93864","Ascella"],
      ["Ascella","HIP 92041"],
      ["HIP 92041","Kaus Borealis"],
      ["Kaus Borealis","Kaus Media"],
      ["Kaus Media","Alnasl"],
      ["Alnasl","Kaus Media"],
      ["Kaus Media","Kaus Australis"],
      ["Kaus Australis","HIP 89642"],
      ["Tianguis","Aldebaran"],
      ["Aldebaran","Gamma Tau"],
      ["Gamma Tau","Bibing"],
      ["Bibing","HIP 15900"],
      ["HIP 15900","Bibing"],
      ["Bibing","Gamma Tau"],
      ["Gamma Tau","Secunda Hyadum"],
      ["Secunda Hyadum","Ain"],
      ["Ain","Elnath"],
      ["HIP 89112","HIP 90422"],
      ["HIP 90422","HIP 90568"],
      ["Atria","HIP 74946"],
      ["HIP 74946","HIP 76440"],
      ["HIP 76440","HIP 77952"],
      ["HIP 77952","Atria"],
      ["Beta Trianguli","Gamma Tri"],
      ["Gamma Tri","Mothallah"],
      ["Mothallah","Beta Trianguli"],
      ["HIP 110838","Lang-Exster"],
      ["Lang-Exster","HIP 114996"],
      ["HIP 114996","HIP 118322"],
      ["HIP 118322","HIP 1599"],
      ["HIP 1599","HIP 2484"],
      ["HIP 2484","HIP 114996"],
      ["Alkaid","Mizar"],
      ["Mizar","Alioth"],
      ["Alioth","Megrez"],
      ["Megrez","Phecda"],
      ["Phecda","Taiyangshou"],
      ["Taiyangshou","Alula Borealis"],
      ["Alula Borealis","Alula Australis"],
      ["Alula Australis","Alula Borealis"],
      ["Alula Borealis","Taiyangshou"],
      ["Taiyangshou","HIP 54539"],
      ["HIP 54539","Tania Australis"],
      ["Tania Australis","Tania Borealis"],
      ["Tania Borealis","Tania Australis"],
      ["Tania Australis","HIP 54539"],
      ["HIP 54539","Taiyangshou"],
      ["Taiyangshou","Phecda"],
      ["Phecda","Merak"],
      ["Merak","Dubhe"],
      ["Dubhe","Megrez"],
      ["Megrez","Dubhe"],
      ["Dubhe","HIP 46733"],
      ["HIP 46733","Muscida"],
      ["Muscida","HIP 48319"],
      ["HIP 48319","Merak"],
      ["Merak","HIP 48319"],
      ["HIP 48319","HIP 46853"],
      ["HIP 46853","Alkaphrah"],
      ["Alkaphrah","Talitha"],
      ["Polaris","Yildun"],
      ["Yildun","Epsilon UMi"],
      ["Epsilon UMi","Zeta UMi"],
      ["Zeta UMi","Kochab"],
      ["Kochab","Pherkad"],
      ["Pherkad","Eta UMi"],
      ["Eta UMi","Zeta UMi"],
      ["HR 3206","Alsephina"],
      ["Alsephina","Markeb"],
      ["Markeb","HIP 48774"],
      ["HIP 48774","HIP 52727"],
      ["HIP 52727","HIP 51986"],
      ["HIP 51986","HIP 50191"],
      ["HIP 50191","HIP 46651"],
      ["HIP 46651","Suhail"],
      ["Suhail","HIP 42884"],
      ["HIP 42884","HIP 42312"],
      ["HIP 42312","HR 3206"],
      ["Spica","HIP 64238"],
      ["HIP 64238","Porrima"],
      ["Porrima","Minelauva"],
      ["Minelauva","Heze"],
      ["Heze","Spica"],
      ["HIP 44382","HIP 41312"],
      ["HIP 41312","HIP 39794"],
      ["HIP 39794","HIP 35228"],
      ["HIP 35228","HIP 34481"],
      ["HIP 34481","HIP 37504"],
      ["HIP 37504","HIP 39794"],
      ["HIP 94703","Anser"],
      ["Anser","HIP 97886"]
    ];

    const CONSTELLATION_CENTERS = [
      { name: "Andromède", ra: 0.5168022, dec: 38.651635 },
      { name: "Machine pneumatique", ra: 10.1468648, dec: -33.275426 },
      { name: "Oiseau de paradis", ra: 16.1320551, dec: -78.756571 },
      { name: "Aigle", ra: 19.5845857, dec: 3.623183 },
      { name: "Verseau", ra: 22.5791568, dec: -8.139784 },
      { name: "Autel", ra: 17.3666807, dec: -55.236071 },
      { name: "Bélier", ra: 2.4871119, dec: 22.175671 },
      { name: "Cocher", ra: 5.3523901, dec: 39.665044 },
      { name: "Bouvier", ra: 14.6013015, dec: 37.716583 },
      { name: "Grand Chien", ra: 6.9231974, dec: -21.648527 },
      { name: "Petit Chien", ra: 7.5540849, dec: 6.759575 },
      { name: "Chiens de chasse", ra: 12.752197, dec: 39.871232 },
      { name: "Burin", ra: 4.6338731, dec: -41.327694 },
      { name: "Girafe", ra: 4.2486959, dec: 62.689433 },
      { name: "Capricorne", ra: 21.072337, dec: -18.909866 },
      { name: "Carène", ra: 8.8729772, dec: -62.758344 },
      { name: "Cassiopée", ra: 0.9898305, dec: 60.362506 },
      { name: "Centaure", ra: 13.3393012, dec: -50.270689 },
      { name: "Céphée", ra: 22.1480391, dec: 67.447285 },
      { name: "Baleine", ra: 2.1325646, dec: -4.464876 },
      { name: "Caméléon", ra: 9.9170763, dec: -79.460404 },
      { name: "Compas", ra: 15.1564319, dec: -61.101656 },
      { name: "Cancer", ra: 8.6960359, dec: 17.912149 },
      { name: "Colombe", ra: 5.9127255, dec: -36.022716 },
      { name: "Chevelure de Bérénice", ra: 12.9445573, dec: 24.65061 },
      { name: "Couronne australe", ra: 19.1001113, dec: -39.005475 },
      { name: "Couronne boréale", ra: 15.7312899, dec: 28.070588 },
      { name: "Coupe", ra: 11.4551146, dec: -16.261251 },
      { name: "Croix du Sud", ra: 12.5027004, dec: -59.695044 },
      { name: "Corbeau", ra: 12.3298251, dec: -20.981135 },
      { name: "Cygne", ra: 20.2398079, dec: 39.052473 },
      { name: "Dauphin", ra: 20.6679143, dec: 14.605033 },
      { name: "Dorade", ra: 5.1043399, dec: -59.547562 },
      { name: "Dragon", ra: 16.6868213, dec: 67.958937 },
      { name: "Petit Cheval", ra: 21.2649945, dec: 8.051015 },
      { name: "Éridan", ra: 3.6152468, dec: -27.092233 },
      { name: "Fourneau", ra: 2.6977165, dec: -30.41916 },
      { name: "Gémeaux", ra: 7.027206, dec: 24.477801 },
      { name: "Grue", ra: 22.5836247, dec: -45.434269 },
      { name: "Hercule", ra: 17.2260833, dec: 34.349786 },
      { name: "Horloge", ra: 3.2320671, dec: -52.393084 },
      { name: "Hydre", ra: 10.0499159, dec: -9.713464 },
      { name: "Hydre mâle", ra: 2.2829737, dec: -71.011294 },
      { name: "Indien", ra: 21.3087613, dec: -54.546622 },
      { name: "Petit Lion", ra: 10.4941463, dec: 35.480578 },
      { name: "Lézard", ra: 22.423463, dec: 45.696462 },
      { name: "Lion", ra: 10.6123594, dec: 16.843199 },
      { name: "Lièvre", ra: 5.5112915, dec: -17.740653 },
      { name: "Balance", ra: 15.4184526, dec: -20.093198 },
      { name: "Loup", ra: 15.4539713, dec: -42.249762 },
      { name: "Lynx", ra: 8.4224904, dec: 46.198649 },
      { name: "Lyre", ra: 18.8204651, dec: 35.883198 },
      { name: "Table", ra: 5.3947294, dec: -74.459584 },
      { name: "Microscope", ra: 21.1201958, dec: -34.792523 },
      { name: "Licorne", ra: 6.9351455, dec: -1.521712 },
      { name: "Mouche", ra: 12.4800017, dec: -69.377935 },
      { name: "Règle", ra: 16.2807436, dec: -49.00745 },
      { name: "Octant", ra: 20.9660718, dec: -83.57822 },
      { name: "Ophiuchus", ra: 16.8865419, dec: -0.776595 },
      { name: "Orion", ra: 5.4584502, dec: 7.279544 },
      { name: "Paon", ra: 19.4531059, dec: -66.15351 },
      { name: "Pégase", ra: 22.8763317, dec: 20.173329 },
      { name: "Persée", ra: 3.5699962, dec: 43.216562 },
      { name: "Phénix", ra: 0.9312856, dec: -48.783047 },
      { name: "Peintre", ra: 6.0897498, dec: -56.573744 },
      { name: "Poisson austral", ra: 22.3911922, dec: -30.923963 },
      { name: "Poissons", ra: 0.5286577, dec: 9.920307 },
      { name: "Poupe", ra: 7.5369253, dec: -35.590585 },
      { name: "Boussole", ra: 8.7481777, dec: -32.073712 },
      { name: "Réticule", ra: 4.058144, dec: -62.058726 },
      { name: "Sculpteur", ra: 23.9247807, dec: -32.324655 },
      { name: "Scorpion", ra: 16.8562783, dec: -33.912243 },
      { name: "Écu", ra: 18.5643693, dec: -9.128561 },
      { name: "Serpent", ra: 16.6987561, dec: 2.408354 },
      { name: "Sextant", ra: 10.171698, dec: -3.043167 },
      { name: "Flèche", ra: 19.7798447, dec: 18.388095 },
      { name: "Sagittaire", ra: 19.1234443, dec: -29.180182 },
      { name: "Taureau", ra: 4.5152631, dec: 17.766542 },
      { name: "Télescope", ra: 18.3703074, dec: -47.014825 },
      { name: "Triangle austral", ra: 15.902661, dec: -67.069058 },
      { name: "Triangle", ra: 2.1070694, dec: 32.830588 },
      { name: "Toucan", ra: 23.4586119, dec: -63.412813 },
      { name: "Grande Ourse", ra: 10.9617629, dec: 52.511244 },
      { name: "Petite Ourse", ra: 15.7632489, dec: 80.018759 },
      { name: "Voiles", ra: 9.4334444, dec: -48.022395 },
      { name: "Vierge", ra: 13.1561384, dec: -3.077846 },
      { name: "Poisson volant", ra: 7.9959181, dec: -69.000029 },
      { name: "Petit Renard", ra: 19.5447817, dec: 23.426191 }
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
      else if (type.includes("planète") || type.includes("planete")) { dist = "Système Solaire"; size = "Diamètre variable"; }
      else if (name === "LUNE") { dist = "384 400 km"; size = "31'"; }
      else if (name === "SOLEIL") { dist = "149.6 millions km"; size = "32'"; }
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

      const azimuth = ((rawAZ * dirAZ / ticksAZ * 360) % 360 + 360) % 360;
      const altitudeCircle = ((rawALT * dirALT / ticksALT * 360) % 360 + 360) % 360;
      
      targetHeading = azimuth;
      targetElevation = altitudeCircle > 180 ? altitudeCircle - 360 : altitudeCircle;
      const phone = !window.__dobsonSimulationActive?.() && window.__dobsonAutoFollow && window.__dobsonPhoneOrientation;
      if (phone && Number.isFinite(phone.azimuth) && Number.isFinite(phone.altitude)) {
        targetHeading = ((phone.azimuth % 360) + 360) % 360;
        targetElevation = phone.altitude;
      }
      
      telescopeReadout.textContent = `AZ ${azimuth.toFixed(1)}° · ALT ${targetElevation.toFixed(1)}°`;
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
      const type = (object.type || '').normalize('NFD').replace(/[\u0300-\u036f]/g, '').toUpperCase();
      const name = (object.name || '').toUpperCase();

      if (name === 'POLARIS') return '#ffedcc';
      if (name === 'SOLEIL') return '#ffcc00';
      if (name === 'LUNE') return '#f5f5f0';
      if (name === 'MERCURE') return '#b0bec5';
      if (name === 'VÉNUS' || name === 'VENUS') return '#fff59d';
      if (name === 'MARS') return '#ff8a65';
      if (name === 'JUPITER') return '#ffb74d';
      if (name === 'SATURNE') return '#ffe082';
      if (name === 'URANUS') return '#80deea';
      if (name === 'NEPTUNE') return '#4fc3f7';
      if (name === 'PLUTON') return '#b0bec5';

      if (type.includes('PLAN')) return '#80d8ff';
      if (type.includes('GAL')) return '#e0b8ff';
      if (type.includes('NEB')) return '#ff9e84';
      if (type.includes('AMAS') || type.includes('GLOB')) return '#ffe066';

      if (object.mag < 1.0) return '#ffffff';
      if (object.mag < 2.5) return '#e0f0ff';
      return '#c2dcff';
    }

    function getObjectType(object) {
      const type = (object.type || '').normalize('NFD').replace(/[\u0300-\u036f]/g, '').toLowerCase();
      const isGal = type.includes('gal');
      const isNeb = type.includes('neb');
      const isAmas = type.includes('amas') || type.includes('glob');
      const isStar = !isGal && !isNeb && !isAmas;
      return { isGal, isNeb, isAmas, isStar };
    }

    function matchesFilter(object) {
      const userMaxMag = getSelectedMagnitude();

      if (object.mag > userMaxMag && !(document.getElementById('showMessierAll').checked && isMessierObject(object))) return false;

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

    function isMessierObject(object) {
      return /^M(?:[1-9]|[1-9]\d|10\d|110)$/i.test((object.name || '').trim());
    }

    // V5.73.0 Refactoring de la fonction pour supporter les projections mathématiques
    function horizontalPosition(object, siderealDegrees, latRadians, useMobileCache = true) {
      const dec = object.dec * Math.PI / 180;
      const hourAngle = (siderealDegrees - object.ra * 15) * Math.PI / 180;
      const latRad = latRadians;

      // Coordonnées horizontales brutes (Alt/Az mathématiques)
      const sinAltitude = Math.sin(dec) * Math.sin(latRad) + Math.cos(dec) * Math.cos(latRad) * Math.cos(hourAngle);
      const altitudeRad = Math.asin(Math.max(-1, Math.min(1, sinAltitude)));
      const azimuthRad = Math.atan2(
        -Math.sin(hourAngle) * Math.cos(dec),
        Math.sin(dec) * Math.cos(latRad) - Math.cos(dec) * Math.sin(latRad) * Math.cos(hourAngle)
      );
      
      // Coordonnées pour l'UI
      const altDeg = altitudeRad * 180 / Math.PI;
      const azDeg = (azimuthRad * 180 / Math.PI + 360) % 360;

      // Position 3D brute (modèle actuel / perspective)
      const cosAltitudeBrut = Math.cos(altitudeRad);
      const positionBrute = [Math.sin(azimuthRad) * cosAltitudeBrut, Math.sin(altitudeRad), Math.cos(azimuthRad) * cosAltitudeBrut];

      return {
        altitude: altDeg,
        azimuth: azDeg,
        position: positionBrute, // Par défaut, modèle actuel
        raw: { // Coordonnées mathématiques pour projections
          alt: altitudeRad,
          az: azimuthRad
        }
      };
    }

    function siderealDegrees(date, observerLongitude = longitude) {
      const julianDate = date.getTime() / 86400000 + 2440587.5;
      const centuries = (julianDate - 2451545.0) / 36525;
      const greenwich = 280.46061837 + 360.98564736629 * (julianDate - 2451545.0)
        + 0.000387933 * centuries * centuries - centuries * centuries * centuries / 38710000;
      return ((greenwich + observerLongitude) % 360 + 360) % 360;
    }

    window.__dobsonCaptureCalibration = () => {
      const phone = window.__dobsonPhoneOrientation;
      if (!useSystemTime || !window.__dobsonAutoFollow || !phone || !phone.headingAvailable
        || !Number.isFinite(phone.azimuth) || !Number.isFinite(phone.altitude)
        || !Number.isFinite(phone.receivedAt) || Date.now() - phone.receivedAt > 2000) return null;
      return { azimuth: phone.azimuth, altitude: phone.altitude, latitude, longitude, time: Date.now() };
    };

    window.__dobsonApplyPhotoCalibration = (solution, capture) => {
      if (!capture) throw new Error('Photo résolue, mais réalignement impossible : utilisez les capteurs et la date réelle avant la prise de vue.');
      if (!window.__dobsonAutoFollow || !useSystemTime || !sceneDiv.classList.contains('dobson-ar-active')) {
        throw new Error('Photo résolue, mais réactivez la caméra AR, les capteurs et la date réelle pour réaligner la vue.');
      }
      if (Math.abs(latitude - capture.latitude) > 0.0001 || Math.abs(longitude - capture.longitude) > 0.0001) {
        throw new Error('Photo résolue, mais le lieu a changé depuis la prise de vue ; prenez une nouvelle photo.');
      }
      const sidereal = siderealDegrees(new Date(capture.time), capture.longitude);
      const position = horizontalPosition(solution, sidereal, capture.latitude * Math.PI / 180, false);
      if (Math.abs(capture.altitude) > 89 || Math.abs(position.altitude) > 89) {
        throw new Error('Photo résolue, mais le réalignement près du zénith ou du nadir est imprécis ; inclinez le téléphone.');
      }
      cameraCalibrationHeading = ((position.azimuth - capture.azimuth + 540) % 360) - 180;
      cameraCalibrationElevation = position.altitude - capture.altitude;
      touchHeading = 0;
      touchElevation = 0;
      alignOffsetHeading = 0;
      alignOffsetElevation = 0;
      window.dispatchEvent(new Event('dobson:resetTouchOffset'));
    };

    window.__dobsonSetTelescopeMarker = (x, y) => {
      const aspect = canvas.clientWidth / canvas.clientHeight;
      if (!Number.isFinite(x) || !Number.isFinite(y) || !Number.isFinite(aspect) || aspect <= 0) return false;
      const fov = fieldOfView * Math.PI / 180;
      let right, up, forward;
      if (projectionMode === 'stéréo') {
        const px = x * fov * aspect;
        const py = y * fov;
        const scale = 1 + (px * px + py * py) / 4;
        right = px / scale;
        up = py / scale;
        forward = (2 - scale) / scale;
        if (forward <= 0) return false;
      } else {
        right = x * Math.tan(fov / 2) * aspect;
        up = y * Math.tan(fov / 2);
        forward = 1;
      }
      const length = Math.hypot(right, up, forward);
      telescopeCameraDirection = [right / length, up / length, forward / length];
      return true;
    };

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

    function normalizeSkyAngles(azimuth, altitude) {
      let normalizedAzimuth = ((azimuth % 360) + 360) % 360;
      let normalizedAltitude = ((altitude + 180) % 360 + 360) % 360 - 180;
      if (normalizedAltitude > 90) {
        normalizedAzimuth = (normalizedAzimuth + 180) % 360;
        normalizedAltitude = 180 - normalizedAltitude;
      } else if (normalizedAltitude < -90) {
        normalizedAzimuth = (normalizedAzimuth + 180) % 360;
        normalizedAltitude = -180 - normalizedAltitude;
      }
      return { azimuth: normalizedAzimuth, altitude: normalizedAltitude };
    }

    function skyCameraBasis(azimuth, altitude) {
      const azimuthRadians = (((azimuth % 360) + 360) % 360) * Math.PI / 180;
      const altitudeRadians = (((altitude % 360) + 360) % 360) * Math.PI / 180;
      const forward = [
        Math.sin(azimuthRadians) * Math.cos(altitudeRadians),
        Math.sin(altitudeRadians),
        Math.cos(azimuthRadians) * Math.cos(altitudeRadians)
      ];
      const right = [Math.cos(azimuthRadians), 0, -Math.sin(azimuthRadians)];
      const up = [
        forward[1] * right[2] - forward[2] * right[1],
        forward[2] * right[0] - forward[0] * right[2],
        forward[0] * right[1] - forward[1] * right[0]
      ];
      return { forward, right, up };
    }

    const objectLabelStates = new Map();
    const constellationLabelStates = new Map();
    let lastObjectLabelLayout = -Infinity;
    let lastObjectLabelLayoutKey = '';

    function updateObjectLabels(screenObjects, width, height, now) {
      const labelGal = document.getElementById('labelGal').checked;
      const labelNeb = document.getElementById('labelNeb').checked;
      const labelAmas = document.getElementById('labelAmas').checked;
      const labelStars = document.getElementById('labelStars').checked;
      const layoutKey = [projectionMode, fieldOfView, showSkyNames, labelGal, labelNeb, labelAmas, labelStars].join('|');
      const updateLayout = !window.__dobsonMobileApp || layoutKey !== lastObjectLabelLayoutKey
        || now - lastObjectLabelLayout >= 80;

      if (updateLayout) {
        lastObjectLabelLayout = now;
        lastObjectLabelLayoutKey = layoutKey;
        const labelMagLimit = fieldOfView > 40 ? 2.5
          : fieldOfView > 25 ? 5.0 : fieldOfView > 15 ? 8.5 : 13.0;
        const candidates = screenObjects.filter(item => {
          if (!showSkyNames) return false;
          const object = item.object;
          if (object.name === 'Polaris' || object.type === 'Planète'
            || object.name === 'Lune' || object.name === 'Soleil') return true;
          const { isGal, isNeb, isAmas, isStar } = getObjectType(object);
          return ((isGal && labelGal) || (isNeb && labelNeb) || (isAmas && labelAmas))
            && (fieldOfView < 60 || object.mag <= labelMagLimit)
            || isStar && labelStars && object.mag <= labelMagLimit;
        });
        candidates.sort((a, b) => {
          const aSelected = objectLabelStates.get(a.object)?.selected ? 1 : 0;
          const bSelected = objectLabelStates.get(b.object)?.selected ? 1 : 0;
          if (aSelected !== bSelected) return bSelected - aSelected;
          return projectionMode !== 'actuel' ? b.depth - a.depth : a.object.mag - b.object.mag;
        });

        const selected = new Set();
        const placedLabelGrid = new Map();
        const collisionPanFactor = projectionMode !== 'actuel' ? 60 / fieldOfView : 34 / fieldOfView;
        const thresholdX = Math.max(1.0, 6.5 / collisionPanFactor);
        const thresholdY = Math.max(0.8, 4.0 / collisionPanFactor);
        const cellWidth = thresholdX * 1.2;
        const cellHeight = thresholdY * 1.2;

        for (const item of candidates) {
          if (window.__dobsonMobileApp && selected.size >= 250) break;
          const left = (item.screenX + 1) * 50;
          const top = (1 - item.screenY) * 50;
          const cellX = Math.floor(left / cellWidth);
          const cellY = Math.floor(top / cellHeight);
          const margin = objectLabelStates.get(item.object)?.selected ? 1 : 1.2;
          let overlap = false;
          for (let x = cellX - 1; x <= cellX + 1 && !overlap; x++) {
            for (let y = cellY - 1; y <= cellY + 1 && !overlap; y++) {
              const cell = placedLabelGrid.get(x + ':' + y);
              overlap = Boolean(cell && cell.some(p =>
                Math.abs(p.left - left) < thresholdX * margin && Math.abs(p.top - top) < thresholdY * margin));
            }
          }
          if (overlap) continue;

          const cellKey = cellX + ':' + cellY;
          if (!placedLabelGrid.has(cellKey)) placedLabelGrid.set(cellKey, []);
          placedLabelGrid.get(cellKey).push({ left, top });
          selected.add(item.object);

          if (!objectLabelStates.has(item.object)) {
            const object = item.object;
            const isPolaris = object.name === 'Polaris';
            const isPlanetOrBody = object.type === 'Planète' || object.name === 'Lune' || object.name === 'Soleil';
            const { isGal, isNeb, isAmas } = getObjectType(object);
            const label = document.createElement('span');
            label.className = isPolaris ? 'object-label polaris' : 'object-label';
            label.style.color = isPolaris ? '#ffedcc' : isPlanetOrBody ? '#80d8ff'
              : isGal ? '#e0b8ff' : isNeb ? '#ff9e84' : isAmas ? '#ffe066' : '#c2dcff';
            label.textContent = /^(?:HIP|HR)\s/.test(object.name)
              ? object.commonName || object.name
              : !isPlanetOrBody && !isPolaris
              && (object.name.startsWith('M') || object.name.startsWith('NGC'))
              ? `${object.name}${object.commonName ? ' (' + object.commonName + ')' : ''}`
              : object.name || object.commonName;
            label.style.opacity = '0';
            labelsElement.appendChild(label);
            objectLabelStates.set(object, { label, selected: false, retiredAt: now });
          }
        }

        for (const [object, state] of objectLabelStates) {
          const wasSelected = state.selected;
          state.selected = selected.has(object);
          if (wasSelected && !state.selected) state.retiredAt = now;
        }
      }

      const onScreen = new Set();
      for (const item of screenObjects) {
        const state = objectLabelStates.get(item.object);
        if (!state) continue;
        onScreen.add(item.object);
        const x = (item.screenX + 1) * width / 2;
        const y = (1 - item.screenY) * height / 2 - 16;
        state.label.style.transform = `translate3d(${x}px, ${y}px, 0)`;
      }
      for (const [object, state] of objectLabelStates) {
        state.label.style.opacity = state.selected && onScreen.has(object) ? '1' : '0';
        if (!state.selected && now - state.retiredAt >= 150) {
          state.label.remove();
          objectLabelStates.delete(object);
        }
      }
      return updateLayout;
    }

    // --- MOTEUR DE RENDU V5.73.0 (SUPPORTE LES PROJECTIONS) ---
    function render() {
      requestAnimationFrame(render);
      if (objects.length === 0) return;
      resizeCanvas();
      
      const width = canvas.clientWidth;
      const height = canvas.clientHeight;
      const aspect = width / height;
      
      // Lissage des mouvements de caméra
      let diffAZ = targetHeading - heading;
      if (diffAZ > 180) diffAZ -= 360;
      if (diffAZ < -180) diffAZ += 360;
      heading = (heading + diffAZ * 0.4 + 360) % 360;
      elevation += (targetElevation - elevation) * 0.4;

      // Calcul de l'orientation finale de la caméra
      const cameraHeading = (heading + touchHeading + alignOffsetHeading + cameraCalibrationHeading + 360) % 360;
      const cameraElevation = elevation + touchElevation + alignOffsetElevation + cameraCalibrationElevation;
      const normalizedCamera = normalizeSkyAngles(cameraHeading, cameraElevation);
      const currentHeading = normalizedCamera.azimuth;
      const currentElevation = normalizedCamera.altitude;
      
      const camAzRad = (((cameraHeading % 360) + 360) % 360) * Math.PI / 180;
      const camElRad = (((cameraElevation % 360) + 360) % 360) * Math.PI / 180;
      
      // Matrice de vue (Vecteurs Forward, Right, Up) pour le modèle ACTUEL
      const { forward, right, up } = skyCameraBasis(cameraHeading, cameraElevation);
      
      // Paramètres de projection
      const fovRad = fieldOfView * Math.PI / 180;
      const tanHalfFov = Math.tan(fovRad / 2);
      
      const currentSkyDate = getSkyDate();
      const sidereal = siderealDegrees(currentSkyDate);
      const latRadians = latitude * Math.PI / 180;

      // V5.73.0 Calcul d'un objet de référence central pour les projections mathématiques
      let centerPosRef = horizontalPosition({ ra: currentHeading / 15, dec: currentElevation }, sidereal, latRadians).position;

      // --- FONCTION DE PROJECTION LOCALE V5.73.0 ---
      const projectPoint = (pos3D, rawHoriz) => {
        let sx, sy, depth = 1;

        if (projectionMode === 'actuel') {
          // --- MODÈLE ACTUEL (Perspective 3D brute) ---
          depth = pos3D[0] * forward[0] + pos3D[1] * forward[1] + pos3D[2] * forward[2];
          if (depth <= 0.0) return null; // Derrière la caméra
          
          sx = (pos3D[0] * right[0] + pos3D[1] * right[1] + pos3D[2] * right[2]) / (depth * tanHalfFov * aspect);
          sy = (pos3D[0] * up[0] + pos3D[1] * up[1] + pos3D[2] * up[2]) / (depth * tanHalfFov);
        
        } else if (projectionMode === 'stéréo') {
          // --- PROJECTION STÉRÉOGRAPHIQUE (Carte conforme) ---
          // Préserve les formes locales et les angles.
          const altRad = rawHoriz.alt;
          const azRad = rawHoriz.az;

          const cosAlt = Math.cos(altRad);
          const sinAlt = Math.sin(altRad);
          const dAz = azRad - camAzRad;
          const cosDAz = Math.cos(dAz);
          const sinDAz = Math.sin(dAz);

          const sinCamEl = Math.sin(camElRad);
          const cosCamEl = Math.cos(camElRad);

          // Calcul de la distance angulaire au centre de la caméra
          const cosAngDist = sinCamEl * sinAlt + cosCamEl * cosAlt * cosDAz;
          if (cosAngDist <= 0) return null; // Trop loin du centre

          // Facteur d'échelle conforme pour la stéréographie
          const k = 2 / (1 + cosAngDist);
          
          sx = k * cosAlt * sinDAz / (fovRad * aspect); // Normalisé par le FOV réel
          sy = k * (cosCamEl * sinAlt - sinCamEl * cosAlt * cosDAz) / fovRad;
          
          depth = cosAngDist; // Pour le tri des étiquettes
        
        } else if (projectionMode === 'gnomon') {
          // --- PROJECTION GNOMONIQUE (Lignes droites) ---
          // Tout grand cercle devient une ligne droite. Idéal pour grilles, conforme seulement au centre.
          const altRad = rawHoriz.alt;
          const azRad = rawHoriz.az;

          const cosAlt = Math.cos(altRad);
          const sinAlt = Math.sin(altRad);
          const dAz = azRad - camAzRad;
          const cosDAz = Math.cos(dAz);
          const sinDAz = Math.sin(dAz);

          const sinCamEl = Math.sin(camElRad);
          const cosCamEl = Math.cos(camElRad);

          const cosAngDist = sinCamEl * sinAlt + cosCamEl * cosAlt * cosDAz;
          if (cosAngDist <= 0.01) return null; // Zone d'ombre (infini)

          sx = cosAlt * sinDAz / (cosAngDist * tanHalfFov * aspect);
          sy = (cosCamEl * sinAlt - sinCamEl * cosAlt * cosDAz) / (cosAngDist * tanHalfFov);
          
          depth = cosAngDist;
        }

        return { x: sx, y: sy, depth: depth };
      };

      let pointingHeading = currentHeading;
      let pointingElevation = currentElevation;
      const telescopeMarker = document.getElementById('dobson-telescope-marker');
      if (telescopeMarker && sceneDiv.classList.contains('dobson-ar-active')) {
        const [localRight, localUp, localForward] = telescopeCameraDirection;
        const direction = forward.map((value, index) =>
          value * localForward + right[index] * localRight + up[index] * localUp);
        const altitude = Math.asin(Math.max(-1, Math.min(1, direction[1])));
        const azimuth = Math.atan2(direction[0], direction[2]);
        pointingHeading = (azimuth * 180 / Math.PI + 360) % 360;
        pointingElevation = altitude * 180 / Math.PI;
        const markerProjection = projectPoint(direction, { alt: altitude, az: azimuth });
        if (markerProjection) {
          telescopeMarker.style.left = `${(markerProjection.x + 1) * width / 2}px`;
          telescopeMarker.style.top = `${(1 - markerProjection.y) * height / 2}px`;
          telescopeMarker.hidden = false;
        } else {
          telescopeMarker.hidden = true;
        }
      }

      // --- GUIDAGE CIBLE FUSIONNÉ ---
      if (activeTargetObject) {
        const targetHoriz = horizontalPosition(activeTargetObject, sidereal, latRadians);
        
        let dAz = targetHoriz.azimuth - pointingHeading;
        if (dAz > 180) dAz -= 360;
        if (dAz < -180) dAz += 360;
        
        let dEl = targetHoriz.altitude - pointingElevation;
        const angularDistance = Math.hypot(dAz * Math.cos(pointingElevation * Math.PI / 180), dEl);

        const targetNameLabel = document.getElementById('targetNameLabel');
        const compassContainer = document.getElementById('compassArrowContainer');
        const compassSvg = document.getElementById('compassSvgIcon');
        const azVal = document.getElementById('azVal');
        const altVal = document.getElementById('altVal');

        if (targetNameLabel) targetNameLabel.textContent = activeTargetObject.name;

        const azSign = dAz > 0 ? '+' : '';
        const elSign = dEl > 0 ? '+' : '';
        if (azVal) azVal.textContent = `AZ: ${azSign}${dAz.toFixed(1)}°`;
        if (altVal) altVal.textContent = `ALT: ${elSign}${dEl.toFixed(1)}°`;

        const dynamicTolerance = Math.max(0.1, Math.min(2.0, (fieldOfView / 34) * 0.5));

        if (angularDistance < dynamicTolerance) {
          if (compassContainer) compassContainer.style.transform = `rotate(0deg)`;
          if (compassSvg) compassSvg.style.stroke = '#69ff84';
          if (targetNameLabel) targetNameLabel.style.color = '#69ff84';
        } else {
          if (targetNameLabel) targetNameLabel.style.color = '#fff';
          if (compassSvg) compassSvg.style.stroke = '#ffb74d';

          const scaledAz = dAz * Math.cos(currentElevation * Math.PI / 180);
          let angleDeg = Math.atan2(scaledAz, dEl) * (180 / Math.PI);
          if (compassContainer) {
            compassContainer.style.transform = `rotate(${angleDeg}deg)`;
          }
        }
      }

      const cardinalPoints = [
        { id: 'labelN', az: 0 }, { id: 'labelE', az: 90 },
        { id: 'labelS', az: 180 }, { id: 'labelW', az: 270 }
      ];
      const directionNames = ['N', 'NE', 'E', 'SE', 'S', 'SO', 'O', 'NO'];
      document.getElementById('skyDirection').textContent = `${directionNames[Math.round(currentHeading / 45) % 8]} · ${Math.round(currentHeading) % 360}°`;
      cardinalPoints.forEach(cp => {
        const azRad = cp.az * Math.PI / 180;
        const pos3D = [Math.sin(azRad), 0, Math.cos(azRad)];
        const proj = projectPoint(pos3D, { alt: 0, az: azRad });
        const elEl = document.getElementById(cp.id);
        let x;
        let y;
        if (proj) {
          x = proj.x;
          y = proj.y;
        } else {
          const relativeAzimuth = ((cp.az - currentHeading + 540) % 360 - 180) * Math.PI / 180;
          x = Math.sin(relativeAzimuth);
          y = -Math.cos(relativeAzimuth) * Math.sin(currentElevation * Math.PI / 180);
        }
        if (Math.hypot(x, y) < 1e-6) {
          x = 0;
          y = 1;
        }
        const maxX = Math.max(0.2, 1 - 48 / Math.max(width, 48));
        const maxY = Math.max(0.2, 1 - 48 / Math.max(height, 48));
        const scale = 1 / Math.max(Math.abs(x) / maxX, Math.abs(y) / maxY, 1e-6);
        x = Math.max(-maxX, Math.min(maxX, x * scale));
        y = Math.max(-maxY, Math.min(maxY, y * scale));
        elEl.style.left = `${(x + 1) * 50}%`;
        elEl.style.top = `${(1 - y) * 50}%`;
        elEl.style.transform = 'translate(-50%, -50%)';
        elEl.style.display = 'block';
      });

      ctx2d.clearRect(0, 0, width, height);

      const showGrid = document.getElementById('showGrid').checked;
      if (showGrid) {
        const gridSampleStep = Math.min(2, Math.max(0.25, fieldOfView / 8));
        // Pas fixe : la grille reste présente et stable quel que soit le zoom
        const gridSpacing = 10;
        const nightGrid = document.documentElement.dataset.nightMode === 'true';
        ctx2d.strokeStyle = nightGrid ? "rgba(200, 70, 70, 0.22)" : "rgba(150, 180, 210, 0.2)";
        ctx2d.lineWidth = 0.6;
        
        // Cercles d'altitude
        for (let altDeg = -90 + gridSpacing; altDeg < 90; altDeg += gridSpacing) {
          if (altDeg === 0) continue;
          const altRad = altDeg * Math.PI / 180;
          let ptsDrawn = 0;
          ctx2d.beginPath();
          for (let aDeg = 0; aDeg <= 360; aDeg += gridSampleStep) {
            const aRad = aDeg * Math.PI / 180;
            const pos3D = [Math.sin(aRad) * Math.cos(altRad), Math.sin(altRad), Math.cos(aRad) * Math.cos(altRad)];
            const proj = projectPoint(pos3D, { alt: altRad, az: aRad });
            
            if (proj) {
              const sx = width / 2 + proj.x * (width / 2);
              const sy = height / 2 - proj.y * (height / 2);
              if (ptsDrawn === 0) ctx2d.moveTo(sx, sy);
              else ctx2d.lineTo(sx, sy);
              ptsDrawn++;
            } else { ptsDrawn = 0; }
          }
          ctx2d.stroke();
        }

        // Horizon
        {
          const altRad = 0;
          ctx2d.strokeStyle = nightGrid ? "rgba(255, 110, 110, 0.7)" : "rgba(120, 200, 255, 0.7)";
          ctx2d.lineWidth = 1.2;
          let ptsDrawn = 0;
          ctx2d.beginPath();
          for (let aDeg = 0; aDeg <= 360; aDeg += gridSampleStep) {
            const aRad = aDeg * Math.PI / 180;
            const pos3D = [Math.sin(aRad), 0, Math.cos(aRad)];
            const proj = projectPoint(pos3D, { alt: altRad, az: aRad });
            
            if (proj) {
              const sx = width / 2 + proj.x * (width / 2);
              const sy = height / 2 - proj.y * (height / 2);
              if (ptsDrawn === 0) ctx2d.moveTo(sx, sy);
              else ctx2d.lineTo(sx, sy);
              ptsDrawn++;
            } else { ptsDrawn = 0; }
          }
          ctx2d.stroke();
        }

        // Méridiens d'azimut
        for (let azDeg = 0; azDeg < 360; azDeg += gridSpacing) {
          const azRad = azDeg * Math.PI / 180;
          let ptsDrawn = 0;
          ctx2d.beginPath();
          for (let altDeg = -90; altDeg <= 90; altDeg += gridSampleStep) {
            const altRad = altDeg * Math.PI / 180;
            const pos3D = [Math.sin(azRad) * Math.cos(altRad), Math.sin(altRad), Math.cos(azRad) * Math.cos(altRad)];
            const proj = projectPoint(pos3D, { alt: altRad, az: azRad });
            
            if (proj) {
              const sx = width / 2 + proj.x * (width / 2);
              const sy = height / 2 - proj.y * (height / 2);
              if (ptsDrawn === 0) ctx2d.moveTo(sx, sy);
              else ctx2d.lineTo(sx, sy);
              ptsDrawn++;
            } else { ptsDrawn = 0; }
          }
          ctx2d.stroke();
        }
      }

      // Constellations
      const showConstellations = document.getElementById('showConstellations').checked;
      if (showConstellations) {
        ctx2d.strokeStyle = "rgba(255, 165, 80, 0.45)";
        ctx2d.lineWidth = 1;
        for (const [starA, starB] of CONSTELLATION_LINES) {
          const sA = objectsByName.get(starA);
          const sB = objectsByName.get(starB);
          if (sA && sB) {
            const hA = horizontalPosition(sA, sidereal, latRadians);
            const hB = horizontalPosition(sB, sidereal, latRadians);
            const a = hA.position;
            const b = hB.position;
            const angle = Math.acos(Math.max(-1, Math.min(1, a[0]*b[0] + a[1]*b[1] + a[2]*b[2])));
            const steps = Math.max(1, Math.ceil(angle / (2 * Math.PI / 180)));
            let drawing = false;
            ctx2d.beginPath();
            for (let i = 0; i <= steps; i++) {
              const t = i / steps;
              const v = a.map((value, index) => value * (1 - t) + b[index] * t);
              const length = Math.hypot(...v);
              if (!length) { drawing = false; continue; }
              const position = v.map(value => value / length);
              const raw = { alt: Math.asin(Math.max(-1, Math.min(1, position[1]))), az: Math.atan2(position[0], position[2]) };
              const point = projectPoint(position, raw);
              if (!point) { drawing = false; continue; }
              const x = width / 2 + point.x * width / 2;
              const y = height / 2 - point.y * height / 2;
              if (drawing) ctx2d.lineTo(x, y);
              else ctx2d.moveTo(x, y);
              drawing = true;
            }
            ctx2d.stroke();
          }
        }
      }

      lastScreenObjects = [];

      // Objets célestes
      for (const object of objects) {
        if (!matchesFilter(object)) continue;
        const horizontal = horizontalPosition(object, sidereal, latRadians);
        
        // V5.73.0 Projection de l'objet selon le mode
        const proj = projectPoint(horizontal.position, horizontal.raw);
        if (!proj) continue; // Derrière ou trop loin
        
        // Clipping UI
        if (Math.abs(proj.x) > 1.2 || Math.abs(proj.y) > 1.2) continue;
        
        const sx = width / 2 + proj.x * (width / 2);
        const sy = height / 2 - proj.y * (height / 2);
        
        const type = (object.type || '').toLowerCase();
        const name = (object.name || '').toUpperCase();
        const colorHex = objectColorHex(object);

        ctx2d.save();
        
        if (name === 'LUNE') {
          ctx2d.fillStyle = '#f5f5f0';
          ctx2d.strokeStyle = '#d1d1c7';
          ctx2d.lineWidth = 1.5;
          ctx2d.beginPath();
          ctx2d.arc(sx, sy, Math.max(6, 10 * (34 / fieldOfView)), 0, 2 * Math.PI);
          ctx2d.fill();
          ctx2d.stroke();
        } else if (name === 'SOLEIL') {
          ctx2d.fillStyle = '#ffcc00';
          ctx2d.shadowColor = '#ff9900';
          ctx2d.shadowBlur = 15;
          ctx2d.beginPath();
          ctx2d.arc(sx, sy, 8, 0, 2 * Math.PI);
          ctx2d.fill();
        } else if (type.includes('planète') || type.includes('planete')) {
          ctx2d.fillStyle = '#80d8ff';
          ctx2d.shadowColor = '#80d8ff';
          ctx2d.shadowBlur = 8;
          ctx2d.beginPath();
          ctx2d.arc(sx, sy, Math.max(3, 4.5 * Math.min(34 / fieldOfView, 2)), 0, 2 * Math.PI);
          ctx2d.fill();
        } else if (type.includes('gal')) {
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
          const magContrast = Math.max(0.3, Math.min(1.2, 0.7 * (34 / fieldOfView))); 
          let starSize = Math.max(0.4, Math.min(3.5, (7.5 - object.mag) * (0.35 * magContrast) + 0.5));
          if (object.name === "Polaris") starSize = 2.0;
          const zoomFactor = 34 / fieldOfView;
          starSize *= Math.min(zoomFactor, 1.8);

          ctx2d.fillStyle = colorHex;
          if (!window.__dobsonMobileApp) {
            ctx2d.shadowColor = colorHex;
            ctx2d.shadowBlur = object.mag < 2.0 ? starSize * 2.5 : starSize * 1.2;
          }
          ctx2d.beginPath();
          ctx2d.arc(sx, sy, starSize, 0, 2 * Math.PI);
          ctx2d.fill();
        }

        ctx2d.restore();
        // V5.73.0 Stockage avec coordonnées projetées
        lastScreenObjects.push({ object, screenX: proj.x, screenY: proj.y, depth: proj.depth });
      }

      viewReadout.textContent = `Offset AZ ${Math.round(currentHeading)}° · ALT ${Math.round(currentElevation)}°`;

      const labelTime = performance.now();
      updateObjectLabels(lastScreenObjects, width, height, labelTime);

      // Étiquettes constellations
      const visibleConstellations = new Set();
      const labelConstellations = showSkyNames && document.getElementById('labelConstellations').checked;
      if (labelConstellations) {
        for (const c of CONSTELLATION_CENTERS) {
          const horiz = horizontalPosition(c, sidereal, latRadians);
          const proj = projectPoint(horiz.position, horiz.raw);
          if (proj && Math.abs(proj.x) <= 1 && Math.abs(proj.y) <= 1) {
            let state = constellationLabelStates.get(c);
            if (!state) {
              const label = document.createElement('span');
              label.className = 'constellation-label';
              label.textContent = c.name;
              constLabelsElement.appendChild(label);
              state = { label, hiddenAt: null };
              constellationLabelStates.set(c, state);
            }
            visibleConstellations.add(c);
            state.hiddenAt = null;
            state.label.style.transform = `translate3d(${(proj.x + 1) * width / 2}px, ${(1 - proj.y) * height / 2}px, 0) translate(-50%, -50%)`;
            state.label.style.opacity = '0.9';
          }
        }
      }
      for (const [constellation, state] of constellationLabelStates) {
        if (visibleConstellations.has(constellation)) continue;
        if (state.hiddenAt === null) state.hiddenAt = labelTime;
        state.label.style.opacity = '0';
        if (labelTime - state.hiddenAt >= 150) {
          state.label.remove();
          constellationLabelStates.delete(constellation);
        }
      }
    }

    async function initialize() {
      try {
        await requestWakeLock();
        await loadSavedSkyPrefs();
        resizeCanvas();

        const [catalogRes, statusRes] = await Promise.all([fetch('/catalog'), fetch('/status')]);
        baseObjects = await catalogRes.json();
        const solarSystem = getSolarSystemObjects(getSkyDate());
        setObjects([...baseObjects, ...solarSystem]);
        restoreSelectedTarget();

        const status = await statusRes.json();
        
        applyEncoderStatus(status);
        heading = targetHeading;
        elevation = targetElevation;

        latitude = Number(status.lat ?? latitude);
        longitude = Number(status.lon ?? longitude);
        
        updateSkyBackground(getSkyDate());
        setStatus(`${objects.length} objets chargés. Prêt.`, "align-ok");
        
        if (window.__dobsonMobileApp) {
          let lastEncoderPoll = 0;
          window.setInterval(() => {
            const now = performance.now();
            const interval = window.__dobsonEspConnected ? 100 : 33;
            if (now - lastEncoderPoll < interval) return;
            lastEncoderPoll = now;
            pollEncoderStatus();
          }, 16);
        } else {
          window.setInterval(pollEncoderStatus, 100);
        }
        render();
      } catch (error) {setStatus(error.message, true); }
    }

    function restoreSelectedTarget() {
      try {
        const savedTarget = JSON.parse(localStorage.getItem('dobson-selected-target') || 'null');
        if (!savedTarget || !Number.isFinite(Number(savedTarget.ra)) || !Number.isFinite(Number(savedTarget.dec))) return;
        const catalogObject = objectsByName.get(savedTarget.name);
        const restoredTarget = {
          ...(catalogObject || {}),
          name: savedTarget.displayName || savedTarget.name,
          commonName: savedTarget.commonName || catalogObject?.commonName || '',
          type: savedTarget.type || catalogObject?.type || '',
          ra: Number(savedTarget.ra),
          dec: Number(savedTarget.dec),
          mag: Number(savedTarget.mag ?? catalogObject?.mag ?? 99)
        };
        const dismissedTarget = localStorage.getItem(dismissedTargetStorageKey);
        if (dismissedTarget === targetIdentity(restoredTarget)) {
          activateTargetSearch(null);
          return;
        }
        activateTargetSearch(restoredTarget);
      } catch (error) {
        console.warn('Impossible de restaurer la cible de la vue encodeurs.', error);
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
        document.getElementById('modalTitle').textContent = `${closestObj.name} (${closestObj.commonName || closestObj.type})`;
        document.getElementById('modalSub').textContent = `${closestObj.type || 'Objet céleste'}`;
        const objectPreview = document.getElementById('objectPreview');
        objectPreview.innerHTML = createObjectPreview(closestObj);
        objectPreview.querySelector('svg').setAttribute('aria-label', `Illustration astronomique pour ${closestObj.commonName || closestObj.name}`);
        document.getElementById('modalMag').textContent = closestObj.mag.toFixed(1);
        document.getElementById('modalDist').textContent = details.dist;
        document.getElementById('modalSize').textContent = details.size;
        document.getElementById('modalCoords').textContent = `AD ${closestObj.ra.toFixed(2)}h · Dec ${closestObj.dec.toFixed(2)}°`;
        document.getElementById('objectModal').hidden = false;
        if (typeof window.__loadSkyObjectImage === 'function') window.__loadSkyObjectImage(closestObj);
      }
    });

    function createObjectPreview(object) {
      const type = (object.type || '').toLowerCase();
      const stars = '<g fill="#eaf0ff"><circle cx="18" cy="24" r="1.2"/><circle cx="75" cy="16" r="1"/><circle cx="132" cy="31" r="1.3"/><circle cx="155" cy="66" r="1"/><circle cx="35" cy="76" r="1"/><circle cx="119" cy="84" r="1.1"/><circle cx="52" cy="48" r=".8"/></g>';
      let objectArtwork;

      if (type.includes('galax')) {
        objectArtwork = '<g transform="rotate(-24 86 51)"><ellipse cx="86" cy="51" rx="50" ry="17" fill="none" stroke="#99b8ff" stroke-width="7" opacity=".8"/><path d="M37 50c15-21 45-27 69-14 14 8 19 23 10 34-10 12-29 12-43 3-9-6-11-16-5-23 5-6 14-7 21-4" fill="none" stroke="#d3a9ff" stroke-width="5" stroke-linecap="round"/><circle cx="86" cy="51" r="9" fill="#fff0bd"/></g>';
      } else if (type.includes('néb') || type.includes('neb')) {
        objectArtwork = '<defs><radialGradient id="nebulaPreview"><stop stop-color="#ffe2a6" stop-opacity=".95"/><stop offset=".36" stop-color="#f394d0" stop-opacity=".75"/><stop offset="1" stop-color="#8f82ff" stop-opacity="0"/></radialGradient></defs><path d="M31 51c4-19 20-29 38-23 7-20 38-17 42 2 22-4 34 15 23 30 14 17-3 34-21 27-16 15-37 5-41-6-19 11-43-6-35-20-10-2-11-7-6-10Z" fill="url(#nebulaPreview)"/>';
      } else if (type.includes('amas') || type.includes('cluster')) {
        objectArtwork = '<g fill="#ffe2a6"><circle cx="52" cy="37" r="2"/><circle cx="77" cy="27" r="1.5"/><circle cx="94" cy="38" r="2.4"/><circle cx="111" cy="31" r="1.4"/><circle cx="64" cy="53" r="1.7"/><circle cx="88" cy="55" r="2.1"/><circle cx="118" cy="55" r="1.8"/><circle cx="47" cy="68" r="1.2"/><circle cx="75" cy="72" r="2"/><circle cx="101" cy="70" r="1.5"/><circle cx="132" cy="69" r="1.2"/></g>';
      } else if (type.includes('plan')) {
        objectArtwork = '<ellipse cx="86" cy="51" rx="48" ry="14" fill="none" stroke="#a9bdff" stroke-width="4"/><circle cx="86" cy="51" r="22" fill="#ffd27a"/><path d="M65 58a48 14 0 0 0 44-1" fill="none" stroke="#eff4ff" stroke-width="4"/>';
      } else {
        objectArtwork = '<circle cx="86" cy="51" r="19" fill="#fff0bd"/><circle cx="86" cy="51" r="30" fill="none" stroke="#f2c877" stroke-opacity=".55" stroke-width="1"/><path d="M86 13v12m0 52v12M48 51h12m52 0h12" stroke="#f2c877" stroke-opacity=".55"/>';
      }

      return `<svg viewBox="0 0 172 102" role="img" aria-label="Illustration astronomique"><defs><linearGradient id="skyPreview" x2="0" y2="1"><stop stop-color="#0b1733"/><stop offset="1" stop-color="#102e4c"/></linearGradient></defs><rect width="172" height="102" rx="9" fill="url(#skyPreview)"/>${stars}${objectArtwork}</svg>`;
    }

    function closeModal() { document.getElementById('objectModal').hidden = true; }
    document.addEventListener('keydown', event => {
      if (event.key === 'Escape' && !document.getElementById('objectModal').hidden) closeModal();
    });

    let dragPoint = null;
    const activePointers = new Map();
    let pinchDistance = null;
    const pointerDistance = () => {
      const [first, second] = [...activePointers.values()];
      return first && second ? Math.hypot(first.x - second.x, first.y - second.y) : null;
    };
    
    const scene = document.getElementById('scene');
    const reverseTouchAzimuth = window.__dobsonMobileApp && localStorage.getItem('dobson-touch-reverse-azimuth') === 'true';
    const reverseTouchAltitude = window.__dobsonMobileApp && localStorage.getItem('dobson-touch-reverse-altitude') === 'true';
    function applyTouchPan(deltaX, deltaY, panPanFactor) {
      touchHeading = ((touchHeading - deltaX * (reverseTouchAzimuth ? -1 : 1) * 0.35 / panPanFactor) % 360 + 360) % 360;
      touchElevation += (window.__dobsonMobileApp ? -deltaY : deltaY) * (reverseTouchAltitude ? -1 : 1) * 0.25 / panPanFactor;
    }

    window.addEventListener('dobson:resetTouchOffset', () => {
      touchHeading = 0;
      touchElevation = 0;
    });

    scene.addEventListener('pointerdown', event => {
      if (event.target instanceof Element && event.target.closest('.scene-zoom-controls, #dobson-sensor-mode, .dobson-scene-alignment, .dobson-scene-time-panel, .dobson-solver-panel, .dobson-scene-tools, button, input, select')) return;
      activePointers.set(event.pointerId, { x: event.clientX, y: event.clientY });
      dragPoint = { x: event.clientX, y: event.clientY };
      scene.setPointerCapture(event.pointerId);
      pinchDistance = activePointers.size === 2 ? pointerDistance() : null;
    });
    scene.addEventListener('pointermove', event => {
      if (activePointers.has(event.pointerId)) activePointers.set(event.pointerId, { x: event.clientX, y: event.clientY });
      if (activePointers.size >= 2) {
        const distance = pointerDistance();
        if (distance && pinchDistance) fieldOfView = Math.max(2, Math.min(75, fieldOfView * pinchDistance / distance));
        pinchDistance = distance;
        dragPoint = null;
        return;
      }
      if (!dragPoint) return;
      // V5.73.0 Sensibilité du pan adaptée selon le mode
      const panPanFactor = (projectionMode !== 'actuel') ? (60 / fieldOfView) : (34 / fieldOfView);

      applyTouchPan(event.clientX - dragPoint.x, event.clientY - dragPoint.y, panPanFactor);
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
      fieldOfView = Math.max(2, Math.min(75, fieldOfView + Math.sign(event.deltaY) * 2));
    }, { passive: false });

    document.getElementById('centerButton').addEventListener('click', () => {
      touchHeading = 0;
      touchElevation = 0;
      alignOffsetHeading = 0;
      alignOffsetElevation = 0;
      cameraCalibrationHeading = 0;
      cameraCalibrationElevation = 0;
      telescopeCameraDirection = [0, 0, 1];
      if (projectionMode === 'actuel') fieldOfView = 34;
      else if (projectionMode === 'stéréo') fieldOfView = 60;
      else fieldOfView = 45;
    });

    window.addEventListener('resize', resizeCanvas);
    checkOrientation();
    initialize();
  </script>
</body>
</html>
)rawliteral";

#endif