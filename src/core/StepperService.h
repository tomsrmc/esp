#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "StepperController.h"

namespace Core {

class StepperService {
public:
    static constexpr const char* kProtocolVersion = "1.0";
    static constexpr const char* kAxisName = "main";

    enum class EventKind {
        Started,
        Completed,
        Stopped,
        Fault,
    };

    StepperService(StepperController& stepper, int feedbackPin = -1);

    void begin();
    void loop();

    int jog(long delta, float speed, bool hasRequestId, uint32_t requestId, JsonDocument& response);
    int getStatus(bool hasRequestId, uint32_t requestId, JsonDocument& response);
    int stop(bool immediate, bool hasRequestId, uint32_t requestId, JsonDocument& response);
    int configure(bool hasMaxSpeed, float maxSpeed, bool hasAcceleration, float acceleration,
                  bool hasRequestId, uint32_t requestId, JsonDocument& response);
    int getCapabilities(bool hasRequestId, uint32_t requestId, JsonDocument& response);

    void fillStatus(JsonObject data);
    void fillCapabilities(JsonObject data) const;
    bool consumePendingEvent(JsonDocument& event);

private:
    struct PendingEvent {
        bool active = false;
        EventKind kind = EventKind::Completed;
        bool hasRequestId = false;
        uint32_t requestId = 0;
        String code;
        String message;
    };

    static constexpr size_t kEventQueueSize = 6;

    void updateFeedback();
    void queueFeedback(int pulses, unsigned long onMs = 40, unsigned long offMs = 40);
    void enqueueEvent(EventKind kind, const char* code, const char* message,
                      bool hasRequestId, uint32_t requestId);
    void initResponse(JsonDocument& response, const char* command, const char* status,
                      const char* code, const char* message, bool hasRequestId,
                      uint32_t requestId) const;
    bool validateMotionConfig(float maxSpeed, float acceleration, JsonDocument& response,
                              const char* command, bool hasRequestId, uint32_t requestId) const;

    StepperController& stepper_;
    int feedbackPin_;
    bool wasMoving_;
    bool stopRequested_;
    bool activeMotionHasRequestId_;
    uint32_t activeMotionRequestId_;
    bool feedbackState_;
    int feedbackTransitionsRemaining_;
    unsigned long feedbackOnMs_;
    unsigned long feedbackOffMs_;
    unsigned long nextFeedbackToggleAtMs_;
    PendingEvent pendingEvents_[kEventQueueSize];
    size_t eventHead_;
    size_t eventTail_;
    size_t eventCount_;
};

}