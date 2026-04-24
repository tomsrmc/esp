#pragma once
#include <WebServer.h>

// Include all endpoint handlers
#include "../endpoints/Health.h"
#include "../endpoints/System.h"
#include "../endpoints/Led.h"
#include "../endpoints/Stepper.h"

namespace Endpoints {
  // Central router: registers all paths and HTTP verbs in one place.
  void registerAll(WebServer& server);
}
