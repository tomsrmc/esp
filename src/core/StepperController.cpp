
#include "StepperController.h"
#include <Arduino.h>

namespace Core {

StepperController::StepperController(int stepPin, int dirPin, int enPin)
    : stepper_(AccelStepper::DRIVER, stepPin, dirPin), enPin_(enPin) {}

void StepperController::begin() {
    pinMode(enPin_, OUTPUT);
    digitalWrite(enPin_, LOW); // Enable driver
    stepper_.setMaxSpeed(800);
    stepper_.setAcceleration(600);
    digitalWrite(enPin_, LOW);
    delay(300);
}

void StepperController::jog(long delta) {
    stepper_.move(delta);
}

void StepperController::loop() {
    if (stepper_.distanceToGo() != 0) {
        stepper_.run();
    }
}

void StepperController::setSpeed(float speed) {
    stepper_.setMaxSpeed(speed);
}

} // namespace Core
