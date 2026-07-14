#include "funk_empfaenger.h"
#include "config.h"
#include "pumpensteuerung.h"

#define FUNK_DEBOUNCE_MS 80

// Die Hauptschleife läuft nur mit ca. 10Hz (delay(100) in main.cpp). Ein
// kurzer RF-Impuls des RX480E-4 könnte dabei zwischen zwei loop()-Durchläufen
// verlorengehen. Daher fangen wir die steigende Flanke per Hardware-Interrupt
// ab (verpasst nichts) und werten sie entprellt im normalen loop() aus. Die
// ISR selbst bleibt bewusst minimal (nur ein volatile Flag setzen).
static volatile bool funkAFlag = false;
static volatile bool funkBFlag = false;
static unsigned long sperrBisA = 0;
static unsigned long sperrBisB = 0;

void IRAM_ATTR funkA_isr() { funkAFlag = true; }
void IRAM_ATTR funkB_isr() { funkBFlag = true; }

void funk_init() {
  pinMode(PIN_FUNK_A, INPUT_PULLDOWN);
  pinMode(PIN_FUNK_B, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(PIN_FUNK_A), funkA_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_FUNK_B), funkB_isr, RISING);
  sperrBisA = sperrBisB = 0;
}

// Prüft ob ein Flag gesetzt ist und wendet die Zeit-Entprellung an.
// Mehrfaches Prellen vor der nächsten Auswertung führt nur zu einem Event,
// da das Flag ein einzelnes bool ist (kein Zähler).
static bool ausgeloest(volatile bool& flag, unsigned long& sperrBis) {
  if (!flag) return false;
  flag = false;
  unsigned long jetzt = millis();
  if ((jetzt - sperrBis) < (unsigned long)FUNK_DEBOUNCE_MS) return false;
  sperrBis = jetzt;
  return true;
}

void funk_loop() {
  if (ausgeloest(funkAFlag, sperrBisA)) pumpe_manuell(true);
  if (ausgeloest(funkBFlag, sperrBisB)) pumpe_manuell(false);
}
