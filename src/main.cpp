
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#if __has_include("config/secrets.h")
#include "config/secrets.h"
#elif __has_include("config/secrets.example.h")
#include "config/secrets.example.h"
#else
#error "Missing config/secrets.h. Copy src/config/secrets.example.h to src/config/secrets.h and update the values."
#endif
#include "core/Led.h"
#include "core/AppServer.h"
#include "core/WiFiManager.h"
#include "core/StepperController.h"
#include "core/StepperService.h"
#include "core/MDNSManager.h"
#include "websocket/WebSocketManager.h"
#include "websocket/MessageRouter.h"
#include "rest_endpoints/routing/Endpoints.h"

using namespace Core;

static const unsigned long HEARTBEAT_PERIOD = 3000;
StepperController stepper(25, 26, 27);
StepperService stepperService(stepper, 2);

namespace {
String buildConnectEnvelope(uint8_t clientNum) {
	JsonDocument doc;
	doc["type"] = "connected";
	doc["client"] = clientNum;
	doc["version"] = StepperService::kProtocolVersion;
	JsonObject capabilities = doc["capabilities"].to<JsonObject>();
	stepperService.fillCapabilities(capabilities);

	String out;
	serializeJson(doc, out);
	return out;
}
}

void setup() {
	Serial.begin(115200);
	delay(200);
	Serial.println();
	Serial.println("Booting ESP32 API with Stepper…");

	stepper.begin();
	stepperService.begin();
	WiFiManager::connect(WIFI_SSID, WIFI_PASSWORD, Serial);
	MDNSManager::begin(MDNS_HOSTNAME);

	// Start HTTP server
	Endpoints::registerAll(server);
	startServer();

	// Start WebSocket server
	WebSocket::Manager::begin(81);
	WebSocket::Manager::onConnect(buildConnectEnvelope);
	WebSocket::Manager::onMessage(WebSocket::handleMessage);
	Serial.println("WebSocket server ready on port 81");
}

void loop() {
	server.handleClient();
	WebSocket::Manager::loop();
	stepperService.loop();
	JsonDocument event;
	while (stepperService.consumePendingEvent(event)) {
		String payload;
		serializeJson(event, payload);
		WebSocket::Manager::broadcast(payload);
	}
	static unsigned long last = 0;
	if (millis() - last > HEARTBEAT_PERIOD) {
		last = millis();
		WebSocket::Manager::sendPing();
	}
}

