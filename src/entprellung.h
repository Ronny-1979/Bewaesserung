#pragma once
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════
//  Gemeinsame Entprellungs-Logik für Taster (gegen GND, INPUT_PULLUP).
//  Ersetzt den vorher doppelt vorhandenen Code in taster.cpp und
//  licht_steuerung.cpp durch eine einzige, gemeinsam genutzte Stelle.
// ═══════════════════════════════════════════════════════════════

struct Entpreller {
  bool          letzterPegel;
  bool          bestaetigt;
  unsigned long aenderungsZeit;
  uint16_t      debounceMs;
};

inline void entprellung_init(Entpreller& e, int pin, uint16_t debounceMs) {
  e.letzterPegel   = digitalRead(pin);
  e.bestaetigt     = e.letzterPegel;
  e.aenderungsZeit = millis();
  e.debounceMs     = debounceMs;
}

// Liefert true genau dann, wenn der entprellte Pegel gerade auf LOW
// gewechselt hat (fallende Flanke, z.B. Taster gegen GND gedrückt).
inline bool entprellung_fallende_flanke(Entpreller& e, int pin) {
  bool aktuell = digitalRead(pin);
  if (aktuell != e.letzterPegel) {
    e.aenderungsZeit = millis();
    e.letzterPegel   = aktuell;
  }
  if ((millis() - e.aenderungsZeit) >= e.debounceMs) {
    if (aktuell != e.bestaetigt) {
      e.bestaetigt = aktuell;
      if (e.bestaetigt == LOW) return true;
    }
  }
  return false;
}
