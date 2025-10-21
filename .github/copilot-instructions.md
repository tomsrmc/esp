# ESP32 HTTP API Development Guide

This is a structured ESP32 Arduino project using PlatformIO that exposes an HTTP REST API with CORS support and LED control.

## Architecture Overview

**Core Pattern**: Separation of concerns through namespaces and routing abstraction
- `Core::` - Infrastructure (server, wifi, LED, mDNS)
- `Endpoints::` - HTTP handlers and routing logic
- Business logic stays in endpoint handlers, routing stays centralized

**Key Files**:
- `src/main.cpp` - Entry point, includes heartbeat LED on pin 2
- `src/core/AppServer.{h,cpp}` - HTTP server wrapper with CORS and response tracking
- `src/endpoints/routing/Routes.cpp` - Central route registration 
- `src/config/secrets.h` - WiFi credentials (gitignored template)

## Development Patterns

### Adding New Endpoints
1. Declare handler in `src/endpoints/routing/Endpoints.h`: `void handleNewFeature();`
2. Implement in `src/endpoints/NewFeature.cpp` using `Core::sendJson()` for responses
3. Register route in `Routes.cpp` using `wrap()` pattern for auto-204 responses
4. Always add OPTIONS handler for CORS: `s.on("/path", HTTP_OPTIONS, handleOptions);`

### Response Handling Convention
- Use `Core::sendJson(code, jsonString)` for JSON responses (auto-adds CORS)
- Use `Core::sendRaw(code, contentType, body)` for other content types
- The `wrap()` function auto-sends 204 if handler doesn't send anything
- Response tracking prevents double-sends via `markResponded()/responseWasSent()`

### LED Control Pattern
```cpp
// Instance LEDs as static in anonymous namespace
namespace {
  Core::Led statusLed(4, false);  // pin 4, active high
  bool ledInit = false;
  void ensureLed() { if (!ledInit) { statusLed.begin(); ledInit = true; } }
}
// Call ensureLed() before using in handlers
```

### JSON Response Pattern
```cpp
StaticJsonDocument<128> doc;  // Size hint for stack allocation
doc["key"] = value;
String out;
serializeJson(doc, out);
Core::sendJson(200, out);
```

## Build & Development Workflow

**PlatformIO Commands**:
- `pio run` - Build for ESP32
- `pio run -t upload` - Flash to device
- `pio device monitor` - Serial monitor (115200 baud)
- `pio run -t clean` - Clean build artifacts

**Configuration**: `platformio.ini` uses `esp32dev` board, Arduino framework, 115200 serial

**Debugging**: Set `build_flags = -DCORE_DEBUG_LEVEL=3` in platformio.ini for verbose Arduino core logs

## Network & Security

**WiFi**: Managed by `Core::WiFiManager::connect()` - blocks until connected
**mDNS**: Device accessible at `{MDNS_HOSTNAME}.local` (default: `esp32.local`)
**CORS**: Automatically added to all responses, supports all origins/methods
**Secrets**: Copy `secrets.h.template` to `secrets.h` with actual credentials

## Testing Endpoints

**Health Check**: `GET /health` → `{"status":"ok"}`
**LED Control**: `POST /run` → Blinks LED on pin 4, returns request size
**System Info**: `GET /system/info` → IP, RSSI, uptime

All endpoints support preflight OPTIONS requests for browser compatibility.