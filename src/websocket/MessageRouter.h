#pragma once
#include <Arduino.h>

namespace WebSocket {
  // Main message router - dispatches to appropriate command handlers
  void handleMessage(uint8_t clientNum, const String& payload);
}
