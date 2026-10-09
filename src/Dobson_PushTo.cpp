// Version Component: main.cpp v1.2.0
// Version Global System: v1.2.7
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "Config.h"
#include "Calibration.h"
#include "ObjectsDB.h"
#include "WebPage.h"
#include "CalibPage.h"
#include "SimuPage.h"
#include "StationPage.h"
#include "ConfigPage.h"
#include "HelpPage.h"
#include "TestPage.h"
#include "ReleaseNotes.h"
#include "Icons.h"
#include "BetaSkyPage.h"

Preferences prefs;
WiFiServer skySafariServer(SKYSAFARI_PORT);
WebServer webServer(WEB_PORT);

volatile long countAZ = 0;
volatile long countALT = 0;

float ticksPerRevAZ = 10000.0;
float ticksPerRevALT = 10000.0;

int dirAZ = 1;
int dirALT = 1;
int activeProfile = 0;

CalibState calState = IDLE;
long startAZ = 0, startALT = 0;
long measuredAZ = 0, measuredALT = 0;

// Variables pour l'alignement pas à pas 2 étoiles
float star1_az = 0, star1_alt = 0;
long star1_countAZ = 0, star1_countALT = 0;
bool star1_set = false;

void IRAM_ATTR isrAZ() {
  if (digitalRead(AZ_A_PIN) == digitalRead(AZ_B_PIN)) countAZ += dirAZ;
  else countAZ -= dirAZ;
}

void IRAM_ATTR isrALT() {
  if (digitalRead(ALT_A_PIN) == digitalRead(ALT_B_PIN)) countALT += dirALT;
  else countALT -= dirALT;
}

void loadSettingsFromPreferences() {
  prefs.begin("dobson", true);
  ticksPerRevAZ = prefs.getFloat("ticksAZ", 10000.0);
  ticksPerRevALT = prefs.getFloat("ticksALT", 10000.0);
  dirAZ = prefs.getBool("revAZ", false) ? -1 : 1;
  dirALT = prefs.getBool("revALT", false) ? -1 : 1;
  prefs.end();
  Serial.printf("[PREFS] Ticks AZ: %.0f | Ticks ALT: %.0f | Dir AZ: %d | Dir ALT: %d\n",
                ticksPerRevAZ, ticksPerRevALT, dirAZ, dirALT);
}

void handleRoot() {
  webServer.send(200, "text/html", HTTP_PAGE);
}
void handleCalibPage() {
  webServer.send(200, "text/html", HTTP_CALIB_PAGE);
}
void handleSimu() {
  webServer.send(200, "text/html", HTTP_SIMU_PAGE);
}
void handleConfigPage() {
  webServer.send(200, "text/html", HTTP_CONFIG_PAGE);
}
void handleReleaseNotesPage() {
  webServer.send(200, "text/html", HTTP_RELEASE_NOTES_PAGE);
}
void handleIcons() {
  webServer.send(200, "image/svg+xml", HTTP_ICONS);
}

void handleCatalog() {
  String json = "[";
  for (size_t i = 0; i < DSO_COUNT; i++) {
    DSO obj;
    memcpy_P(&obj, &DSO_CATALOG[i], sizeof(DSO));

    json += "{";
    json += "\"name\":\"" + String(obj.name) + "\",";
    json += "\"commonName\":\"" + String(obj.commonName) + "\",";
    json += "\"type\":\"" + String(obj.type) + "\",";
    json += "\"ra\":" + String(obj.ra, 3) + ",";
    json += "\"dec\":" + String(obj.dec, 2) + ",";
    json += "\"mag\":" + String(obj.mag, 1);
    json += "}";
    if (i < DSO_COUNT - 1) json += ",";
  }
  json += "]";
  webServer.send(200, "application/json", json);
}

String getSitesJsonFromPrefs() {
  prefs.begin("sites_db", true);
  String json = prefs.getString("json", "");
  prefs.end();

  if (json == "" || json == "{}") {
    json = "{\"chavanod\":{\"name\":\"Chavanod\",\"lat\":45.89,\"lon\":6.05},\"annecy\":{\"name\":\"Annecy\",\"lat\":45.90,\"lon\":6.12},\"lyon\":{\"name\":\"Lyon\",\"lat\":45.76,\"lon\":4.83},\"paris\":{\"name\":\"Paris\",\"lat\":48.85,\"lon\":2.35}}";
    prefs.begin("sites_db", false);
    prefs.putString("json", json);
    prefs.end();
  }
  return json;
}

void handleGetSites() {
  webServer.send(200, "application/json", getSitesJsonFromPrefs());
}

void handleStatus() {
  prefs.begin("dobson", true);
  float lat = prefs.getFloat("lat", 45.89);
  float lon = prefs.getFloat("lon", 6.05);
  String siteName = prefs.getString("siteName", "Chavanod");
  prefs.end();

  String json = "{";
  json += "\"rawAZ\":" + String(countAZ) + ",";
  json += "\"rawALT\":" + String(countALT) + ",";
  json += "\"ticksAZ\":" + String(ticksPerRevAZ, 0) + ",";
  json += "\"ticksALT\":" + String(ticksPerRevALT, 0) + ",";
  json += "\"dirAZ\":" + String(dirAZ) + ",";
  json += "\"dirALT\":" + String(dirALT) + ",";
  json += "\"revAZ\":" + String(dirAZ == -1 ? "true" : "false") + ",";
  json += "\"revALT\":" + String(dirALT == -1 ? "true" : "false") + ",";
  json += "\"profile\":" + String(activeProfile) + ",";
  json += "\"lat\":" + String(lat, 4) + ",";
  json += "\"lon\":" + String(lon, 4) + ",";
  json += "\"siteName\":\"" + siteName + "\"";
  json += "}";
  webServer.send(200, "application/json", json);
}

void handleCmd() {
  if (!webServer.hasArg("action")) {
    webServer.send(400, "text/plain", "Bad Request: Missing action parameter");
    return;
  }

  String act = webServer.arg("action");

  if (act == "set_config") {
    if (webServer.hasArg("ticksAZ") && webServer.hasArg("ticksALT") && webServer.hasArg("revAZ") && webServer.hasArg("revALT") && webServer.hasArg("lat") && webServer.hasArg("lon")) {

      ticksPerRevAZ = webServer.arg("ticksAZ").toFloat();
      ticksPerRevALT = webServer.arg("ticksALT").toFloat();
      bool revAZ = webServer.arg("revAZ").toInt() == 1;
      bool revALT = webServer.arg("revALT").toInt() == 1;
      dirAZ = revAZ ? -1 : 1;
      dirALT = revALT ? -1 : 1;

      float lat = webServer.arg("lat").toFloat();
      float lon = webServer.arg("lon").toFloat();
      String siteName = webServer.hasArg("siteName") ? webServer.arg("siteName") : "Lieu personnalisé";

      prefs.begin("dobson", false);
      prefs.putFloat("ticksAZ", ticksPerRevAZ);
      prefs.putFloat("ticksALT", ticksPerRevALT);
      prefs.putBool("revAZ", revAZ);
      prefs.putBool("revALT", revALT);
      prefs.putFloat("lat", lat);
      prefs.putFloat("lon", lon);
      prefs.putString("siteName", siteName);
      prefs.end();

      Serial.printf("[CONFIG] Configuration mise a jour dans l'ESP32 : Site %s (%.4f, %.4f)\n", siteName.c_str(), lat, lon);
      webServer.send(200, "application/json", "{\"status\":\"OK\"}");
    } else {
      webServer.send(400, "text/plain", "Bad Request: Missing config parameters");
    }

  } else if (act == "set_location") {
    if (webServer.hasArg("lat") && webServer.hasArg("lon")) {
      float lat = webServer.arg("lat").toFloat();
      float lon = webServer.arg("lon").toFloat();
      String siteName = webServer.hasArg("siteName") ? webServer.arg("siteName") : "Lieu personnalisé";

      prefs.begin("dobson", false);
      prefs.putFloat("lat", lat);
      prefs.putFloat("lon", lon);
      prefs.putString("siteName", siteName);
      prefs.end();
      Serial.printf("[WEB] Position enregistree sur ESP32 : %s (Lat %.2f, Lon %.2f)\n", siteName.c_str(), lat, lon);
      webServer.send(200, "text/plain", "OK");
    } else {
      webServer.send(400, "text/plain", "Bad Request: Missing lat or lon");
    }

  } else if (act == "set_zero_alt") {
    countALT = 0;
    Serial.println("[STATION] Remise a 0 Altitude (Horizontale)");
    webServer.send(200, "text/plain", "OK");

  } else if (act == "set_zero_az") {
    countAZ = 0;
    Serial.println("[STATION] Remise a 0 Azimut (Nord)");
    webServer.send(200, "text/plain", "OK");

  } else if (act == "set_polaris") {
    countAZ = 0;
    prefs.begin("dobson", true);
    float currentLat = prefs.getFloat("lat", 45.89);
    prefs.end();
    countALT = lroundf(((currentLat / 360.0f) * ticksPerRevALT) * dirALT);
    Serial.printf("[STATION] Aligne sur Polaris : AZ=0 deg, ALT=%.2f deg\n", currentLat);
    webServer.send(200, "text/plain", "OK");

  } else if (act == "align_star") {
    if (webServer.hasArg("az") && webServer.hasArg("alt")) {
      float targetAZ = webServer.arg("az").toFloat();
      float targetALT = webServer.arg("alt").toFloat();

      countAZ = lroundf(((targetAZ / 360.0f) * ticksPerRevAZ) * dirAZ);
      countALT = lroundf(((targetALT / 360.0f) * ticksPerRevALT) * dirALT);

      Serial.printf("[ALIGN] Recalage sur etoile : AZ=%.2f deg (%ld tics), ALT=%.2f deg (%ld tics)\n",
                    targetAZ, countAZ, targetALT, countALT);
      webServer.send(200, "text/plain", "OK");
    } else {
      webServer.send(400, "text/plain", "Bad Request: Missing az or alt");
    }

  } else if (act == "align_star1") {
    if (webServer.hasArg("az") && webServer.hasArg("alt")) {
      star1_az = webServer.arg("az").toFloat();
      star1_alt = webServer.arg("alt").toFloat();
      star1_countAZ = countAZ;
      star1_countALT = countALT;
      star1_set = true;

      Serial.printf("[ALIGN 2-STARS] Etoile 1 enregistree : AZ=%.2f, ALT=%.2f (Ticks: %ld, %ld)\n",
                    star1_az, star1_alt, star1_countAZ, star1_countALT);
      webServer.send(200, "application/json", "{\"status\":\"OK\",\"step\":1}");
    } else {
      webServer.send(400, "text/plain", "Bad Request: Missing az or alt");
    }

  } else if (act == "align_star2") {
    if (!star1_set) {
      webServer.send(400, "text/plain", "Veuillez valider l'etoile 1 d'abord");
      return;
    }
    if (webServer.hasArg("az") && webServer.hasArg("alt")) {
      float star2_az = webServer.arg("az").toFloat();
      float star2_alt = webServer.arg("alt").toFloat();

      float targetAZ = (star1_az + star2_az) / 2.0f;
      float targetALT = (star1_alt + star2_alt) / 2.0f;

      countAZ = lroundf(((targetAZ / 360.0f) * ticksPerRevAZ) * dirAZ);
      countALT = lroundf(((targetALT / 360.0f) * ticksPerRevALT) * dirALT);

      star1_set = false;
      Serial.printf("[ALIGN 2-STARS] Alignement finalise : AZ=%.2f deg, ALT=%.2f deg\n", targetAZ, targetALT);
      webServer.send(200, "application/json", "{\"status\":\"OK\",\"step\":2}");
    } else {
      webServer.send(400, "text/plain", "Bad Request: Missing az or alt");
    }

  } else if (act == "align") {
    countAZ = 0;
    countALT = 0;
    Serial.println("[CMD] Alignement / Reset des compteurs execute");
    webServer.send(200, "text/plain", "OK");

  } else if (act == "save_sites") {
    if (webServer.hasArg("json")) {
      String json = webServer.arg("json");
      prefs.begin("sites_db", false);
      prefs.putString("json", json);
      prefs.end();
      Serial.println("[SITE] Base de lieux mise a jour dans la Flash ESP32");
      webServer.send(200, "application/json", "{\"status\":\"OK\"}");
    } else {
      webServer.send(400, "text/plain", "Bad Request: Missing json");
    }

  } else {
    webServer.send(400, "text/plain", "Unknown action");
  }
}

void handleCalib() {
  if (!webServer.hasArg("cmd")) {
    webServer.send(400, "text/plain", "Bad Request");
    return;
  }

  String cmd = webServer.arg("cmd");

  if (cmd == "start") {
    countAZ = 0;
    countALT = 0;
    Serial.println("[CALIB] Demarrage etalonnage : Compteurs remis a 0");

  } else if (cmd == "fin_az") {
    if (abs(countAZ) > 0) {
      ticksPerRevAZ = abs(countAZ);
      prefs.begin("dobson", false);
      prefs.putFloat("ticksAZ", ticksPerRevAZ);
      prefs.end();
      Serial.printf("[CALIB] AZ enregistre : %.0f pas/rev\n", ticksPerRevAZ);
    }

  } else if (cmd == "fin_alt_90") {
    if (abs(countALT) > 0) {
      ticksPerRevALT = abs(countALT) * 4.0;
      prefs.begin("dobson", false);
      prefs.putFloat("ticksALT", ticksPerRevALT);
      prefs.end();
      Serial.printf("[CALIB] ALT (90 deg) enregistre : %.0f pas/rev\n", ticksPerRevALT);
    }
  }

  webServer.send(200, "text/plain", "OK");
}

void handleSimStep() {
  if (webServer.hasArg("axis") && webServer.hasArg("delta")) {
    String axis = webServer.arg("axis");
    long delta = webServer.arg("delta").toInt();

    if (axis == "az") countAZ += delta * dirAZ;
    else if (axis == "alt") countALT += delta * dirALT;
    else if (axis == "reset") {
      countAZ = 0;
      countALT = 0;
    }
    Serial.printf("[SIMU] Step sur %s (delta: %ld) -> AZ: %ld | ALT: %ld\n", axis.c_str(), delta, countAZ, countALT);
  }
  webServer.send(200, "text/plain", "OK");
}

void handleStationPage() {
  webServer.send(200, "text/html", HTTP_STATION_PAGE);
}
void handleBetaSkyPage() {
  webServer.send(200, "text/html", HTTP_BETA_SKY_PAGE);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== DEMARRAGE DOBSON PUSH-TO ===");

  loadCalibration();
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

  Serial.printf("[WIFI] AP Ouvert Pret: %s (IP: %s)\n", WIFI_SSID, WiFi.softAPIP().toString().c_str());

  webServer.on("/", handleRoot);
  webServer.on("/station", handleStationPage);
  webServer.on("/beta_sky", handleBetaSkyPage);
  webServer.on("/calib_page", handleCalibPage);
  webServer.on("/config", handleConfigPage);
  webServer.on("/releasenotes", handleReleaseNotesPage);
  webServer.on("/icons.svg", handleIcons);
  webServer.on("/simu", handleSimu);
  webServer.on("/catalog", handleCatalog);
  webServer.on("/get_sites", handleGetSites);
  webServer.on("/status", handleStatus);
  webServer.on("/cmd", handleCmd);
  webServer.on("/calib", handleCalib);
  webServer.on("/simstep", handleSimStep);
  webServer.on("/test", []() {
    webServer.send(200, "text/html", HTTP_TEST_PAGE);
  });
  webServer.on("/help", []() {
    webServer.send(200, "text/html", HTTP_HELP_PAGE);
  });
  webServer.begin();
  Serial.println("[WEB] Serveur Web demarre sur le port 80.");

  skySafariServer.begin();
  skySafariServer.setNoDelay(true);
  Serial.printf("[SKYSAFARI] Serveur TCP ecoute sur le port %d.\n", SKYSAFARI_PORT);
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

void loop() {
  webServer.handleClient();

  if (skySafariServer.hasClient()) {
    if (!activeClient || !activeClient.connected()) {
      activeClient = skySafariServer.available();
      activeClient.setNoDelay(true);
      Serial.printf("[SKYSAFARI] CLIENT CONNECTE (IP: %s)\n", activeClient.remoteIP().toString().c_str());
    } else {
      WiFiClient extraClient = skySafariServer.available();
      extraClient.stop();
    }
  }

  if (activeClient && activeClient.connected()) {
    while (activeClient.available() > 0) {
      char c = activeClient.read();
      if (c == '\r' || c == '\n') continue;

      if (c == 'Q' || c == 'R') {
        sendEncoderData(activeClient);
      } else if (c == 'a' || c == 'q') {
        sendEncoderResolution(activeClient);
      } else if (c == 'H') {
        activeClient.print("y\r");
      } else if (c == 'V') {
        activeClient.print("V1.0\r");
      } else {
        sendEncoderData(activeClient);
      }
    }
  }

  yield();
}