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
    
    .step-box { background: #2a2a2a; border-left: 4px solid #ff9800; padding: 10px; border-radius: 4px; margin-bottom: 12px; font-size: 0.85em; color: #ddd; }
    .status-ok { color: #4caf50; font-weight: bold; }
  </style>
</head>
<body>
  <div class="card">
    <a href="/" class="btn btn-sec">🏠 Retour à la page principale</a>
    <h2>🎯 Mise en Station & Recalage</h2>

    <!-- Quick Zero Section -->
    <div class="section-title">1. Calage Rapide sur Repères</div>
    <button class="btn" onclick="sendCmd('set_zero_alt')">📐 Mettre à zéro l'Altitude (Tube à l'horizontale)</button>
    <button class="btn" onclick="sendCmd('set_zero_az')">🧭 Mettre à zéro l'Azimut (Pointe au Nord)</button>
    <button class="btn" onclick="sendCmd('set_polaris')">⭐ Recalage rapide sur la Polaire</button>

    <!-- 2 Stars Alignment Step by Step -->
    <div class="section-title">2. Alignement Guidé 2 Étoiles</div>
    
    <div class="step-box" id="stepGuide">
      <strong>Procédure :</strong><br>
      1. Vise la 1ère étoile dans le télescope, entre ses coordonnées puis valide.<br>
      2. Vise la 2ème étoile, entre ses coordonnées et valide pour terminer.
    </div>

    <div id="star1Group">
      <div style="font-weight:bold; color:#ffb74d; margin-bottom:5px;">Étape 1 : Première Étoile</div>
      <div class="form-group">
        <label>Azimut Théorique (°)</label>
        <input type="number" step="0.1" id="az1" placeholder="ex: 120.5">
      </div>
      <div class="form-group">
        <label>Altitude Théorique (°)</label>
        <input type="number" step="0.1" id="alt1" placeholder="ex: 45.0">
      </div>
      <button class="btn btn-step1" onclick="validateStar1()">⭐ Valider Étoile 1</button>
    </div>

    <div id="star2Group" style="margin-top: 15px; opacity: 0.5;">
      <div style="font-weight:bold; color:#e91e63; margin-bottom:5px;">Étape 2 : Seconde Étoile</div>
      <div class="form-group">
        <label>Azimut Théorique (°)</label>
        <input type="number" step="0.1" id="az2" placeholder="ex: 215.0" disabled>
      </div>
      <div class="form-group">
        <label>Altitude Théorique (°)</label>
        <input type="number" step="0.1" id="alt2" placeholder="ex: 30.0" disabled>
      </div>
      <button class="btn btn-step2" id="btnStar2" onclick="validateStar2()" disabled>⭐ Valider Étoile 2 & Terminer</button>
    </div>

  </div>

  <script>
    function sendCmd(action) {
      fetch(`/cmd?action=${action}`)
        .then(r => r.text())
        .then(res => alert("Action exécutée : " + action));
    }

    function validateStar1() {
      const az = document.getElementById('az1').value;
      const alt = document.getElementById('alt1').value;

      if (!az || !alt) {
        alert("Veuillez saisir l'Azimut et l'Altitude de la première étoile.");
        return;
      }

      fetch(`/cmd?action=align_star1&az=${az}&alt=${alt}`)
        .then(r => r.json())
        .then(d => {
          if (d.status === "OK") {
            document.getElementById('stepGuide').innerHTML = '<span class="status-ok">✔ Étoile 1 enregistrée !</span><br>Déplace le télescope vers la 2ème étoile, saisis ses coordonnées et valide.';
            
            // Déblocage étape 2
            document.getElementById('az2').disabled = false;
            document.getElementById('alt2').disabled = false;
            document.getElementById('btnStar2').disabled = false;
            document.getElementById('star2Group').style.opacity = '1.0';
          }
        });
    }

    function validateStar2() {
      const az = document.getElementById('az2').value;
      const alt = document.getElementById('alt2').value;

      if (!az || !alt) {
        alert("Veuillez saisir l'Azimut et l'Altitude de la seconde étoile.");
        return;
      }

      fetch(`/cmd?action=align_star2&az=${az}&alt=${alt}`)
        .then(r => r.json())
        .then(d => {
          if (d.status === "OK") {
            alert("Alignement sur 2 étoiles réussi ! Les compteurs du télescope sont recalés.");
            location.reload();
          }
        });
    }
  </script>
</body>
</html>
)rawliteral";

#endif