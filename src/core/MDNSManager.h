#pragma once
#include <ESPmDNS.h>

namespace Core {
  namespace MDNSManager {
    void begin(const char* hostname = "esp32");
  }
}
