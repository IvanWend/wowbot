#pragma once
// DotStar 16x16 face over hardware SPI (VSPI), Phase 3.
// Static per-expression patterns first; animation is a later optimization.
#include <Arduino.h>
#include <Adafruit_DotStar.h>
#include "expressions.h"

class LedMatrix {
 public:
  void begin();
  void showExpression(Expression e);
  void setBrightness(uint8_t b);
  void clear();
  void update();  // reserved for future animation

 private:
  Adafruit_DotStar* _strip = nullptr;
  int _pixel(int x, int y);  // handles serpentine vs progressive panel wiring
  void _fillRect(int x, int y, int w, int h, uint32_t color);
};
