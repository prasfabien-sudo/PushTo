// Version Component: ReleaseNotes.h v1.2.0
// Version Global System: v1.2.0
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
  </style>
</head>
<body>
  <div class="card">
    <a href="/" class="btn-back">⬅️ Retour à l'Accueil</a>
    
    <div class="header-row">
      <h1>📋 Release Notes</h1>
      <span class="version-tag">v1.2.0</span>
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
        <li><span class="badge badge-feat">Feat</span> Affichage dynamique de la source de localisation (`📍 GPS Mobile` ou `📍 Lieu d'observation`).</li>
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
</body>
</html>
)rawliteral";

#endif