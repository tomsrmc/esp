#pragma once
#include <WiFi.h>

namespace Core {
  namespace WiFiManager {
    void connect(const char* ssid, const char* password, HardwareSerial& out = Serial);
  }
}
