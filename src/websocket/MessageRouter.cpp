#include "MessageRouter.h"
#include "WebSocketManager.h"
#include "Commands.h"
#include <ArduinoJson.h>

namespace WebSocket {
  void handleMessage(uint8_t clientNum, const String& payload) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      String response = "{\"error\":\"Invalid JSON\",\"code\":\"PARSE_ERROR\"}";
      Manager::sendToClient(clientNum, response);
      return;
    }

    // Get command name
    const char* command = doc["command"] | "";
    
    // Route to appropriate handler
    bool handled = false;
    
    if (strcmp(command, "blink") == 0) {
      handled = Commands::handleBlink(clientNum, doc);
    }
    else if (strcmp(command, "status") == 0) {
      handled = Commands::handleStatus(clientNum, doc);
    }
    
    if (!handled) {
      String response = "{\"error\":\"Unknown command\",\"code\":\"UNKNOWN_COMMAND\",\"received\":\"" + String(command) + "\"}";
      Manager::sendToClient(clientNum, response);
    }
  }
}
