#pragma once
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════
//  Gemeinsame Entprellungs-Logik für Taster & Funk-Eingänge.
//
//  Die Hauptschleife läuft nur mit ca. 10Hz (delay(100) in main.cpp). Ein
//  kurzer Tasten-/RF-Impuls könnte dabei zwischen zwei loop()-Durchläufen
//  verlorengehen, wenn man nur digitalRead() pollt. Daher fangen wir die
//  auslösende Flanke per Hardware-Interrupt ab (verpasst nichts) und werten
//  sie zeitbasiert entprellt im normalen loop() aus: Auslösungen innerhalb
//  von sperrMs nach der letzten akzeptierten werden ignoriert (fängt
//  sowohl mechanisches Prellen als auch RF-Mehrfachimpulse ab).
// ═══════════════════════════════════════════════════════════════

struct Entpreller {
  volatile bool flag;
  unsigned long sperrBis;
};

inline void entprellung_init(Entpreller& e) {
  e.flag     = false;
  e.sperrBis = 0;
}

// Aus loop() aufzurufen. Liefert true genau dann, wenn seit dem letzten
// Interrupt-Flag mindestens sperrMs seit der letzten Auslösung vergangen sind.
inline bool entprellung_ausgeloest(Entpreller& e, uint16_t sperrMs) {
  if (!e.flag) return false;
  e.flag = false;
  unsigned long jetzt = millis();
  if ((jetzt - e.sperrBis) < (unsigned long)sperrMs) return false;
  e.sperrBis = jetzt;
  return true;
}
