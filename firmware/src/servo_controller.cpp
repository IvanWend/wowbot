#include "servo_controller.h"
#include "config_pins.h"

namespace {

constexpr int HEAD_NEUTRAL = 90;  // degrees
constexpr int ARM_NEUTRAL = 90;

// Gesture keyframes: head-angle sequences, non-blocking. These are PLACEHOLDER
// angles — retune once the servo is physically mounted and you know its range.
// Durations are the ms to reach each keyframe.
const Keyframe WAVE[] = {
    {30, 300}, {150, 300}, {30, 300}, {150, 300}, {HEAD_NEUTRAL, 300},
};
const Keyframe NOD[] = {
    {HEAD_NEUTRAL, 150}, {115, 250}, {HEAD_NEUTRAL, 250}, {115, 250}, {HEAD_NEUTRAL, 150},
};
const Keyframe SHAKE[] = {
    {60, 200}, {120, 200}, {60, 200}, {120, 200}, {HEAD_NEUTRAL, 200},
};
const Keyframe LOOK_LEFT[]  = {{30, 450}};
const Keyframe LOOK_RIGHT[] = {{150, 450}};
const Keyframe TILT[]       = {{115, 450}};
const Keyframe DANCE[] = {
    {30, 200}, {150, 200}, {30, 200}, {150, 200}, {HEAD_NEUTRAL, 300},
    {150, 200}, {30, 200}, {HEAD_NEUTRAL, 200},
};

}  // namespace

void ServoController::begin() {
  // 50 Hz is the standard servo PWM period. ESP32Servo auto-allocates an LEDC timer;
  // if you later drive many channels and hit timer conflicts, allocate explicitly
  // with ESP32PWM::allocateTimer(n) before the first attach().
  _head.setPeriodHertz(50);
  _head.attach(SERVO_HEAD_PIN, 500, 2400);  // 500-2400us covers SG90 + TD-811MG
  _head.write(HEAD_NEUTRAL);

#if ARM_ENABLED
  _arm.setPeriodHertz(50);
  _arm.attach(SERVO_ARM_PIN, 500, 2400);
  _arm.write(ARM_NEUTRAL);
#endif
}

void ServoController::update() {
  if (_frames == nullptr) return;

  unsigned long now = millis();
  const Keyframe& kf = _frames[_index];
  unsigned long elapsed = now - _frameStart;

  int angle = kf.angle;
  if (elapsed < kf.ms) {
    // Linear interpolation toward the keyframe.
    angle = _startAngle +
            (int)((long)(kf.angle - _startAngle) * (long)elapsed / (long)kf.ms);
  }
  _head.write(angle);

  if (elapsed >= kf.ms) {
    _startAngle = kf.angle;
    if (++_index >= _count) {
      _frames = nullptr;  // gesture done; hold the final pose
      _index = 0;
      _count = 0;
    } else {
      _frameStart = now;
    }
  }
}

void ServoController::trigger(Gesture g) {
  switch (g) {
    case Gesture::WAVE:       _startGesture(WAVE, sizeof(WAVE) / sizeof(WAVE[0])); break;
    case Gesture::NOD:        _startGesture(NOD, sizeof(NOD) / sizeof(NOD[0])); break;
    case Gesture::SHAKE:      _startGesture(SHAKE, sizeof(SHAKE) / sizeof(SHAKE[0])); break;
    case Gesture::LOOK_LEFT:  _startGesture(LOOK_LEFT, sizeof(LOOK_LEFT) / sizeof(LOOK_LEFT[0])); break;
    case Gesture::LOOK_RIGHT: _startGesture(LOOK_RIGHT, sizeof(LOOK_RIGHT) / sizeof(LOOK_RIGHT[0])); break;
    case Gesture::TILT:       _startGesture(TILT, sizeof(TILT) / sizeof(TILT[0])); break;
    case Gesture::DANCE:      _startGesture(DANCE, sizeof(DANCE) / sizeof(DANCE[0])); break;
    case Gesture::NONE:
    default:                  setNeutral(); break;
  }
}

void ServoController::setNeutral() {
  _frames = nullptr;
  _head.write(HEAD_NEUTRAL);
#if ARM_ENABLED
  _arm.write(ARM_NEUTRAL);
#endif
}

Gesture ServoController::gestureFromToken(const String& token) {
  String t = token;
  t.trim();
  t.toUpperCase();
  if (t == "WAVE")       return Gesture::WAVE;
  if (t == "NOD")        return Gesture::NOD;
  if (t == "SHAKE")      return Gesture::SHAKE;
  if (t == "LOOK_LEFT")  return Gesture::LOOK_LEFT;
  if (t == "LOOK_RIGHT") return Gesture::LOOK_RIGHT;
  if (t == "TILT")       return Gesture::TILT;
  if (t == "DANCE")      return Gesture::DANCE;
  return Gesture::NONE;
}

void ServoController::_startGesture(const Keyframe* frames, uint8_t count) {
  _frames = frames;
  _count = count;
  _index = 0;
  _frameStart = millis();
  _startAngle = _head.read();
}
