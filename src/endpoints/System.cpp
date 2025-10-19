#include <ArduinoJson.h>
#include <WiFi.h>
#include "../core/AppServer.h"

using Core::sendJson;

namespace Endpoints {
  void handleInfo() {
    StaticJsonDocument<192> doc;
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
    doc["uptimeMs"] = millis();
    String out;
    serializeJson(doc, out);
    sendJson(200, out);
  }

  
}
