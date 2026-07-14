#pragma once
#include "config.h"

// Alle GPIOs sind in v12 FEST in config.h definiert.
// pinRegen/pinWasser/pinPumpe/pinTaster sind keine Variablen mehr.
// Nur noch Pegel und Relais-Polarität sind einstellbar.

extern bool pegelRegenHigh;
extern bool pegelWasserHigh;
extern bool pumpeAktivHigh;

extern bool sensorRegenAktiv;
extern bool sensorWasserAktiv;

extern bool     automatikAn;
extern uint32_t betriebsSekGesamt;

extern Tagprogramm woche[7];

extern LogEintrag logBuf[LOG_MAX];
extern int        logAnzahl;
extern int        logNaechster;

void speicher_init();
void speicher_pegel_speichern();
void speicher_sensor_aktiv_speichern();
void speicher_pumpe_pol_speichern();
void speicher_timer_speichern();
void speicher_betrieb_speichern();
void speicher_automatik_speichern();
void speicher_zeit_speichern();
void speicher_alles_speichern();

void   log_eintrag(const char* text, uint32_t ts);
String log_als_json();
