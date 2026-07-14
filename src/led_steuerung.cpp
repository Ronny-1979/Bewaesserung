#include "led_steuerung.h"
#include "config.h"

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
  digitalWrite(PIN_LED_AUTOMATIK, automatikAn     ? HIGH : LOW);
  digitalWrite(PIN_LED_WASSER,    wasserVorhanden ? HIGH : LOW);
  digitalWrite(PIN_LED_REGEN,     regenAktiv      ? HIGH : LOW);
  digitalWrite(PIN_LED_PUMPE,     pumpeLaeuft     ? HIGH : LOW);
}
