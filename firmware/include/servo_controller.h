#pragma once
// Non-blocking 2-axis pan-tilt gestures via the ESP32Servo (LEDC) library.
// Phase 2 = 2x SG90 (pan GPIO13 / tilt GPIO14); Phase 2.5 = TD-811MG arm (ARM_ENABLED).
// Each gesture is a list of {pan, tilt, ms} keyframes advanced by millis() in update().
#include <Arduino.h>
#include <ESP32Servo.h>

enum class Gesture : uint8_t {
  NONE, WAVE, NOD, SHAKE, LOOK_LEFT, LOOK_RIGHT, TILT, DANCE
};

struct Keyframe {
  int pan;           // target pan angle (degrees)
  int tilt;          // target tilt angle (degrees)
  unsigned long ms;  // time to reach it
};

class ServoController {
 public:
  void begin();
  void update();  // non-blocking; call from loop()
  void trigger(Gesture g);
  void setNeutral();

  static Gesture gestureFromToken(const String& token);

 private:
  Servo _pan;   // SG90 on SERVO_PAN_PIN (yaw)
  Servo _tilt;  // SG90 on SERVO_TILT_PIN (pitch)
  Servo _arm;   // TD-811MG on SERVO_ARM_PIN (parked until Phase 2.5)

  const Keyframe* _frames = nullptr;
  uint8_t _count = 0;
  uint8_t _index = 0;
  unsigned long _frameStart = 0;
  int _startPan = 90;
  int _startTilt = 90;

  void _startGesture(const Keyframe* frames, uint8_t count);
};
