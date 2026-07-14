#pragma once
#include <Arduino.h>
#include <time.h>

extern bool          zeitGesetzt;
extern struct tm     manuelleZeit;
extern unsigned long zeitBasisMillis;

void      zeit_setzen(int tag, int mon, int jahr, int std, int min, int sek);
struct tm zeit_aktuell();
uint32_t  zeit_als_unix();
String    zeit_als_string();
int       zeit_wochentag();
