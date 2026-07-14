#include "funk_empfaenger.h"
#include "config.h"
#include "pumpensteuerung.h"
#include "entprellung.h"

#define FUNK_DEBOUNCE_MS 80

static Entpreller funkA;
static Entpreller funkB;

void IRAM_ATTR funkA_isr() { funkA.flag = true; }
void IRAM_ATTR funkB_isr() { funkB.flag = true; }

void funk_init() {
  pinMode(PIN_FUNK_A, INPUT_PULLDOWN);
  pinMode(PIN_FUNK_B, INPUT_PULLDOWN);
  entprellung_init(funkA);
  entprellung_init(funkB);
  attachInterrupt(digitalPinToInterrupt(PIN_FUNK_A), funkA_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_FUNK_B), funkB_isr, RISING);
}

void funk_loop() {
  if (entprellung_ausgeloest(funkA, FUNK_DEBOUNCE_MS)) pumpe_manuell(true);
  if (entprellung_ausgeloest(funkB, FUNK_DEBOUNCE_MS)) pumpe_manuell(false);
}
