#pragma once
#include <WebSocketsServer.h>
#include <functional>

namespace WebSocket {
  namespace Manager {
    using MessageHandler = std::function<void(uint8_t clientNum, const String& payload)>;

  void begin(uint16_t port = 81);
  void loop();
  void onMessage(MessageHandler handler);
  void broadcast(const String& message);
  void sendToClient(uint8_t clientNum, const String& message);
  void sendPing();
  }
}
