# Dobson Push-To Mobile

Cette branche ajoute l'application Android au projet ESP32 existant. Le firmware unique à la racine du dépôt conserve l'interface Web et SkySafari par Wi‑Fi, et fournit en parallèle la télémétrie Bluetooth LE à l'application mobile. Les deux modes utilisent les mêmes encodeurs et les réglages mémorisés sur l'ESP32.

## Fonctionnalités Android

- Connexion BLE à l'ESP32 et affichage des compteurs et angles AZ/ALT.
- Catalogue Messier, NGC et planètes, recherche, tri par proximité et estimation de visibilité.
- Calcul astronomique des coordonnées horizontales et pointage d'une cible.
- Alignement sur une étoile, alignement guidé sur deux étoiles et compensation d'offsets persistante.
- Guidage graphique AZ/ALT, mode nuit, sites d'observation et localisation GPS.
- Étalonnage des résolutions d'encodeurs et simulation locale des déplacements.
- Serveur TCP SkySafari sur le port 4030, maintenu par un service Android de premier plan.
- Visée céleste 3D actualisée en direct par les capteurs d'orientation du téléphone, avec objets célestes superposés au champ visé.
- Mode réalité augmentée avec aperçu caméra arrière et objets célestes superposés ; le réticule du télescope ESP32 et celui du téléphone sont distincts.
- Plate solving en ligne avec Astrometry.net pour une photo prise ou importée. Une confirmation est demandée avant chaque envoi ; la soumission est déclarée non publique. La clé API peut être conservée chiffrée sur l'appareil via Android Keystore.
- Interface mobile revue : navigation courte, palette nuit/rouge, cartes et boutons cohérents.
- Écrans d'aide, de tests et de notes de version.

La vue céleste utilise une projection perspective et s'oriente selon le téléphone (boussole, accéléromètre/gyroscope) pour afficher les objets dans la direction visée. Comme sur la vue Web, elle permet de filtrer les objets, de faire glisser la vue et de zoomer par pincement ; « Recentrer » revient à la direction des capteurs. Les objets sont sélectionnables depuis la scène ou la liste des objets visibles. Le mode réalité augmentée affiche aussi l'aperçu de la caméra arrière sous les marqueurs ; l'autorisation caméra n'est demandée qu'à son activation. Le téléphone utilise le nord magnétique corrigé de la déclinaison locale et le réticule des encodeurs ESP32 reste affiché séparément.

## Architecture et matériel

- `../src/Dobson_PushTo.cpp` et `../include/` : firmware PlatformIO combiné, commun au Web, à SkySafari et au BLE.
- `android-app/` : application native Kotlin.
- Broches conservées : AZ A/B sur GPIO 18/19 ; ALT A/B sur GPIO 21/22.
- Le firmware transmet les compteurs d'encodeurs par BLE. Les identifiants BLE et la trame binaire sont décrits dans [PROTOCOL.md](./PROTOCOL.md).
- Android 6.0 (API 23) ou ultérieur est requis. Les versions récentes d'Android demanderont des permissions Bluetooth et localisation selon les fonctions utilisées.

Le mobile reçoit les mesures de l'ESP32 par BLE, tandis que le navigateur accède à son point d'accès Wi‑Fi `Dobson_PushTo` (adresse habituelle `192.168.4.1`). Ces deux liaisons peuvent rester actives en même temps. SkySafari communique séparément par TCP/IP : l'application Android peut héberger le serveur sur le téléphone, port `4030`; le téléphone et SkySafari doivent alors pouvoir se joindre sur le réseau local.

Le plate solving s'effectue en ligne via Astrometry.net. Il requiert un compte et une clé API Astrometry.net. L'utilisateur peut enregistrer la clé sur l'appareil ; elle est chiffrée avec AES-GCM et une clé conservée par Android Keystore. Un bouton permet de la supprimer. La photo n'est envoyée qu'après confirmation dans l'application ; la demande marque la soumission comme non publique (`publicly_visible=n`), mais l'image est transmise et traitée par un tiers : consulte les règles du service avant utilisation. Une connexion Internet est requise.

## Compiler et installer

Le projet Android est distribué sous GPL-3.0-or-later ; voir [LICENSE](./android-app/LICENSE) et [THIRD_PARTY_NOTICES.md](./android-app/THIRD_PARTY_NOTICES.md). Le projet contient un Gradle Wrapper et peut être compilé dans VS Code ou depuis un terminal, avec JDK 17 et le SDK Android :

```powershell
cd android-app
.\gradlew.bat assembleDebug
```

L'APK debug est généré dans `android-app/app/build/outputs/apk/debug/app-debug.apk`. Pour l'installer par USB, active le débogage USB et exécute :

```powershell
.\gradlew.bat installDebug
```

Sinon, copie l'APK sur le téléphone et autorise son installation depuis le gestionnaire de fichiers. Au premier lancement, accorde les permissions nécessaires puis lance la recherche BLE pour connecter le module « Dobson Push-To ».

Pour le firmware combiné, ouvre la racine du dépôt avec PlatformIO, puis compile et téléverse l'environnement `esp32dev`. Le profil `huge_app.csv` réserve l'espace flash requis par le firmware combiné ; téléverse le firmware par USB avec PlatformIO.

### Développement sans matériel

La version debug de l'application peut se connecter au simulateur ESP32 Node.js de la racine du dépôt :

1. Sur l'ordinateur, démarre `node .\simulate_esp32.js` depuis la racine du dépôt. Le simulateur expose son API HTTP sur le port 8000.
2. Démarre un émulateur Android.
3. Dans l'application debug, depuis l'accueil, choisis **Connecter le simulateur ESP32**. L'émulateur Android joint le serveur hôte par `10.0.2.2:8000`.
4. Dans **Simulation**, les boutons AZ/ALT déplacent les compteurs du simulateur partagé ; les changements faits par la page Web simulée sont aussi reçus par l'application.

Ce raccordement HTTP est disponible uniquement dans l'APK debug ; l'APK release conserve la connexion BLE réelle. Les fonctions téléphone matérielles absentes de l'émulateur (BLE périphérique, GPS et caméra) demandent des simulations propres à l'émulateur ou un vrai appareil.

## Tests et limites

Les tests unitaires Android se lancent avec `.\gradlew.bat testDebugUnitTest`. Le firmware se compile avec `pio run` depuis la racine du dépôt.

Une image astronomique lisible et une connexion Internet sont nécessaires pour le plate solving en ligne. Le fonctionnement avec un télescope réel, le GPS, le plate solver et SkySafari doit être vérifié sur les appareils concernés. La connexion BLE et le serveur SkySafari sont indépendants ; l'ESP32 n'héberge pas le serveur TCP.
