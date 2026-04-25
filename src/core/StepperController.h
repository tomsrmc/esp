#pragma once
#include <AccelStepper.h>

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
    static constexpr float kDefaultSpeed = 6000.0f;
    static constexpr float kMaxSpeed = 12000.0f;
    static constexpr float kMinAcceleration = 1.0f;
    static constexpr float kDefaultAcceleration = 60000.0f;
    static constexpr float kMaxAcceleration = 120000.0f;

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
    AccelStepper stepper_;
    int enPin_;
    bool enabled_;
};
}
