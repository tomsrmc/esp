#include "Stepper.h"
#include "../../core/StepperController.h"
#include "../WebSocketManager.h"
#include "../../core/Led.h"
#include <ArduinoJson.h>


namespace {
Core::Led onboardLed(2, false); // GPIO 2, not active low
bool ledInit = false;
void ensureLed() {
    if (!ledInit) {
        onboardLed.begin();
        ledInit = true;
    }
}
}

namespace WebSocket {
namespace Commands {
bool handleStepperJog(uint8_t clientNum, const JsonDocument& doc) {
    ensureLed();
    long delta = doc["delta"] | 80; // default: 1mm if 80 steps/mm
    float speed = doc["speed"] | 800; // default: 800 (steps/sec)
    stepper.setSpeed(speed);
    stepper.jog(delta);
    // Blink onboard LED
    onboardLed.pulse(50, 50, 2); // quick double blink
    // Respond
    JsonDocument response;
    response["status"] = "ok";
    response["command"] = "stepper_jog";
    response["delta"] = delta;
    response["speed"] = speed;
    String responseStr;
    serializeJson(response, responseStr);
    WebSocket::Manager::sendToClient(clientNum, responseStr);
    return true;
}
}
}
