#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config/secrets.h"

#include "core/Led.h"
#include "core/AppServer.h"
#include "core/WiFiManager.h"
#include "core/MDNSManager.h"

#include "websocket/WebSocketManager.h"
#include "websocket/MessageRouter.h"

#include "rest_endpoints/routing/Endpoints.h"

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

  // Start HTTP server
  Endpoints::registerAll(server);
  startServer();

  // Start WebSocket server
  WebSocket::Manager::begin(81);
  WebSocket::Manager::onMessage(WebSocket::handleMessage);
  Serial.println("WebSocket server ready on port 81");
}

void loop() {
  server.handleClient();
  WebSocket::Manager::loop();
  
  static unsigned long last = 0;
  if (millis() - last > HEARTBEAT_PERIOD) {
    last = millis();
    heartbeatLed.pulse(100, 0, 1);
  }
}
