#include "zeitverwaltung.h"

bool          zeitGesetzt     = false;
struct tm     manuelleZeit    = {0};
unsigned long zeitBasisMillis = 0;
static time_t epochBasis      = 0;   // einmal berechnet, gecacht

void zeit_setzen(int tag, int mon, int jahr, int std, int min, int sek) {
  manuelleZeit          = {0};
  manuelleZeit.tm_mday  = tag;
  manuelleZeit.tm_mon   = mon - 1;
  manuelleZeit.tm_year  = jahr - 1900;
  manuelleZeit.tm_hour  = std;
  manuelleZeit.tm_min   = min;
  manuelleZeit.tm_sec   = sek;
  manuelleZeit.tm_isdst = -1;
  epochBasis      = mktime(&manuelleZeit);   // einmal berechnen + cachen
  zeitBasisMillis = millis();
  zeitGesetzt     = true;
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
