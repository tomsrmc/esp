#pragma once
#include <Arduino.h>

namespace WebSocketHandlers {
  void handleMessage(uint8_t clientNum, const String& payload);
}
