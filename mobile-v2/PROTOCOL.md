# Protocole BLE Dobson Push-To

## GATT

| Élément | UUID |
| --- | --- |
| Service télémétrie | `77b7a000-79c4-4baf-9b33-7f6a9d3d0001` |
| Caractéristique télémétrie | `77b7a001-79c4-4baf-9b33-7f6a9d3d0001` |

Le firmware combiné à la racine du dépôt diffuse ce service en même temps que le point d'accès Wi‑Fi et les interfaces Web/SkySafari. La caractéristique est lisible et émet des notifications. Le téléphone active les notifications après la découverte du service.

## Trame de notification

Chaque notification contient exactement 16 octets, en ordre little-endian :

| Offset | Taille | Type | Valeur |
| ---: | ---: | --- | --- |
| 0 | 4 | `int32` signé | Compteur AZ corrigé du sens configuré |
| 4 | 4 | `int32` signé | Compteur ALT corrigé du sens configuré |
| 8 | 4 | `float32` | Angle AZ relatif, en degrés |
| 12 | 4 | `float32` | Angle ALT relatif, en degrés |

Calcul : `angle = ticks × 360 / ticksParTour`. L'ESP32 émet une notification environ toutes les 250 ms. Les compteurs démarrent à zéro au redémarrage ; aucun recalage céleste n'est transmis par BLE. La configuration des résolutions et des sens est commune au Web et au service BLE.
