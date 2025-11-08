#pragma once
#include <AccelStepper.h>

namespace Core {
class StepperController {
public:
    StepperController(int stepPin, int dirPin, int enPin);
    void begin();
    void jog(long delta);
    void loop();
    void setSpeed(float speed);
private:
    AccelStepper stepper_;
    int enPin_;
};
}
