
#include "StepperController.h"
#include <Arduino.h>

namespace Core {

StepperController::StepperController(int stepPin, int dirPin, int enPin)
    : stepper_(AccelStepper::DRIVER, stepPin, dirPin), enPin_(enPin), enabled_(false) {}

void StepperController::begin() {
    pinMode(enPin_, OUTPUT);
    digitalWrite(enPin_, LOW); // Enable driver
    enabled_ = true;
    stepper_.setMinPulseWidth(20);
    stepper_.setMaxSpeed(800.0f);
    stepper_.setAcceleration(600.0f);
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
    stepper_.run();
}

void StepperController::setSpeed(float speed) {
    setMaxSpeed(speed);
}

bool StepperController::setMaxSpeed(float speed) {
    if (!isValidMaxSpeed(speed)) {
        return false;
    }
    stepper_.setMaxSpeed(speed);
    return true;
}

bool StepperController::setAcceleration(float acceleration) {
    if (!isValidAcceleration(acceleration)) {
        return false;
    }
    stepper_.setAcceleration(acceleration);
    return true;
}

void StepperController::stop() {
    stepper_.stop();
}

void StepperController::immediateStop() {
    long position = stepper_.currentPosition();
    stepper_.moveTo(position);
    stepper_.setCurrentPosition(position);
    stepper_.setSpeed(0.0f);
}

StepperState StepperController::getState() {
    return {
        stepper_.currentPosition(),
        stepper_.targetPosition(),
        stepper_.distanceToGo(),
        isMoving(),
        enabled_,
        stepper_.speed(),
        stepper_.maxSpeed(),
        stepper_.acceleration(),
    };
}

long StepperController::currentPosition() {
    return stepper_.currentPosition();
}

long StepperController::targetPosition() {
    return stepper_.targetPosition();
}

long StepperController::distanceToGo() {
    return stepper_.distanceToGo();
}

float StepperController::currentSpeed() {
    return stepper_.speed();
}

float StepperController::maxSpeed() {
    return stepper_.maxSpeed();
}

float StepperController::acceleration() {
    return stepper_.acceleration();
}

bool StepperController::isMoving() {
    return stepper_.distanceToGo() != 0 || stepper_.speed() != 0.0f;
}

bool StepperController::isEnabled() const {
    return enabled_;
}

bool StepperController::isValidMaxSpeed(float speed) {
    return speed >= kMinSpeed && speed <= kMaxSpeed;
}

bool StepperController::isValidAcceleration(float acceleration) {
    return acceleration >= kMinAcceleration && acceleration <= kMaxAcceleration;
}

} // namespace Core
