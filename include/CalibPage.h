#ifndef CALIBPAGE_H
#define CALIBPAGE_H

#include <pgmspace.h>

const char HTTP_CALIB_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Étalonnage des Encodeurs</title>
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #121212; color: #fff; padding: 15px; margin: 0; }
    .card { background: #1e1e1e; border-radius: 12px; padding: 20px; max-width: 600px; margin: 0 auto; text-align: center; box-shadow: 0 4px 12px rgba(0,0,0,0.5); }
    h2 { color: #ff9800; margin-top: 0; }
    .step-box { background: #2a2a2a; border-radius: 8px; padding: 15px; margin: 15px 0; border-left: 4px solid #2196f3; text-align: left; }
    .step-title { font-weight: bold; color: #81d4fa; margin-bottom: 5px; }
    .btn { padding: 12px 20px; background: #4caf50; color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer; width: 100%; margin-top: 10px; transition: all 0.15s ease-in-out; }
    .btn:hover { filter: brightness(1.15); }
    .btn:active { transform: scale(0.97); filter: brightness(0.85); }
    .btn-sec { background: #424242; text-decoration: none; display: inline-block; margin-bottom: 15px; width: 100%; box-sizing: border-box; }.val-highlight { font-size: 1.3em; color: #ffb74d; font-weight: bold; }
    .deg-highlight { font-size: 1.1em; color: #4caf50; margin-top: 4px; font-weight: normal; }
  </style>
</head>
<body>
  <div class="card">
    <a href="/" class="btn btn-sec">← Retour au tableau de bord</a>
    <h2>Étalonnage des Encodeurs</h2>

    <div class="step-box">
      <div class="step-title">1. Démarrage</div>
      <p>Positionnez le télescope à l'horizontale (0° avec niveau à bulle) et sur un repère Azimut fixe.</p>
      <button class="btn" onclick="sendCalib('start')">Démarrer l'étalonnage</button>
    </div>

    <div class="step-box">
      <div class="step-title">2. Axe Azimut (Tour complet 360°)</div>
      <p>Faites effectuer un tour complet de <strong>360°</strong> à l'azimut jusqu'à revenir sur votre repère.</p>
      <div>Pas mesurés : <span class="val-highlight" id="rawAZ">0</span></div>
      <div>Angle équivalent : <span class="deg-highlight" id="degAZ">0.0°</span></div>
      <button class="btn" onclick="sendCalib('fin_az')">Valider Azimut (360°)</button>
    </div>

    <div class="step-box">
      <div class="step-title">3. Axe Altitude (Basculement 90°)</div>
      <p>Pointez à l'horizontale (0°), puis montez le tube à la verticale exacte (<strong>90°</strong> au zénith).</p>
      <div>Pas mesurés (sur 90°) : <span class="val-highlight" id="rawALT">0</span></div>
      <div>Angle calculé : <span class="deg-highlight" id="degALT">0.0°</span></div>
      <button class="btn" onclick="sendCalib('fin_alt_90')">Valider Altitude (90°)</button>
    </div>
  </div>

  <script>
    let ticksAZ = 10000;
    let ticksALT = 10000;

    function updateStatus() {
      fetch('/status').then(r => r.json()).then(d => {
        ticksAZ = d.ticksAZ || 10000;
        ticksALT = d.ticksALT || 10000;

        let rawAZ = d.rawAZ;
        let rawALT = d.rawALT;

        let angleAZ = ((rawAZ / ticksAZ) * 360.0) % 360.0;
        let angleALT = (rawALT / ticksALT) * 360.0;
        if (angleAZ < 0) angleAZ += 360.0;

        document.getElementById('rawAZ').innerText = rawAZ;
        document.getElementById('degAZ').innerText = `${angleAZ.toFixed(1)}°`;

        document.getElementById('rawALT').innerText = rawALT;
        document.getElementById('degALT').innerText = `${angleALT.toFixed(1)}° (Cible : 90°)`;
      });
    }

    function sendCalib(cmd) {
      fetch(`/calib?cmd=${cmd}`).then(() => updateStatus());
    }

    setInterval(updateStatus, 250);
  </script>
</body>
</html>
)rawliteral";

#endif