# ESP32 Control Firmware

PlatformIO + Arduino firmware for an ESP32 that exposes:

- a REST API on port `80`
- a WebSocket API on port `81`
- mDNS discovery via `http://<hostname>.local`
- single-axis stepper control through both REST and WebSocket commands

This project is the device-side half of the system. It runs on the ESP32 and is designed to be controlled by the Node.js client project in `espcontrol`, which can back a CLI, local service, Electron app, browser UI, or other host-side software.

Hardware-specific setup details for the current ESP32 board, stepper driver, and actuator are documented in `HARDWARE.md`.

## What this project does

On boot, the firmware:

1. starts serial logging at `115200`
2. initializes the stepper controller
3. connects to Wi‑Fi in station mode
4. starts mDNS using the configured hostname
5. starts an HTTP server on port `80`
6. starts a WebSocket server on port `81`

In the main loop, the firmware:

- services HTTP requests
- services WebSocket traffic
- advances in-progress stepper motion and feedback with the shared stepper service
- broadcasts queued stepper lifecycle events over WebSocket
- sends periodic WebSocket ping frames to connected clients

## Current exposed functionality

### REST endpoints

- `GET /health` — simple health check
- `GET /system/info` — current IP, RSSI, uptime, and free heap
- `GET /system/status` — legacy alias for `/system/info`
- `GET /system/capabilities` — protocol version, supported commands, events, and safety limits
- `POST /led/blink` — pulse the onboard LED
- `POST /stepper/jog` — queue a relative stepper move
- `GET /stepper/status` — current motion state for the single motor
- `POST /stepper/stop` — request a stop, optionally immediate
- `GET /stepper/config` — current motion config plus protocol limits
- `POST /stepper/config` — update runtime max speed and/or acceleration within firmware safety rails

### WebSocket commands

- `blink` — pulse the onboard LED
- `status` — return runtime information
- `stepper_jog` — queue a relative stepper move
- `stepper_status` — read current motion state
- `stepper_stop` — stop active motion
- `stepper_config` — read or update runtime motion config
- `capabilities` — return protocol version, supported commands/events, and safe ranges

### WebSocket events

- `stepper.started` — motion transitioned from idle to moving
- `stepper.completed` — motion reached its target
- `stepper.stopped` — motion was stopped before normal completion
- `stepper.fault` — command or configuration was rejected by firmware validation

## Motor control contract

Stepper control is now routed through a shared firmware-side service so REST and WebSocket commands use the same validation, safety limits, and state model.

### Response envelope

All motor-related responses use the same envelope:

```json
{
	"version": "1.0",
	"axis": "main",
	"command": "stepper_jog",
	"status": "ok",
	"code": "STEPPER_JOG_ACCEPTED",
	"message": "Jog queued",
	"id": 3,
	"data": {
		"delta": 160,
		"requestedSpeed": 800,
		"stepper": {
			"axis": "main",
			"currentPosition": 0,
			"targetPosition": 160,
			"distanceToGo": 160,
			"moving": true,
			"enabled": true,
			"currentSpeed": 0,
			"maxSpeed": 800,
			"acceleration": 600
		}
	}
}
```

Notes:

- `id` is echoed when the caller supplied a WebSocket request ID
- `version` is the protocol version exposed by the firmware
- `axis` is reserved for future expansion but remains single-axis today
- `data.stepper` is the authoritative motion state surface for both REST and WebSocket stepper commands

### Connect handshake

The initial WebSocket `connected` frame now includes:

- `version`
- `capabilities.commands`
- `capabilities.events`
- `capabilities.limits`

This lets the host detect supported commands and safe configuration ranges at connect time instead of assuming a fixed firmware revision.

## Architecture overview

The codebase is intentionally split into small modules:

- `src/main.cpp` — boot sequence and main loop
- `src/core` — Wi‑Fi, mDNS, LED, HTTP server, and stepper helpers
- `src/rest_endpoints` — HTTP route registration and handlers
- `src/websocket` — WebSocket server, router, and command handlers

### Runtime flow

#### HTTP flow

1. `main.cpp` registers routes through `Endpoints::registerAll(server)`.
2. The shared `WebServer` instance from `src/core/AppServer.*` listens on port `80`.
3. Requests are dispatched to handlers in `src/rest_endpoints/endpoints`.
4. Responses are sent with CORS headers enabled.

#### WebSocket flow

1. `main.cpp` starts `WebSocket::Manager::begin(81)`.
2. Incoming text frames are passed to `WebSocket::handleMessage()`.
3. `MessageRouter.cpp` parses JSON and dispatches by `command`.
4. Command handlers in `src/websocket/commands` build JSON responses and send them back to the originating client.

#### Stepper flow

1. `main.cpp` creates a global `StepperController stepper(25, 26, 27)`.
2. `main.cpp` creates a shared `StepperService` that owns motion validation, runtime config, and lifecycle events.
3. HTTP and WebSocket handlers delegate to that service instead of duplicating motion logic.
4. `StepperService::loop()` keeps motion progressing, handles non-blocking LED feedback, and queues lifecycle events.

This means stepper motion is queued and then advanced incrementally in the loop rather than completed inside the request handler itself.

## Requirements

- ESP32 development board
- PlatformIO
- Arduino framework for ESP32
- Wi‑Fi network the ESP32 can join
- a compatible external stepper driver if you want to use the stepper features

## Configuration

This project expects a local secrets file that is intentionally ignored by Git:

- `src/config/secrets.h`

If that file is missing, the firmware now falls back to the committed template at `src/config/secrets.example.h` so the project still compiles with placeholder values.

For local development, copy the template to `src/config/secrets.h` and replace the placeholders before flashing.

### Example `src/config/secrets.h`

```cpp
#pragma once

#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define MDNS_HOSTNAME "esp32"
```

Notes:

- `MDNS_HOSTNAME` becomes `http://<hostname>.local`
- the current firmware advertises only the HTTP service through mDNS
- the WebSocket server is still reachable at the same host on port `81`

## Build and flash

From the `esp` folder:

### Build

```bash
pio run
```

### Upload

```bash
pio run --target upload
```

### Open serial monitor

```bash
pio device monitor
```

## PlatformIO configuration

Current environment summary:

- board: `esp32dev`
- framework: `arduino`
- monitor speed: `115200`

Libraries in use:

- `ArduinoJson`
- `WebSockets`
- `FastAccelStepper`

## Startup behavior

At runtime, you should see logs similar to:

- boot message with stepper support
- Wi‑Fi connect progress
- assigned IP address
- mDNS hostname announcement
- WebSocket server startup on port `81`

Hardware details currently wired into the firmware:

- onboard LED on pin `2`
- stepper step pin `25`
- stepper direction pin `26`
- stepper enable pin `27`

Tested motion hardware:

- controller board: ESP32-DevKitC-V4 compatible ESP32-WROOM-32D development board with USB-C and CP2102
- stepper driver: BIGTREETECH TMC2209 V1.3
- actuator: OpenBuilds Belt Driven V-Slot NEMA17 Linear Actuator Bundle
- motor: NEMA17 motor included with the OpenBuilds actuator bundle

Driver assumptions in the current firmware:

- the TMC2209 is driven in standalone STEP/DIR/EN mode
- UART configuration and tuning are not implemented in this project
- the enable line is assumed to be active-low
- `delta` values are raw steps, not millimeters
- travel per command depends on the TMC2209 microstep setting and the actuator mechanics

The main loop also broadcasts WebSocket ping frames approximately every 3 seconds.

## HTTP API

Base URL examples:

- `http://esp32.local`
- `http://192.168.x.x`

All HTTP responses include permissive CORS headers:

- `Access-Control-Allow-Origin: *`
- `Access-Control-Allow-Methods: GET,POST,OPTIONS`
- `Access-Control-Allow-Headers: Content-Type,Authorization`

### `GET /health`

Returns a simple liveness response.

#### Example response

```json
{
	"status": "ok"
}
```

### `GET /system/info`

Returns runtime and network state.

#### Example response

```json
{
	"ip": "192.168.1.42",
	"rssi": -55,
	"uptimeMs": 123456,
	"freeHeap": 215432
}
```

### `GET /system/status`

Legacy alias for `GET /system/info`.

### `POST /led/blink`

Pulses the onboard LED.

#### Request body

```json
{
	"onMs": 100,
	"offMs": 100,
	"times": 3
}
```

Defaults used by the REST handler if fields are omitted:

- `onMs = 100`
- `offMs = 100`
- `times = 3`

#### Example response

```json
{
	"status": "ok",
	"command": "blink",
	"onMs": 100,
	"offMs": 100,
	"times": 3
}
```

### `POST /stepper/jog`

Queues a relative stepper move.

#### Request body

```json
{
	"delta": 160,
	"speed": 800
}
```

Defaults used if fields are omitted:

- `delta = 80`
- `speed = 800`

#### Example response

```json
{
	"status": "ok",
	"command": "stepper_jog",
	"delta": 160,
	"speed": 800
}
```

### REST error responses

The JSON POST endpoints return `400` for malformed bodies.

#### Missing or empty body

```json
{
	"error": "Missing JSON body"
}
```

or

```json
{
	"error": "Empty request body"
}
```

#### Invalid JSON

```json
{
	"error": "Invalid JSON",
	"code": "PARSE_ERROR"
}
```

### `OPTIONS`

`OPTIONS` is registered for the available REST routes and returns `204` for CORS preflight support.

## WebSocket API

WebSocket endpoint:

```text
ws://<host>:81
```

Examples:

- `ws://esp32.local:81`
- `ws://192.168.1.42:81`

### Connection handshake

When a client connects, the server immediately sends:

```json
{
	"type": "connected",
	"client": 0,
	"version": "1.0",
	"capabilities": {
		"protocolVersion": "1.0"
	}
}
```

Most clients should wait for this message before sending commands.

### Request format

Clients send JSON messages with a `command` field.

Command handlers preserve request `id` values in their response envelopes and in emitted stepper lifecycle events when an `id` is supplied.

#### Generic format

```json
{
	"id": 1,
	"command": "status"
}
```

### Supported commands

#### `blink`

Pulse the onboard LED.

##### Request

```json
{
	"id": 1,
	"command": "blink",
	"onMs": 100,
	"offMs": 100,
	"times": 5
}
```

Defaults used by the WebSocket handler if omitted:

- `onMs = 50`
- `offMs = 50`
- `times = 10`

##### Response

```json
{
	"id": 1,
	"status": "ok",
	"command": "blink",
	"onMs": 100,
	"offMs": 100,
	"times": 5
}
```

#### `status`

Return runtime information.

##### Request

```json
{
	"id": 2,
	"command": "status"
}
```

##### Response

```json
{
	"id": 2,
	"status": "ok",
	"uptime": 123456,
	"freeHeap": 215432,
	"ip": "192.168.1.42",
	"rssi": -55
}
```

#### `stepper_jog`

Queue a relative stepper move.

##### Request

```json
{
	"command": "stepper_jog",
	"delta": 160,
	"speed": 800
}
```

Defaults used by the handler if omitted:

- `delta = 160`
- `speed = 800`

##### Response

```json
{
	"status": "ok",
	"command": "stepper_jog",
	"delta": 160,
	"speed": 800
}
```

Important:

- a successful `stepper_jog` response confirms that the ESP32 accepted the command
- it does not confirm that the axis physically moved
- the example value `delta = 160` is only a transport example and should not be treated as a fixed travel distance without calibration

### WebSocket control traffic

The server periodically broadcasts WebSocket ping frames. These are protocol-level keepalive frames, not JSON application messages.

### Error responses

#### Invalid JSON

```json
{
	"error": "Invalid JSON",
	"code": "PARSE_ERROR"
}
```

#### Unknown command

```json
{
	"error": "Unknown command",
	"code": "UNKNOWN_COMMAND",
	"received": "example"
}
```

## Project structure

```text
esp/
├─ platformio.ini
├─ src/
│  ├─ main.cpp
│  ├─ core/
│  │  ├─ AppServer.*
│  │  ├─ Led.h
│  │  ├─ MDNSManager.*
│  │  ├─ StepperController.*
│  │  └─ WiFiManager.*
│  ├─ rest_endpoints/
│  │  ├─ endpoints/
│  │  │  ├─ Health.*
│  │  │  ├─ Led.*
│  │  │  ├─ Stepper.*
│  │  │  └─ System.*
│  │  └─ routing/
│  │     ├─ Endpoints.h
│  │     └─ Routes.cpp
│  └─ websocket/
│     ├─ Commands.h
│     ├─ MessageRouter.*
│     ├─ WebSocketManager.*
│     └─ commands/
│        ├─ Blink.*
│        ├─ Status.*
│        └─ Stepper.*
```

## How this project works with `espcontrol`

The recommended division of responsibility is:

- `esp` runs on the device and exposes transport-level APIs
- `espcontrol` runs on the host machine and handles discovery, connection management, and command invocation
- your UI talks to `espcontrol` or imports its modules directly

Typical UI flow:

1. power on the ESP32
2. firmware joins Wi‑Fi and becomes reachable at `http://<hostname>.local`
3. the host application resolves the hostname and checks `GET /health`
4. the host or UI reads `GET /system/info` or opens a WebSocket session
5. the UI sends `blink`, `status`, or `stepper_jog` requests and renders the response

## Known limitations and current gaps

These are important for anyone building on the current code:

1. There is no authentication.
2. There is no HTTPS or secure WebSocket transport.
3. mDNS advertises the HTTP service, but not a WebSocket-specific service record.
4. `blink` handlers are synchronous and block while pulsing the LED.
5. Motion remains single-axis only; there is no homing, endstop, or multi-axis planner.
6. Stepper pins and the default motion profile are currently hardcoded in firmware.
7. TMC2209 UART-only diagnostics and tuning are not implemented.
8. The companion `espcontrol` project still includes a legacy `/run` helper that this firmware does not implement.

## Motion setup notes

For the documented TMC2209 and OpenBuilds belt actuator combination:

- verify motor power on the driver VM input before testing any move command
- keep ESP32 ground and driver logic ground common
- set the driver current limit for the installed NEMA17 motor before running loaded motion
- confirm the TMC2209 microstep configuration, because it changes the effective travel for a given `delta`
- if direction is reversed, swap motor coil wiring or invert the direction signal in firmware

The firmware currently assumes only basic step and direction signaling. It does not read back driver state, detect stalls, or configure TMC2209 features over UART.

## Troubleshooting

### `stepper_jog` returns `ok` but the motor does not move

- confirm the TMC2209 has motor supply power as well as logic power
- confirm the enable input polarity matches the firmware assumption that `EN` is active-low
- confirm the ESP32, driver, and power supply share a common ground
- confirm the TMC2209 is in standalone STEP/DIR mode if UART is not wired and configured
- confirm the motor current limit is high enough for the OpenBuilds NEMA17 motor
- confirm the motor actually locks or resists turning by hand when the driver is enabled
- inspect the STEP signal on GPIO `25` with a scope or logic analyzer if the command is accepted but motion is still absent
- if the motor twitches but does not travel, reduce speed, verify coil pairing, and re-check microstep settings

## Extending the firmware

To add a new REST endpoint:

1. add a handler under `src/rest_endpoints/endpoints`
2. include it in `src/rest_endpoints/routing/Endpoints.h`
3. register the route in `src/rest_endpoints/routing/Routes.cpp`

To add a new WebSocket command:

1. add a handler under `src/websocket/commands`
2. include it from `src/websocket/Commands.h` or directly where routed
3. route the new `command` name in `src/websocket/MessageRouter.cpp`
4. update `espcontrol` so the host-side client has a matching wrapper if needed

## Troubleshooting

### Device does not connect to Wi‑Fi

- verify `WIFI_SSID` and `WIFI_PASSWORD`
- confirm the ESP32 is within range
- check the serial monitor for connection retries

### `esp32.local` does not resolve

- ensure mDNS works on your local network
- try the device IP from the serial output instead
- some Windows environments require Bonjour or equivalent mDNS support

### HTTP works but WebSocket fails

- confirm port `81` is reachable
- confirm the client is using `ws://<host>:81`
- wait for the initial `connected` message before sending commands

### `stepper_jog` appears to do nothing

- verify the driver wiring on pins `25`, `26`, and `27`
- verify the stepper driver enable polarity matches the current firmware assumptions
- try a small move first, then inspect serial logs and power delivery

### UI command returns `UNKNOWN_COMMAND`

- verify the command exists in `src/websocket/MessageRouter.cpp`
- confirm the host-side code matches the firmware version flashed to the device

## Current firmware status

This firmware is a solid base for an ESP32 control plane and is ready for:

- health checks
- device status display in a UI
- simple LED control tests
- basic stepper jogging

If you want a richer system next, the most useful additions are:

- more device commands
- stepper completion/progress events
- non-blocking LED command handling
- structured shared request/response schemas
- authentication for less-trusted networks