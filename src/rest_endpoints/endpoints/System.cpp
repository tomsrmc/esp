#include "System.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Arduino.h>
#include "../../core/AppServer.h"

using Core::sendJson;

namespace Endpoints {
  void handleInfo() {
    JsonDocument doc;
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
    doc["uptimeMs"] = millis();
    doc["freeHeap"] = ESP.getFreeHeap();
    String out;
    serializeJson(doc, out);
    sendJson(200, out);
  }

  
}
