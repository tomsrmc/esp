#include "StepperService.h"

namespace Core {
namespace {

const char* eventNameFor(StepperService::EventKind kind) {
    switch (kind) {
        case StepperService::EventKind::Started:
            return "stepper.started";
        case StepperService::EventKind::Completed:
            return "stepper.completed";
        case StepperService::EventKind::Stopped:
            return "stepper.stopped";
        case StepperService::EventKind::Fault:
            return "stepper.fault";
    }
    return "stepper.unknown";
}

}

StepperService::StepperService(StepperController& stepper, int feedbackPin)
    : stepper_(stepper),
      feedbackPin_(feedbackPin),
      wasMoving_(false),
      stopRequested_(false),
      activeMotionHasRequestId_(false),
      activeMotionRequestId_(0),
      feedbackState_(false),
      feedbackTransitionsRemaining_(0),
      feedbackOnMs_(40),
      feedbackOffMs_(40),
      nextFeedbackToggleAtMs_(0),
      eventHead_(0),
      eventTail_(0),
      eventCount_(0) {}

void StepperService::begin() {
    if (feedbackPin_ >= 0) {
        pinMode(feedbackPin_, OUTPUT);
        digitalWrite(feedbackPin_, LOW);
    }
    wasMoving_ = stepper_.isMoving();
}

void StepperService::loop() {
    stepper_.loop();
    updateFeedback();

    bool moving = stepper_.isMoving();
    if (!wasMoving_ && moving) {
        enqueueEvent(EventKind::Started, "STEPPER_STARTED", "Motion started",
                     activeMotionHasRequestId_, activeMotionRequestId_);
    } else if (wasMoving_ && !moving) {
        if (stopRequested_) {
            enqueueEvent(EventKind::Stopped, "STEPPER_STOPPED", "Motion stopped",
                         activeMotionHasRequestId_, activeMotionRequestId_);
        } else {
            enqueueEvent(EventKind::Completed, "STEPPER_COMPLETED", "Motion completed",
                         activeMotionHasRequestId_, activeMotionRequestId_);
        }
        stopRequested_ = false;
        activeMotionHasRequestId_ = false;
        activeMotionRequestId_ = 0;
    }
    wasMoving_ = moving;
}

int StepperService::jog(long delta, float speed, bool hasRequestId, uint32_t requestId,
                        JsonDocument& response) {
    if (delta == 0) {
        initResponse(response, "stepper_jog", "error", "INVALID_DELTA",
                     "delta must be non-zero", hasRequestId, requestId);
        enqueueEvent(EventKind::Fault, "INVALID_DELTA", "Rejected jog with zero delta",
                     hasRequestId, requestId);
        return 400;
    }

    if (!validateMotionConfig(speed, stepper_.acceleration(), response, "stepper_jog",
                              hasRequestId, requestId)) {
        enqueueEvent(EventKind::Fault, "INVALID_SPEED", "Rejected jog outside speed limits",
                     hasRequestId, requestId);
        return 400;
    }

    stepper_.setMaxSpeed(speed);
    stepper_.jog(delta);
    activeMotionHasRequestId_ = hasRequestId;
    activeMotionRequestId_ = requestId;
    stopRequested_ = false;
    queueFeedback(2);

    initResponse(response, "stepper_jog", "ok", "STEPPER_JOG_ACCEPTED",
                 "Jog queued", hasRequestId, requestId);
    JsonObject data = response["data"].to<JsonObject>();
    data["delta"] = delta;
    data["requestedSpeed"] = speed;
    fillStatus(data.createNestedObject("stepper"));
    return 200;
}

int StepperService::getStatus(bool hasRequestId, uint32_t requestId, JsonDocument& response) {
    initResponse(response, "stepper_status", "ok", "STEPPER_STATUS",
                 "Stepper status", hasRequestId, requestId);
    JsonObject data = response["data"].to<JsonObject>();
    fillStatus(data.createNestedObject("stepper"));
    return 200;
}

int StepperService::stop(bool immediate, bool hasRequestId, uint32_t requestId,
                         JsonDocument& response) {
    if (immediate) {
        stepper_.immediateStop();
    } else {
        stepper_.stop();
    }
    stopRequested_ = true;
    queueFeedback(1);

    initResponse(response, "stepper_stop", "ok", "STEPPER_STOP_REQUESTED",
                 immediate ? "Immediate stop requested" : "Stop requested",
                 hasRequestId, requestId);
    JsonObject data = response["data"].to<JsonObject>();
    data["immediate"] = immediate;
    fillStatus(data.createNestedObject("stepper"));
    return 200;
}

int StepperService::configure(bool hasMaxSpeed, float maxSpeed,
                              bool hasAcceleration, float acceleration,
                              bool hasRequestId, uint32_t requestId,
                              JsonDocument& response) {
    if (!hasMaxSpeed && !hasAcceleration) {
        initResponse(response, "stepper_config", "ok", "STEPPER_CONFIG",
                     "Current motion config", hasRequestId, requestId);
        JsonObject data = response["data"].to<JsonObject>();
        fillStatus(data.createNestedObject("stepper"));
        fillCapabilities(data.createNestedObject("capabilities"));
        return 200;
    }

    float nextMaxSpeed = hasMaxSpeed ? maxSpeed : stepper_.maxSpeed();
    float nextAcceleration = hasAcceleration ? acceleration : stepper_.acceleration();
    if (!validateMotionConfig(nextMaxSpeed, nextAcceleration, response, "stepper_config",
                              hasRequestId, requestId)) {
        enqueueEvent(EventKind::Fault, "INVALID_CONFIG",
                     "Rejected config outside safe limits", hasRequestId, requestId);
        return 400;
    }

    if (hasMaxSpeed) {
        stepper_.setMaxSpeed(maxSpeed);
    }
    if (hasAcceleration) {
        stepper_.setAcceleration(acceleration);
    }

    initResponse(response, "stepper_config", "ok", "STEPPER_CONFIG_UPDATED",
                 "Motion config updated", hasRequestId, requestId);
    JsonObject data = response["data"].to<JsonObject>();
    fillStatus(data.createNestedObject("stepper"));
    fillCapabilities(data.createNestedObject("capabilities"));
    return 200;
}

int StepperService::getCapabilities(bool hasRequestId, uint32_t requestId,
                                    JsonDocument& response) {
    initResponse(response, "capabilities", "ok", "CAPABILITIES",
                 "Firmware capabilities", hasRequestId, requestId);
    JsonObject data = response["data"].to<JsonObject>();
    fillCapabilities(data);
    return 200;
}

void StepperService::fillStatus(JsonObject data) {
    StepperState state = stepper_.getState();
    data["axis"] = kAxisName;
    data["currentPosition"] = state.currentPosition;
    data["targetPosition"] = state.targetPosition;
    data["distanceToGo"] = state.distanceToGo;
    data["moving"] = state.moving;
    data["enabled"] = state.enabled;
    data["currentSpeed"] = state.currentSpeed;
    data["maxSpeed"] = state.maxSpeed;
    data["acceleration"] = state.acceleration;
}

void StepperService::fillCapabilities(JsonObject data) const {
    data["protocolVersion"] = kProtocolVersion;
    data["axis"] = kAxisName;

    JsonArray commands = data["commands"].to<JsonArray>();
    commands.add("stepper_jog");
    commands.add("stepper_status");
    commands.add("stepper_stop");
    commands.add("stepper_config");
    commands.add("capabilities");

    JsonArray events = data["events"].to<JsonArray>();
    events.add("stepper.started");
    events.add("stepper.completed");
    events.add("stepper.stopped");
    events.add("stepper.fault");

    JsonObject limits = data["limits"].to<JsonObject>();
    limits["minSpeed"] = StepperController::kMinSpeed;
    limits["maxSpeed"] = StepperController::kMaxSpeed;
    limits["minAcceleration"] = StepperController::kMinAcceleration;
    limits["maxAcceleration"] = StepperController::kMaxAcceleration;

    JsonObject transport = data["transport"].to<JsonObject>();
    transport["rest"] = true;
    transport["websocket"] = true;
}

bool StepperService::consumePendingEvent(JsonDocument& event) {
    if (eventCount_ == 0) {
        return false;
    }

    PendingEvent pending = pendingEvents_[eventHead_];
    pendingEvents_[eventHead_].active = false;
    eventHead_ = (eventHead_ + 1) % kEventQueueSize;
    --eventCount_;

    event.clear();
    event["type"] = "event";
    event["event"] = eventNameFor(pending.kind);
    event["version"] = kProtocolVersion;
    event["axis"] = kAxisName;
    event["code"] = pending.code;
    event["message"] = pending.message;
    if (pending.hasRequestId) {
        event["id"] = pending.requestId;
    }

    JsonObject data = event["data"].to<JsonObject>();
    fillStatus(data.createNestedObject("stepper"));
    return true;
}

void StepperService::updateFeedback() {
    if (feedbackPin_ < 0 || feedbackTransitionsRemaining_ <= 0) {
        return;
    }

    unsigned long now = millis();
    if (now < nextFeedbackToggleAtMs_) {
        return;
    }

    feedbackState_ = !feedbackState_;
    digitalWrite(feedbackPin_, feedbackState_ ? HIGH : LOW);
    --feedbackTransitionsRemaining_;

    if (feedbackTransitionsRemaining_ <= 0 && !feedbackState_) {
        digitalWrite(feedbackPin_, LOW);
        return;
    }

    nextFeedbackToggleAtMs_ = now + (feedbackState_ ? feedbackOnMs_ : feedbackOffMs_);
}

void StepperService::queueFeedback(int pulses, unsigned long onMs, unsigned long offMs) {
    if (feedbackPin_ < 0 || pulses <= 0) {
        return;
    }

    feedbackOnMs_ = onMs;
    feedbackOffMs_ = offMs;
    feedbackTransitionsRemaining_ += pulses * 2;
    if (feedbackTransitionsRemaining_ == pulses * 2) {
        nextFeedbackToggleAtMs_ = millis();
    }
}

void StepperService::enqueueEvent(EventKind kind, const char* code, const char* message,
                                  bool hasRequestId, uint32_t requestId) {
    if (eventCount_ == kEventQueueSize) {
        eventHead_ = (eventHead_ + 1) % kEventQueueSize;
        --eventCount_;
    }

    PendingEvent& pending = pendingEvents_[eventTail_];
    pending.active = true;
    pending.kind = kind;
    pending.hasRequestId = hasRequestId;
    pending.requestId = requestId;
    pending.code = code;
    pending.message = message;

    eventTail_ = (eventTail_ + 1) % kEventQueueSize;
    ++eventCount_;
}

void StepperService::initResponse(JsonDocument& response, const char* command,
                                  const char* status, const char* code,
                                  const char* message, bool hasRequestId,
                                  uint32_t requestId) const {
    response.clear();
    response["version"] = kProtocolVersion;
    response["axis"] = kAxisName;
    response["command"] = command;
    response["status"] = status;
    response["code"] = code;
    response["message"] = message;
    if (hasRequestId) {
        response["id"] = requestId;
    }
}

bool StepperService::validateMotionConfig(float maxSpeed, float acceleration,
                                          JsonDocument& response,
                                          const char* command,
                                          bool hasRequestId,
                                          uint32_t requestId) const {
    if (!StepperController::isValidMaxSpeed(maxSpeed)) {
        initResponse(response, command, "error", "INVALID_SPEED",
                     "maxSpeed is outside safe limits", hasRequestId, requestId);
        JsonObject data = response["data"].to<JsonObject>();
        fillCapabilities(data.createNestedObject("capabilities"));
        return false;
    }

    if (!StepperController::isValidAcceleration(acceleration)) {
        initResponse(response, command, "error", "INVALID_ACCELERATION",
                     "acceleration is outside safe limits", hasRequestId, requestId);
        JsonObject data = response["data"].to<JsonObject>();
        fillCapabilities(data.createNestedObject("capabilities"));
        return false;
    }

    return true;
}

}