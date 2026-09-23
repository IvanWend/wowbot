#pragma once
// WowBot pin map — ESP32-S3 DevKitC (N16R8). VERIFY AGAINST YOUR ACTUAL BOARD's
// silkscreen with a multimeter before wiring. See docs/ROADMAP.md for the full table.
//
// AVOID on the S3:
//   GPIO19/20 — native USB D-/D+ (reserved; this IS the laptop link)
//   GPIO0, GPIO3, GPIO45, GPIO46 — strapping pins (boot mode / JTAG / VDD_SPI)

// Laptop link = native USB CDC (GPIO19/20). `Serial` maps to it under
// ARDUINO_USB_MODE=1 (see platformio.ini). Baud is virtual on USB CDC.
#define SERIAL_BAUD 115200

// I2S0 — INMP441 mic (RX, Phase 6). 16 kHz / 16-bit mono up to the laptop.
#define I2S_MIC_WS  4    // L/R word select
#define I2S_MIC_SCK 5    // bit clock
#define I2S_MIC_SD  6    // serial data in

// I2S1 — MAX98357A class-D amp (TX, Phase 6). TTS audio down from the laptop.
#define I2S_SPKR_BCLK 7  // bit clock
#define I2S_SPKR_LRC  8  // L/R clock (word select)
#define I2S_SPKR_DIN  9  // serial data out

// DotStar 16x16 matrix (APA102). Bit-banged SPI on any GPIO (Adafruit_DotStar data/clk
// constructor); pins chosen to stay off the strapping pins and I2S lanes.
#define DOTSTAR_DATA 11        // -> matrix DATA
#define DOTSTAR_CLOCK 12       // -> matrix CLOCK
#define DOTSTAR_WIDTH 16
#define DOTSTAR_HEIGHT 16
#define DOTSTAR_PIXELS (DOTSTAR_WIDTH * DOTSTAR_HEIGHT)
#define DOTSTAR_BRIGHTNESS 40   // 0-255; cap to avoid the full-white current spike
#define DOTSTAR_SERPENTINE 0    // set 1 if alternate rows of the panel run backwards

// Servos (ESP32Servo / LEDC).
#define SERVO_PAN_PIN 13        // SG90 — pan (yaw)
#define SERVO_TILT_PIN 14       // SG90 — tilt (pitch)
#define SERVO_ARM_PIN 21        // TD-811MG — heavy joint (own >=6 V rail, common ground)
#define ARM_ENABLED 0           // set 1 in Phase 2.5 after the arm's power rail is verified

// LCD 2004 caption/status (Phase 4) over I2C.
#define LCD_SDA 17
#define LCD_SCL 18

// No general-purpose onboard LED on the S3 DevKit (power LED only). Serial `ACK:`
// replies (see main.cpp) are the feedback until the matrix (Phase 3) and servos
// (Phase 2) are wired.
