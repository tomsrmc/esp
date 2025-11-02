#pragma once
#include <WebSocketsServer.h>
#include <functional>

namespace Core {
  class WebSocketManager {
  public:
    using MessageHandler = std::function<void(uint8_t clientNum, const String& payload)>;

    static void begin(uint16_t port = 81);
    static void loop();
    static void onMessage(MessageHandler handler);
    static void broadcast(const String& message);
    static void sendToClient(uint8_t clientNum, const String& message);

  private:
    static WebSocketsServer* server_;
    static MessageHandler messageHandler_;
    static void handleEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
  };
}
