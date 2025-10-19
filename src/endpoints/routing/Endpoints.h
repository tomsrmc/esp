#pragma once
#include <WebServer.h>

namespace Endpoints {
  // One function per endpoint: implement the logic only (no routing).
  void handleHealth();
  void handleRun();
  void handleInfo();

  // Central router: registers all paths and HTTP verbs in one place.
  void registerAll(WebServer& server);
}
