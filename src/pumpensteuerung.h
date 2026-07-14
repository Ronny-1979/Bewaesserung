#pragma once
#include <Arduino.h>

extern bool pumpeTimerAn;
extern bool pumpeManAn;
extern bool pumpeManAus;   // Manuelles AUS überschreibt einen aktiven Timer

void   pumpe_init();
bool   pumpe_laeuft();
void   pumpe_manuell(bool an);
void   pumpe_schalten(bool an);
void   pumpe_loop(bool regenAktiv, bool wasserVorhanden);
void   pumpe_betrieb_ticker();
String pumpe_betriebszeit_string();
