#include "WebSocketManager.h"
#include <Arduino.h>

namespace WebSocket {
  namespace Manager {
    namespace {
      WebSocketsServer* server_ = nullptr;
      std::function<void(uint8_t, const String&)> messageHandler_ = nullptr;
      std::function<String(uint8_t)> connectHandler_ = nullptr;

      void handleEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        switch (type) {
          case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected\n", num);
            break;

          case WStype_CONNECTED: {
            IPAddress ip = server_->remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
            String welcome = connectHandler_
              ? connectHandler_(num)
              : "{\"type\":\"connected\",\"client\":" + String(num) + "}";
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

    void begin(uint16_t port) {
      if (server_) return;
      server_ = new WebSocketsServer(port);
      server_->begin();
      server_->onEvent(handleEvent);
      Serial.printf("WebSocket server started on port %d\n", port);
    }

    void loop() {
      if (server_) {
        server_->loop();
      }
    }

    void onMessage(std::function<void(uint8_t, const String&)> handler) {
      messageHandler_ = handler;
    }

    void onConnect(std::function<String(uint8_t)> handler) {
      connectHandler_ = handler;
    }

    void broadcast(const String& message) {
      if (server_) {
        String msg = message;
        server_->broadcastTXT(msg);
      }
    }

    void sendToClient(uint8_t clientNum, const String& message) {
      if (server_) {
        String msg = message;
        server_->sendTXT(clientNum, msg);
      }
    }

    void sendPing() {
      if (server_) {
        server_->broadcastPing();
        Serial.println("Ping sent to all clients");
      }
    }
  }
}
