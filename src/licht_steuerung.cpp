#include "licht_steuerung.h"
#include "config.h"
#include "storage.h"
#include "zeitverwaltung.h"
#include "entprellung.h"

bool lichtAn = false;

static Entpreller lichtTaster;

void licht_init() {
  // Active LOW (Optokoppler): HIGH = AUS beim Start
  pinMode(PIN_LICHT, OUTPUT);
  digitalWrite(PIN_LICHT, HIGH);
  lichtAn = false;
  pinMode(PIN_LICHT_TASTER, INPUT_PULLUP);
  entprellung_init(lichtTaster, PIN_LICHT_TASTER, 50);
  // Hinweis: Pin-Info wird zentral einmal in main.cpp::setup() geloggt
}

void licht_schalten(bool an) {
  lichtAn = an;
  // Active LOW: an=true → LOW → Relais EIN → Licht an
  digitalWrite(PIN_LICHT, an ? LOW : HIGH);
  log_eintrag(an ? "Licht EIN" : "Licht AUS", zeit_als_unix());
}

void licht_toggle() { licht_schalten(!lichtAn); }

void licht_taster_loop() {
  if (entprellung_fallende_flanke(lichtTaster, PIN_LICHT_TASTER)) licht_toggle();
}
