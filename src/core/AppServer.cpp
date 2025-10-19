#include "AppServer.h"
#include <Arduino.h>

namespace Core {
  WebServer server(80);

  static bool gResponded = false;

  void resetResponseFlag() { gResponded = false; }
  void markResponded() { gResponded = true; }
  bool responseWasSent() { return gResponded; }

  void addCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type,Authorization");
  }

  void handleOptions() {
    addCORS();
    server.send(204);
    markResponded();
  }

  void sendRaw(int code, const String& contentType, const String& body) {
    addCORS();
    server.send(code, contentType, body);
    markResponded();
  }

  void sendJson(int code, const String& json) {
    sendRaw(code, "application/json", json);
  }

  void startServer() {
    server.begin();
  }
}
