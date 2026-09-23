#pragma once
// Serial command protocol (laptop -> ESP32-S3), newline-terminated ASCII.
// See docs/ARCHITECTURE.md for the full contract.
//   EXP:<EXPR>      set face expression (Phase 3)
//   MOV:<MOVE>      trigger a non-blocking gesture (Phase 2)
//   BRIGHT:<0-255>  set DotStar brightness (Phase 3)
//   PING            -> "PONG"
//
// Audio is NOT a token — it flows as length-prefixed binary frames (Phase 6), distinct
// from these newline control tokens.
#include <Arduino.h>

struct Command {
  enum Type : uint8_t { NONE, EXP, MOV, BRIGHT, PING, UNKNOWN };
  Type type = NONE;
  String value;   // EXP/MOV name (or raw line for UNKNOWN)
  int number = 0; // parsed numeric for BRIGHT
};

inline Command parseCommand(const String& line) {
  Command c;
  int colon = line.indexOf(':');
  String head = (colon < 0) ? line : line.substring(0, colon);
  String value = (colon < 0) ? "" : line.substring(colon + 1);
  head.trim();
  value.trim();
  head.toUpperCase();

  if (head == "EXP")           { c.type = Command::EXP;    c.value = value; }
  else if (head == "MOV")      { c.type = Command::MOV;    c.value = value; }
  else if (head == "BRIGHT")   { c.type = Command::BRIGHT; c.number = value.toInt(); }
  else if (head == "PING")     { c.type = Command::PING; }
  else                         { c.type = Command::UNKNOWN; c.value = line; }
  return c;
}
