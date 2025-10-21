#include <ArduinoJson.h>
#include "../core/AppServer.h"
#include "../core/Led.h"

using Core::server;
using Core::addCORS;

namespace {
  Core::Led blueLed(2, false);  // Blue LED on pin 2
  bool ledInit = false;
  void ensureLed() {
    if (!ledInit) { blueLed.begin(); ledInit = true; }
  }
}

namespace Endpoints {
  void handleRun() {
    addCORS();
    String body = server.hasArg("plain") ? server.arg("plain") : "";
    ensureLed();
    // Rapid blink: 10 times, 50ms on, 50ms off = ~1 second total
    blueLed.pulse(50, 50, 10);

    JsonDocument doc;
    doc["ok"] = true;
    doc["receivedBytes"] = body.length();
    String out;
    serializeJson(doc, out);
    Core::sendJson(200, out);
  }
}
