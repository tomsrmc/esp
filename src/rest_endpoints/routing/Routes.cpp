#include "../../core/AppServer.h"
#include "Endpoints.h"

using Core::handleOptions;
using Core::resetResponseFlag;
using Core::responseWasSent;
using Core::addCORS;
using Core::server;
using Core::markResponded;

namespace Endpoints {

  // Wrap a handler: reset flag, invoke, auto-respond 204 if nothing sent.
  static std::function<void()> wrap(void (*handler)()) {
    return [handler]() {
      resetResponseFlag();
      handler();
      if (!responseWasSent()) {
        addCORS();
        server.send(204);
        markResponded();
      }
    };
  }

  void registerAll(WebServer& s) {
    // Health
    s.on("/health", HTTP_GET, wrap(Endpoints::handleHealth));
    s.on("/health", HTTP_OPTIONS, handleOptions);

    // System info
    s.on("/system/info", HTTP_GET, wrap(Endpoints::handleInfo));
    s.on("/system/info", HTTP_OPTIONS, handleOptions);

    // Alias for legacy status command
    s.on("/system/status", HTTP_GET, wrap(Endpoints::handleInfo));
    s.on("/system/status", HTTP_OPTIONS, handleOptions);

    // LED blink
    s.on("/led/blink", HTTP_POST, wrap(Endpoints::handleBlink));
    s.on("/led/blink", HTTP_OPTIONS, handleOptions);

    // Stepper jog
    s.on("/stepper/jog", HTTP_POST, wrap(Endpoints::handleStepperJog));
    s.on("/stepper/jog", HTTP_OPTIONS, handleOptions);
  }
}
