#ifndef CONFIG_H
#define CONFIG_H

// Configuration Wi-Fi AP
#define WIFI_SSID "Dobson_PushTo"
#define WIFI_PASS ""

// Ports
#define WEB_PORT 80
#define SKYSAFARI_PORT 4030

// Broches Encodeurs (ESP32)
// 🌀 Encodeur Azimut (AZ) :
// Fil Marron  5V de l'ESP32   
// Fil Bleu  GND de l'ESP32   
// Fil Noir (Phase A)  GPIO 18 (AZ_A_PIN)   
// Fil Blanc (Phase B)  GPIO 19 (AZ_B_PIN)   
#define AZ_A_PIN 18
#define AZ_B_PIN 19
// 🔭 Encodeur Altitude (ALT) :
// Fil Marron  5V de l'ESP32   
// Fil Bleu  GND de l'ESP32   
// Fil Noir (Phase A)  GPIO 21 (ALT_A_PIN)   
// Fil Blanc (Phase B)  GPIO 22 (ALT_B_PIN)   
#define ALT_A_PIN 21
#define ALT_B_PIN 22



#endif