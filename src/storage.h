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
extern bool battSchutzAktiv;   // true = Pumpe wird bei kritischer Batterie gesperrt (Default)

extern bool     automatikAn;
extern uint32_t betriebsSekGesamt;

// Urlaubsmodus: pausiert die Automatik für eine feste Anzahl Tage und
// aktiviert sie danach automatisch wieder (siehe urlaub_tick()).
extern bool     urlaubsModusAktiv;
extern uint32_t urlaubsEndeUnix;   // 0 = kein Enddatum bekannt (Zeit war nicht gesetzt)

extern Tagprogramm woche[7];

extern LogEintrag logBuf[LOG_MAX];
extern int        logAnzahl;
extern int        logNaechster;

void speicher_init();
void speicher_pegel_speichern();
void speicher_sensor_aktiv_speichern();
void speicher_batt_schutz_speichern();
void speicher_pumpe_pol_speichern();
void speicher_timer_speichern();
void speicher_betrieb_speichern();
void speicher_automatik_speichern();
void speicher_urlaub_speichern();
void speicher_zeit_speichern();
void speicher_alles_speichern();

// Zentrale Stelle für JEDEN manuellen Eingriff in die Automatik (WebIF-Button
// oder physischer Taster). Beendet dabei automatisch einen laufenden
// Urlaubsmodus, damit der nicht unbemerkt im Hintergrund weiterläuft, nachdem
// jemand die Automatik von Hand umgeschaltet hat.
void automatik_setzen(bool an);

// Urlaubsmodus: Automatik für 'tage' Tage pausieren, danach automatisch
// wieder aktivieren. urlaub_tick() muss regelmäßig aus der Hauptschleife
// aufgerufen werden, um das automatische Ende zu erkennen.
void urlaub_starten(uint16_t tage);
void urlaub_beenden();
void urlaub_tick();

void   log_eintrag(const char* text, uint32_t ts);
String log_als_json();
