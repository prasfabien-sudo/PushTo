// Version Component: main.cpp v1.3.0
// Version Global System: v1.3.0
#include <WiFi.h>
#include <Preferences.h>
#include "Config.h"

Preferences prefs;
WiFiServer skySafariServer(SKYSAFARI_PORT);

volatile long countAZ = 0;
volatile long countALT = 0;
portMUX_TYPE encoderActivityMux = portMUX_INITIALIZER_UNLOCKED;
volatile bool encoderActivity = false;

float ticksPerRevAZ = 10000.0;
float ticksPerRevALT = 10000.0;

void IRAM_ATTR isrAZ() {
  if (digitalRead(AZ_A_PIN) == digitalRead(AZ_B_PIN)) ++countAZ;
  else --countAZ;
  portENTER_CRITICAL_ISR(&encoderActivityMux);
  encoderActivity = true;
  portEXIT_CRITICAL_ISR(&encoderActivityMux);
}

void IRAM_ATTR isrALT() {
  if (digitalRead(ALT_A_PIN) == digitalRead(ALT_B_PIN)) ++countALT;
  else --countALT;
  portENTER_CRITICAL_ISR(&encoderActivityMux);
  encoderActivity = true;
  portEXIT_CRITICAL_ISR(&encoderActivityMux);
}

void updateEncoderLed() {
  constexpr unsigned long interval = 120;
  static bool ledOn = false;
  static bool pendingActivity = false;
  static unsigned long lastChange = 0;
  const unsigned long now = millis();

  portENTER_CRITICAL(&encoderActivityMux);
  pendingActivity = pendingActivity || encoderActivity;
  encoderActivity = false;
  portEXIT_CRITICAL(&encoderActivityMux);

  if (ledOn && now - lastChange >= interval) {
    ledOn = false;
    lastChange = now;
    digitalWrite(ENCODER_LED_PIN, !ENCODER_LED_ON);
  } else if (!ledOn && pendingActivity && now - lastChange >= interval) {
    ledOn = true;
    pendingActivity = false;
    lastChange = now;
    digitalWrite(ENCODER_LED_PIN, ENCODER_LED_ON);
  }
}

void reportEncoderStatus() {
  constexpr unsigned long interval = 2000;
  static unsigned long lastReport = 0;
  const unsigned long now = millis();
  if (now - lastReport < interval) return;
  lastReport = now;

  const long az = countAZ;
  const long alt = countALT;
  Serial.printf("[ENCODEURS] AZ=%ld (A=%d B=%d) | ALT=%ld (A=%d B=%d)\n",
                az, digitalRead(AZ_A_PIN), digitalRead(AZ_B_PIN),
                alt, digitalRead(ALT_A_PIN), digitalRead(ALT_B_PIN));
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

  digitalWrite(ENCODER_LED_PIN, !ENCODER_LED_ON);
  pinMode(ENCODER_LED_PIN, OUTPUT);

  pinMode(AZ_A_PIN, INPUT_PULLUP);
  pinMode(AZ_B_PIN, INPUT_PULLUP);
  pinMode(ALT_A_PIN, INPUT_PULLUP);
  pinMode(ALT_B_PIN, INPUT_PULLUP);

  Serial.printf("[ENCODEURS] AZ: A=GPIO%d B=GPIO%d | ALT: A=GPIO%d B=GPIO%d\n",
                AZ_A_PIN, AZ_B_PIN, ALT_A_PIN, ALT_B_PIN);
  Serial.printf("[ENCODEURS] Niveaux initiaux AZ A=%d B=%d | ALT A=%d B=%d\n",
                digitalRead(AZ_A_PIN), digitalRead(AZ_B_PIN),
                digitalRead(ALT_A_PIN), digitalRead(ALT_B_PIN));
  Serial.printf("[ENCODEURS] LED GPIO%d active a l'etat %s\n",
                ENCODER_LED_PIN, ENCODER_LED_ON == HIGH ? "haut" : "bas");
  attachInterrupt(digitalPinToInterrupt(AZ_A_PIN), isrAZ, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ALT_A_PIN), isrALT, CHANGE);
  Serial.println("[ENCODEURS] Interruptions actives sur les phases A.");
  Serial.println("[ENCODEURS] Deplacer chaque axe : compteurs et niveaux affiches toutes les 2 secondes.");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_11dBm);

  Serial.printf("[WIFI] AP encodeurs pret: %s (IP: %s)\n",
                WIFI_SSID, WiFi.softAPIP().toString().c_str());

  skySafariServer.begin();
  skySafariServer.setNoDelay(true);
  Serial.printf("[ENCODEURS] Flux TCP disponible sur le port %d.\n", SKYSAFARI_PORT);
  Serial.println("[SYSTEME] Demarrage termine.");
}

void loop() {
  updateEncoderLed();
  reportEncoderStatus();
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
