#include "WebSocketManager.h"
#include <Arduino.h>

namespace Core {
  WebSocketsServer* WebSocketManager::server_ = nullptr;
  WebSocketManager::MessageHandler WebSocketManager::messageHandler_ = nullptr;

  void WebSocketManager::begin(uint16_t port) {
    if (server_) return;
    server_ = new WebSocketsServer(port);
    server_->begin();
    server_->onEvent(handleEvent);
    Serial.printf("WebSocket server started on port %d\n", port);
  }

  void WebSocketManager::loop() {
    if (server_) {
      server_->loop();
    }
  }

  void WebSocketManager::onMessage(MessageHandler handler) {
    messageHandler_ = handler;
  }

  void WebSocketManager::broadcast(const String& message) {
    if (server_) {
      String msg = message;  // Copy to non-const
      server_->broadcastTXT(msg);
    }
  }

  void WebSocketManager::sendToClient(uint8_t clientNum, const String& message) {
    if (server_) {
      String msg = message;  // Copy to non-const
      server_->sendTXT(clientNum, msg);
    }
  }

  void WebSocketManager::handleEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
      case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected\n", num);
        break;

      case WStype_CONNECTED: {
        IPAddress ip = server_->remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        // Send welcome message
        String welcome = "{\"type\":\"connected\",\"client\":" + String(num) + "}";
        server_->sendTXT(num, welcome);
        break;
      }

      case WStype_TEXT:
        Serial.printf("[%u] Received: %s\n", num, payload);
        if (messageHandler_) {
          String message = String((char*)payload);
          messageHandler_(num, message);
        }
        break;

      case WStype_ERROR:
        Serial.printf("[%u] Error\n", num);
        break;

      default:
        break;
    }
  }
}
