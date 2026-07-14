#include "pumpensteuerung.h"
#include "config.h"
#include "storage.h"
#include "zeitverwaltung.h"

bool pumpeTimerAn = false;
bool pumpeManAn   = false;

static bool          letztesPumpeAn = false;
static unsigned long letzterTick    = 0;

void pumpe_init() {
  letzterTick    = millis();
  letztesPumpeAn = false;
}

bool pumpe_laeuft()       { return pumpeTimerAn || pumpeManAn; }
void pumpe_manuell(bool an) { pumpeManAn = an; }

void pumpe_schalten(bool an) {
  // Active LOW (Optokoppler): pumpeAktivHigh=false → LOW=EIN
  bool pegel = pumpeAktivHigh ? an : !an;
  digitalWrite(PIN_PUMPE, pegel ? HIGH : LOW);
}

void pumpe_loop(bool regenAktiv, bool wasserVorhanden) {
  if (!automatikAn || !zeitGesetzt) {
    pumpeTimerAn = false;
    pumpe_schalten(false);   // Relais sofort AUS — nicht auf nächsten loop() warten
    return;
  }
  struct tm jetzt  = zeit_aktuell();
  int wt           = jetzt.tm_wday;
  int jetztMin     = jetzt.tm_hour * 60 + jetzt.tm_min;
  bool soll        = false;

  for (int t = 0; t < TIMER_PRO_TAG; t++) {
    TagTimer& tim = woche[wt].timer[t];
    if (!tim.aktiv || !wasserVorhanden || regenAktiv) continue;

    int einMin = tim.einH * 60 + tim.einM;
    int ausMin = einMin + (int)tim.dauerMin;  // kann > 1439 sein (über Mitternacht)

    if (ausMin <= 1440) {
      // ── Normalfall: kein Überlauf über Mitternacht ────────────
      // Beispiel: 22:00 + 60min = 23:00 → jetztMin zwischen 22:00 und 23:00
      if (jetztMin >= einMin && jetztMin < ausMin) { soll = true; break; }
    } else {
      // ── Überlauf über Mitternacht ─────────────────────────────
      // Beispiel: 23:30 + 60min → ausMin = 1470 → nach Mitternacht 1470-1440 = 30min = 00:30
      // Phase 1: Vor Mitternacht (jetztMin >= 23:30)
      if (jetztMin >= einMin) { soll = true; break; }
      // Phase 2: Nach Mitternacht am Folgetag (jetztMin < 00:30)
      // Wochentag ist bereits weitergeschaltet → Vortag-Timer prüfen
      int wtVortag    = (wt + 6) % 7;   // Vortag (z.B. Dienstag wenn jetzt Mittwoch)
      int ausMinRest  = ausMin - 1440;  // Minuten nach Mitternacht
      for (int t2 = 0; t2 < TIMER_PRO_TAG; t2++) {
        TagTimer& tim2 = woche[wtVortag].timer[t2];
        if (!tim2.aktiv || !wasserVorhanden || regenAktiv) continue;
        int einMin2 = tim2.einH * 60 + tim2.einM;
        int ausMin2 = einMin2 + (int)tim2.dauerMin;
        if (ausMin2 > 1440 && jetztMin < (ausMin2 - 1440)) { soll = true; break; }
      }
      if (soll) break;
    }
  }
  pumpeTimerAn = soll;
}

void pumpe_betrieb_ticker() {
  unsigned long jetzt = millis();
  if ((jetzt - letzterTick) < 1000) return;
  letzterTick = jetzt;
  bool laeuft = pumpe_laeuft();
  if (laeuft  && !letztesPumpeAn) log_eintrag("Pumpe EIN", zeit_als_unix());
  if (!laeuft &&  letztesPumpeAn) {
    char buf[48];
    snprintf(buf, sizeof(buf), "Pumpe AUS (%s)", pumpe_betriebszeit_string().c_str());
    log_eintrag(buf, zeit_als_unix());
    speicher_betrieb_speichern();
  }
  if (laeuft) betriebsSekGesamt++;
  letztesPumpeAn = laeuft;
}

String pumpe_betriebszeit_string() {
  uint32_t h   = betriebsSekGesamt / 3600;
  uint32_t min = (betriebsSekGesamt % 3600) / 60;
  char buf[24];
  snprintf(buf, sizeof(buf), "%uh %02umin", h, min);
  return String(buf);
}
