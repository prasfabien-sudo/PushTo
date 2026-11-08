// Version Component: main.cpp v1.3.0
// Version Global System: v1.3.0
#include <WiFi.h>
#include <Preferences.h>
#include "Config.h"

Preferences prefs;
WiFiServer skySafariServer(SKYSAFARI_PORT);

volatile long countAZ = 0;
volatile long countALT = 0;

float ticksPerRevAZ = 10000.0;
float ticksPerRevALT = 10000.0;

void IRAM_ATTR isrAZ() {
  if (digitalRead(AZ_A_PIN) == digitalRead(AZ_B_PIN)) ++countAZ;
  else --countAZ;
}

void IRAM_ATTR isrALT() {
  if (digitalRead(ALT_A_PIN) == digitalRead(ALT_B_PIN)) ++countALT;
  else --countALT;
}

void loadSettingsFromPreferences() {
  prefs.begin("dobson", true);
  ticksPerRevAZ = prefs.getFloat("ticksAZ", 10000.0);
  ticksPerRevALT = prefs.getFloat("ticksALT", 10000.0);
  prefs.end();
  Serial.printf("[PREFS] Ticks AZ: %.0f | Ticks ALT: %.0f\n", ticksPerRevAZ, ticksPerRevALT);
}

void sendEncoderData(WiFiClient& client) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%+06ld\t%+06ld\r", countAZ, countALT);
  client.print(buf);
}

void sendEncoderResolution(WiFiClient& client) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%+06ld\t%+06ld\r", (long)ticksPerRevAZ, (long)ticksPerRevALT);
  client.print(buf);
}

WiFiClient activeClient;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== DEMARRAGE DOBSON PUSH-TO ===");

  loadSettingsFromPreferences();

  pinMode(AZ_A_PIN, INPUT_PULLUP);
  pinMode(AZ_B_PIN, INPUT_PULLUP);
  pinMode(ALT_A_PIN, INPUT_PULLUP);
  pinMode(ALT_B_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(AZ_A_PIN), isrAZ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ALT_A_PIN), isrALT, CHANGE);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_11dBm);

  Serial.printf("[WIFI] AP encodeurs pret: %s (IP: %s)\n",
                WIFI_SSID, WiFi.softAPIP().toString().c_str());

  skySafariServer.begin();
  skySafariServer.setNoDelay(true);
  Serial.printf("[ENCODEURS] Flux TCP disponible sur le port %d.\n", SKYSAFARI_PORT);
}

void loop() {
  if (skySafariServer.hasClient()) {
    if (!activeClient || !activeClient.connected()) {
      activeClient = skySafariServer.available();
      activeClient.setNoDelay(true);
      Serial.printf("[ENCODEURS] Client connecte (IP: %s)\n", activeClient.remoteIP().toString().c_str());
    } else {
      WiFiClient extraClient = skySafariServer.available();
      extraClient.stop();
    }
  }

  if (activeClient && activeClient.connected()) {
    while (activeClient.available() > 0) {
      const char command = activeClient.read();
      if (command == 'Q' || command == 'R') {
        sendEncoderData(activeClient);
      } else if (command == 'a' || command == 'q') {
        sendEncoderResolution(activeClient);
      } else if (command == 'H') {
        activeClient.print("y\r");
      } else if (command == 'V') {
        activeClient.print("V1.0\r");
      }
    }
  }

  yield();
}
