#include <ArduinoJson.h>
#include "../core/AppServer.h"
#include "../core/Led.h"

using Core::server;
using Core::addCORS;

namespace {
  Core::Led led(4, false);
  bool ledInit = false;
  void ensureLed() {
    if (!ledInit) { led.begin(); ledInit = true; }
  }
}

namespace Endpoints {
  void handleRun() {
    addCORS();
    String body = server.hasArg("plain") ? server.arg("plain") : "";
    ensureLed();
    led.pulse(500, 500, 1);

    StaticJsonDocument<128> doc;
    doc["ok"] = true;
    doc["receivedBytes"] = body.length();
    String out;
    serializeJson(doc, out);
    Core::sendJson(200, out);
  }
}
