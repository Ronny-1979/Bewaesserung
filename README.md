# Bewässerungssteuerung v12 (ESP32)

Automatisierte Garten-Bewässerungssteuerung auf Basis eines ESP32, mit
Wochenprogramm-Timern, Regen-/Wasserstandssensor, Funk-Fernbedienung,
OLED-Anzeige, Status-LEDs und einem WebIF (eigener Access Point) zur
Konfiguration.

## Features

- Wochenprogramm mit bis zu 3 Timern pro Tag (inkl. Timer über Mitternacht)
- Regensensor & Wasserstandssensor mit einstellbarem Pegel (HIGH/LOW-aktiv),
  jeweils einzeln per WebIF deaktivierbar (z.B. bei Defekt/nicht angeschlossen);
  deaktivierter Sensor blinkt an der zugehörigen Status-LED langsam
- Automatik EIN/AUS per physischem Taster oder WebIF
- Manuelle Pumpensteuerung per WebIF-Button oder Funk-Fernbedienung
  (QIACHIP RX480E-4, interruptbasiert). Manuelles AUS überschreibt auch
  einen gerade aktiven Timer zuverlässig und hebt sich automatisch wieder
  auf, sobald das Zeitfenster endet
- Separater Beleuchtungs-Kanal mit eigenem Taster
- Batterie-Überwachung via Spannungsteiler am ADC
- OLED-Display (SSD1306, 128×64): Uhrzeit, Batteriespannung, Automatik-Status
- 4 Status-LEDs (Automatik, Wasser, Regen, Pumpe)
- Web-Dashboard (SPIFFS, `data/index.html`) mit Live-Status, Timer-Editor,
  Log, Backup-Export/-Import
- Betriebsstunden-Zähler (autosave alle 5 Min. während die Pumpe läuft),
  persistente Einstellungen via NVS (Preferences)
- Kein NTP/RTC — die Uhr läuft rein intern, wird aber alle 5 Minuten sowie
  sofort nach manuellem Setzen in den Flash gesichert. Nach einem Neustart/
  Stromausfall wird die letzte bekannte Zeit automatisch wiederhergestellt
  (näherungsweise) — im WebIF erscheint dann ein Hinweis, die Zeit bei
  Gelegenheit neu zu setzen
- Regensensor mit 15 Minuten Nachlaufzeit: ein kurzer Schauer unterbricht
  die Bewässerung nicht sofort wieder, sobald es kurz trocken wird
- Batterie-Schutz: Pumpe wird bei kritisch niedriger Spannung komplett
  gesperrt (auch manuell/Funk), um Tiefentladung der Batterie zu vermeiden
- Zeitgesteuerte manuelle Bewässerung ("X Minuten gießen"), unabhängig vom
  Wochenplan, schaltet sich nach Ablauf automatisch wieder ab
- Urlaubsmodus: pausiert die Automatik für eine feste Anzahl Tage und
  aktiviert sie danach automatisch wieder
- Dashboard zeigt eine Vorschau der nächsten geplanten Bewässerung
  (Wochentag, Uhrzeit, verbleibende Zeit)

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

## REST-API (WebIF)

| Endpunkt | Methode | Zweck |
|---|---|---|
| `/api/status` | GET | Live-Status (Sensoren, Pumpe, Licht, Zeit, Timer, ...) |
| `/api/export` | GET | Komplette Konfiguration als JSON-Backup |
| `/api/import` | POST | Backup wieder einspielen |
| `/api/pegel` | POST | Aktiv-Pegel der Sensoren setzen |
| `/api/sensoraktiv` | POST | Regen-/Wasserstandssensor aktivieren/deaktivieren |
| `/api/pumpepol` | POST | Relais-Polarität der Pumpe setzen |
| `/api/automatik` | POST | Wochenprogramm ein-/ausschalten |
| `/api/zeit` | POST | Uhrzeit manuell setzen |
| `/api/timer` | POST | Einzelnen Timer-Slot speichern |
| `/api/pumpe` | POST | Pumpe manuell ein-/ausschalten |
| `/api/pumpezeit` | POST | Pumpe für X Minuten starten (schaltet automatisch ab) |
| `/api/urlaubstart` | POST | Urlaubsmodus starten (Parameter: Tage) |
| `/api/urlaubende` | POST | Urlaubsmodus manuell beenden |
| `/api/licht` | POST | Beleuchtung ein-/ausschalten |
| `/api/log` | GET | Ereignisverlauf (nur RAM, geht bei Neustart verloren) |

## Bekannte Grenzen

- Kein Login/Authentifizierung im WebIF — jeder im WLAN kann steuern
- AP-Passwort ist der Standardwert `12345678`
- Log-Verlauf ist reiner RAM-Ringpuffer, nicht dauerhaft gespeichert
- Wiederhergestellte Zeit nach Neustart ist nur eine Näherung (Uhr stand
  seit dem letzten Speichern still)
- Bewusst kein Sicherheits-Timeout für manuell (nicht zeitgesteuert)
  eingeschaltete Pumpe/Beleuchtung — läuft unbegrenzt, bis manuell wieder
  ausgeschaltet (für zeitlich begrenztes Gießen: `/api/pumpezeit` nutzen)

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
  entprellung.h          gemeinsame Entprellungs-Hilfsfunktion (Taster)
  storage.*              NVS-Persistenz, Log, Urlaubsmodus
  zeitverwaltung.*        manuelle Zeitverwaltung (kein NTP, kein RTC)
  sensor.*                Regen-/Wasserstandssensor (inkl. Regen-Nachlaufzeit)
  taster.*                Automatik-Taster (entprellt)
  funk_empfaenger.*        RX480E-4 Funk-Fernbedienung (interruptbasiert)
  led_steuerung.*          Status-LEDs
  pumpensteuerung.*        Timer-Logik, Relaissteuerung, Batteriesperre,
                           zeitgesteuerte manuelle Bewässerung
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
