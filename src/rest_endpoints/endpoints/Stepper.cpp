#include "Stepper.h"
#include <ArduinoJson.h>
#include "../../core/AppServer.h"
#include "../../core/StepperService.h"

using Core::sendJson;
using Core::server;

extern Core::StepperService stepperService;

namespace {
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

  void sendDocument(int code, JsonDocument& doc) {
    String out;
    serializeJson(doc, out);
    sendJson(code, out);
  }
}

namespace Endpoints {
  void handleStepperJog() {
    JsonDocument doc;
    if (!deserializeBody(doc)) {
      return;
    }

    long delta = doc["delta"] | 80;
    float speed = doc["speed"] | 800.0f;
    JsonDocument response;
    int httpCode = stepperService.jog(delta, speed, false, 0, response);
    sendDocument(httpCode, response);
  }

  void handleStepperStatus() {
    JsonDocument response;
    int httpCode = stepperService.getStatus(false, 0, response);
    sendDocument(httpCode, response);
  }

  void handleStepperStop() {
    JsonDocument doc;
    bool immediate = true;

    if (server.hasArg("plain") && !server.arg("plain").isEmpty()) {
      if (!deserializeBody(doc)) {
        return;
      }
      immediate = doc["immediate"].isNull() ? true : doc["immediate"].as<bool>();
    }

    JsonDocument response;
    int httpCode = stepperService.stop(immediate, false, 0, response);
    sendDocument(httpCode, response);
  }

  void handleStepperConfig() {
    if (server.method() == HTTP_GET) {
      JsonDocument response;
      int httpCode = stepperService.getStatus(false, 0, response);
      response["command"] = "stepper_config";
      response["code"] = "STEPPER_CONFIG";
      response["message"] = "Stepper config and status";
      JsonObject data = response["data"].as<JsonObject>();
      data.remove("stepper");
      stepperService.fillStatus(data.createNestedObject("stepper"));
      stepperService.fillCapabilities(data.createNestedObject("capabilities"));
      sendDocument(httpCode, response);
      return;
    }

    JsonDocument doc;
    if (!deserializeBody(doc)) {
      return;
    }

    JsonDocument response;
    bool hasMaxSpeed = !doc["maxSpeed"].isNull();
    bool hasAcceleration = !doc["acceleration"].isNull();
    float maxSpeed = doc["maxSpeed"] | 0.0f;
    float acceleration = doc["acceleration"] | 0.0f;
    int httpCode = stepperService.configure(hasMaxSpeed, maxSpeed, hasAcceleration, acceleration,
                                            false, 0, response);
    sendDocument(httpCode, response);
  }
}
