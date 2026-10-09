#ifndef HELPPAGE_H
#define HELPPAGE_H

#include <pgmspace.h>

const char HTTP_HELP_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Aide & Connexion SkySafari</title>
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #121212; color: #fff; padding: 15px; margin: 0; }
    .card { background: #1e1e1e; border-radius: 12px; padding: 20px; max-width: 600px; margin: 0 auto; box-shadow: 0 4px 12px rgba(0,0,0,0.5); }
    h2 { color: #ff9800; margin-top: 10px; text-align: center; }
    h3 { color: #80cbc4; border-bottom: 1px solid #333; padding-bottom: 5px; margin-top: 20px; }
    .btn { padding: 12px 20px; background: #4caf50; color: white; border: none; border-radius: 8px; font-size: 1em; font-weight: bold; cursor: pointer; width: 100%; margin-top: 15px; transition: all 0.15s ease-in-out; }
    .btn:hover { filter: brightness(1.15); }
    .btn:active { transform: scale(0.97); filter: brightness(0.85); }
    .btn-sec { background: #2196f3; text-decoration: none; display: block; text-align: center; margin-bottom: 15px; width: 100%; box-sizing: border-box; }
    
    ol { padding-left: 20px; line-height: 1.6; }
    li { margin-bottom: 10px; color: #e0e0e0; }
    .highlight { color: #ffb74d; font-weight: bold; }
    .code-box { background: #2a2a2a; padding: 8px 12px; border-radius: 6px; font-family: monospace; color: #4caf50; display: inline-block; margin: 4px 0; }
    .note { background: #263238; border-left: 4px solid #00897b; padding: 10px; border-radius: 4px; font-size: 0.9em; margin-top: 15px; color: #b0bec5; }
  </style>
</head>
<body>
  <div class="card">
    <a href="/" class="btn btn-sec">🏠 Retour à la page principale</a>
    <h2>📖 Guide de Connexion SkySafari</h2>

    <h3>1. Connexion au réseau Wi-Fi</h3>
    <ol>
      <li>Coupe tes données mobile.</li>
      <li>Ouvre les paramètres Wi-Fi de ton smartphone ou de ta tablette.</li>
      <li>Connecte-toi au réseau Wi-Fi émis par le télescope : <br><span class="code-box">Dobson_PushTo</span></li>
      <li>Aucun mot de passe n'est requis. Veille à maintenir la connexion Wi-Fi même si ton téléphone indique "Pas d'accès Internet".</li>
    </ol>

    <h3>2. Configuration dans SkySafari</h3>
    <p style="font-size:0.9em; color:#ccc;">Configuration valable pour SkySafari Plus ou Pro (iOS / Android) :</p>
    <ol>
      <li>Ouvre l'application <strong>SkySafari</strong>.</li>
      <li>Rends-toi dans <strong>Settings</strong> (Paramètres) ➔ <strong>Telescope</strong> ➔ <strong>Setup</strong>.</li>
      <li>Renseigne les paramètres suivants :
        <ul>
          <li><strong>Equipment Type :</strong> <span class="highlight">Basic Encoder System</span></li>
          <li><strong>Mount Type :</strong> <span class="highlight">Alt-Az Push-To</span></li>
          <li><strong>Communication Type :</strong> <span class="highlight">Wi-Fi or Ethernet</span></li>
          <li><strong>IP Address :</strong> <span class="code-box">192.168.4.1</span></li>
          <li><strong>Port Number :</strong> <span class="code-box">4030</span></li>
        </ul>
      </li>
      <li>Dans la section <strong>Encoder Steps Per Revolution</strong> (Résolution des encodeurs) :
        <ul>
          <li>Indique le nombre de pas réglé sur ton ESP32 pour l'Azimut et l'Altitude (ex: <span class="highlight">10000</span> pas par tour).</li>
        </ul>
      </li>
    </ol>

    <h3>3. Connexion & Utilisation</h3>
    <ol>
      <li>Reviens à la carte du ciel principale dans SkySafari.</li>
      <li>Appuie sur le bouton <strong>Scope</strong> ➔ <strong>Connect</strong> dans le menu inférieur.</li>
      <li>Une reticule représentant la position de ton télescope apparaît à l'écran. Il se déplace en temps réel lorsque tu bouges le Dobson.</li>
    </ol>

    <div class="note">
      💡 <strong>Conseil :</strong> Pense à effectuer la mise en station (recalage au Nord et à l'horizontale) depuis la page <strong>🎯 Mise en Station / Alignement</strong> de l'interface Web avant de pointer tes premiers objets dans SkySafari.
    </div>
  </div>
</body>
</html>
)rawliteral";

#endif