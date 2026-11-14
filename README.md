# Dobson Push-To

Système d'assistance au pointage pour télescope Dobson basé sur un microcontrôleur ESP32.

Ce projet transforme un télescope manuel en un système "Push-To" capable de lire les encodeurs et de transmettre leur position sur le réseau Wi-Fi local.

## Vue d'ensemble

Le système est conçu pour :

- mesurer les déplacements du télescope via des encodeurs rotatifs,
- transmettre les compteurs et leur résolution par TCP,
- afficher le pointage et gérer les réglages dans l'application mobile.

## Fonctionnalités principales

- Point d'accès Wi‑Fi embarqué : `Dobson_PushTo`
- Flux TCP de compatibilité SkySafari sur le port 4030 (lecture des encodeurs uniquement)
- Gestion des encodeurs AZ/ALT avec interruptions matérielles
- Lecture des résolutions d'encodeurs enregistrées
- Réglages, localisation et alignements gérés localement par l'application mobile

## Matériel ciblé

- Carte : ESP32 Dev Module
- Framework : Arduino / PlatformIO
- Encodeurs : 2 axes (Azimut et Altitude)
- Interface réseau : Wi‑Fi embarqué
- Contrôle : application mobile ou logiciel de planétarium compatible SkySafari

## Configuration matérielle

Les broches utilisées dans le code sont les suivantes :

- AZ A : GPIO 18
- AZ B : GPIO 19
- ALT A : GPIO 21
- ALT B : GPIO 22

Le fichier de configuration est défini dans :

- `include/Config.h`

## Prérequis

Avant de compiler et téléverser le projet, il faut installer :

- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO IDE](https://platformio.org/)
- Un ESP32 branché sur un port USB

## Installation

1. Cloner le dépôt :

```bash
git clone https://github.com/<votre_utilisateur>/Dobson_PushTo.git
cd Dobson_PushTo
```

2. Ouvrir le dossier dans VS Code.

3. Vérifier que PlatformIO détecte le projet.

4. Lancer un build :

- `PlatformIO: Build`

5. Téléverser le firmware sur l'ESP32 :

- `PlatformIO: Upload`

6. Ouvrir le moniteur série pour observer les logs :

- `PlatformIO: Serial Monitor`

## Simulation locale sur PC

Il est possible d'essayer l'interface sans ESP32 grâce au serveur de simulation Node.js inclus dans le dépôt. Il sert les pages HTML du firmware et simule les réponses des encodeurs, la configuration, les lieux et les commandes web. Les valeurs simulées sont conservées en mémoire et sont réinitialisées à l'arrêt du serveur.

Prérequis : Node.js, sans bibliothèque supplémentaire.

Depuis la racine du projet, lancer :

```powershell
node .\simulate_esp32.js
```

Puis ouvrir [http://127.0.0.1:8000](http://127.0.0.1:8000) dans le navigateur. Arrêter la simulation avec `Ctrl+C` dans le terminal.

Cette simulation valide l'interface et les échanges HTTP, mais ne remplace pas un test du firmware sur ESP32 : elle ne reproduit ni les interruptions électriques des encodeurs, ni le Wi-Fi embarqué, ni la connexion TCP SkySafari.

La barre de navigation propose un mode nuit rouge pour préserver la vision nocturne pendant l'observation. Son état est mémorisé dans le navigateur et partagé entre les pages.

### Vue 3D du ciel (bêta)

La vue 3D de l'application mobile affiche le catalogue dans une projection céleste rendue localement, sans bibliothèque distante. Le catalogue, les préférences et la position du téléphone sont locaux; l'ESP32 n'est interrogé pour les encodeurs qu'après connexion explicite, par TCP sur le port 4030. Le glissement tactile ajoute un décalage manuel.

### Application mobile native

Le dossier [`mobile-v2`](./mobile-v2/README.md) contient une application Capacitor pour Android et iOS. Ses écrans sont extraits des pages HTML actuelles du firmware pendant la compilation, sans changer le catalogue de l'ESP32. Le catalogue mobile local comprend aussi plus de 8 700 étoiles HYG. La vue ciel mobile propose une caméra en superposition, des capteurs d'orientation commandés séparément et une résolution astrométrique en ligne Astrometry.net; la clé API est conservée dans Android Keystore/iOS Keychain. La résolution hors ligne et le téléchargement d'index ne sont pas encore intégrés.

La connexion à un réseau local est soumise aux confirmations et restrictions du système mobile. La préparation, les dépendances, la compilation et les prérequis iOS/Android sont décrits dans le guide de l'application mobile.

## Configuration du projet

Le fichier principal de configuration PlatformIO est :

- `platformio.ini`

Configuration actuelle :

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
build_flags = -Iinclude
```

## Utilisation

### 1. Démarrage du système

À l'allumage, le système :

- initialise la communication série,
- charge les paramètres enregistrés dans la mémoire flash,
- configure les broches des encodeurs,
- active les interruptions,
- démarre le point d'accès Wi‑Fi,
- ouvre le flux TCP des encodeurs sur le port 4030.

### 2. Connexion des encodeurs

Le système met en place un réseau Wi‑Fi local et accepte une connexion TCP sur le port 4030.

Lorsque l'ESP32 est démarré, il est accessible sur :

- SSID : `Dobson_PushTo`
- IP locale du point d'accès : fournie via la console série

Le protocole renvoie les compteurs d'encodeurs pour `Q`/`R` et leur résolution pour `a`/`q`. L'ESP32 ne sert plus de pages web et n'accepte plus de commandes de configuration ou d'alignement ; ces réglages sont conservés dans l'application mobile.

### Diagnostic TCP de l'ESP32

Connecter le PC au Wi-Fi `Dobson_PushTo`, même s'il n'offre pas d'accès Internet. Fermer SkySafari et l'application mobile : le firmware ne permet qu'un client TCP simultané. Vérifier dans le moniteur série à 115200 bauds l'adresse IP et le démarrage du serveur sur le port 4030. Par défaut, l'adresse du point d'accès est `192.168.4.1`.

Dans PowerShell, exécuter ce diagnostic manuel, qui envoie les commandes sans retour à la ligne et affiche leurs réponses terminées par un retour chariot :

```powershell
$client = [System.Net.Sockets.TcpClient]::new()
try {
    $connection = $client.ConnectAsync("192.168.4.1", 4030)
    if (-not $connection.Wait(2000)) { throw "Délai de connexion TCP dépassé." }
    $stream = $client.GetStream()
    $stream.ReadTimeout = 2000
    foreach ($command in @("Q", "a")) {
        $bytes = [System.Text.Encoding]::ASCII.GetBytes($command)
        $stream.Write($bytes, 0, $bytes.Length)
        $response = [System.Text.StringBuilder]::new()
        do {
            $value = $stream.ReadByte()
            if ($value -lt 0) { throw "Connexion fermée pendant la réponse à $command." }
            if ($value -ne 13) { [void]$response.Append([char]$value) }
            if ($response.Length -gt 64) { throw "Réponse trop longue à $command." }
        } while ($value -ne 13)
        Write-Host "${command}: $response"
    }
} finally {
    $client.Dispose()
}
```

`Q` renvoie deux compteurs signés AZ et ALT, séparés par une tabulation ; `a` renvoie deux résolutions positives. Par exemple, au démarrage, `Q` peut répondre `+00000	+00000` et `a` `+10000	+10000`. Relancer après avoir déplacé chaque axe pour vérifier que son compteur change. Si TCP ou les réponses échouent, noter le message, vérifier le bon réseau, l'adresse série et l'absence d'autre client, puis redémarrer l'ESP32. Si TCP répond mais que les compteurs ne changent pas, contrôler séparément l'alimentation et le câblage des encodeurs. Une résolution incorrecte peut fausser les angles sans empêcher la connexion.

La LED interne clignote lorsqu'une impulsion est détectée sur la phase A de l'un des deux encodeurs, même sans connexion Wi-Fi. Tester les deux axes séparément. Cela confirme la détection d'impulsions, mais pas le bon sens de rotation ni la lecture TCP. Le projet configure par défaut une LED active à l'état haut sur GPIO 2, présente sur de nombreuses cartes ESP32 DevKit ; certaines cartes n'ont pas cette LED ou utilisent une autre broche ou polarité. Adapter `ENCODER_LED_PIN` et `ENCODER_LED_ON` dans `include/Config.h` à la carte, et ne pas utiliser une broche déjà affectée à un autre périphérique.

Le moniteur série à 115200 bauds affiche au démarrage les broches, les niveaux initiaux, l'activation des interruptions et la LED configurée. Ensuite, il affiche toutes les deux secondes les compteurs AZ/ALT et les niveaux instantanés des phases A/B. Déplacer lentement un axe à la fois et vérifier que son compteur varie. Les niveaux affichés sont des instantanés et peuvent rester identiques même en présence d'impulsions entre deux relevés ; des entrées en pull-up peuvent aussi être à 1 lorsqu'elles ne sont pas raccordées. Ces messages ne suffisent donc pas à valider le câblage ou la tension des signaux.

## Calibration

La calibration est gérée par l'application mobile et conservée localement.

Les réglages mémorisés incluent :

- `ticksAZ`
- `ticksALT`
- `revAZ`
- `revALT`
- `lat`
- `lon`
- `siteName`

Le firmware conserve uniquement les résolutions d'encodeurs dans les préférences ESP32.

## Structure du projet

```text
Dobson_PushTo/
├── include/                  # Fichiers d'en-tête
│   ├── CalibPage.h
│   ├── Config.h
│   ├── ConfigPage.h
│   ├── HelpPage.h
│   ├── ObjectsDB.h
│   ├── ReleaseNotes.h
│   ├── SimuPage.h
│   ├── StationPage.h
│   └── WebPage.h
├── src/
│   └── Dobson_PushTo.cpp    # Point d'entrée principal
├── platformio.ini           # Configuration PlatformIO
├── .gitignore
├── README.md
└── LICENSE (si ajouté plus tard)
```

## Développement

Le code est structuré en plusieurs blocs :

- gestion des interruptions encodeurs,
- gestion des préférences système,
- lecture des commandes de requête TCP,
- protocole SkySafari.

Les traitements principaux se trouvent dans :

- `src/Dobson_PushTo.cpp`
- `include/ObjectsDB.h`

## Limitations / points d'attention

- Le système est pensé pour un usage matériel dédié au télescope et à l'ESP32.
- La précision dépend de la qualité de la calibration mécanique et de la configuration des encodeurs.
- La stabilité du point d'accès Wi‑Fi dépend de l'environnement de travail et de la qualité du matériel.

## À venir / pistes d'évolution

- amélioration de la précision de calcul de l'angle,
- ajout d'un mode de calibration guidée,
- gestion de profils utilisateur multiples,
- ajout d'une base de données étendue d'objets célestes,
- extension du contrôle depuis l'application mobile.

## Notes de version

### v1.2.7 — Octobre 2026
- La vue 3D suit en temps réel les angles AZ/ALT calculés depuis les encodeurs ; le recentrage revient au pointage du télescope.

### v1.2.6 — Octobre 2026
- Bouton du mode nuit déplacé dans les raccourcis de la vue 3D et rendu plus visible avec un contraste inversé.

### v1.2.5 — Octobre 2026
- Accès direct en bas d'écran au menu principal, à la mise en station, à la vue 3D du ciel et à l'aide ; les autres pages restent dans « Plus ».

### v1.2.4 — Octobre 2026
- Déplacement du pavé du lieu d'observation au-dessus de la position du télescope.
- Mise à jour du catalogue sans recréer les lignes à chaque rafraîchissement, supprimant le clignotement des types et de la visibilité.
- Retour de la vue 3D bêta au pilotage tactile uniquement ; retrait du serveur HTTPS expérimental et des commandes de capteurs.

### v1.2.0 — Octobre 2026
- Séparation explicite entre la version globale système et les numéros de version individuels par composant (`.h` / `.cpp`).

### v1.1.1 — Octobre 2026
- Création du composant dédié aux notes de version (`ReleaseNotes.h`).
- Intégration du lien Release Notes dans le menu de navigation et sur le tag de version principal.

### v1.1.0 — Octobre 2026
- Géolocalisation GPS native du smartphone (`navigator.geolocation`) avec repli automatique sur les coordonnées ESP32.
- Affichage dynamique de la source de localisation (`GPS Mobile` ou `Lieu d'observation`).

### v1.0.4 — Octobre 2026
- Correction du calcul de précession sur l'Ascension Droite (RA) aligné sur Stellarium.

### v1.0.3 — Octobre 2026
- Correction de réfraction atmosphérique appliquée à l'altitude apparente.
- Précession astronomique IAU76 appliquée aux coordonnées RA/DEC J2000.

### v1.0.2 — Octobre 2026
- Formatage précisé des coordonnées RA/DEC (HH:MM:SS / DD:MM:SS).

### v1.0.1 — Octobre 2026
- Affichage du tag de version sémantique en haut à droite de l'interface.

### v1.0.0 — Octobre 2026
- Refonte complète de l'interface Web principale avec double jauge de guidage (AZ/ALT).
- Intégration du catalogue enrichi `ObjectsDB.h` avec éphémérides des planètes.
- Calcul de LST en UTC strict et filtrage d'objets par type/proximité.

## Licence

Ce projet est fourni à titre éducatif et de démonstration. Vérifiez la licence exacte avant toute diffusion publique ou commerciale.

## Auteur

Projet conçu pour le pilotage assisté d'un télescope Dobson avec ESP32.

## Remerciements

Merci aux communautés Arduino, PlatformIO et astronomes amateurs qui ont inspiré ce type d'outil de pointage assisté.
