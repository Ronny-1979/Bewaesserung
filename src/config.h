#pragma once
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════
//  KONFIGURATION  –  Bewässerungssteuerung v12
//
//  v9:  Batteriespannung via ADC (GPIO34 + Spannungsteiler)
//  v10: OLED SSD1306 I2C
//  v11: 2-Kanal Relaismodul + Licht-Taster
//  v12: Alle GPIOs FEST (kein WebIF-Umschalten der Pins mehr)
//       OLED zeigt nur: Uhrzeit | Batteriespannung | Automatik EIN/AUS
//       Pegel HIGH/LOW und Relais-Polarität bleiben im WebIF einstellbar
//
//  RELAISMODUL JQC-3FF-S-Z mit Optokoppler PC817 → ACTIVE LOW:
//    GPIO LOW  = Relais zieht an = Last EIN
//    GPIO HIGH = Relais fällt ab = Last AUS
//    DEFAULT_PUMPE_AKTIV_HIGH = false  (active LOW)
//
//  ANSCHLUSS RELAISMODUL:
//    JD-VCC → 5V   VCC → 5V   GND → GND
//    IN1    → GPIO26 (Pumpe)
//    IN2    → GPIO27 (Beleuchtung)
// ═══════════════════════════════════════════════════════════════

// ── WiFi ───────────────────────────────────────────────────────
#define AP_SSID     "Bewaesserung"
#define AP_PASSWORD "12345678"
#define AP_IP_STR   "192.168.4.1"

// ── Alle GPIOs FEST ────────────────────────────────────────────
#define PIN_REGEN    4    // Regensensor Signal
#define PIN_WASSER   16   // Wasserstandssensor Signal
#define PIN_PUMPE    26   // Relais Kanal 1 (IN1)
#define PIN_TASTER   25   // Automatik-Taster
#define PIN_LICHT    27   // Relais Kanal 2 (IN2)
#define PIN_LICHT_TASTER 14   // Externer Licht-Taster

// ── Funk-Empfänger RX480E-4 ────────────────────────────────────
#define PIN_FUNK_A   32   // Taste A → Pumpe EIN
#define PIN_FUNK_B   33   // Taste B → Pumpe AUS

// ── Status-LEDs ────────────────────────────────────────────────
// GPIO → 330Ω → LED(+Anode) → LED(–Kathode) → GND
#define PIN_LED_AUTOMATIK  18   // Blau  – Automatik EIN/AUS
#define PIN_LED_WASSER     19   // Grün  – Wasser vorhanden
#define PIN_LED_REGEN      21   // Gelb  – Regen erkannt
#define PIN_LED_PUMPE      22   // Rot   – Pumpe läuft

// ── Batterie-ADC ───────────────────────────────────────────────
// Spannungsteiler: Batt(+) → R1(100kΩ) → GPIO34 → R2(33kΩ) → GND
#define PIN_BATT_ADC   34
#define BATT_TEILER    4.0303f   // (100+33)/33
#define BATT_ADC_REF   3.3f
#define BATT_ADC_MAX   4095.0f
#define BATT_SAMPLES   20

// ── Batterie-Schutz ──────────────────────────────────────────────
// Liegt die gemessene Spannung in diesem Bereich, wird die Pumpe komplett
// gesperrt (auch manuell/Funk), um die Batterie vor Tiefentladung zu
// schützen. Unterhalb von BATT_KRITISCH_MIN wird von einem fehlenden oder
// nicht angeschlossenen Sensor ausgegangen — dort wird NICHT gesperrt.
// Der Schutz selbst lässt sich im WebIF deaktivieren (z.B. für einen
// Notfall, in dem die Pumpe trotz schwacher Batterie laufen soll).
#define BATT_KRITISCH_MIN     0.5f
#define BATT_KRITISCH_MAX     11.8f
#define DEFAULT_BATT_SCHUTZ_AKTIV true

// ── OLED SSD1306 0,96" I2C ─────────────────────────────────────
// VCC→3.3V  GND→GND  SDA→GPIO5  SCL→GPIO17
#define OLED_SDA      5
#define OLED_SCL      17
#define OLED_WIDTH    128
#define OLED_HEIGHT   64
#define OLED_I2C_ADDR 0x3C   // alternativ 0x3D

// ── Sensor-Pegel-Defaults ──────────────────────────────────────
// true  = Sensor aktiv bei HIGH → interner PULLDOWN
// false = Sensor aktiv bei LOW  → interner PULLUP
#define DEFAULT_PEGEL_REGEN_HIGH   true
#define DEFAULT_PEGEL_WASSER_HIGH  true

// ── Sensor-Aktivierung ──────────────────────────────────────────
// false = Sensor deaktiviert → wird von der Pumpensteuerung ignoriert
//         (Regen: wirkt dann wie "kein Regen", Wasser: wie "Wasser voll")
#define DEFAULT_SENSOR_REGEN_AKTIV   true
#define DEFAULT_SENSOR_WASSER_AKTIV  true

// ── Relais-Polarität ───────────────────────────────────────────
// false = active LOW (Optokoppler PC817) → LOW = Relais EIN
// true  = active HIGH → HIGH = Relais EIN
#define DEFAULT_PUMPE_AKTIV_HIGH   false

// ── NVS ────────────────────────────────────────────────────────
#define NVS_NAMESPACE "bew12"

// ── Grenzen ────────────────────────────────────────────────────
#define LOG_MAX       100
#define TIMER_PRO_TAG   3

// ── Datenstrukturen ────────────────────────────────────────────
struct TagTimer {
  bool     aktiv;
  uint8_t  einH;
  uint8_t  einM;
  uint16_t dauerMin;
};
struct Tagprogramm {
  TagTimer timer[TIMER_PRO_TAG];
};
struct LogEintrag {
  uint32_t timestamp;
  char     text[48];
};
