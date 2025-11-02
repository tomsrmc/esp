#include "WebSocketHandlers.h"
#include "WebSocketManager.h"
#include "Led.h"
#include <ArduinoJson.h>

namespace {
  Core::Led blueLed(2, false);
  bool ledInit = false;
  
  void ensureLed() {
    if (!ledInit) {
      blueLed.begin();
      ledInit = true;
    }
  }
}

namespace WebSocketHandlers {
  void handleMessage(uint8_t clientNum, const String& payload) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      String response = "{\"error\":\"Invalid JSON\",\"code\":\"PARSE_ERROR\"}";
      Core::WebSocketManager::sendToClient(clientNum, response);
      return;
    }

    // Check for command type
    const char* command = doc["command"] | "";
    
    if (strcmp(command, "blink") == 0) {
      ensureLed();
      
      // Get parameters with defaults
      int onMs = doc["onMs"] | 50;
      int offMs = doc["offMs"] | 50;
      int times = doc["times"] | 10;
      
      // Perform the blink
      blueLed.pulse(onMs, offMs, times);
      
      // Send success response
      JsonDocument response;
      response["status"] = "ok";
      response["command"] = "blink";
      response["onMs"] = onMs;
      response["offMs"] = offMs;
      response["times"] = times;
      
      String responseStr;
      serializeJson(response, responseStr);
      Core::WebSocketManager::sendToClient(clientNum, responseStr);
    }
    else if (strcmp(command, "status") == 0) {
      // Return system status
      JsonDocument response;
      response["status"] = "ok";
      response["uptime"] = millis();
      response["freeHeap"] = ESP.getFreeHeap();
      
      String responseStr;
      serializeJson(response, responseStr);
      Core::WebSocketManager::sendToClient(clientNum, responseStr);
    }
    else {
      String response = "{\"error\":\"Unknown command\",\"code\":\"UNKNOWN_COMMAND\"}";
      Core::WebSocketManager::sendToClient(clientNum, response);
    }
  }
}
