# Bewässerungssteuerung v12 (ESP32)

Automatisierte Garten-Bewässerungssteuerung auf Basis eines ESP32, mit
Wochenprogramm-Timern, Regen-/Wasserstandssensor, Funk-Fernbedienung,
OLED-Anzeige, Status-LEDs und einem WebIF (eigener Access Point) zur
Konfiguration.

## Features

- Wochenprogramm mit bis zu 3 Timern pro Tag (inkl. Timer über Mitternacht)
- Regensensor & Wasserstandssensor mit einstellbarem Pegel (HIGH/LOW-aktiv)
- Automatik EIN/AUS per physischem Taster oder WebIF
- Manuelle Pumpensteuerung per Funk-Fernbedienung (QIACHIP RX480E-4)
- Separater Beleuchtungs-Kanal mit eigenem Taster
- Batterie-Überwachung via Spannungsteiler am ADC
- OLED-Display (SSD1306, 128×64): Uhrzeit, Batteriespannung, Automatik-Status
- 4 Status-LEDs (Automatik, Wasser, Regen, Pumpe)
- Web-Dashboard (SPIFFS, `data/index.html`) mit Live-Status, Timer-Editor,
  Log, Backup-Export/-Import
- Betriebsstunden-Zähler, persistente Einstellungen via NVS (Preferences)

## Hardware

- ESP32 Dev Board
- 2-Kanal-Relaismodul (JQC-3FF-S-Z, Optokoppler PC817, active LOW)
- SSD1306 OLED 0,96" I2C
- Regensensor + Wasserstandssensor (digitaler Ausgang)
- Funk-Empfänger RX480E-4 (2 Kanäle, momentary)
- Spannungsteiler zur Batteriespannungsmessung (100kΩ / 33kΩ)

### Pinbelegung (fest in `src/config.h`)

| Funktion              | GPIO |
|------------------------|------|
| Regensensor            | 4    |
| Wasserstandssensor      | 16   |
| Pumpe (Relais IN1)     | 26   |
| Automatik-Taster       | 25   |
| Beleuchtung (Relais IN2)| 27   |
| Licht-Taster           | 14   |
| Funk A (Pumpe EIN)     | 32   |
| Funk B (Pumpe AUS)     | 33   |
| LED Automatik          | 18   |
| LED Wasser             | 19   |
| LED Regen              | 21   |
| LED Pumpe               | 22   |
| Batterie-ADC           | 34   |
| OLED SDA               | 5    |
| OLED SCL               | 17   |

## Build & Flash

Projekt basiert auf [PlatformIO](https://platformio.org/).

```bash
pio run -t upload           # Firmware flashen
pio run -t uploadfs         # data/index.html als SPIFFS-Image flashen
```

Nach dem Start spannt der ESP32 einen eigenen WLAN-Access-Point auf
(SSID `Bewaesserung`, Passwort `12345678`, IP `192.168.4.1`) — dort ist
das Web-Dashboard erreichbar.

## Projektstruktur

```
src/
  main.cpp              Setup & Loop
  config.h              Pinbelegung, Defaults, Datenstrukturen
  storage.*              NVS-Persistenz, Log
  zeitverwaltung.*        manuelle Zeitverwaltung (kein NTP, kein RTC)
  sensor.*                Regen-/Wasserstandssensor
  taster.*                Automatik-Taster (entprellt)
  funk_empfaenger.*        RX480E-4 Funk-Fernbedienung
  led_steuerung.*          Status-LEDs
  pumpensteuerung.*        Timer-Logik & Relaissteuerung Pumpe
  licht_steuerung.*        Beleuchtungskanal & Taster
  webserver_routes.*      REST-API + Webserver
  oled_anzeige.*           OLED-Anzeige
data/
  index.html              Web-Dashboard (SPIFFS)
```

## Versionshistorie (Kurzfassung)

- v9: Batteriespannung via ADC
- v10: OLED SSD1306
- v11: 2-Kanal-Relaismodul + Licht-Taster
- v12: Alle GPIOs fest verdrahtet, OLED zeigt nur noch Uhrzeit/Batterie/Automatik
