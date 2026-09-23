#pragma once
// Canonical expressions. Values must stay in sync with brain/schema.py (names are
// matched case-insensitively on the firmware).
#include <Arduino.h>

enum class Expression : uint8_t {
  NEUTRAL, HAPPY, SAD, CURIOUS, ANGRY, SURPRISED, SLEEPY, THINKING, EXCITED,
  UNKNOWN
};

inline Expression expressionFromToken(const String& token) {
  String t = token;
  t.trim();
  t.toUpperCase();
  if (t == "HAPPY")      return Expression::HAPPY;
  if (t == "SAD")        return Expression::SAD;
  if (t == "CURIOUS")    return Expression::CURIOUS;
  if (t == "ANGRY")      return Expression::ANGRY;
  if (t == "SURPRISED")  return Expression::SURPRISED;
  if (t == "SLEEPY")     return Expression::SLEEPY;
  if (t == "THINKING")   return Expression::THINKING;
  if (t == "EXCITED")    return Expression::EXCITED;
  if (t == "NEUTRAL")    return Expression::NEUTRAL;
  return Expression::UNKNOWN;
}
