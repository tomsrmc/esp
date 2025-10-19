#pragma once
#include <WebServer.h>

namespace Core {
  extern WebServer server;

  // CORS & convenience
  void addCORS();
  void handleOptions();
  void startServer();
  void sendJson(int code, const String& json);

  // Response tracking (for fallback replies)
  void markResponded();
  bool responseWasSent();
  void resetResponseFlag();

  // Raw send with tracking (use this instead of server.send when possible)
  void sendRaw(int code, const String& contentType, const String& body);
}
