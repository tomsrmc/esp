#include "MDNSManager.h"
#include <Arduino.h>

namespace Core {
  namespace MDNSManager {
    void begin(const char* hostname) {
      if (MDNS.begin(hostname)) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("mDNS: http://%s.local\n", hostname);
      } else {
        Serial.println("⚠️ mDNS failed to start");
      }
    }
  }
}
