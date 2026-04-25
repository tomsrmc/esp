# Hardware

This file documents the specific hardware currently assumed by the `esp` firmware and the companion `espcontrol` client.

## Current hardware set

- controller board: 2PCS ESP32 Type C Development Board, ESP32-DevKitC-V4 compatible, ESP32-WROOM-32D, CP2102, 2.4 GHz Wi-Fi + Bluetooth
- USB interface: USB-C connector with CP2102 USB-to-UART bridge
- stepper driver: BIGTREETECH TMC2209 V1.3
- actuator: OpenBuilds Belt Driven V-Slot NEMA17 Linear Actuator Bundle
- motor: NEMA17 motor included with the OpenBuilds actuator bundle

## ESP32 board assumptions

- MCU family: ESP32
- module: ESP32-WROOM-32D
- development board class: ESP32-DevKitC-V4 compatible
- firmware target from `platformio.ini`: `esp32dev`
- USB serial chip: CP2102
- serial monitor speed: `115200`
- flash method used by this project: USB upload through PlatformIO

Important:

- this project is not currently configured for OTA flashing
- the board must be connected over USB to build, flash, and open the serial monitor
- the firmware uses Wi-Fi for runtime control after flashing, not for firmware upload

## Firmware pin mapping

The current firmware hardcodes these pins in `src/main.cpp`:

- onboard LED: GPIO `2`
- step pulse output: GPIO `25`
- step direction output: GPIO `26`
- stepper enable output: GPIO `27`

The firmware creates the stepper controller as:

```cpp
StepperController stepper(25, 26, 27);
```

## TMC2209 operating mode

The current project uses the BIGTREETECH TMC2209 V1.3 in basic standalone motion mode:

- control mode: STEP / DIR / EN
- UART configuration: not used by the current firmware
- driver telemetry: not used by the current firmware
- stall detection: not used by the current firmware
- current tuning over software: not used by the current firmware

Current firmware assumptions:

- `EN` is active-low
- a LOW level on GPIO `27` enables the driver
- motion commands are relative jogs executed by FastAccelStepper on ESP32 hardware timers
- the firmware applies a short direction-change guard time before issuing new pulses
- the firmware uses FastAccelStepper auto-enable with a small enable lead time and delayed disable

## Wiring requirements

Minimum required logic wiring between the ESP32 and the TMC2209:

- ESP32 GPIO `25` -> TMC2209 `STEP`
- ESP32 GPIO `26` -> TMC2209 `DIR`
- ESP32 GPIO `27` -> TMC2209 `EN`
- ESP32 `GND` -> TMC2209 logic `GND`

Motor and power wiring requirements:

- connect the NEMA17 motor coils to the TMC2209 motor outputs with correct coil pairing
- provide motor supply voltage to the driver VM input
- ensure the motor power supply ground and ESP32 ground are common
- do not rely on USB power alone to run the motor driver stage

Practical notes:

- if the ESP32 accepts commands but the motor does not hold torque, suspect driver enable state, missing VM power, or current limit settings
- if the motor only vibrates or twitches, suspect incorrect coil pairing or an overly aggressive speed or microstep configuration
- if motion direction is inverted, swap motor coil orientation or invert the direction signal in firmware

## Required driver setup

Before testing motion, verify these TMC2209-specific items:

- set the driver current limit appropriately for the installed OpenBuilds NEMA17 motor
- confirm the chosen microstep mode on the driver board
- confirm the board is actually operating in standalone STEP/DIR mode if UART is not wired
- confirm the `EN` input polarity matches the current firmware assumption
- confirm cooling is adequate if the driver will run under load

The firmware does not currently auto-detect or configure any of these settings.

## Motion calibration notes

The firmware and CLI treat `delta` as raw steps:

- `delta = 160` means 160 step pulses worth of relative motion
- it does not inherently mean a fixed millimeter distance

Actual travel depends on:

- motor full steps per revolution
- TMC2209 microstep setting
- pulley tooth count
- belt pitch
- the mechanical ratio of the OpenBuilds actuator

Because of that, any statement such as `160 steps = 2 mm` is only valid after calibration on the exact hardware configuration.

## Flashing and monitoring

For this board and project:

- connect the ESP32 to the host by USB-C
- find the actual port with `pio device list` (usually `/dev/cu.usbserial-110` on this machine)
- flash with `pio run --target upload`
- monitor serial output with `pio device monitor`

Expected serial behavior after the recent stepper diagnostics changes:

- boot log at `115200`
- a stepper initialization line showing step pin, direction pin, enable pin, speed, acceleration, and direction delay
- a jog queue line each time a `stepper_jog` command is accepted

## Recommended first-power checklist

- flash the firmware over USB before testing motion
- verify Wi-Fi connection and mDNS startup in the serial monitor
- verify the driver has both logic ground and motor power
- verify the motor becomes energized when the driver is enabled
- send a small jog command first
- if the command returns `ok` but there is no motion, inspect the STEP line on GPIO `25`

## Current limitations tied to hardware

- no OTA update path is configured
- no UART integration for the TMC2209 is implemented
- no endstops or homing hardware are documented in this project yet
- no automatic calibration from steps to millimeters is implemented
- no completion or progress event is emitted when a jog finishes