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

    // Run
    s.on("/run", HTTP_POST, wrap(Endpoints::handleRun));
    s.on("/run", HTTP_OPTIONS, handleOptions);

    // System info
    s.on("/system/info", HTTP_GET, wrap(Endpoints::handleInfo));
    s.on("/system/info", HTTP_OPTIONS, handleOptions);
  }
}
