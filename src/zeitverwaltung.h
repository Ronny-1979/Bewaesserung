#pragma once
#include <Arduino.h>
#include <time.h>

extern bool          zeitGesetzt;
extern bool          zeitApproximiert;   // true = Zeit stammt aus Flash-Wiederherstellung, nicht vom Nutzer gesetzt

void      zeit_setzen(int tag, int mon, int jahr, int std, int min, int sek);
void      zeit_setzen_unix(uint32_t epoch);   // Wiederherstellung nach Neustart (naeherungsweise)
void      zeit_tick();                        // regelmäßig aus loop() aufrufen (millis-Überlauf-Schutz)
struct tm zeit_aktuell();
uint32_t  zeit_als_unix();
String    zeit_als_string();
int       zeit_wochentag();
String    unix_als_datum_string(uint32_t epoch);   // z.B. für Urlaubsmodus-Enddatum
