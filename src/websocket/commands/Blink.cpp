#include "Blink.h"
#include "../WebSocketManager.h"
#include "../../core/Led.h"

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

namespace WebSocket {
  namespace Commands {
    
  bool handleBlink(uint8_t clientNum, const JsonDocument& doc) {
    ensureLed();
    
    // Get parameters with defaults
    int onMs = doc["onMs"] | 50;
    int offMs = doc["offMs"] | 50;
    int times = doc["times"] | 10;
    
    // Get client timestamp if provided (milliseconds)
    unsigned long clientTimestamp = doc["timestamp"] | 0;
    
    // Capture time right before executing the blink
    unsigned long commandStartMs = millis();
    
    // Perform the blink
    blueLed.pulse(onMs, offMs, times);
    
    // Send success response
    JsonDocument response;
    response["status"] = "ok";
    response["command"] = "blink";
    response["onMs"] = onMs;
    response["offMs"] = offMs;
    response["times"] = times;
    
    if (clientTimestamp > 0) {
      unsigned long commandDelayMs = commandStartMs - clientTimestamp;
      response["commandDelayMs"] = commandDelayMs;
    }
    
    String responseStr;
    serializeJson(response, responseStr);
    WebSocket::Manager::sendToClient(clientNum, responseStr);
    
    return true;
  }  }
}
