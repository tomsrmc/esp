#include <Arduino.h>
#include <WiFi.h>
#include <Firmata.h>
#include "config/secrets.h"

// Firmata over WiFi configuration
WiFiServer server(3030);  // Standard Firmata port
bool connectedToWiFi = false;

void wifiConnect() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  connectedToWiFi = true;
  Serial.println();
  Serial.print("WiFi connected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Firmata server starting on port 3030");
  Serial.println();
  
  server.begin();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 WiFi Firmata starting...");

  // Connect to WiFi
  wifiConnect();
  
  // Configure Firmata
  Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);
  Firmata.attach(ANALOG_MESSAGE, analogWriteCallback);
  Firmata.attach(DIGITAL_MESSAGE, digitalWriteCallback);
  Firmata.attach(REPORT_ANALOG, reportAnalogCallback);
  Firmata.attach(REPORT_DIGITAL, reportDigitalCallback);
  Firmata.attach(SET_PIN_MODE, setPinModeCallback);
  Firmata.attach(SET_DIGITAL_PIN_VALUE, setPinValueCallback);
  Firmata.attach(START_SYSEX, sysexCallback);
  Firmata.attach(SYSTEM_RESET, systemResetCallback);

  // Initialize pins
  for (int pin = 0; pin < TOTAL_PINS; pin++) {
    if (IS_PIN_DIGITAL(pin)) {
      setPinModeCallback(pin, OUTPUT);
    }
  }

  Serial.println("Firmata initialized. Waiting for connections...");
}

void loop() {
  // Handle WiFi client connections
  static WiFiClient client;
  
  if (!client.connected()) {
    client = server.available();
    if (client) {
      Serial.println("Client connected");
      Firmata.begin(client);
    }
  }
  
  if (client.connected()) {
    while (Firmata.available()) {
      Firmata.processInput();
    }
  }
  
  // Handle reporting
  if (client.connected()) {
    for (int pin = 0; pin < TOTAL_PINS; pin++) {
      if (IS_PIN_DIGITAL(pin) && Firmata.getPinMode(pin) == INPUT) {
        if (digitalRead(pin) != Firmata.getPinState(pin)) {
          Firmata.setPinState(pin, digitalRead(pin));
          Firmata.sendDigitalPort(pin / 8, readPort(pin / 8, 0xFF));
        }
      }
    }
  }
}

// Firmata callback functions
void analogWriteCallback(byte pin, int value) {
  if (IS_PIN_PWM(pin)) {
    analogWrite(pin, value);
    Firmata.setPinState(pin, value);
  }
}

void digitalWriteCallback(byte port, int value) {
  byte pin, lastPin, mask = 1, pinWriteMask = 0;

  if (port < TOTAL_PORTS) {
    lastPin = port * 8 + 8;
    if (lastPin > TOTAL_PINS) lastPin = TOTAL_PINS;
    for (pin = port * 8; pin < lastPin; pin++) {
      if (Firmata.getPinMode(pin) == OUTPUT || Firmata.getPinMode(pin) == INPUT_PULLUP) {
        pinWriteMask |= mask;
        digitalWrite(pin, (value & mask) ? HIGH : LOW);
        Firmata.setPinState(pin, (value & mask) ? HIGH : LOW);
      }
      mask = mask << 1;
    }
  }
}

void setPinModeCallback(byte pin, int mode) {
  if (IS_PIN_DIGITAL(pin)) {
    switch (mode) {
      case INPUT:
        pinMode(pin, INPUT);
        Firmata.setPinMode(pin, INPUT);
        break;
      case INPUT_PULLUP:
        pinMode(pin, INPUT_PULLUP);
        Firmata.setPinMode(pin, INPUT_PULLUP);
        break;
      case OUTPUT:
        pinMode(pin, OUTPUT);
        Firmata.setPinMode(pin, OUTPUT);
        break;
      case PWM:
        if (IS_PIN_PWM(pin)) {
          pinMode(pin, OUTPUT);
          Firmata.setPinMode(pin, PWM);
        }
        break;
    }
  }
}

void setPinValueCallback(byte pin, int value) {
  if (Firmata.getPinMode(pin) == OUTPUT) {
    digitalWrite(pin, value);
    Firmata.setPinState(pin, value);
  }
}

void reportAnalogCallback(byte analogPin, int value) {
  // Enable/disable analog input reporting
}

void reportDigitalCallback(byte port, int value) {
  // Enable/disable digital port reporting
}

void sysexCallback(byte command, byte argc, byte *argv) {
  // Handle system exclusive messages
}

void systemResetCallback() {
  // Reset system
  for (int pin = 0; pin < TOTAL_PINS; pin++) {
    if (IS_PIN_DIGITAL(pin)) {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
      Firmata.setPinMode(pin, OUTPUT);
      Firmata.setPinState(pin, LOW);
    }
  }
}

int readPort(int port, int bitmask) {
  int out = 0, pin = port * 8;
  if (IS_PIN_DIGITAL(pin + 0) && (bitmask & 0x01) && Firmata.getPinMode(pin + 0) != OUTPUT) out |= digitalRead(pin + 0) << 0;
  if (IS_PIN_DIGITAL(pin + 1) && (bitmask & 0x02) && Firmata.getPinMode(pin + 1) != OUTPUT) out |= digitalRead(pin + 1) << 1;
  if (IS_PIN_DIGITAL(pin + 2) && (bitmask & 0x04) && Firmata.getPinMode(pin + 2) != OUTPUT) out |= digitalRead(pin + 2) << 2;
  if (IS_PIN_DIGITAL(pin + 3) && (bitmask & 0x08) && Firmata.getPinMode(pin + 3) != OUTPUT) out |= digitalRead(pin + 3) << 3;
  if (IS_PIN_DIGITAL(pin + 4) && (bitmask & 0x10) && Firmata.getPinMode(pin + 4) != OUTPUT) out |= digitalRead(pin + 4) << 4;
  if (IS_PIN_DIGITAL(pin + 5) && (bitmask & 0x20) && Firmata.getPinMode(pin + 5) != OUTPUT) out |= digitalRead(pin + 5) << 5;
  if (IS_PIN_DIGITAL(pin + 6) && (bitmask & 0x40) && Firmata.getPinMode(pin + 6) != OUTPUT) out |= digitalRead(pin + 6) << 6;
  if (IS_PIN_DIGITAL(pin + 7) && (bitmask & 0x80) && Firmata.getPinMode(pin + 7) != OUTPUT) out |= digitalRead(pin + 7) << 7;
  return out;
}
