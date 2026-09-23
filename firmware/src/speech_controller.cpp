#include "speech_controller.h"
#include "config_pins.h"
#include <string.h>

// Protocol (from DFRobot_SpeechSynthesis, the official library):
//   frame: 0xFD, len_hi, len_lo, 0x01, <encoding>, <payload>
//   len    = number of bytes AFTER the two length bytes = 2 + payload
//   encoding: 0x00 = ASCII/English, 0x03 = Chinese (UTF-16LE)  [0x03 not used yet]
//   module ACKs 0x41 (synthesis done) then 0x4F (playback done) on UART2 RX.
namespace {

constexpr size_t MAX_SPEECH = 400;  // byte cap per synthesis frame (safe margin)

// Control text: bracketed markers the chip treats as settings, not speech.
const char CFG_VOLUME[]  = "[v5]";  // volume 5 of 9
const char CFG_ENGLISH[] = "[h2]";  // English word (not letter) pronunciation
const char CFG_VOICE[]   = "[m3]";  // female-1 voice

}  // namespace

void SpeechController::begin() {
  Serial2.begin(SPEECH_BAUD, SERIAL_8N1, SPEECH_RX, SPEECH_TX);
  delay(200);  // let the module boot
  _drain();

  _sendAscii(CFG_VOLUME, strlen(CFG_VOLUME));   delay(120);
  _sendAscii(CFG_ENGLISH, strlen(CFG_ENGLISH)); delay(120);
  _sendAscii(CFG_VOICE, strlen(CFG_VOICE));     delay(120);
  delay(300);
  _drain();  // discard the ACKs the config frames produced
}

void SpeechController::say(const char* text) {
  if (text == nullptr) return;
  size_t len = strlen(text);
  if (len == 0) return;
  if (len > MAX_SPEECH) len = MAX_SPEECH;
  _sendAscii(text, len);
  _speaking = true;
}

void SpeechController::update() {
  // Drain UART2 RX. Only a 0x4F while speaking ends the utterance; everything else
  // (0x41 synth-acks, config-ack leftovers, idle bytes) is discarded.
  while (Serial2.available()) {
    uint8_t b = Serial2.read();
    if (_speaking && b == 0x4F) {
      _speaking = false;
      Serial.println("DONE:SAY");
    }
  }
}

void SpeechController::_sendAscii(const char* text, size_t len) {
  uint16_t length = (uint16_t)(len + 2);
  uint8_t head[5] = {
      0xFD,
      (uint8_t)(length >> 8),
      (uint8_t)(length & 0xFF),
      0x01,   // START_SYNTHESIS
      0x00,   // ASCII / English encoding
  };
  Serial2.write(head, sizeof(head));
  for (size_t i = 0; i < len; i++) {
    Serial2.write((uint8_t)(text[i] & 0x7F));  // ASCII only (matches the library's English path)
  }
}

void SpeechController::_drain() {
  while (Serial2.available()) Serial2.read();
}
