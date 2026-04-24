#include "Led.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "../../core/AppServer.h"
#include "../../core/Led.h"

using Core::sendJson;
using Core::server;

namespace {
  Core::Led blueLed(2, false);
  bool ledInit = false;

  void ensureLed() {
    if (!ledInit) {
      blueLed.begin();
      ledInit = true;
    }
  }

  bool deserializeBody(JsonDocument& doc) {
    if (!server.hasArg("plain")) {
      JsonDocument res;
      res["error"] = "Missing JSON body";
      String out;
      serializeJson(res, out);
      sendJson(400, out);
      return false;
    }

    String body = server.arg("plain");
    if (body.isEmpty()) {
      JsonDocument res;
      res["error"] = "Empty request body";
      String out;
      serializeJson(res, out);
      sendJson(400, out);
      return false;
    }

    DeserializationError err = deserializeJson(doc, body);
    if (err) {
      JsonDocument res;
      res["error"] = "Invalid JSON";
      res["code"] = "PARSE_ERROR";
      String out;
      serializeJson(res, out);
      sendJson(400, out);
      return false;
    }
    return true;
  }
}

namespace Endpoints {
  void handleBlink() {
    JsonDocument doc;
    if (!deserializeBody(doc)) {
      return;
    }

    int onMs = doc["onMs"] | 100;
    int offMs = doc["offMs"] | 100;
    int times = doc["times"] | 3;

    ensureLed();
    for (int i = 0; i < times; i++) {
      blueLed.write(true);
      delay(onMs);
      blueLed.write(false);
      delay(offMs);
    }

    JsonDocument response;
    response["status"] = "ok";
    response["command"] = "blink";
    response["onMs"] = onMs;
    response["offMs"] = offMs;
    response["times"] = times;

    String out;
    serializeJson(response, out);
    sendJson(200, out);
  }
}
