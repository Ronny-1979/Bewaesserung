#include "sensor.h"
#include "config.h"
#include "storage.h"

void sensor_init() {
  // Regen-Sensor
  pinMode(PIN_REGEN,  pegelRegenHigh  ? INPUT_PULLDOWN : INPUT_PULLUP);
  // Wasser-Sensor
  pinMode(PIN_WASSER, pegelWasserHigh ? INPUT_PULLDOWN : INPUT_PULLUP);
}

bool sensor_regen_aktiv() {
  bool p = digitalRead(PIN_REGEN);
  return pegelRegenHigh ? (p == HIGH) : (p == LOW);
}

bool sensor_wasser_vorhanden() {
  bool p = digitalRead(PIN_WASSER);
  return pegelWasserHigh ? (p == HIGH) : (p == LOW);
}
