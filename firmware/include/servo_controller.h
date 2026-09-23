#pragma once
// Non-blocking servo gestures via the ESP32Servo (LEDC) library.
// Phase 2 = SG90 head; Phase 2.5 = TD-811MG arm (gated behind ARM_ENABLED).
#include <Arduino.h>
#include <ESP32Servo.h>

enum class Gesture : uint8_t {
  NONE, WAVE, NOD, SHAKE, LOOK_LEFT, LOOK_RIGHT, TILT, DANCE
};

struct Keyframe {
  int angle;         // target head angle (degrees)
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
  Servo _head;  // SG90 on SERVO_HEAD_PIN
  Servo _arm;   // TD-811MG on SERVO_ARM_PIN (parked until Phase 2.5)

  const Keyframe* _frames = nullptr;
  uint8_t _count = 0;
  uint8_t _index = 0;
  unsigned long _frameStart = 0;
  int _startAngle = 90;

  void _startGesture(const Keyframe* frames, uint8_t count);
};
