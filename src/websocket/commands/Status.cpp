#include "Status.h"
#include "../WebSocketManager.h"
#include <WiFi.h>

namespace WebSocket {
  namespace Commands {

    bool handleStatus(uint8_t clientNum, const JsonDocument& doc) {
      JsonDocument response;
      response["status"] = "ok";
      response["uptime"] = millis();
      response["freeHeap"] = ESP.getFreeHeap();
      response["ip"] = WiFi.localIP().toString();
      response["rssi"] = WiFi.RSSI();
      
      String responseStr;
      serializeJson(response, responseStr);
      WebSocket::Manager::sendToClient(clientNum, responseStr);
      
      return true;
    }

  }
}
