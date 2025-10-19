#pragma once
#include <Arduino.h>

namespace Core {
  class Led {
  public:
    Led(int pin, bool activeLow = false) : pin_(pin), activeLow_(activeLow) {}
    void begin() {
      pinMode(pin_, OUTPUT);
      write(false);
    }
    void write(bool on) {
      digitalWrite(pin_, activeLow_ ? !on : on);
    }
    void pulse(unsigned long onMs = 100, unsigned long offMs = 100, int times = 1) {
      for (int i = 0; i < times; i++) {
        write(true);
        delay(onMs);
        write(false);
        delay(offMs);
      }
    }
  private:
    int pin_;
    bool activeLow_;
  };
}
