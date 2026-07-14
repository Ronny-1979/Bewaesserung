#pragma once
#include <Arduino.h>
void led_init();
void led_update(bool automatikAn, bool wasserVorhanden, bool regenAktiv, bool pumpeLaeuft);
