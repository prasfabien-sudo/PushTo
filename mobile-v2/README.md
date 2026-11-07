# Dobson Push-To — application mobile

Cette application Capacitor réutilise les huit écrans HTML du firmware. La commande `npm run build` les extrait directement depuis `../include`, extrait aussi les icônes SVG et le catalogue, puis produit les fichiers web intégrés à Android et iOS. Les écrans ne sont donc pas recopiés ni maintenus en parallèle.

Le catalogue mobile embarqué contient le catalogue ESP32 d'origine (445 objets, inchangé côté firmware) et 8 870 étoiles de magnitude 6,5 ou plus brillantes issues de HYG 4.4. HYG est distribué sous licence CC BY-SA 4.0 par Astronexus : https://codeberg.org/astronexus/hyg. Le sous-ensemble mobile est un dérivé sous la même licence.

## Planétarium mobile et réalité augmentée

La vue du ciel mobile charge le catalogue et ses préférences localement et utilise la position GPS du téléphone. Hors connexion explicite à l'ESP32, elle n'appelle pas ses routes de télémétrie; après connexion, elle lit `/status` pour les encodeurs. La page du firmware servie par l'ESP32 conserve son fonctionnement et son catalogue d'origine.

Le bouton **Caméra AR**, dans le coin supérieur droit de la vue du ciel, affiche uniquement la caméra arrière sous le planétarium. Il n'active ni ne désactive les capteurs. La commande **Activer la boussole et les capteurs**, dans « Connexion et capteurs », gère séparément les mesures d'orientation du téléphone. L'accès à la caméra et à la position est demandé par Android/iOS. La superposition caméra n'est pas un suivi spatial ARKit/ARCore.

## Résolution Astrometry.net en ligne

Sous la carte du ciel, ouvrir **Résolution astrométrique en ligne**, saisir votre clé API Astrometry.net et la sauvegarder. Elle est chiffrée avec Android Keystore ou conservée dans le trousseau iOS. Une image n'est envoyée à Astrometry.net qu'après avoir choisi une image et lancé la résolution; l'envoi est marqué privé. Une connexion Internet est nécessaire. Le service traite l'image et renvoie le centre et l'échelle de la solution.

Le solveur Astrometry.net hors ligne et le téléchargement de ses index ne sont pas encore inclus. ASTAP ne fournit pas de binaire Android/iOS officiel; ses bases de bureau ne sont pas utilisables seules sans porter son moteur. Le portage du moteur local Astrometry.net et un gestionnaire d'index restent à réaliser et à valider sur appareils.

## Préparation

```powershell
cd .\mobile-v2
npm install
npm test
npm run build
```

Le projet Android généré est inclus. Pour synchroniser ses ressources natives après une modification, lancer `npm run cap:sync`. Android nécessite Android Studio, le SDK Android et un JDK 21. Le jumelage assisté au Wi-Fi nécessite Android 10 ou ultérieur.

Le projet iOS généré utilise Swift Package Manager. Sa compilation nécessite macOS, Xcode et iOS 15 ou ultérieur. Sur macOS, synchroniser les ressources puis ouvrir le projet Xcode :

```powershell
npm run cap:sync
```

Dans Android Studio, ouvrir le projet `android` et lancer l'application sur un appareil. Dans Xcode, ouvrir `ios/App/App.xcodeproj`, sélectionner une équipe de signature et lancer l'application sur un iPhone.

## Connexion au Dobson

Dans le panneau « Connexion et capteurs », saisir le SSID de l'ESP32 (`Dobson_PushTo` par défaut), puis demander la connexion. L'adresse du serveur par défaut est `http://192.168.4.1`; elle peut être modifiée si le firmware utilise une autre adresse. Les pages existantes utilisent ensuite les mêmes routes HTTP que l'interface web.

Les systèmes mobiles contrôlent la connexion Wi-Fi : Android affiche une demande de connexion et lie le trafic de l'application au réseau choisi; iOS demande l'autorisation de rejoindre le réseau. L'application ne peut ni contourner ces confirmations, ni empêcher le système de basculer vers un autre réseau. iOS peut aussi conserver le réseau enregistré lors de futures connexions.

Sur iOS, le projet déclare déjà l'entitlement `com.apple.developer.networking.HotspotConfiguration` dans `App/App.entitlements`. Activer également la capacité **Hotspot Configuration** pour l'identifiant de l'application dans Apple Developer, puis sélectionner une équipe et un profil de signature qui incluent cette capacité. L'accès au réseau local est déclaré dans `Info.plist` par `npm run cap:sync`.

## Capteurs et catalogue

Le bouton « Activer la boussole et les capteurs » utilise la normale à l'écran comme axe de visée et diffuse son azimut et son altitude signée, y compris sous l'horizon. Orientez l'écran vers la zone observée; l'axe supérieur du téléphone ne sert pas de direction visée. Avant toute mesure, la vue démarre à l'horizon. Le déplacement tactile peut franchir le zénith sans butée. Les capteurs sont sollicités à environ 60 Hz, selon les capacités du téléphone. La vue mobile actualise le pointage local à environ 30 Hz et laisse le rendu Canvas suivre la fréquence d'affichage; les positions des étoiles et les filtres sont optimisés pour réduire le travail répété. La résolution du Canvas et les effets d'ombre sont aussi réduits sur mobile pour alléger le rendu. La collecte s'arrête lorsque l'application passe en arrière-plan.

Le catalogue mobile est embarqué dans les ressources locales de l'application et chargé seulement sur les trois pages qui l'affichent. Il n'est pas réenvoyé à un plugin natif ni recopié dans SQLite au démarrage. Sur l'écran d'accueil mobile, les 100 premiers résultats sont affichés au départ; « Afficher plus d’objets » ajoute les résultats suivants sans limiter la recherche au catalogue complet. Les critères textuels/type/magnitude sont appliqués avant les calculs astronomiques, et les positions sont mises en cache par minute et lieu; cela évite de refaire les conversions de coordonnées pour toute la liste à chaque actualisation. Le recalcul de cette liste est espacé pour garder l'interface fluide. Hors connexion explicite, l'état local évite les appels de télémétrie répétés; après connexion, les routes de contrôle communiquent avec l'ESP32.

## Vérification

`npm test` vérifie l'initialisation du catalogue local, le routage du planétarium, la résolution en ligne, l'extraction des pages, les icônes et le catalogue, ainsi que la réécriture des liens. `npm run build` valide la production des ressources Capacitor. La compilation iOS et les essais caméra, Wi-Fi/capteurs et envoi Astrometry.net sur appareil doivent être faits dans Xcode et sur téléphone.
