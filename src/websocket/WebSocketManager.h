#pragma once
#include <WebSocketsServer.h>
#include <functional>

namespace WebSocket {
  namespace Manager {
    using MessageHandler = std::function<void(uint8_t clientNum, const String& payload)>;
    using ConnectHandler = std::function<String(uint8_t clientNum)>;

  void begin(uint16_t port = 81);
  void loop();
  void onMessage(MessageHandler handler);
  void onConnect(ConnectHandler handler);
  void broadcast(const String& message);
  void sendToClient(uint8_t clientNum, const String& message);
  void sendPing();
  }
}
