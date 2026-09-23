#include "led_matrix.h"
#include "config_pins.h"

void LedMatrix::begin() {
  // DOTSTAR_BRG is the native byte order for APA102/SK9822.
  _strip = new Adafruit_DotStar(DOTSTAR_PIXELS, DOTSTAR_DATA, DOTSTAR_CLOCK, DOTSTAR_BRG);
  _strip->begin();
  _strip->setBrightness(DOTSTAR_BRIGHTNESS);  // cap: full white = 15 A
  _strip->clear();
  _strip->show();
}

void LedMatrix::setBrightness(uint8_t b) {
  if (_strip == nullptr) return;
  _strip->setBrightness(b);
  _strip->show();
}

void LedMatrix::clear() {
  if (_strip == nullptr) return;
  _strip->clear();
  _strip->show();
}

void LedMatrix::update() {
  // Future: per-frame animation of the 256 px. Not needed for v1 — static patterns
  // (a few dozen lit pixels) are plenty and keep the current draw well under 1 A.
}

void LedMatrix::showExpression(Expression e) {
  if (_strip == nullptr) return;
  _strip->clear();

  // Placeholder palette + face. Real per-expression art comes later; this is enough
  // to prove the matrix, SPI, and brightness cap all work.
  uint32_t eye = _strip->Color(90, 90, 220);  // neutral blue
  switch (e) {
    case Expression::HAPPY:      eye = _strip->Color(255, 200, 40);  break;
    case Expression::EXCITED:    eye = _strip->Color(255, 120, 40);  break;
    case Expression::SAD:        eye = _strip->Color(60, 60, 180);   break;
    case Expression::ANGRY:      eye = _strip->Color(255, 40, 40);   break;
    case Expression::SURPRISED:  eye = _strip->Color(255, 255, 255); break;
    case Expression::SLEEPY:     eye = _strip->Color(40, 40, 100);   break;
    case Expression::CURIOUS:    eye = _strip->Color(80, 220, 180);  break;
    case Expression::THINKING:   eye = _strip->Color(180, 120, 255); break;
    default: break;  // NEUTRAL / UNKNOWN
  }

  // Two 2x2 eyes near the top.
  _fillRect(3, 3, 2, 2, eye);
  _fillRect(11, 3, 2, 2, eye);

  // Mouth near the bottom, simple shape per expression.
  const int mouthY = 12;
  switch (e) {
    case Expression::HAPPY:
    case Expression::EXCITED:   _fillRect(5, mouthY, 6, 2, eye); break;     // wide grin
    case Expression::SAD:       _fillRect(6, mouthY + 1, 4, 1, eye); break; // flat frown
    case Expression::SURPRISED: _fillRect(7, mouthY, 2, 2, eye); break;     // little "o"
    case Expression::ANGRY:     _fillRect(5, mouthY + 1, 6, 1, eye); break; // gritted line
    case Expression::CURIOUS:
    case Expression::THINKING:  _fillRect(7, mouthY, 2, 1, eye); break;     // small dot
    default:                    _fillRect(6, mouthY, 4, 1, eye); break;     // neutral line
  }

  _strip->show();
}

int LedMatrix::_pixel(int x, int y) {
#if DOTSTAR_SERPENTINE
  return (y & 1) ? (y * DOTSTAR_WIDTH + (DOTSTAR_WIDTH - 1 - x))
                 : (y * DOTSTAR_WIDTH + x);
#else
  return y * DOTSTAR_WIDTH + x;
#endif
}

void LedMatrix::_fillRect(int x, int y, int w, int h, uint32_t color) {
  for (int yy = y; yy < y + h; yy++) {
    for (int xx = x; xx < x + w; xx++) {
      _strip->setPixelColor(_pixel(xx, yy), color);
    }
  }
}
