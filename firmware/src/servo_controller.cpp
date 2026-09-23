#include "servo_controller.h"
#include "config_pins.h"

namespace {

constexpr int PAN_NEUTRAL = 90;   // degrees, centered
constexpr int TILT_NEUTRAL = 90;  // degrees, level
constexpr int ARM_NEUTRAL = 90;

// Gesture keyframes: {pan, tilt, ms} targets, non-blocking. Angles are PLACEHOLDERs —
// retune once the servos are physically mounted and you know their safe travel.
// ms is the time to reach each keyframe.
//
// The 2-axis head covers nod/shake/look/tilt. "wave" needs an arm servo, so until
// Phase 2.5 it degrades to a gentle pan sweep.
const Keyframe WAVE[] = {
    {40, TILT_NEUTRAL, 300}, {140, TILT_NEUTRAL, 300}, {40, TILT_NEUTRAL, 300},
    {140, TILT_NEUTRAL, 300}, {PAN_NEUTRAL, TILT_NEUTRAL, 300},
};
const Keyframe NOD[] = {
    {PAN_NEUTRAL, TILT_NEUTRAL, 150}, {PAN_NEUTRAL, 115, 250}, {PAN_NEUTRAL, TILT_NEUTRAL, 250},
    {PAN_NEUTRAL, 115, 250}, {PAN_NEUTRAL, TILT_NEUTRAL, 150},
};
const Keyframe SHAKE[] = {
    {60, TILT_NEUTRAL, 200}, {120, TILT_NEUTRAL, 200}, {60, TILT_NEUTRAL, 200},
    {120, TILT_NEUTRAL, 200}, {PAN_NEUTRAL, TILT_NEUTRAL, 200},
};
const Keyframe LOOK_LEFT[]  = {{40, TILT_NEUTRAL, 450}};
const Keyframe LOOK_RIGHT[] = {{140, TILT_NEUTRAL, 450}};
const Keyframe TILT[]       = {{PAN_NEUTRAL, 115, 450}};
const Keyframe DANCE[] = {
    {40, 110, 200}, {140, 70, 200}, {40, 110, 200}, {140, 70, 200}, {PAN_NEUTRAL, TILT_NEUTRAL, 300},
    {140, 110, 200}, {40, 70, 200}, {PAN_NEUTRAL, TILT_NEUTRAL, 200},
};

}  // namespace

void ServoController::begin() {
  // 50 Hz is the standard servo PWM period. ESP32Servo auto-allocates an LEDC timer;
  // if you later drive many channels and hit timer conflicts, allocate explicitly
  // with ESP32PWM::allocateTimer(n) before the first attach().
  _pan.setPeriodHertz(50);
  _pan.attach(SERVO_PAN_PIN, 500, 2400);  // 500-2400us covers SG90 + TD-811MG
  _pan.write(PAN_NEUTRAL);

  _tilt.setPeriodHertz(50);
  _tilt.attach(SERVO_TILT_PIN, 500, 2400);
  _tilt.write(TILT_NEUTRAL);

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

  int pan = kf.pan;
  int tilt = kf.tilt;
  if (elapsed < kf.ms) {
    // Linear interpolation toward the keyframe, on both axes.
    pan  = _startPan  + (int)((long)(kf.pan  - _startPan)  * (long)elapsed / (long)kf.ms);
    tilt = _startTilt + (int)((long)(kf.tilt - _startTilt) * (long)elapsed / (long)kf.ms);
  }
  _pan.write(pan);
  _tilt.write(tilt);

  if (elapsed >= kf.ms) {
    _startPan  = kf.pan;
    _startTilt = kf.tilt;
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
  _pan.write(PAN_NEUTRAL);
  _tilt.write(TILT_NEUTRAL);
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
  _startPan = _pan.read();
  _startTilt = _tilt.read();
}
