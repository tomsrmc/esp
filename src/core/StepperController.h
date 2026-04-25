#pragma once

#include <FastAccelStepper.h>

namespace Core {

struct StepperState {
    long currentPosition;
    long targetPosition;
    long distanceToGo;
    bool moving;
    bool enabled;
    float currentSpeed;
    float maxSpeed;
    float acceleration;
};

class StepperController {
public:
    static constexpr float kMinSpeed = 1.0f;
    static constexpr float kDefaultSpeed = 24000.0f;
    static constexpr float kMaxSpeed = 100000.0f;
    static constexpr float kMinAcceleration = 1.0f;
    static constexpr float kDefaultAcceleration = 250000.0f;
    static constexpr float kMaxAcceleration = 800000.0f;
    static constexpr uint16_t kDirectionChangeDelayMicros = 2U;
    static constexpr uint16_t kEnableDelayMicros = 20U;

    StepperController(int stepPin, int dirPin, int enPin);
    void begin();
    void jog(long delta);
    void loop();
    void setSpeed(float speed);
    bool setMaxSpeed(float speed);
    bool setAcceleration(float acceleration);
    void stop();
    void immediateStop();
    StepperState getState();
    long currentPosition();
    long targetPosition();
    long distanceToGo();
    float currentSpeed();
    float maxSpeed();
    float acceleration();
    bool isMoving();
    bool isEnabled() const;
    static bool isValidMaxSpeed(float speed);
    static bool isValidAcceleration(float acceleration);

private:
    void applyMotionProfile();
    StepperState buildState() const;

    FastAccelStepperEngine engine_;
    FastAccelStepper* stepper_;
    int stepPin_;
    int dirPin_;
    int enPin_;
    bool enabled_;
    bool ready_;
    int32_t lastCommandedTarget_;
    float configuredMaxSpeed_;
    float configuredAcceleration_;
};
}
