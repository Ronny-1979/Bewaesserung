#pragma once
#include <Arduino.h>
extern bool lichtAn;
void licht_init();
void licht_taster_loop();
void licht_schalten(bool an);
void licht_toggle();
