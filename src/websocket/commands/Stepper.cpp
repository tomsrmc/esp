#include "Stepper.h"
#include "../../core/StepperService.h"
#include "../WebSocketManager.h"
#include <ArduinoJson.h>

extern Core::StepperService stepperService;

namespace {
bool readRequestId(const JsonDocument& doc, uint32_t& requestId) {
    if (doc["id"].is<unsigned int>()) {
        requestId = doc["id"].as<unsigned int>();
        return true;
    }
    if (doc["id"].is<unsigned long>()) {
        requestId = doc["id"].as<unsigned long>();
        return true;
    }
    if (doc["id"].is<int>()) {
        int signedId = doc["id"].as<int>();
        if (signedId >= 0) {
            requestId = static_cast<uint32_t>(signedId);
            return true;
        }
    }
    return false;
}

void sendResponse(uint8_t clientNum, JsonDocument& response) {
    String responseStr;
    serializeJson(response, responseStr);
    WebSocket::Manager::sendToClient(clientNum, responseStr);
}
}

namespace WebSocket {
namespace Commands {
bool handleStepperJog(uint8_t clientNum, const JsonDocument& doc) {
    JsonDocument response;
    uint32_t requestId = 0;
    bool hasRequestId = readRequestId(doc, requestId);
    long delta = doc["delta"] | 80;
    float speed = doc["speed"] | 800.0f;
    stepperService.jog(delta, speed, hasRequestId, requestId, response);
    sendResponse(clientNum, response);
    return true;
}

bool handleStepperStatus(uint8_t clientNum, const JsonDocument& doc) {
    JsonDocument response;
    uint32_t requestId = 0;
    bool hasRequestId = readRequestId(doc, requestId);
    stepperService.getStatus(hasRequestId, requestId, response);
    sendResponse(clientNum, response);
    return true;
}

bool handleStepperStop(uint8_t clientNum, const JsonDocument& doc) {
    JsonDocument response;
    uint32_t requestId = 0;
    bool hasRequestId = readRequestId(doc, requestId);
    bool immediate = doc["immediate"].isNull() ? true : doc["immediate"].as<bool>();
    stepperService.stop(immediate, hasRequestId, requestId, response);
    sendResponse(clientNum, response);
    return true;
}

bool handleStepperConfig(uint8_t clientNum, const JsonDocument& doc) {
    JsonDocument response;
    uint32_t requestId = 0;
    bool hasRequestId = readRequestId(doc, requestId);
    bool hasMaxSpeed = !doc["maxSpeed"].isNull();
    bool hasAcceleration = !doc["acceleration"].isNull();
    float maxSpeed = doc["maxSpeed"] | 0.0f;
    float acceleration = doc["acceleration"] | 0.0f;
    stepperService.configure(hasMaxSpeed, maxSpeed, hasAcceleration, acceleration,
                             hasRequestId, requestId, response);
    sendResponse(clientNum, response);
    return true;
}

bool handleCapabilities(uint8_t clientNum, const JsonDocument& doc) {
    JsonDocument response;
    uint32_t requestId = 0;
    bool hasRequestId = readRequestId(doc, requestId);
    stepperService.getCapabilities(hasRequestId, requestId, response);
    sendResponse(clientNum, response);
    return true;
}
}
}
