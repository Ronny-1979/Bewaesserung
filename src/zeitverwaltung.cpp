#include "zeitverwaltung.h"

bool          zeitGesetzt      = false;
bool          zeitApproximiert = false;
static struct tm     manuelleZeit    = {0};   // nur intern als mktime()-Zwischenspeicher gebraucht
static unsigned long zeitBasisMillis = 0;
static time_t epochBasis       = 0;   // einmal berechnet, gecacht

void zeit_setzen(int tag, int mon, int jahr, int std, int min, int sek) {
  manuelleZeit          = {0};
  manuelleZeit.tm_mday  = tag;
  manuelleZeit.tm_mon   = mon - 1;
  manuelleZeit.tm_year  = jahr - 1900;
  manuelleZeit.tm_hour  = std;
  manuelleZeit.tm_min   = min;
  manuelleZeit.tm_sec   = sek;
  manuelleZeit.tm_isdst = -1;
  epochBasis       = mktime(&manuelleZeit);   // einmal berechnen + cachen
  zeitBasisMillis  = millis();
  zeitGesetzt      = true;
  zeitApproximiert = false;   // vom Nutzer bewusst gesetzt → verlässlich
}

// Wird beim Boot verwendet, um die zuletzt bekannte Zeit aus dem Flash
// wiederherzustellen. Das ist nur eine Annäherung (die Zeit stand seit dem
// letzten Speichern still), aber besser als gar keine Zeit — die
// Wochenprogramm-Automatik kann so sofort weiterlaufen statt komplett zu
// pausieren, bis jemand die Uhr manuell neu stellt.
void zeit_setzen_unix(uint32_t epoch) {
  epochBasis       = (time_t)epoch;
  zeitBasisMillis  = millis();
  zeitGesetzt      = true;
  zeitApproximiert = true;
}

// millis()-Überlauf-Schutz: Die Zeitberechnung (millis() - zeitBasisMillis)
// ist nur korrekt, solange seit dem letzten Setzen der Basis weniger als
// ~49,7 Tage (2^32 ms) vergangen sind. Ohne diese Funktion würde die Uhr
// bei längerem Dauerbetrieb schlagartig ~49,7 Tage zurückspringen — und die
// periodische Zeitsicherung würde die falsche Zeit sogar in den Flash
// zementieren. Daher wird die Basis einmal täglich "nachgezogen": die volle
// Sekundenzahl wandert in epochBasis, der Sub-Sekunden-Rest bleibt in
// zeitBasisMillis erhalten, damit keine Millisekunden verloren gehen.
void zeit_tick() {
  if (!zeitGesetzt) return;
  unsigned long verg = millis() - zeitBasisMillis;
  if (verg >= 86400000UL) {   // einmal am Tag
    unsigned long ganzeSek = verg / 1000;
    epochBasis      += (time_t)ganzeSek;
    zeitBasisMillis += ganzeSek * 1000UL;   // Rest-Millisekunden erhalten
  }
}

struct tm zeit_aktuell() {
  if (!zeitGesetzt) { struct tm l = {0}; return l; }
  uint32_t  vergSek = (millis() - zeitBasisMillis) / 1000;
  time_t    ep      = epochBasis + (time_t)vergSek;
  struct tm erg;
  localtime_r(&ep, &erg);
  return erg;
}

uint32_t zeit_als_unix() {
  if (!zeitGesetzt) return 0;
  // gecachte Basis verwenden — kein mktime() mehr nötig
  return (uint32_t)epochBasis +
         (uint32_t)((millis() - zeitBasisMillis) / 1000);
}

String zeit_als_string() {
  if (!zeitGesetzt) return "nicht gesetzt";
  struct tm t = zeit_aktuell();
  char buf[32];
  snprintf(buf, sizeof(buf), "%02d.%02d.%04d %02d:%02d:%02d",
    t.tm_mday, t.tm_mon+1, t.tm_year+1900,
    t.tm_hour, t.tm_min,   t.tm_sec);
  return String(buf);
}

int zeit_wochentag() { return zeit_aktuell().tm_wday; }

// Formatiert einen beliebigen Unix-Zeitstempel als TT.MM.JJJJ — unabhängig
// von der laufenden Uhr, z.B. für das Urlaubsmodus-Enddatum.
String unix_als_datum_string(uint32_t epoch) {
  if (epoch == 0) return "-";
  time_t ep = (time_t)epoch;
  struct tm t;
  localtime_r(&ep, &t);
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d.%02d.%04d", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
  return String(buf);
}
