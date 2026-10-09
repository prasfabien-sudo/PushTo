# Dobson Push-To

Système d'assistance au pointage pour télescope Dobson basé sur un microcontrôleur ESP32.

Ce projet transforme un télescope manuel en un système "Push-To" capable de mesurer en temps réel l'azimut et l'altitude grâce à des encodeurs, de conserver les réglages en mémoire flash, de servir une interface web locale et de communiquer avec des logiciels de planétarium comme SkySafari.

## Vue d'ensemble

Le système est conçu pour :

- mesurer les déplacements du télescope via des encodeurs rotatifs,
- calculer les positions en azimut et en altitude,
- fournir une interface web embarquée pour configurer et calibrer le système,
- enregistrer les paramètres dans la mémoire non volatile de l'ESP32,
- proposer une base d'objets astronomiques (catalogue Messier) pour aider au repérage,
- se connecter à des applications tierces via le protocole SkySafari.

## Fonctionnalités principales

- Point d'accès Wi‑Fi embarqué : `Dobson_PushTo`
- Serveur web local sur le port 80
- Serveur SkySafari sur le port 4030
- Gestion des encodeurs AZ/ALT avec interruptions matérielles
- Calibration des ticks par révolution
- Inversion de sens de rotation configurable
- Réglage de la position géographique (latitude / longitude)
- Réinitialisation de zéro en azimut et en altitude
- Alignement sur Polaris
- Alignement assisté sur étoile / double étoile
- Sauvegarde des paramètres dans la flash ESP32
- Catalogue d'objets astronomiques (Messier)

## Matériel ciblé

- Carte : ESP32 Dev Module
- Framework : Arduino / PlatformIO
- Encodeurs : 2 axes (Azimut et Altitude)
- Interface réseau : Wi‑Fi embarqué
- Démonstration / contrôle : navigateur web local ou logiciel de planétarium compatible SkySafari

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
- ouvre les serveurs web et SkySafari.

### 2. Accès web

Le système met en place un réseau Wi‑Fi local.

Lorsque l'ESP32 est démarré, il est accessible sur :

- SSID : `Dobson_PushTo`
- IP locale du point d'accès : fournie via la console série

Depuis un navigateur, il est possible d'accéder aux pages suivantes :

- `/` : page principale
- `/station` : station de contrôle
- `/calib_page` : calibration
- `/config` : paramètres de configuration
- `/simu` : simulation
- `/help` : aide
- `/releasenotes` : notes de version
- `/test` : page de test

### 3. API serveur

Le système expose plusieurs endpoints HTTP simples :

- `/status` : retourne l'état des compteurs et paramètres
- `/cmd` : actions de configuration et d'alignement
- `/calib` : calibration
- `/simstep` : simulation de pas d'encodeur
- `/catalog` : catalogue des objets astronomiques
- `/get_sites` : liste des lieux enregistrés

Exemples d'actions via `cmd` :

- `set_config`
- `set_location`
- `set_zero_alt`
- `set_zero_az`
- `set_polaris`
- `align_star`
- `align_star1`
- `align_star2`
- `save_sites`

## Calibration

La calibration est essentielle pour obtenir une précision correcte.

Les réglages mémorisés incluent :

- `ticksAZ`
- `ticksALT`
- `revAZ`
- `revALT`
- `lat`
- `lon`
- `siteName`

La calibration est conservée dans la mémoire flash de l'ESP32 via `Preferences`.

## Structure du projet

```text
Dobson_PushTo/
├── include/                  # Fichiers d'en-tête
│   ├── CalibPage.h
│   ├── Calibration.h
│   ├── Config.h
│   ├── ConfigPage.h
│   ├── HelpPage.h
│   ├── ObjectsDB.h
│   ├── ReleaseNotes.h
│   ├── SimuPage.h
│   ├── StationPage.h
│   ├── TestPage.h
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
- routes HTTP,
- logique de calibration,
- interface web embarquée,
- protocole SkySafari.

Les traitements principaux se trouvent dans :

- `src/Dobson_PushTo.cpp`
- `include/Calibration.h`
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
- extension du contrôle depuis une page web plus avancée.

## Notes de version

### v1.2.0 — Octobre 2026
- Séparation explicite entre la version globale système et les numéros de version individuels par composant (`.h` / `.cpp`).

### v1.1.1 — Octobre 2026
- Création du composant dédié aux notes de version (`ReleaseNotes.h`).
- Intégration du lien Release Notes dans le menu de navigation et sur le tag de version principal.

### v1.1.0 — Octobre 2026
- Géolocalisation GPS native du smartphone (`navigator.geolocation`) avec repli automatique sur les coordonnées ESP32.
- Affichage dynamique de la source de localisation (`📍 GPS Mobile` ou `📍 Lieu d'observation`).

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
