#pragma once
#include <Arduino.h>

extern bool pumpeTimerAn;
extern bool pumpeManAn;
extern bool pumpeManAus;      // Manuelles AUS überschreibt einen aktiven Timer
extern bool pumpeManZeitAktiv; // true während einer zeitlich begrenzten manuellen Bewässerung
extern bool battKritisch;      // true = Batterie zu schwach, Pumpe komplett gesperrt

void   pumpe_init();
bool   pumpe_laeuft();
void   pumpe_manuell(bool an);
void   pumpe_manuell_zeit(uint16_t minuten);   // einmalig für X Minuten gießen, dann automatisch aus
uint32_t pumpe_manuell_rest_sek();             // Restzeit der zeitgesteuerten manuellen Bewässerung
void   pumpe_batterie_pruefen(float battV);    // Batteriesperre aktualisieren (aus main.cpp aufrufen)
void   pumpe_schalten(bool an);
void   pumpe_loop(bool regenAktiv, bool wasserVorhanden);
void   pumpe_betrieb_ticker();
String pumpe_betriebszeit_string();
