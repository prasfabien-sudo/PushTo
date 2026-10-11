// Version Component: ReleaseNotes.h v1.2.13
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
    <div class="header-row">
      <h1><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-clipboard-list'></use></svg> Release Notes</h1>
      <span class="version-tag">v5.5.1</span>
    </div>

    <!-- v5.24.0 (Vue 3D - Vue 3D avec orientation automatique au nord (sur Pixel )) -->
    <div class="release-item">
      <div class="release-header">
        <span class="release-version">Vue 3D v5.24.0</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-feat">Feat</span> Utilisation de tous les capteurs internes du Pixel pour aligner automatiquement sur le nord magnétique</li>
      </ul>
    </div>

    <!-- v5.5.1 (Vue 3D - Filtre magnitude & Plein écran auto) -->
    <div class="release-item">
      <div class="release-header">
        <span class="release-version">Vue 3D v5.5.1</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-feat">Feat</span> Ajout d'un sélecteur de magnitude maximale pour filtrer dynamiquement les objets affichés (œil nu, jumelles, petits et grands télescopes).</li>
        <li><span class="badge badge-feat">Feat</span> Basculement automatique en mode plein écran immersif dès que l'appareil passe en orientation paysage, et désactivation automatique en mode portrait.</li>
        <li><span class="badge badge-feat">Feat</span> Intégration d'un bouton de contrôle manuel du plein écran, disponible uniquement en mode portrait.</li>
      </ul>
    </div>

    <!-- v5.5.0 (Catalogue & Constellations) -->
    <div class="release-item">
      <div class="release-header">
        <span class="release-version">Vue 3D v5.5.0</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-feat">Feat</span> Intégration complète du réseau de lignes et de centres pour les 44 constellations majeures de l'hémisphère Nord et de la zone zodiacale.</li>
      </ul>
    </div>

    <!-- v5.4.0 (Vue 3D - Simulation Temporelle) -->
    <div class="release-item">
      <div class="release-header">
        <span class="release-version">Vue 3D v5.4.0</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-feat">Feat</span> Ajout d'un sélecteur de date/heure interactif permettant de simuler l'affichage du ciel à n'importe quel moment (avec bouton "Direct").</li>
      </ul>
    </div>

    <!-- v5.3.0 (Vue 3D - Grille planétarium & Pôles) -->
    <div class="release-item">
      <div class="release-header">
        <span class="release-version">Vue 3D v5.3.0</span>
        <span class="release-date">Octobre 2026</span>
      </div>
      <ul>
        <li><span class="badge badge-feat">Feat</span> Intégration d'une grille altazimutale complète et affichage dynamique des points cardinaux (N, S, E, O).</li>
      </ul>
    </div>

  </div>

  <nav class="bottom-nav" aria-label="Navigation principale">
    <a class="nav-item" href="/beta_sky" title="Vue 3D du ciel"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-globe-2'></use></svg></span></a>
    <a class="nav-item" href="/" title="Mode Encodeurs"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-telescope'></use></svg></span></a>
    <a class="nav-item" href="/station" title="Mise en station"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-compass'></use></svg></span></a>
    <a class="nav-item" href="/calib_page" title="Calibration"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-ruler'></use></svg></span></a>
    <a class="nav-item" href="/config" title="Configuration"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-settings'></use></svg></span></a>
    <a class="nav-item" href="/help" title="Aide SkySafari"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-book-open'></use></svg></span></a>
    <a class="nav-item" href="/simu" title="Simulation"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-gamepad-2'></use></svg></span></a>
    <a class="nav-item" href="/test" title="Tests système"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-flask-conical'></use></svg></span></a>
    <a class="nav-item" href="/releasenotes" aria-current="page" title="Notes de version"><span class="nav-icon"><svg class='ui-icon' aria-hidden='true' focusable='false'><use href='/icons.svg#icon-clipboard-list'></use></svg></span></a>
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
  </script>
</body>
</html>
)rawliteral";

#endif