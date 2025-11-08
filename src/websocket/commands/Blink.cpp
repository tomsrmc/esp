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

    // Get request ID if provided
    int requestId = doc["id"] | 0;

  // Get parameters with defaults, allow rapid blink
  int onMs = doc["onMs"] | 100;
  int offMs = doc["offMs"] | 100;
  int times = doc["times"] | 3;

    for (int i = 0; i < times; i++) {
      blueLed.write(true);
      delay(onMs);
      blueLed.write(false);
      delay(offMs);
    }

    // Send success response
    JsonDocument response;
    if (requestId > 0) {
      response["id"] = requestId;
    }
    response["status"] = "ok";
    response["command"] = "blink";
    response["onMs"] = onMs;
    response["offMs"] = offMs;
    response["times"] = times;

    String responseStr;
    serializeJson(response, responseStr);
    WebSocket::Manager::sendToClient(clientNum, responseStr);

    return true;
  }  }
}
