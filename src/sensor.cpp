#include "sensor.h"
#include "config.h"
#include "storage.h"

// Nachlaufzeit nach Regenende: verhindert, dass ein kurzer Schauer die
// Bewässerung im Sekundentakt unterbricht/wieder freigibt. Nach dem letzten
// erkannten Regen gilt der Boden noch für diese Zeit als "nass".
#define REGEN_NACHLAUF_MS (15UL * 60UL * 1000UL)   // 15 Minuten
static unsigned long regenNachlaufBis = 0;

void sensor_init() {
  // Regen-Sensor
  pinMode(PIN_REGEN,  pegelRegenHigh  ? INPUT_PULLDOWN : INPUT_PULLUP);
  // Wasser-Sensor
  pinMode(PIN_WASSER, pegelWasserHigh ? INPUT_PULLDOWN : INPUT_PULLUP);
}

bool sensor_regen_aktiv() {
  if (!sensorRegenAktiv) { regenNachlaufBis = 0; return false; }   // deaktiviert → wirkt wie "kein Regen"
  bool p = digitalRead(PIN_REGEN);
  bool aktivRoh = pegelRegenHigh ? (p == HIGH) : (p == LOW);
  if (aktivRoh) {
    regenNachlaufBis = millis() + REGEN_NACHLAUF_MS;
    return true;
  }
  // Kein Regen mehr erkannt, aber Nachlaufzeit seit dem letzten Regen noch
  // nicht abgelaufen — überlaufsicher dank Vorzeichen-Trick (millis()-Wrap).
  return (long)(regenNachlaufBis - millis()) > 0;
}

bool sensor_wasser_vorhanden() {
  if (!sensorWasserAktiv) return true;   // deaktiviert → wirkt wie "Wasser vorhanden"
  bool p = digitalRead(PIN_WASSER);
  return pegelWasserHigh ? (p == HIGH) : (p == LOW);
}
