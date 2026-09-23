#include <Arduino.h>
#include "config_pins.h"
#include "tokens.h"
#include "expressions.h"
#include "servo_controller.h"
#include "led_matrix.h"

ServoController servos;
LedMatrix face;

// Serial acknowledgment for recognized commands. The S3 DevKit has no general-purpose
// onboard LED (power LED only), so before the matrix and servos are wired, ACK is the
// visible confirmation that a token was parsed and dispatched.
static void ack(const Command& c) {
  switch (c.type) {
    case Command::EXP:    Serial.print("ACK:EXP:");    Serial.println(c.value);  break;
    case Command::MOV:    Serial.print("ACK:MOV:");    Serial.println(c.value);  break;
    case Command::BRIGHT: Serial.print("ACK:BRIGHT:"); Serial.println(c.number); break;
    default: break;
  }
}

static void handle(const Command& c) {
  switch (c.type) {
    case Command::EXP:
      face.showExpression(expressionFromToken(c.value));
      break;
    case Command::MOV:
      servos.trigger(ServoController::gestureFromToken(c.value));
      break;
    case Command::BRIGHT:
      face.setBrightness((uint8_t)constrain(c.number, 0, 255));
      break;
    case Command::PING:
      Serial.println("PONG");  // PONG is its own acknowledgment
      return;
    case Command::UNKNOWN:
    default:
      Serial.print("ERR:unrecognized:");
      Serial.println(c.value);
      return;
  }
  ack(c);
}

void setup() {
  Serial.begin(SERIAL_BAUD);  // USB CDC (virtual baud)

  servos.begin();
  face.begin();

  face.showExpression(Expression::NEUTRAL);  // neutral face (if the matrix is wired)

  Serial.println("READY");
}

void loop() {
  servos.update();   // advance any active gesture (non-blocking)
  face.update();     // reserved for future animation

  while (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length()) {
      handle(parseCommand(line));
    }
  }
}
