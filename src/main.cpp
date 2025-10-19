#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config/secrets.h"

#include "core/Led.h"
#include "core/AppServer.h"
#include "core/WiFiManager.h"
#include "core/MDNSManager.h"

#include "endpoints/routing/Endpoints.h"

using namespace Core;

static Led heartbeatLed(2, false);
static const unsigned long HEARTBEAT_PERIOD = 3000;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("Booting ESP32 API…");

  heartbeatLed.begin();
  WiFiManager::connect(WIFI_SSID, WIFI_PASSWORD, Serial);
  MDNSManager::begin(MDNS_HOSTNAME);

  Endpoints::registerAll(server);
  startServer();
}

void loop() {
  server.handleClient();
  static unsigned long last = 0;
  if (millis() - last > HEARTBEAT_PERIOD) {
    last = millis();
    heartbeatLed.pulse(100, 0, 1);
  }
}
