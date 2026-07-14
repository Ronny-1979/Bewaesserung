#include "led_steuerung.h"
#include "config.h"
#include "storage.h"

void led_init() {
  pinMode(PIN_LED_AUTOMATIK, OUTPUT);
  pinMode(PIN_LED_WASSER,    OUTPUT);
  pinMode(PIN_LED_REGEN,     OUTPUT);
  pinMode(PIN_LED_PUMPE,     OUTPUT);
  // Selbsttest: alle LEDs 600ms EIN
  digitalWrite(PIN_LED_AUTOMATIK, HIGH);
  digitalWrite(PIN_LED_WASSER,    HIGH);
  digitalWrite(PIN_LED_REGEN,     HIGH);
  digitalWrite(PIN_LED_PUMPE,     HIGH);
  delay(600);
  digitalWrite(PIN_LED_AUTOMATIK, LOW);
  digitalWrite(PIN_LED_WASSER,    LOW);
  digitalWrite(PIN_LED_REGEN,     LOW);
  digitalWrite(PIN_LED_PUMPE,     LOW);
}

void led_update(bool automatikAn, bool wasserVorhanden,
                bool regenAktiv,  bool pumpeLaeuft) {
  digitalWrite(PIN_LED_AUTOMATIK, automatikAn ? HIGH : LOW);

  // Regen-/Wasser-LED: normaler Zustand, aber langsames Blinken (1Hz) wenn
  // der jeweilige Sensor deaktiviert ist — sonst ist am Gerät selbst (ohne
  // WebIF) nicht erkennbar, dass "kein Regen"/"Wasser voll" nur simuliert ist.
  bool blinkPhase = (millis() / 500) % 2 == 0;
  digitalWrite(PIN_LED_REGEN,
    sensorRegenAktiv  ? (regenAktiv      ? HIGH : LOW) : (blinkPhase ? HIGH : LOW));
  digitalWrite(PIN_LED_WASSER,
    sensorWasserAktiv ? (wasserVorhanden ? HIGH : LOW) : (blinkPhase ? HIGH : LOW));

  digitalWrite(PIN_LED_PUMPE, pumpeLaeuft ? HIGH : LOW);
}
