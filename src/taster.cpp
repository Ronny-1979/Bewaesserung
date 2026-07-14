#include "taster.h"
#include "config.h"
#include "storage.h"
#include "zeitverwaltung.h"
#include "entprellung.h"

static Entpreller taster;

void taster_init() {
  pinMode(PIN_TASTER, INPUT_PULLUP);
  entprellung_init(taster, PIN_TASTER, 50);
}

void taster_loop() {
  if (entprellung_fallende_flanke(taster, PIN_TASTER)) {
    bool neu = !automatikAn;
    automatik_setzen(neu);
    log_eintrag(neu ? "Automatik EIN (Taster)"
                    : "Automatik AUS (Taster)", zeit_als_unix());
  }
}
