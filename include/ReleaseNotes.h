// Version Component: ReleaseNotes.h v1.2.7
// Version Global System: v1.2.7
#ifndef RELEASENOTES_H
#define RELEASENOTES_H

#include <pgmspace.h>

const char HTTP_RELEASE_NOTES_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Release Notes - Dobson Push-To</title>
  <style>
    .ui-icon { display: inline-block; width: 1em; height: 1em; vertical-align: -0.15em; fill: none; stroke: currentColor; stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; }
    .nav-icon .ui-icon { vertical-align: middle; }
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #121212; color: #e0e0e0; margin: 0; padding: 15px; }
    .card { background: #1e1e1e; border-radius: 12px; padding: 20px; max-width: 650px; margin: 0 auto; box-shadow: 0 4px 12px rgba(0,0,0,0.5); }
    .header-row { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; border-bottom: 1px solid #333; padding-bottom: 10px; }
    h1 { color: #ff5252; margin: 0; font-size: 1.4em; }
    .version-tag { font-size: 0.8em; color: #ffb74d; font-weight: bold; background: #2a2a2a; padding: 4px 8px; border-radius: 4px; border: 1px solid #333; }
    
    .btn-back { display: inline-block; padding: 8px 12px; background: #37474f; color: #fff; text-decoration: none; border-radius: 6px; font-size: 0.85em; font-weight: bold; margin-bottom: 15px; }
    .btn-back:hover { background: #455a64; }

    .release-item { background: #263238; border-radius: 8px; padding: 15px; margin-bottom: 15px; border-left: 4px solid #00897b; }
    .release-item.minor { border-left-color: #2196f3; }
    .release-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px; }
    .release-version { font-size: 1.1em; font-weight: bold; color: #80cbc4; }
    .release-date { font-size: 0.8em; color: #90a4ae; }
    
    ul { margin: 5px 0 0 0; padding-left: 20px; font-size: 0.9em; line-height: 1.4; color: #cfd8dc; }
    li { margin-bottom: 4px; }
    .badge { font-size: 0.7em; padding: 2px 5px; border-radius: 3px; font-weight: bold; text-transform: uppercase; margin-right: 5px; }
    .badge-feat { background: #1b5e20; color: #81c784; }
    .badge-fix { background: #b71c1c; color: #ef9a9a; }
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
    <div class="header-row">
      <h1><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-clipboard-list'></use></svg> Release Notes</h1>
      <span class="version-tag">v1.2.7</span>
    </div>

    <!-- v1.2.7 -->
    <div class="release-item">
      <div class="release-header">
        <span class="release-version">v1.2.7</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-fix">Fix</span> La vue 3D suit désormais en temps réel les angles AZ/ALT calculés depuis les encodeurs ; le recentrage revient au pointage du télescope.</li>
      </ul>
    </div>

    <!-- v1.2.6 -->
    <div class="release-item">
      <div class="release-header">
        <span class="release-version">v1.2.6</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-fix">Fix</span> Bouton du mode nuit déplacé dans les raccourcis de la vue 3D et rendu plus visible avec un contraste inversé.</li>
      </ul>
    </div>

    <!-- v1.2.5 -->
    <div class="release-item minor">
      <div class="release-header">
        <span class="release-version">v1.2.5</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-feat">Feat</span> Accès direct en bas d'écran au menu principal, à la mise en station, à la vue 3D du ciel et à l'aide ; les autres pages restent dans « Plus ».</li>
      </ul>
    </div>

    <!-- v1.2.4 -->
    <div class="release-item">
      <div class="release-header">
        <span class="release-version">v1.2.4</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-feat">Feat</span> Déplacement du pavé du lieu d'observation au-dessus de la position du télescope.</li>
        <li><span class="badge badge-fix">Fix</span> Mise à jour du catalogue sans recréer les lignes à chaque rafraîchissement, supprimant le clignotement des types et de la visibilité.</li>
        <li><span class="badge badge-fix">Fix</span> Retour de la vue 3D bêta au pilotage tactile uniquement ; retrait du serveur HTTPS expérimental et des commandes de capteurs.</li>
      </ul>
    </div>

    <!-- v1.2.0 -->
    <div class="release-item minor">
      <div class="release-header">
        <span class="release-version">v1.2.0</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-feat">Feat</span> Séparation explicite entre la version globale système (affichée) et les numéros de version individuels par composant (`.h`/`.cpp`).</li>
      </ul>
    </div>

    <!-- v1.1.1 -->
    <div class="release-item">
      <div class="release-header">
        <span class="release-version">v1.1.1</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-feat">Feat</span> Création du composant dédié aux notes de version (`ReleaseNotes.h`).</li>
        <li><span class="badge badge-feat">Feat</span> Intégration du lien Release Notes dans le menu de navigation et sur le tag de version principal.</li>
      </ul>
    </div>

    <!-- v1.1.0 -->
    <div class="release-item minor">
      <div class="release-header">
        <span class="release-version">v1.1.0</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-feat">Feat</span> Géolocalisation GPS native du smartphone (`navigator.geolocation`) avec repli automatique sur les coordonnées ESP32.</li>
        <li><span class="badge badge-feat">Feat</span> Affichage dynamique de la source de localisation (`<svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-map-pin'></use></svg> GPS Mobile` ou `<svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-map-pin'></use></svg> Lieu d'observation`).</li>
      </ul>
    </div>

    <!-- v1.0.4 -->
    <div class="release-item">
      <div class="release-header">
        <span class="release-version">v1.0.4</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-fix">Fix</span> Correction du calcul de précession sur l'Ascension Droite (RA) aligné sur Stellarium.</li>
      </ul>
    </div>

    <!-- v1.0.3 -->
    <div class="release-item">
      <div class="release-header">
        <span class="release-version">v1.0.3</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-fix">Fix</span> Correction de réfraction atmosphérique appliquée à l'altitude apparente.</li>
        <li><span class="badge badge-fix">Fix</span> Précession astronomique IAU76 appliquée aux coordonnées RA/DEC J2000.</li>
      </ul>
    </div>

    <!-- v1.0.2 -->
    <div class="release-item">
      <div class="release-header">
        <span class="release-version">v1.0.2</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-fix">Fix</span> Formatage précisé des coordonnées RA/DEC (HH:MM:SS / DD:MM:SS).</li>
      </ul>
    </div>

    <!-- v1.0.1 -->
    <div class="release-item">
      <div class="release-header">
        <span class="release-version">v1.0.1</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-feat">Feat</span> Affichage du tag de version sémantique en haut à droite de l'interface.</li>
      </ul>
    </div>

    <!-- v1.0.0 -->
    <div class="release-item minor">
      <div class="release-header">
        <span class="release-version">v1.0.0</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-feat">Feat</span> Refonte complète de l'interface Web principale avec double jauge de guidage (AZ/ALT).</li>
        <li><span class="badge badge-feat">Feat</span> Intégration du catalogue enrichi `ObjectsDB.h` avec éphémérides des planètes.</li>
        <li><span class="badge badge-feat">Feat</span> Calcul de LST en UTC strict et filtrage d'objets par type/proximité.</li>
      </ul>
    </div>

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
        <a class="nav-more-link" href="/test"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-flask-conical'></use></svg> Tests système</a>
        <a class="nav-more-link" href="/releasenotes" aria-current="page"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-clipboard-list'></use></svg> Notes de version</a>
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