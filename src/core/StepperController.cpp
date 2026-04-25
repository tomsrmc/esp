
#include "StepperController.h"
#include <Arduino.h>

namespace Core {

StepperController::StepperController(int stepPin, int dirPin, int enPin)
    : stepper_(AccelStepper::DRIVER, stepPin, dirPin), enPin_(enPin) {}

void StepperController::begin() {
    pinMode(enPin_, OUTPUT);
    digitalWrite(enPin_, LOW); // Enable driver
    stepper_.setMinPulseWidth(20);
    stepper_.setMaxSpeed(800);
    stepper_.setAcceleration(600);
    digitalWrite(enPin_, LOW);
    delay(300);
    Serial.printf("Stepper ready: enablePin=%d maxSpeed=%.1f accel=%.1f minPulseWidth=%u\n",
                  enPin_, stepper_.maxSpeed(), stepper_.acceleration(), 20U);
}

void StepperController::jog(long delta) {
    stepper_.move(delta);
    Serial.printf("Stepper jog queued: delta=%ld target=%ld remaining=%ld\n",
                  delta, stepper_.targetPosition(), stepper_.distanceToGo());
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
