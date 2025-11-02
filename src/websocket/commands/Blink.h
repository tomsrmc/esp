#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

namespace WebSocket {
  namespace Commands {
    bool handleBlink(uint8_t clientNum, const JsonDocument& doc);
  }
}
