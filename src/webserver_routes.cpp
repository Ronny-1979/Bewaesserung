#include "webserver_routes.h"
#include "config.h"
#include "storage.h"
#include "zeitverwaltung.h"
#include "pumpensteuerung.h"
#include "licht_steuerung.h"
#include "sensor.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>

WebServer server(80);

extern bool  regenAktiv;
extern bool  wasserVorhanden;
extern float battSpannung;

static void sendJson(const String& j) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", j);
}

// ── GET /api/status ───────────────────────────────────────────
static void handleStatus() {
  String timerJson = "[";
  for (int d = 0; d < 7; d++) {
    if (d) timerJson += ",";
    timerJson += "[";
    for (int t = 0; t < TIMER_PRO_TAG; t++) {
      if (t) timerJson += ",";
      TagTimer& tm = woche[d].timer[t];
      timerJson += "{\"a\":" + String(tm.aktiv?1:0)
                + ",\"h\":" + String(tm.einH)
                + ",\"m\":" + String(tm.einM)
                + ",\"d\":" + String(tm.dauerMin) + "}";
    }
    timerJson += "]";
  }
  timerJson += "]";

  String json = "{";
  json += "\"zeit\":\""          + zeit_als_string()            + "\",";
  json += "\"wochentag\":"       + String(zeit_wochentag())     + ",";
  json += "\"zeitApprox\":"      + String(zeitApproximiert ?1:0) + ",";
  json += "\"regen\":"           + String(regenAktiv      ?1:0) + ",";
  json += "\"wasser\":"          + String(wasserVorhanden  ?1:0) + ",";
  json += "\"pumpe\":"           + String(pumpe_laeuft()   ?1:0) + ",";
  json += "\"pumpeMan\":"        + String(pumpeManAn       ?1:0) + ",";
  json += "\"pumpeManAus\":"     + String(pumpeManAus      ?1:0) + ",";
  json += "\"pumpeManZeitAktiv\":" + String(pumpeManZeitAktiv ?1:0) + ",";
  json += "\"pumpeManRestSek\":" + String(pumpe_manuell_rest_sek()) + ",";
  json += "\"battKritisch\":"    + String(battKritisch     ?1:0) + ",";
  json += "\"automatik\":"       + String(automatikAn      ?1:0) + ",";
  json += "\"urlaubAktiv\":"     + String(urlaubsModusAktiv?1:0) + ",";
  json += "\"urlaubEndeDatum\":\"" + unix_als_datum_string(urlaubsEndeUnix) + "\",";
  json += "\"licht\":"           + String(lichtAn          ?1:0) + ",";
  json += "\"betrieb\":\""       + pumpe_betriebszeit_string()  + "\",";
  json += "\"betriebSek\":"      + String(betriebsSekGesamt)    + ",";
  // Feste Pins für WebIF-Info (nur Anzeige, kein Dropdown mehr)
  json += "\"pinRegen\":"        + String(PIN_REGEN)             + ",";
  json += "\"pinWasser\":"       + String(PIN_WASSER)            + ",";
  json += "\"pinPumpe\":"        + String(PIN_PUMPE)             + ",";
  json += "\"pinTaster\":"       + String(PIN_TASTER)            + ",";
  json += "\"pinLicht\":"        + String(PIN_LICHT)             + ",";
  json += "\"pinLichtTaster\":"  + String(PIN_LICHT_TASTER)      + ",";
  json += "\"pinFunkA\":"        + String(PIN_FUNK_A)            + ",";
  json += "\"pinFunkB\":"        + String(PIN_FUNK_B)            + ",";
  json += "\"pegelRegenHigh\":"  + String(pegelRegenHigh  ?1:0) + ",";
  json += "\"pegelWasserHigh\":"+ String(pegelWasserHigh ?1:0) + ",";
  json += "\"pumpeAktivHigh\":"  + String(pumpeAktivHigh  ?1:0) + ",";
  json += "\"sensorRegenAktiv\":"  + String(sensorRegenAktiv  ?1:0) + ",";
  json += "\"sensorWasserAktiv\":"+ String(sensorWasserAktiv ?1:0) + ",";
  json += "\"battV\":"           + String(battSpannung, 2)       + ",";
  json += "\"timer\":"           + timerJson;
  json += "}";
  sendJson(json);
}

// ── GET /api/export ───────────────────────────────────────────
static void handleExport() {
  String json = "{\"version\":5,";
  json += "\"automatik\":"      + String(automatikAn    ?1:0) + ",";
  json += "\"betriebSek\":"     + String(betriebsSekGesamt)   + ",";
  json += "\"pegelRegenHigh\":" + String(pegelRegenHigh ?1:0) + ",";
  json += "\"pegelWasserHigh\":"+ String(pegelWasserHigh?1:0) + ",";
  json += "\"pumpeAktivHigh\":" + String(pumpeAktivHigh ?1:0) + ",";
  json += "\"sensorRegenAktiv\":"  + String(sensorRegenAktiv  ?1:0) + ",";
  json += "\"sensorWasserAktiv\":"+ String(sensorWasserAktiv ?1:0) + ",";
  json += "\"timer\":[";
  for (int d = 0; d < 7; d++) {
    if (d) json += ",";
    json += "[";
    for (int t = 0; t < TIMER_PRO_TAG; t++) {
      if (t) json += ",";
      TagTimer& tm = woche[d].timer[t];
      json += "{\"a\":" + String(tm.aktiv?1:0)
            + ",\"h\":" + String(tm.einH)
            + ",\"m\":" + String(tm.einM)
            + ",\"d\":" + String(tm.dauerMin) + "}";
    }
    json += "]";
  }
  json += "]}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Content-Disposition",
                    "attachment; filename=\"bewaesserung_backup.json\"");
  server.send(200, "application/json", json);
}

// ── POST /api/import ──────────────────────────────────────────
static void handleImport() {
  if (!server.hasArg("plain")) { sendJson("{\"ok\":0,\"err\":\"Kein Body\"}"); return; }
  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    sendJson("{\"ok\":0,\"err\":\"JSON ungueltig\"}"); return;
  }
  if (doc["pegelRegenHigh"].is<int>())
    pegelRegenHigh  = doc["pegelRegenHigh"].as<int>() != 0;
  if (doc["pegelWasserHigh"].is<int>())
    pegelWasserHigh = doc["pegelWasserHigh"].as<int>() != 0;
  if (doc["pumpeAktivHigh"].is<int>())
    pumpeAktivHigh  = doc["pumpeAktivHigh"].as<int>() != 0;
  if (doc["sensorRegenAktiv"].is<int>())
    sensorRegenAktiv  = doc["sensorRegenAktiv"].as<int>() != 0;
  if (doc["sensorWasserAktiv"].is<int>())
    sensorWasserAktiv = doc["sensorWasserAktiv"].as<int>() != 0;
  if (doc["betriebSek"].is<uint32_t>())
    betriebsSekGesamt = doc["betriebSek"].as<uint32_t>();
  if (doc["automatik"].is<int>())
    automatikAn = doc["automatik"].as<int>() != 0;
  if (doc["timer"].is<JsonArray>()) {
    JsonArray tage = doc["timer"].as<JsonArray>();
    for (int d = 0; d < 7 && d < (int)tage.size(); d++) {
      if (!tage[d].is<JsonArray>()) continue;
      JsonArray slots = tage[d].as<JsonArray>();
      for (int t = 0; t < TIMER_PRO_TAG && t < (int)slots.size(); t++) {
        JsonObject o = slots[t].as<JsonObject>();
        if (o["a"].is<int>()) woche[d].timer[t].aktiv = o["a"].as<int>() != 0;
        if (o["h"].is<int>()) { int v=o["h"].as<int>(); woche[d].timer[t].einH=(v<0)?0:(v>23)?23:v; }
        if (o["m"].is<int>()) { int v=o["m"].as<int>(); woche[d].timer[t].einM=(v<0)?0:(v>59)?59:v; }
        if (o["d"].is<int>()) { int v=o["d"].as<int>(); woche[d].timer[t].dauerMin=(v<1)?1:(v>360)?360:v; }
      }
    }
  }
  speicher_alles_speichern();
  sensor_init();
  pumpe_schalten(pumpe_laeuft());
  log_eintrag("Einstellungen importiert", zeit_als_unix());
  sendJson("{\"ok\":1}");
}

// ── POST /api/pegel ───────────────────────────────────────────
static void handlePegel() {
  bool changed = false;
  if (server.hasArg("regenHigh"))  { pegelRegenHigh  = server.arg("regenHigh")  == "1"; changed=true; }
  if (server.hasArg("wasserHigh")) { pegelWasserHigh = server.arg("wasserHigh") == "1"; changed=true; }
  if (changed) {
    speicher_pegel_speichern();
    sensor_init();
    log_eintrag("Sensorpegel geaendert", zeit_als_unix());
    sendJson("{\"ok\":1}");
  } else {
    sendJson("{\"ok\":0,\"err\":\"Parameter fehlen\"}");
  }
}

// ── POST /api/sensoraktiv ─────────────────────────────────────
static void handleSensorAktiv() {
  bool changed = false;
  if (server.hasArg("regenAktiv"))  { sensorRegenAktiv  = server.arg("regenAktiv")  == "1"; changed=true; }
  if (server.hasArg("wasserAktiv")) { sensorWasserAktiv = server.arg("wasserAktiv") == "1"; changed=true; }
  if (changed) {
    speicher_sensor_aktiv_speichern();
    log_eintrag("Sensor-Aktivierung geaendert", zeit_als_unix());
    sendJson("{\"ok\":1}");
  } else {
    sendJson("{\"ok\":0,\"err\":\"Parameter fehlen\"}");
  }
}

// ── POST /api/pumpepol ────────────────────────────────────────
static void handlePumpePol() {
  if (!server.hasArg("aktivHigh")) { sendJson("{\"ok\":0,\"err\":\"Parameter fehlen\"}"); return; }
  bool neu = server.arg("aktivHigh") == "1";
  if (neu != pumpeAktivHigh) {
    pumpeAktivHigh = neu;
    speicher_pumpe_pol_speichern();
    pumpe_schalten(pumpe_laeuft());
    log_eintrag(pumpeAktivHigh ? "Pumpe-Pol: active HIGH"
                               : "Pumpe-Pol: active LOW", zeit_als_unix());
  }
  sendJson("{\"ok\":1}");
}

// ── POST /api/automatik ───────────────────────────────────────
static void handleAutomatik() {
  if (!server.hasArg("an")) { sendJson("{\"ok\":0,\"err\":\"Parameter fehlen\"}"); return; }
  bool neu = server.arg("an") == "1";
  if (neu != automatikAn) {
    automatik_setzen(neu);
    log_eintrag(automatikAn ? "Automatik EIN (WebIF)"
                            : "Automatik AUS (WebIF)", zeit_als_unix());
  }
  sendJson("{\"ok\":1}");
}

// ── POST /api/zeit ────────────────────────────────────────────
static void handleZeit() {
  if (!server.hasArg("t")||!server.hasArg("m")||!server.hasArg("j")||
      !server.hasArg("h")||!server.hasArg("mi")||!server.hasArg("s")) {
    sendJson("{\"ok\":0,\"err\":\"Parameter fehlen\"}"); return;
  }
  zeit_setzen(server.arg("t").toInt(), server.arg("m").toInt(),
              server.arg("j").toInt(), server.arg("h").toInt(),
              server.arg("mi").toInt(), server.arg("s").toInt());
  speicher_zeit_speichern();
  log_eintrag("Zeit gesetzt", zeit_als_unix());
  sendJson("{\"ok\":1}");
}

// ── POST /api/timer ───────────────────────────────────────────
static void handleTimer() {
  if (!server.hasArg("d")||!server.hasArg("t")||
      !server.hasArg("h")||!server.hasArg("m")||!server.hasArg("dur")) {
    sendJson("{\"ok\":0,\"err\":\"Ungueltige Parameter\"}"); return;
  }
  int d=server.arg("d").toInt(), t=server.arg("t").toInt();
  if (d<0||d>6||t<0||t>=TIMER_PRO_TAG) {
    sendJson("{\"ok\":0,\"err\":\"Ungueltige Parameter\"}"); return;
  }
  int h=server.arg("h").toInt(), m=server.arg("m").toInt(), dur=server.arg("dur").toInt();
  woche[d].timer[t].aktiv    = server.arg("a") == "1";
  woche[d].timer[t].einH     = (h<0)?0:(h>23)?23:h;
  woche[d].timer[t].einM     = (m<0)?0:(m>59)?59:m;
  woche[d].timer[t].dauerMin = (dur<1)?1:(dur>360)?360:dur;
  speicher_timer_speichern();
  sendJson("{\"ok\":1}");
}

// ── POST /api/pumpe ───────────────────────────────────────────
static void handlePumpe() {
  if (!server.hasArg("an")) { sendJson("{\"ok\":0,\"err\":\"Parameter fehlen\"}"); return; }
  pumpe_manuell(server.arg("an") == "1");
  sendJson("{\"ok\":1}");
}

// ── POST /api/pumpezeit ────────────────────────────────────────
static void handlePumpeZeit() {
  if (!server.hasArg("minuten")) { sendJson("{\"ok\":0,\"err\":\"Parameter fehlen\"}"); return; }
  int minuten = server.arg("minuten").toInt();
  if (minuten < 1) minuten = 1;
  pumpe_manuell_zeit((uint16_t)minuten);
  log_eintrag("Manuelle Zeitbewaesserung gestartet", zeit_als_unix());
  sendJson("{\"ok\":1}");
}

// ── POST /api/urlaubstart ──────────────────────────────────────
static void handleUrlaubStart() {
  if (!server.hasArg("tage")) { sendJson("{\"ok\":0,\"err\":\"Parameter fehlen\"}"); return; }
  int tage = server.arg("tage").toInt();
  if (tage < 1) tage = 1;
  urlaub_starten((uint16_t)tage);
  sendJson("{\"ok\":1}");
}

// ── POST /api/urlaubende ───────────────────────────────────────
static void handleUrlaubEnde() {
  urlaub_beenden();
  sendJson("{\"ok\":1}");
}

// ── POST /api/licht ───────────────────────────────────────────
static void handleLicht() {
  if (!server.hasArg("an")) { sendJson("{\"ok\":0,\"err\":\"Parameter fehlen\"}"); return; }
  licht_schalten(server.arg("an") == "1");
  sendJson("{\"ok\":1}");
}

// ── GET /api/log ──────────────────────────────────────────────
static void handleLog() { sendJson(log_als_json()); }

// ── GET / ─────────────────────────────────────────────────────
static void handleRoot() {
  if (SPIFFS.exists("/index.html")) {
    File f = SPIFFS.open("/index.html", "r");
    server.streamFile(f, "text/html"); f.close();
  } else {
    server.send(503, "text/plain",
      "SPIFFS fehlt. PlatformIO: 'Upload Filesystem Image' ausfuehren.");
  }
}

void webserver_init() {
  if (!SPIFFS.begin(true)) Serial.println("SPIFFS Mount fehlgeschlagen!");
  server.on("/",              HTTP_GET,  handleRoot);
  server.on("/api/status",    HTTP_GET,  handleStatus);
  server.on("/api/export",    HTTP_GET,  handleExport);
  server.on("/api/import",    HTTP_POST, handleImport);
  server.on("/api/pegel",     HTTP_POST, handlePegel);
  server.on("/api/sensoraktiv", HTTP_POST, handleSensorAktiv);
  server.on("/api/pumpepol",  HTTP_POST, handlePumpePol);
  server.on("/api/automatik", HTTP_POST, handleAutomatik);
  server.on("/api/zeit",      HTTP_POST, handleZeit);
  server.on("/api/timer",     HTTP_POST, handleTimer);
  server.on("/api/pumpe",     HTTP_POST, handlePumpe);
  server.on("/api/pumpezeit", HTTP_POST, handlePumpeZeit);
  server.on("/api/urlaubstart", HTTP_POST, handleUrlaubStart);
  server.on("/api/urlaubende",  HTTP_POST, handleUrlaubEnde);
  server.on("/api/licht",     HTTP_POST, handleLicht);
  server.on("/api/log",       HTTP_GET,  handleLog);
  server.onNotFound([]() { server.send(404,"text/plain","Nicht gefunden"); });
  server.begin();
  Serial.println("Webserver: http://" AP_IP_STR);
}

void webserver_loop() { server.handleClient(); }
