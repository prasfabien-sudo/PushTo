# Dobson Push-To — application mobile

Cette application Capacitor réutilise les huit écrans HTML du firmware. La commande `npm run build` les extrait directement depuis `../include`, extrait aussi les icônes SVG et le catalogue, puis produit les fichiers web intégrés à Android et iOS. Les écrans ne sont donc pas recopiés ni maintenus en parallèle.

Le catalogue mobile embarqué contient le catalogue ESP32 d'origine (445 objets, inchangé côté firmware) et 8 870 étoiles de magnitude 6,5 ou plus brillantes issues de HYG 4.4. HYG est distribué sous licence CC BY-SA 4.0 par Astronexus : https://codeberg.org/astronexus/hyg. Le sous-ensemble mobile est un dérivé sous la même licence.

## Planétarium mobile et réalité augmentée

La vue du ciel mobile charge le catalogue, ses préférences et ses réglages localement et utilise la position GPS du téléphone. Après connexion au Wi-Fi de l'ESP32, le plugin natif lit les compteurs et leur résolution par TCP sur le port 4030 ; l'ESP32 ne sert pas de pages web et n'accepte pas de commandes. Les alignements et décalages restent donc locaux au téléphone. Le rendu utilise WebGL pour dessiner les étoiles, objets, constellations et grilles ; les calculs de coordonnées et la sélection restent sur le processeur, et les étiquettes restent en HTML. Si WebGL est indisponible ou que son contexte est perdu, le rendu Canvas 2D prend le relais.

Dans **Configuration > Options du ciel > Navigation au doigt**, les sens de l'azimut et de l'altitude peuvent être inversés indépendamment. Ces réglages locaux n'affectent pas les encodeurs.

Dans le menu de la vue du ciel, **Caméra AR** affiche la caméra arrière sous le planétarium et active automatiquement le suivi par capteurs. Un bouton de prise de vue apparaît dans la scène : la photo contient une image brute du flux vidéo, sans la superposition du ciel. Elle est enregistrée automatiquement dans l'album **AstroPhoto** de la galerie système. Sur iOS, autoriser l'accès complet aux photos pour créer ou retrouver cet album ; sur Android 9 et antérieur, autoriser l'accès au stockage. Si l'enregistrement échoue, l'aperçu affiche l'erreur et la photo reste utilisable pendant que la page est ouverte. Dans l'aperçu, le bouton **IA** lance la résolution et **Télécharger** propose une alternative d'enregistrement. L'accès à la caméra et à la position est demandé par Android/iOS. La superposition caméra n'est pas un suivi spatial ARKit/ARCore.

## Résolution Astrometry.net en ligne

Dans **Configuration > Paramètres techniques > Clé API Astrometry.net**, saisir et enregistrer la clé. Elle est visible dans ce champ et chiffrée avec Android Keystore ou conservée dans le trousseau iOS. Dans la vue 3D, seule une photo prise en AR peut être résolue avec **IA**. Pour choisir une autre image, utiliser **Configuration > Paramètres techniques > Outils avancés > Résolution d’image**. Une image n'est envoyée à Astrometry.net qu'après avoir lancé la résolution ; l'envoi est marqué privé. Une connexion Internet est nécessaire. Le service renvoie le centre et l'échelle de la solution, sans modifier automatiquement le pointage.

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

Dans le panneau « Connexion ESP32 », saisir le SSID (`Dobson_PushTo` par défaut), puis demander la connexion. L'application lit les encodeurs via TCP à l'adresse locale `192.168.4.1`, port `4030`. Aucun serveur HTTP n'est nécessaire sur l'ESP32.

Les systèmes mobiles contrôlent la connexion Wi-Fi : Android affiche une demande de connexion et lie le trafic de l'application au réseau choisi; iOS demande l'autorisation de rejoindre le réseau. L'application ne peut ni contourner ces confirmations, ni empêcher le système de basculer vers un autre réseau. iOS peut aussi conserver le réseau enregistré lors de futures connexions.

Sur iOS, le projet déclare déjà l'entitlement `com.apple.developer.networking.HotspotConfiguration` dans `App/App.entitlements`. Activer également la capacité **Hotspot Configuration** pour l'identifiant de l'application dans Apple Developer, puis sélectionner une équipe et un profil de signature qui incluent cette capacité. L'accès au réseau local est déclaré dans `Info.plist` par `npm run cap:sync`.

## Capteurs et catalogue

Le bouton Auto utilise la normale à l'écran comme axe de visée et diffuse son azimut et son altitude signée, y compris sous l'horizon. Orientez l'écran vers la zone observée; l'axe supérieur du téléphone ne sert pas de direction visée. Avant toute mesure, la vue démarre à l'horizon. Le déplacement tactile peut franchir le zénith sans butée. Les capteurs sont sollicités à environ 60 Hz, selon les capacités du téléphone. La vue mobile actualise le pointage local à environ 30 Hz et laisse le rendu suivre la fréquence d'affichage; les positions des étoiles et les filtres sont optimisés pour réduire le travail répété. La résolution du Canvas et les effets d'ombre sont aussi réduits sur mobile pour alléger le rendu. La collecte s'arrête lorsque l'application passe en arrière-plan.

Le catalogue mobile est embarqué dans les ressources locales de l'application et chargé seulement sur les trois pages qui l'affichent. Il n'est pas réenvoyé à un plugin natif ni recopié dans SQLite au démarrage. Sur l'écran d'accueil mobile, les 100 premiers résultats sont affichés au départ; « Afficher plus d’objets » ajoute les résultats suivants sans limiter la recherche au catalogue complet. Les critères textuels/type/magnitude sont appliqués avant les calculs astronomiques, et les positions sont mises en cache par minute et lieu; cela évite de refaire les conversions de coordonnées pour toute la liste à chaque actualisation. Le recalcul de cette liste est espacé pour garder l'interface fluide. Hors connexion, la vue utilise les capteurs et la position du téléphone; après connexion, seul le flux des encodeurs est lu sur l'ESP32.

## Vérification

`npm test` vérifie l'initialisation du catalogue local, le routage du planétarium, la résolution en ligne, l'extraction des pages, les icônes et le catalogue, ainsi que la réécriture des liens. `npm run build` valide la production des ressources Capacitor. La compilation iOS et les essais caméra, Wi-Fi/capteurs et envoi Astrometry.net sur appareil doivent être faits dans Xcode et sur téléphone.
