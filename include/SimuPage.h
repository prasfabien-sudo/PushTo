#ifndef SIMUPAGE_H
#define SIMUPAGE_H

#include <pgmspace.h>

const char HTTP_SIMU_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Simulateur Push-To</title>
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #121212; color: #fff; padding: 15px; margin: 0; }
    .card { background: #1e1e1e; border-radius: 12px; padding: 20px; max-width: 500px; margin: 0 auto; text-align: center; }
    .btn { padding: 12px 15px; background: #2196f3; color: white; border: none; border-radius: 8px; font-weight: bold; cursor: pointer; margin: 5px; transition: all 0.15s ease-in-out; }
    .btn:hover { filter: brightness(1.15); }
    .btn:active { transform: scale(0.95); filter: brightness(0.85); }
    .btn-sec { background: #424242; text-decoration: none; display: inline-block; margin-bottom: 15px; width: 100%; box-sizing: border-box; }
    .val { font-size: 1.2em; color: #ffb74d; margin: 10px 0; }
    .val-neg { color: #ff5252; font-weight: bold; }
  </style>
</head>
<body>
  <div class="card">
    <a href="/" class="btn btn-sec">← Retour au tableau de bord</a>
    <h2>Simulateur en Degrés</h2>

    <div class="axis-title">Axe Azimut (AZ)</div>
    <div class="info-res" id="infoAZ">Résolution AZ: -- pas/rev</div>
    <div class="btn-group">
      <button class="btn btn-neg" onclick="stepDeg('az', -90)">-90°</button>
      <button class="btn btn-neg" onclick="stepDeg('az', -45)">-45°</button>
      <button class="btn btn-neg" onclick="stepDeg('az', -10)">-10°</button>
      <button class="btn btn-neg" onclick="stepDeg('az', -1)">-1°</button>
      <button class="btn btn-neg" onclick="stepDeg('az', -0.1)">-0.1°</button>
      <button class="btn" onclick="stepDeg('az', 0.1)">+0.1°</button>
      <button class="btn" onclick="stepDeg('az', 1)">+1°</button>
      <button class="btn" onclick="stepDeg('az', 10)">+10°</button>
      <button class="btn" onclick="stepDeg('az', 45)">+45°</button>
      <button class="btn" onclick="stepDeg('az', 90)">+90°</button>
    </div>

    <div class="axis-title">Axe Altitude (ALT)</div>
    <div class="info-res" id="infoALT">Résolution ALT: -- pas/rev</div>
    <div class="btn-group">
      <button class="btn btn-neg" onclick="stepDeg('alt', -90)">-90°</button>
      <button class="btn btn-neg" onclick="stepDeg('alt', -45)">-45°</button>
      <button class="btn btn-neg" onclick="stepDeg('alt', -10)">-10°</button>
      <button class="btn btn-neg" onclick="stepDeg('alt', -1)">-1°</button>
      <button class="btn btn-neg" onclick="stepDeg('alt', -0.1)">-0.1°</button>
      <button class="btn" onclick="stepDeg('alt', 0.1)">+0.1°</button>
      <button class="btn" onclick="stepDeg('alt', 1)">+1°</button>
      <button class="btn" onclick="stepDeg('alt', 10)">+10°</button>
      <button class="btn" onclick="stepDeg('alt', 45)">+45°</button>
      <button class="btn" onclick="stepDeg('alt', 90)">+90°</button>
    </div>
  </div>

  <script>
    let ticksAZ = 10000;
    let ticksALT = 10000;

    function updateVal() {
      fetch('/status').then(r => r.json()).then(d => {
        let degAZ = ((d.rawAZ / (d.ticksAZ || 10000)) * 360.0) % 360.0;
        let degALT = ((d.rawALT / (d.ticksALT || 10000)) * 360.0) % 360.0;
        if (degAZ < 0) degAZ += 360.0;
        if (degALT < 0) degALT += 360.0;

        const elAZ = document.getElementById('valAZ');
        const elALT = document.getElementById('valALT');

        elAZ.innerText = `${degAZ.toFixed(1)}° (${d.rawAZ} pas)`;
        elALT.innerText = `${degALT.toFixed(1)}° (${d.rawALT} pas)`;

        // Couleur rouge si les pas bruts sont négatifs
        if (d.rawAZ < 0) elAZ.classList.add('val-neg');
        else elAZ.classList.remove('val-neg');

        if (d.rawALT < 0) elALT.classList.add('val-neg');
        else elALT.classList.remove('val-neg');
      });
    }

    function stepDeg(axis, deg) {
      let ticks = (axis === 'az') ? ticksAZ : ticksALT;
      let deltaTicks = Math.round((deg / 360.0) * ticks);
      fetch(`/simstep?axis=${axis}&delta=${deltaTicks}`);
    }

    fetchStatus();
  </script>
</body>
</html>
)rawliteral";

#endif