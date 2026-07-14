#include "licht_steuerung.h"
#include "config.h"
#include "storage.h"
#include "zeitverwaltung.h"

bool lichtAn = false;

static bool          lt_letzterPegel   = HIGH;
static bool          lt_bestaetigt     = HIGH;
static unsigned long lt_aenderungsZeit = 0;

void licht_init() {
  // Active LOW (Optokoppler): HIGH = AUS beim Start
  pinMode(PIN_LICHT, OUTPUT);
  digitalWrite(PIN_LICHT, HIGH);
  lichtAn = false;
  pinMode(PIN_LICHT_TASTER, INPUT_PULLUP);
  lt_letzterPegel   = digitalRead(PIN_LICHT_TASTER);
  lt_bestaetigt     = lt_letzterPegel;
  lt_aenderungsZeit = millis();
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
  bool aktuell = digitalRead(PIN_LICHT_TASTER);
  if (aktuell != lt_letzterPegel) {
    lt_aenderungsZeit = millis();
    lt_letzterPegel   = aktuell;
  }
  if ((millis() - lt_aenderungsZeit) >= 50) {
    if (aktuell != lt_bestaetigt) {
      lt_bestaetigt = aktuell;
      if (lt_bestaetigt == LOW) licht_toggle();
    }
  }
}
