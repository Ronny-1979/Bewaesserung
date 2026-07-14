#include "storage.h"
#include <Preferences.h>
#include "zeitverwaltung.h"

static Preferences prefs;

bool pegelRegenHigh    = DEFAULT_PEGEL_REGEN_HIGH;
bool pegelWasserHigh   = DEFAULT_PEGEL_WASSER_HIGH;
bool pumpeAktivHigh    = DEFAULT_PUMPE_AKTIV_HIGH;
bool sensorRegenAktiv  = DEFAULT_SENSOR_REGEN_AKTIV;
bool sensorWasserAktiv = DEFAULT_SENSOR_WASSER_AKTIV;
bool automatikAn       = true;
uint32_t betriebsSekGesamt = 0;

Tagprogramm woche[7];

LogEintrag logBuf[LOG_MAX];
int        logAnzahl    = 0;
int        logNaechster = 0;

void speicher_init() {
  prefs.begin(NVS_NAMESPACE, false);
  pegelRegenHigh    = prefs.getBool("prH", DEFAULT_PEGEL_REGEN_HIGH);
  pegelWasserHigh   = prefs.getBool("pwH", DEFAULT_PEGEL_WASSER_HIGH);
  pumpeAktivHigh    = prefs.getBool("paH", DEFAULT_PUMPE_AKTIV_HIGH);
  sensorRegenAktiv  = prefs.getBool("srA", DEFAULT_SENSOR_REGEN_AKTIV);
  sensorWasserAktiv = prefs.getBool("swA", DEFAULT_SENSOR_WASSER_AKTIV);
  automatikAn       = prefs.getBool("aut", true);
  betriebsSekGesamt = prefs.getUInt("bSek", 0);

  for (int d = 0; d < 7; d++) {
    for (int t = 0; t < TIMER_PRO_TAG; t++) {
      char k[8];
      snprintf(k, sizeof(k), "t%d%d", d, t);
      uint32_t v = prefs.getUInt(k, 0);
      woche[d].timer[t].aktiv    = (v >> 21) & 0x01;
      woche[d].timer[t].einH     = (v >> 16) & 0x1F;
      woche[d].timer[t].einM     = (v >> 10) & 0x3F;
      woche[d].timer[t].dauerMin =  v        & 0x3FF;
    }
  }

  // Zuletzt bekannte Zeit wiederherstellen (nur Näherung, siehe zeit_setzen_unix).
  // So läuft die Wochenprogramm-Automatik nach einem Reboot/Stromausfall sofort
  // weiter, statt komplett zu pausieren bis die Uhr manuell neu gesetzt wird.
  uint32_t letzteZeit = prefs.getUInt("zEp", 0);
  if (letzteZeit > 0) {
    zeit_setzen_unix(letzteZeit);
    log_eintrag("Zeit aus Flash wiederhergestellt (naeherungsweise)", letzteZeit);
  }
}

void speicher_pegel_speichern() {
  prefs.putBool("prH", pegelRegenHigh);
  prefs.putBool("pwH", pegelWasserHigh);
}

void speicher_sensor_aktiv_speichern() {
  prefs.putBool("srA", sensorRegenAktiv);
  prefs.putBool("swA", sensorWasserAktiv);
}

void speicher_pumpe_pol_speichern() {
  prefs.putBool("paH", pumpeAktivHigh);
}

void speicher_timer_speichern() {
  for (int d = 0; d < 7; d++) {
    for (int t = 0; t < TIMER_PRO_TAG; t++) {
      char k[8];
      snprintf(k, sizeof(k), "t%d%d", d, t);
      uint32_t v =
        ((uint32_t)(woche[d].timer[t].aktiv    ? 1:0)  << 21) |
        ((uint32_t)(woche[d].timer[t].einH     & 0x1F) << 16) |
        ((uint32_t)(woche[d].timer[t].einM     & 0x3F) << 10) |
        ((uint32_t)(woche[d].timer[t].dauerMin & 0x3FF));
      prefs.putUInt(k, v);
    }
  }
}

void speicher_betrieb_speichern()   { prefs.putUInt("bSek", betriebsSekGesamt); }
void speicher_automatik_speichern() { prefs.putBool("aut",  automatikAn); }

// Speichert die aktuelle Zeit als "letzten bekannten Stand" in den Flash —
// wird periodisch aus main.cpp aufgerufen sowie sofort nach jedem manuellen
// Setzen der Uhr über's WebIF (siehe handleZeit in webserver_routes.cpp).
void speicher_zeit_speichern() {
  if (zeitGesetzt) prefs.putUInt("zEp", zeit_als_unix());
}

void speicher_alles_speichern() {
  speicher_pegel_speichern();
  speicher_sensor_aktiv_speichern();
  speicher_pumpe_pol_speichern();
  speicher_timer_speichern();
  speicher_betrieb_speichern();
  speicher_automatik_speichern();
}

void log_eintrag(const char* text, uint32_t ts) {
  logBuf[logNaechster].timestamp = ts;
  strncpy(logBuf[logNaechster].text, text, sizeof(logBuf[0].text) - 1);
  logBuf[logNaechster].text[sizeof(logBuf[0].text) - 1] = '\0';
  logNaechster = (logNaechster + 1) % LOG_MAX;
  if (logAnzahl < LOG_MAX) logAnzahl++;
}

String log_als_json() {
  String s = "[";
  for (int i = 0; i < logAnzahl; i++) {
    int idx = ((logNaechster - 1 - i) + LOG_MAX) % LOG_MAX;
    if (i > 0) s += ",";
    s += "{\"ts\":" + String(logBuf[idx].timestamp) + ",\"txt\":\"";
    for (const char* p = logBuf[idx].text; *p; p++) {
      unsigned char c = (unsigned char)*p;
      if      (c == '"')  s += "\\\"";
      else if (c == '\\') s += "\\\\";
      else if (c >= 0x20 && c <= 0x7E) s += (char)c;
      else s += '?';
    }
    s += "\"}";
  }
  return s + "]";
}
