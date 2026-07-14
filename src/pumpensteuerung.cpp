#include "pumpensteuerung.h"
#include "config.h"
#include "storage.h"
#include "zeitverwaltung.h"

bool pumpeTimerAn = false;
bool pumpeManAn   = false;
bool pumpeManAus  = false;
bool pumpeManZeitAktiv = false;
bool battKritisch  = false;

static unsigned long pumpeManStopMillis = 0;

static bool          letztesPumpeAn         = false;
static unsigned long letzterTick            = 0;
static unsigned long letzterBetriebAutosave = 0;

void pumpe_init() {
  letzterTick            = millis();
  letzterBetriebAutosave = millis();
  letztesPumpeAn         = false;
}

// Manuelles AUS (WebIF-Button oder Funk-Taste B) überschreibt auch einen
// gerade aktiven Timer — sonst könnte man die Pumpe während eines laufenden
// Bewässerungsfensters nicht wirklich abschalten. Das manuelle EIN hebt die
// Sperre wieder auf. Die Sperre selbst löst sich automatisch, sobald das
// Zeitfenster regulär endet (siehe pumpe_loop) — zukünftige Timer laufen
// danach ganz normal wieder an.
//
// battKritisch sperrt die Pumpe zusätzlich komplett (auch manuell/Funk),
// unabhängig von allem anderen — schützt die Batterie vor Tiefentladung.
// battSchutzAktiv ist der Notfall-Override im WebIF: ist der Schutz
// deaktiviert, wird battKritisch weiterhin angezeigt/geloggt, blockiert
// die Pumpe aber nicht mehr.
bool pumpe_laeuft() {
  bool battSperrt = battKritisch && battSchutzAktiv;
  return !battSperrt && !pumpeManAus && (pumpeTimerAn || pumpeManAn);
}

void pumpe_manuell(bool an) {
  pumpeManAn        = an;
  pumpeManAus       = !an;
  pumpeManZeitAktiv = false;   // ein regulärer Toggle beendet eine laufende Zeit-Bewässerung
}

// Startet eine einmalige, zeitlich begrenzte manuelle Bewässerung — läuft
// unabhängig vom Wochenprogramm für 'minuten' Minuten und schaltet sich
// danach von selbst wieder ab (siehe pumpe_betrieb_ticker).
void pumpe_manuell_zeit(uint16_t minuten) {
  if (minuten < 1)   minuten = 1;
  if (minuten > 360) minuten = 360;   // gleiche Obergrenze wie Wochenprogramm-Timer
  pumpeManAn        = true;
  pumpeManAus       = false;
  pumpeManZeitAktiv = true;
  pumpeManStopMillis = millis() + (unsigned long)minuten * 60000UL;
}

uint32_t pumpe_manuell_rest_sek() {
  if (!pumpeManZeitAktiv) return 0;
  long rest = (long)(pumpeManStopMillis - millis());
  return rest > 0 ? (uint32_t)(rest / 1000) : 0;
}

void pumpe_batterie_pruefen(float battV) {
  bool neu = (battV > BATT_KRITISCH_MIN && battV < BATT_KRITISCH_MAX);
  if (neu && !battKritisch) {
    log_eintrag(battSchutzAktiv ? "Pumpe gesperrt: Batterie kritisch"
                                : "Batterie kritisch (Schutz deaktiviert, Pumpe NICHT gesperrt)",
                zeit_als_unix());
  }
  if (!neu && battKritisch) log_eintrag("Batteriesperre aufgehoben", zeit_als_unix());
  battKritisch = neu;
}

void pumpe_schalten(bool an) {
  // Active LOW (Optokoppler): pumpeAktivHigh=false → LOW=EIN
  bool pegel = pumpeAktivHigh ? an : !an;
  digitalWrite(PIN_PUMPE, pegel ? HIGH : LOW);
}

void pumpe_loop(bool regenAktiv, bool wasserVorhanden) {
  if (!automatikAn || !zeitGesetzt) {
    pumpeTimerAn = false;
    pumpeManAus  = false;   // sauberer Neustart, falls Automatik wieder aktiviert wird
    pumpe_schalten(false);   // Relais sofort AUS — nicht auf nächsten loop() warten
    return;
  }
  struct tm jetzt  = zeit_aktuell();
  int wt           = jetzt.tm_wday;
  int jetztMin     = jetzt.tm_hour * 60 + jetzt.tm_min;
  bool soll        = false;

  // 1) Heutige Timer prüfen. Bei einem Überlauf-Timer (geht über Mitternacht)
  //    zählt hier nur der Teil VOR Mitternacht — der Teil danach wird unten
  //    unabhängig davon behandelt, ob heute überhaupt ein Timer läuft.
  for (int t = 0; t < TIMER_PRO_TAG && !soll; t++) {
    TagTimer& tim = woche[wt].timer[t];
    if (!tim.aktiv || !wasserVorhanden || regenAktiv) continue;

    int einMin = tim.einH * 60 + tim.einM;
    int ausMin = einMin + (int)tim.dauerMin;  // kann > 1439 sein (über Mitternacht)

    if (ausMin <= 1440) {
      // Normalfall: kein Überlauf über Mitternacht
      // Beispiel: 22:00 + 60min = 23:00 → jetztMin zwischen 22:00 und 23:00
      if (jetztMin >= einMin && jetztMin < ausMin) soll = true;
    } else {
      // Überlauf über Mitternacht — Teil VOR Mitternacht (z.B. 23:30–23:59)
      if (jetztMin >= einMin) soll = true;
    }
  }

  // 2) Rückblick auf GESTERN: lief da ein Überlauf-Timer, der bis in die
  //    frühen Morgenstunden von HEUTE reicht? Das muss unabhängig davon
  //    geprüft werden, ob heute selbst ein Timer konfiguriert ist — sonst
  //    würde ein Timer über Mitternacht immer exakt um 00:00 abgeschnitten,
  //    sobald der Folgetag keinen eigenen Überlauf-Timer hat.
  if (!soll) {
    int wtVortag = (wt + 6) % 7;   // Vortag (z.B. Dienstag wenn jetzt Mittwoch)
    for (int t2 = 0; t2 < TIMER_PRO_TAG && !soll; t2++) {
      TagTimer& tim2 = woche[wtVortag].timer[t2];
      if (!tim2.aktiv || !wasserVorhanden || regenAktiv) continue;
      int einMin2 = tim2.einH * 60 + tim2.einM;
      int ausMin2 = einMin2 + (int)tim2.dauerMin;
      // Beispiel: 23:30 + 60min → ausMin2 = 1470 → nach Mitternacht bis 00:30
      if (ausMin2 > 1440 && jetztMin < (ausMin2 - 1440)) soll = true;
    }
  }

  // Sobald kein Zeitfenster mehr aktiv ist, löst sich eine manuelle AUS-Sperre
  // automatisch wieder auf — der nächste planmäßige Timer ist davon unberührt.
  if (!soll) pumpeManAus = false;

  pumpeTimerAn = soll;
}

void pumpe_betrieb_ticker() {
  unsigned long jetzt = millis();
  if ((jetzt - letzterTick) < 1000) return;
  letzterTick = jetzt;

  // Zeitgesteuerte manuelle Bewässerung: nach Ablauf automatisch beenden.
  // Wichtig: hier NICHT pumpeManAus setzen — das würde einen zufällig
  // parallel aktiven Wochenplan-Timer blockieren, was nicht der Sinn ist.
  // Es soll nur die manuelle Komponente enden, der Timer bleibt unberührt.
  if (pumpeManZeitAktiv && (long)(jetzt - pumpeManStopMillis) >= 0) {
    pumpeManZeitAktiv = false;
    pumpeManAn        = false;
    log_eintrag("Manuelle Zeitbewaesserung beendet", zeit_als_unix());
  }

  bool laeuft = pumpe_laeuft();
  if (laeuft  && !letztesPumpeAn) log_eintrag("Pumpe EIN", zeit_als_unix());
  if (!laeuft &&  letztesPumpeAn) {
    char buf[48];
    snprintf(buf, sizeof(buf), "Pumpe AUS (%s)", pumpe_betriebszeit_string().c_str());
    log_eintrag(buf, zeit_als_unix());
    speicher_betrieb_speichern();
    letzterBetriebAutosave = jetzt;
  }
  if (laeuft) betriebsSekGesamt++;
  letztesPumpeAn = laeuft;

  // Autosave alle 5 Minuten während die Pumpe durchläuft — sonst geht bei
  // einem Stromausfall mitten im Lauf die bis dahin gezählte Zeit verloren.
  if (laeuft && (jetzt - letzterBetriebAutosave) >= 300000UL) {
    letzterBetriebAutosave = jetzt;
    speicher_betrieb_speichern();
  }
}

String pumpe_betriebszeit_string() {
  uint32_t h   = betriebsSekGesamt / 3600;
  uint32_t min = (betriebsSekGesamt % 3600) / 60;
  char buf[24];
  snprintf(buf, sizeof(buf), "%uh %02umin", h, min);
  return String(buf);
}
