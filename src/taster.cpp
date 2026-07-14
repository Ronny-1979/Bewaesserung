#include "taster.h"
#include "config.h"
#include "storage.h"
#include "zeitverwaltung.h"

#define DEBOUNCE_MS 50

static bool          letzterPegel   = HIGH;
static bool          bestaetigt     = HIGH;
static unsigned long aenderungsZeit = 0;

void taster_init() {
  pinMode(PIN_TASTER, INPUT_PULLUP);
  letzterPegel   = digitalRead(PIN_TASTER);
  bestaetigt     = letzterPegel;
  aenderungsZeit = millis();
}

void taster_loop() {
  bool aktuell = digitalRead(PIN_TASTER);
  if (aktuell != letzterPegel) {
    aenderungsZeit = millis();
    letzterPegel   = aktuell;
  }
  if ((millis() - aenderungsZeit) >= DEBOUNCE_MS) {
    if (aktuell != bestaetigt) {
      bestaetigt = aktuell;
      if (bestaetigt == LOW) {
        automatikAn = !automatikAn;
        speicher_automatik_speichern();
        log_eintrag(automatikAn ? "Automatik EIN (Taster)"
                                : "Automatik AUS (Taster)", zeit_als_unix());
      }
    }
  }
}
