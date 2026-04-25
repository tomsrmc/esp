
#include "StepperController.h"
#include <Arduino.h>

namespace Core {

StepperController::StepperController(int stepPin, int dirPin, int enPin)
    : stepper_(nullptr),
      stepPin_(stepPin),
      dirPin_(dirPin),
      enPin_(enPin),
      enabled_(false),
      ready_(false),
      lastCommandedTarget_(0),
      configuredMaxSpeed_(kDefaultSpeed),
      configuredAcceleration_(kDefaultAcceleration) {}

void StepperController::begin() {
    engine_.init();
    stepper_ = engine_.stepperConnectToPin(stepPin_);
    if (stepper_ == nullptr) {
        Serial.printf("Stepper init failed: step pin %d could not be attached\n", stepPin_);
        enabled_ = false;
        ready_ = false;
        return;
    }

    stepper_->setDirectionPin(dirPin_, true, kDirectionChangeDelayMicros);
    stepper_->setEnablePin(enPin_, true);
    stepper_->setAutoEnable(true);
    stepper_->setDelayToEnable(kEnableDelayMicros);
    stepper_->setDelayToDisable(4);
    applyMotionProfile();

    stepper_->forceStopAndNewPosition(0);
    lastCommandedTarget_ = 0;
    enabled_ = true;
    ready_ = true;
    delay(300);
    Serial.printf("Stepper ready: stepPin=%d dirPin=%d enablePin=%d maxSpeed=%.1f accel=%.1f dirDelayUs=%u\n",
                  stepPin_, dirPin_, enPin_, configuredMaxSpeed_, configuredAcceleration_,
                  kDirectionChangeDelayMicros);
}

void StepperController::jog(long delta) {
    if (!ready_) {
        return;
    }

    stepper_->move(delta);
    lastCommandedTarget_ += static_cast<int32_t>(delta);
    Serial.printf("Stepper jog queued: delta=%ld target=%ld remaining=%ld\n",
                  delta, static_cast<long>(lastCommandedTarget_),
                  static_cast<long>(distanceToGo()));
}

void StepperController::loop() {
    if (!ready_) {
        return;
    }
}

void StepperController::setSpeed(float speed) {
    setMaxSpeed(speed);
}

bool StepperController::setMaxSpeed(float speed) {
    if (!isValidMaxSpeed(speed)) {
        return false;
    }
    configuredMaxSpeed_ = speed;
    if (ready_) {
        applyMotionProfile();
    }
    return true;
}

bool StepperController::setAcceleration(float acceleration) {
    if (!isValidAcceleration(acceleration)) {
        return false;
    }
    configuredAcceleration_ = acceleration;
    if (ready_) {
        applyMotionProfile();
    }
    return true;
}

void StepperController::stop() {
    if (!ready_) {
        return;
    }

    stepper_->stopMove();
    lastCommandedTarget_ = stepper_->targetPos();
}

void StepperController::immediateStop() {
    if (!ready_) {
        return;
    }

    int32_t position = stepper_->getCurrentPosition();
    stepper_->forceStopAndNewPosition(position);
    lastCommandedTarget_ = position;
}

StepperState StepperController::getState() {
    return buildState();
}

long StepperController::currentPosition() {
    return buildState().currentPosition;
}

long StepperController::targetPosition() {
    return buildState().targetPosition;
}

long StepperController::distanceToGo() {
    StepperState state = buildState();
    return state.distanceToGo;
}

float StepperController::currentSpeed() {
    return buildState().currentSpeed;
}

float StepperController::maxSpeed() {
    return configuredMaxSpeed_;
}

float StepperController::acceleration() {
    return configuredAcceleration_;
}

bool StepperController::isMoving() {
    return buildState().moving;
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

void StepperController::applyMotionProfile() {
    if (!ready_ || stepper_ == nullptr) {
        return;
    }

    stepper_->setSpeedInHz(static_cast<uint32_t>(configuredMaxSpeed_));
    stepper_->setAcceleration(static_cast<uint32_t>(configuredAcceleration_));
}

StepperState StepperController::buildState() const {
    if (!ready_ || stepper_ == nullptr) {
        return {
            0,
            lastCommandedTarget_,
            lastCommandedTarget_,
            false,
            false,
            0.0f,
            configuredMaxSpeed_,
            configuredAcceleration_,
        };
    }

    int32_t current = stepper_->getCurrentPosition();
    int32_t target = stepper_->targetPos();
    int32_t remaining = target - current;
    bool moving = stepper_->isRunning() || remaining != 0;

    return {
        current,
        target,
        remaining,
        moving,
        enabled_,
        moving ? configuredMaxSpeed_ : 0.0f,
        configuredMaxSpeed_,
        configuredAcceleration_,
    };
}

} // namespace Core
