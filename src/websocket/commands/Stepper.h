#include "../../core/StepperController.h"

// Use the global stepper instance from main.cpp
extern Core::StepperController stepper;
#pragma once
#include <ArduinoJson.h>

namespace WebSocket {
namespace Commands {
bool handleStepperJog(uint8_t clientNum, const JsonDocument& doc);
}
}
