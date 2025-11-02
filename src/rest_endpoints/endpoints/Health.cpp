#include "Health.h"
#include <ArduinoJson.h>
#include "../../core/AppServer.h"

using Core::sendJson;

namespace Endpoints {
  void handleHealth() {
    JsonDocument doc;
    doc["status"] = "ok";
    String out;
    serializeJson(doc, out);
    sendJson(200, out);
  }
}
