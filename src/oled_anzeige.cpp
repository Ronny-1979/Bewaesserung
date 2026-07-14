#include "oled_anzeige.h"
#include "config.h"
#include "zeitverwaltung.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
static bool oledOk = false;

void oled_init() {
  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setTimeout(50);   // I2C-Timeout: verhindert Loop-Blockade bei OLED-Hänger
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println("OLED: SSD1306 nicht gefunden! Kabel/Adresse pruefen.");
    return;
  }
  oledOk = true;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(14, 26);
  display.println("Bewaesserung v12");
  display.display();
  Serial.printf("OLED      : SDA=GPIO%d | SCL=GPIO%d | Addr=0x%02X\n",
    OLED_SDA, OLED_SCL, OLED_I2C_ADDR);
}

// ── Layout 128×64 ──────────────────────────────────────────────
//
//  ┌──────────────────────────────────┐
//  │                                  │  y=0  (leer, oberer Rand)
//  │         14:23:45                 │  y=8   Uhrzeit groß (2×)
//  │                                  │
//  │      ──────────────────          │  y=28  Linie
//  │                                  │
//  │       Batterie: 12.75V           │  y=34
//  │       [████████████░░]           │  y=44  Balken
//  │                                  │
//  │       Automatik:  EIN            │  y=56
//  └──────────────────────────────────┘
//
void oled_update(float battV, bool autoAn) {
  if (!oledOk) return;
  display.clearDisplay();

  // ── Uhrzeit groß (textSize=2 → 12×16 px pro Zeichen) ────────
  if (zeitGesetzt) {
    struct tm t = zeit_aktuell();
    char uhr[9];
    snprintf(uhr, sizeof(uhr), "%02d:%02d:%02d",
      t.tm_hour, t.tm_min, t.tm_sec);
    display.setTextSize(2);
    // Zentriert: 8 Zeichen × 12px = 96px → (128-96)/2 = 16
    display.setCursor(16, 6);
    display.print(uhr);
  } else {
    display.setTextSize(2);
    display.setCursor(28, 6);
    display.print("--:--:--");
  }

  // Trennlinie
  display.setTextSize(1);
  display.drawFastHLine(10, 28, 108, SSD1306_WHITE);

  // ── Batteriespannung ─────────────────────────────────────────
  // Batterie-Zeile: "Bat: 12.75V  Voll" – alles auf einer Zeile, passt in 128px
  // "Bat: " = 5 × 6 = 30px, "12.75V" = 6 × 6 = 36px, "  " = 12px, "Voll" = 4 × 6 = 24px
  // Gesamt: 30+36+12+24 = 102px ab x=12 → endet bei x=114 ✓
  const char* kl = (battV >= 12.7f) ? "Voll" :
                   (battV >= 12.4f) ? "75% " :
                   (battV >= 12.2f) ? "50% " :
                   (battV >= 11.8f) ? "Schw" :
                   (battV >  0.5f)  ? "KRIT" : "----";
  char voltBuf[8];
  snprintf(voltBuf, sizeof(voltBuf), "%.2fV", battV);
  display.setCursor(12, 33);
  display.print("Bat:");
  display.print(voltBuf);
  display.print("  ");
  display.print(kl);

  // Batterie-Balken (104px breit, 8px hoch, y=43)
  // 104px + 2px Rand + 3px Pol = 109px ab x=12 → endet bei x=121 ✓
  display.drawRect(12, 43, 104, 8, SSD1306_WHITE);
  display.fillRect(116, 45, 3, 4, SSD1306_WHITE);  // Pol
  // Füllstand: 0–4 Stufen à 24px (4×24=96px in 102px Innenraum ✓)
  int seg = 0;
  if      (battV >= 12.7f) seg = 4;
  else if (battV >= 12.4f) seg = 3;
  else if (battV >= 12.2f) seg = 2;
  else if (battV >= 11.8f) seg = 1;
  if (seg > 0) display.fillRect(14, 45, seg * 24, 4, SSD1306_WHITE);

  // Trennlinie
  display.drawFastHLine(10, 54, 108, SSD1306_WHITE);

  // ── Automatik ────────────────────────────────────────────────
  display.setCursor(12, 57);
  display.print("Automatik: ");
  display.print(autoAn ? "EIN" : "AUS");

  display.display();
}
