#include "taster.h"
#include "config.h"
#include "storage.h"
#include "zeitverwaltung.h"
#include "entprellung.h"

#define TASTER_DEBOUNCE_MS 50

static Entpreller taster;

static void IRAM_ATTR taster_isr() { taster.flag = true; }

void taster_init() {
  pinMode(PIN_TASTER, INPUT_PULLUP);
  entprellung_init(taster);
  attachInterrupt(digitalPinToInterrupt(PIN_TASTER), taster_isr, FALLING);
}

void taster_loop() {
  if (entprellung_ausgeloest(taster, TASTER_DEBOUNCE_MS)) {
    bool neu = !automatikAn;
    automatik_setzen(neu);
    log_eintrag(neu ? "Automatik EIN (Taster)"
                    : "Automatik AUS (Taster)", zeit_als_unix());
  }
}
