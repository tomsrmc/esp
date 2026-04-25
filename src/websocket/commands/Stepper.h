#pragma once
#include <ArduinoJson.h>

namespace WebSocket {
namespace Commands {
bool handleStepperJog(uint8_t clientNum, const JsonDocument& doc);
bool handleStepperStatus(uint8_t clientNum, const JsonDocument& doc);
bool handleStepperStop(uint8_t clientNum, const JsonDocument& doc);
bool handleStepperConfig(uint8_t clientNum, const JsonDocument& doc);
bool handleCapabilities(uint8_t clientNum, const JsonDocument& doc);
}
}
