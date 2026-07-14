#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "storage.h"
#include "zeitverwaltung.h"
#include "sensor.h"
#include "taster.h"
#include "funk_empfaenger.h"
#include "led_steuerung.h"
#include "pumpensteuerung.h"
#include "licht_steuerung.h"
#include "webserver_routes.h"
#include "oled_anzeige.h"

bool  regenAktiv      = false;
bool  wasserVorhanden = false;
float battSpannung    = 0.0f;

static unsigned long letzteOledUpdate  = 0;
static unsigned long letzteZeitSicherung = 0;
#define ZEIT_AUTOSAVE_MS (5UL * 60UL * 1000UL)   // alle 5 Minuten

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== Bewässerungssteuerung v12 ===");
  Serial.println("    Alle GPIOs fest | OLED: Zeit+Batt+Auto");

  // 1. Flash laden
  speicher_init();

  // 2. LEDs + Selbsttest
  led_init();

  // 3. Sensoren (Pull je nach Pegel-Einstellung)
  sensor_init();

  // 4. Batterie-ADC
  analogSetAttenuation(ADC_11db);
  pinMode(PIN_BATT_ADC, INPUT);

  // 5. Pumpen-Relais Kanal 1
  pinMode(PIN_PUMPE, OUTPUT);
  pumpe_schalten(false);

  // 6. Licht-Relais Kanal 2 + Licht-Taster
  licht_init();

  // 7. Automatik-Taster
  taster_init();

  // 8. Funk-Empfänger
  funk_init();

  // 9. Pumpen-Ticker
  pumpe_init();

  // 10. OLED
  oled_init();

  Serial.printf("Sensoren  : Regen=GPIO%d (%s, %s) | Wasser=GPIO%d (%s, %s)\n",
    PIN_REGEN,  pegelRegenHigh  ? "PULLDOWN/aktiv=HIGH" : "PULLUP/aktiv=LOW",
    sensorRegenAktiv  ? "aktiviert" : "DEAKTIVIERT",
    PIN_WASSER, pegelWasserHigh ? "PULLDOWN/aktiv=HIGH" : "PULLUP/aktiv=LOW",
    sensorWasserAktiv ? "aktiviert" : "DEAKTIVIERT");
  Serial.printf("Pumpe     : GPIO%d | Pol=%s\n", PIN_PUMPE,
    pumpeAktivHigh ? "active HIGH" : "active LOW");
  Serial.printf("Licht     : Relais=GPIO%d | Taster=GPIO%d\n",
    PIN_LICHT, PIN_LICHT_TASTER);
  Serial.printf("AutoTaster: GPIO%d\n", PIN_TASTER);
  Serial.printf("Funk      : A=GPIO%d (EIN) | B=GPIO%d (AUS)\n",
    PIN_FUNK_A, PIN_FUNK_B);
  Serial.printf("LEDs      : Auto=GPIO%d | Wasser=GPIO%d | Regen=GPIO%d | Pumpe=GPIO%d\n",
    PIN_LED_AUTOMATIK, PIN_LED_WASSER, PIN_LED_REGEN, PIN_LED_PUMPE);
  Serial.printf("Automatik : %s\n", automatikAn ? "EIN" : "AUS");
  if (zeitGesetzt) {
    Serial.printf("Zeit      : %s %s\n", zeit_als_string().c_str(),
      zeitApproximiert ? "(aus Flash wiederhergestellt, naeherungsweise)" : "(gesetzt)");
  } else {
    Serial.println("Zeit      : nicht gesetzt");
  }
  if (urlaubsModusAktiv) {
    Serial.printf("Urlaub    : aktiv bis %s\n", unix_als_datum_string(urlaubsEndeUnix).c_str());
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.printf("AP        : SSID='%s'  PW='%s'  IP=%s\n",
    AP_SSID, AP_PASSWORD, AP_IP_STR);

  webserver_init();
  log_eintrag("System gestartet", zeit_als_unix());
  Serial.println("Bereit.\n");
}

void loop() {
  // 1. Sensoren lesen
  regenAktiv      = sensor_regen_aktiv();
  wasserVorhanden = sensor_wasser_vorhanden();

  // 2. Batteriespannung (Mittelwert)
  {
    long sum = 0;
    for (int i = 0; i < BATT_SAMPLES; i++) sum += analogRead(PIN_BATT_ADC);
    float v_adc  = (sum / (float)BATT_SAMPLES) * (BATT_ADC_REF / BATT_ADC_MAX);
    battSpannung = v_adc * BATT_TEILER;
  }
  pumpe_batterie_pruefen(battSpannung);

  // 3. Automatik-Taster
  taster_loop();

  // 4. Licht-Taster
  licht_taster_loop();

  // 5. Funk
  funk_loop();

  // 6. Timer-Logik
  pumpe_loop(regenAktiv, wasserVorhanden);

  // 7. Pumpe physisch schalten
  pumpe_schalten(pumpe_laeuft());

  // 8. LEDs
  led_update(automatikAn, wasserVorhanden, regenAktiv, pumpe_laeuft());

  // 9. Betriebsstunden
  pumpe_betrieb_ticker();

  // 10. OLED alle 1000ms
  unsigned long jetzt = millis();
  if (jetzt - letzteOledUpdate >= 1000) {
    letzteOledUpdate = jetzt;
    oled_update(battSpannung, automatikAn);
  }

  // 11. Webserver
  webserver_loop();

  // 12. Zeit alle 5 Minuten sichern (Absicherung gegen Stromausfall)
  if (jetzt - letzteZeitSicherung >= ZEIT_AUTOSAVE_MS) {
    letzteZeitSicherung = jetzt;
    speicher_zeit_speichern();
  }

  // 13. Urlaubsmodus: prüfen ob automatisch beendet werden soll
  urlaub_tick();

  delay(100);
}
