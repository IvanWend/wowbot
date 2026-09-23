#pragma once
// DFRobot Gravity Speech Synthesis Module V2.0 (SKU DFR0760) over UART2 (Phase 1.5).
//
// This module is a text-to-speech chip, NOT a TF-card MP3 player — it synthesizes
// speech from text. The laptop sends SAY:<text> and the firmware streams it here.
// English ASCII is implemented; the module also speaks Chinese (UTF-16LE payload,
// encoding byte 0x03) which is a follow-up if the voice loop ever emits non-ASCII.
#include <Arduino.h>

class SpeechController {
 public:
  void begin();                 // UART2 @ SPEECH_BAUD, send voice/volume config
  void say(const char* text);   // non-blocking: send the frame, speak in background
  void update();                // poll for the playback-complete ACK (0x4F)
  bool busy() const { return _speaking; }

 private:
  bool _speaking = false;
  void _sendAscii(const char* text, size_t len);
  void _drain();
};
