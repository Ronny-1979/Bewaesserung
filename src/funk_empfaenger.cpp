#include "funk_empfaenger.h"
#include "config.h"
#include "pumpensteuerung.h"

#define FUNK_DEBOUNCE_MS 80

struct FunkKanal { int pin; bool letzterPegel; unsigned long sperrBis; };
static FunkKanal kanalA = { PIN_FUNK_A, LOW, 0 };
static FunkKanal kanalB = { PIN_FUNK_B, LOW, 0 };

void funk_init() {
  pinMode(PIN_FUNK_A, INPUT_PULLDOWN);
  pinMode(PIN_FUNK_B, INPUT_PULLDOWN);
  kanalA.letzterPegel = digitalRead(PIN_FUNK_A);
  kanalB.letzterPegel = digitalRead(PIN_FUNK_B);
  kanalA.sperrBis = kanalB.sperrBis = 0;
}

static bool steigende_flanke(FunkKanal& k) {
  bool aktuell = digitalRead(k.pin);
  if ((millis() - k.sperrBis) < (unsigned long)FUNK_DEBOUNCE_MS) {
    k.letzterPegel = aktuell; return false;
  }
  if (aktuell == HIGH && k.letzterPegel == LOW) {
    k.letzterPegel = HIGH; k.sperrBis = millis(); return true;
  }
  k.letzterPegel = aktuell; return false;
}

void funk_loop() {
  if (steigende_flanke(kanalA)) pumpe_manuell(true);
  if (steigende_flanke(kanalB)) pumpe_manuell(false);
}
