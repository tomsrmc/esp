#include "Stepper.h"
#include <ArduinoJson.h>
#include "../../core/AppServer.h"
#include "../../core/StepperController.h"
#include "../../core/Led.h"

using Core::sendJson;
using Core::server;

extern Core::StepperController stepper;

namespace {
  Core::Led onboardLed(2, false);
  bool ledInit = false;

  void ensureLed() {
    if (!ledInit) {
      onboardLed.begin();
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
  void handleStepperJog() {
    JsonDocument doc;
    if (!deserializeBody(doc)) {
      return;
    }

    long delta = doc["delta"] | 80;
    float speed = doc["speed"] | 800;

    stepper.setSpeed(speed);
    stepper.jog(delta);

    ensureLed();
    onboardLed.pulse(50, 50, 2);

    JsonDocument response;
    response["status"] = "ok";
    response["command"] = "stepper_jog";
    response["delta"] = delta;
    response["speed"] = speed;

    String out;
    serializeJson(response, out);
    sendJson(200, out);
  }
}
