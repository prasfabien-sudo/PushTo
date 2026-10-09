#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Preferences.h>

extern Preferences prefs;
extern float ticksPerRevAZ;
extern float ticksPerRevALT;
extern int activeProfile;

enum CalibState {
  IDLE,
  WAITING_AZ,
  WAITING_ALT
};

extern CalibState calState;

inline void loadCalibration() {
  prefs.begin("pushto", true);
  String keyAZ = "ticksAZ_" + String(activeProfile);
  String keyALT = "ticksALT_" + String(activeProfile);
  ticksPerRevAZ = prefs.getFloat(keyAZ.c_str(), 10000.0);
  ticksPerRevALT = prefs.getFloat(keyALT.c_str(), 10000.0);
  prefs.end();
}

inline void saveCalibration(long measuredAZ, long measuredALT) {
  prefs.begin("pushto", false);
  String keyAZ = "ticksAZ_" + String(activeProfile);
  String keyALT = "ticksALT_" + String(activeProfile);
  
  if (measuredAZ > 0) {
    ticksPerRevAZ = (float)measuredAZ;
    prefs.putFloat(keyAZ.c_str(), ticksPerRevAZ);
  }
  if (measuredALT > 0) {
    ticksPerRevALT = (float)measuredALT;
    prefs.putFloat(keyALT.c_str(), ticksPerRevALT);
  }
  prefs.end();
}

inline void saveSettings() {
  prefs.begin("pushto", false);
  prefs.putInt("profile", activeProfile);
  prefs.end();
}

#endif