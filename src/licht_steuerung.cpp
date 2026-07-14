#include "licht_steuerung.h"
#include "config.h"
#include "storage.h"
#include "zeitverwaltung.h"
#include "entprellung.h"

#define LICHT_TASTER_DEBOUNCE_MS 50

bool lichtAn = false;

static Entpreller lichtTaster;

static void IRAM_ATTR licht_taster_isr() { lichtTaster.flag = true; }

void licht_init() {
  // Active LOW (Optokoppler): HIGH = AUS beim Start
  pinMode(PIN_LICHT, OUTPUT);
  digitalWrite(PIN_LICHT, HIGH);
  lichtAn = false;
  pinMode(PIN_LICHT_TASTER, INPUT_PULLUP);
  entprellung_init(lichtTaster);
  attachInterrupt(digitalPinToInterrupt(PIN_LICHT_TASTER), licht_taster_isr, FALLING);
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
  if (entprellung_ausgeloest(lichtTaster, LICHT_TASTER_DEBOUNCE_MS)) licht_toggle();
}
