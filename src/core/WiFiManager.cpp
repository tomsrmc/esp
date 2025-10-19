#include "WiFiManager.h"
#include <Arduino.h>

namespace Core {
  namespace WiFiManager {
    void connect(const char* ssid, const char* password, HardwareSerial& out) {
      out.printf("Connecting to SSID: %s\n", ssid);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      unsigned long start = millis();
      while (WiFi.status() != WL_CONNECTED) {
        delay(400);
        out.print(".");
        if (millis() - start > 30000) {
          out.println("\n⚠️ Wi-Fi connect timeout, retrying...");
          start = millis();
        }
      }
      out.printf("\n✅ Wi-Fi connected! IP: %s\n", WiFi.localIP().toString().c_str());
    }
  }
}
