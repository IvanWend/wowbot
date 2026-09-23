#pragma once
// WowBot pin map — VERIFY AGAINST YOUR ACTUAL BOARD with a multimeter before wiring.
// Assumes a standard 30-pin ESP32-WROOM-32 DevKit layout (Maker-ESP32 V1.8 clone).
// See docs/ROADMAP.md for the full table and the pins to AVOID (0, 2, 12, 6-11, 34-39).

// UART0 = USB-serial to the laptop. Do NOT repurpose (TX0=GPIO1, RX0=GPIO3).
#define UART0_BAUD 115200

// UART speech-synthesis module on UART2 (DFRobot Gravity Speech Synthesis V2.0, SKU
// DFR0760). Set the module's physical switch to UART (not I2C).
#define SPEECH_RX 16    // ESP32 GPIO16 -> module TX
#define SPEECH_TX 17    // ESP32 GPIO17 -> module RX
#define SPEECH_BAUD 115200

// DotStar 16x16 matrix over hardware SPI (VSPI).
#define DOTSTAR_DATA 23         // MOSI -> DATA
#define DOTSTAR_CLOCK 18        // SCK  -> CLOCK
#define DOTSTAR_WIDTH 16
#define DOTSTAR_HEIGHT 16
#define DOTSTAR_PIXELS (DOTSTAR_WIDTH * DOTSTAR_HEIGHT)
#define DOTSTAR_BRIGHTNESS 40   // 0-255; CAP to avoid the 15 A full-white ceiling
#define DOTSTAR_SERPENTINE 0    // set 1 if alternate rows of the panel run backwards

// Servos (ESP32Servo / LEDC).
#define SERVO_HEAD_PIN 13       // SG90 — head/gesture
#define SERVO_ARM_PIN 27        // TD-811MG — heavy joint (own 5 V/3 A+ rail, common ground)
#define ARM_ENABLED 0           // set 1 in Phase 2.5 after the arm's power rail is verified

// No general-purpose onboard LED on this board (power + TX/RX LEDs only, all
// hardware-defined). Visual feedback comes from the DotStar matrix (Phase 3) and the
// servos (Phase 2); serial ACK replies (see main.cpp) confirm token dispatch before
// any actuator is wired.
//
// GPIO2 remains a boot-strapping pin — do NOT wire external peripherals to it.
