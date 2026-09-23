# WowBot — Progress

Last updated: 2026-09-18

Running record of what's built, what's verified, and what's next. Phases mirror
`ROADMAP.md`; the serial contract lives in `ARCHITECTURE.md`.

## Status at a glance

| Phase | Status | Result |
|---|---|---|
| 0 — laptop brain | ✅ Done | voice loop verified end-to-end |
| 1 — serial link | ✅ Done | `PING`→`PONG`, `ACK:` confirmed (on Maker-ESP32; port to S3 pending) |
| 2 — pan-tilt head | ⬜ Next | 2× SG90 purchased, needs wiring |
| 2.5 — TD-811MG arm | ⬜ Optional | own ≥6 V rail |
| 3 — DotStar face | ⬜ Pending | firmware scaffolded, needs wiring |
| 4 — LCD 2004 caption | ⬜ Pending | owned, needs I2C wiring + driver |
| 5 — body / packaging | ⬜ Pending | open-frame |
| 6 — audio (mic + speaker) | 🔶 Core | parts purchased; binary framing TBD |
| 7 — portfolio polish | ⬜ Pending | |

## Checklist

### Phase 0 — laptop brain (laptop only)
- [x] Project structure + draft files
- [x] Voice loop: VAD → faster-whisper → Ollama (JSON) → TTS
- [x] Token contract (`schema.py`: LLM JSON → `RobotAction` → `EXP:`/`MOV:`)
- [x] Verified end-to-end by user

### Phase 1 — serial link
- [x] ESP32 firmware (PlatformIO) with ESP32Servo
- [x] Serial token parser (`tokens.h`) + dispatch (`main.cpp`)
- [x] Flashed to Maker-ESP32 V1.8 (COM3 @ 115200)
- [x] Verified: `PING`→`PONG`, `EXP:HAPPY`→`ACK:EXP:HAPPY`
- [ ] Re-flash to ESP32-S3 (native USB CDC), confirm `READY`/`PONG` on the new board

### Phase 2 — pan-tilt head
- [ ] Verify ESP32-S3 pin map (GPIO13 pan / GPIO14 tilt)
- [ ] Wire 2× SG90 (signal + common ground + 5 V from LRS-35-5)
- [ ] Verify `MOV:NOD`/`MOV:SHAKE`/`MOV:LOOK_LEFT` via non-blocking `millis()` state machine

### Phase 3 — DotStar face
- [ ] Wire matrix (SPI DATA/CLK) + 1000 µF cap at power pins
- [ ] Verify `EXP:` sets the face, `BRIGHT:` scales it

### Phase 4 — LCD 2004 caption/status
- [ ] Wire I2C (SDA/SCL), add `lcd_controller.*`
- [ ] Show what it heard / its reply as text

### Phase 6 — bidirectional audio (core)
- [ ] Wire INMP441 (I2S0) + MAX98357A (I2S1)
- [ ] Firmware: I2S capture + playback, length-prefixed binary framing
- [ ] Laptop: swap `pyttsx3` → `edge-tts`/`piper` (emit audio bytes); stream PCM both ways
- [ ] Verify mic PCM → Whisper, and TTS → robot speaker

## Key facts & decisions from this session

- **Hardware upgraded (2026-09-18).** Brain = ESP32-S3 (N16R8, native USB); audio =
  INMP441 mic + MAX98357A amp + 3 W speaker (full-duplex streaming); head = 2-axis
  pan-tilt (2× SG90); power = Mean Well LRS-35-5 (5 V / 7 A) + MP1584EN buck + 1000 µF caps.
- **Maker-ESP32 V1.8 retired** to a test mule — its motor-driver pinout was never verified.
- **JQ8400-FN is now optional** — the MAX98357A streams all audio, so the MP3 module is
  redundant unless kept for zero-latency SFX clips.
- **`pyttsx3` can't stream to the robot** (it drives the Windows sound device directly);
  the laptop TTS must emit audio bytes — `edge-tts` (MP3) or `piper` (WAV).
- PlatformIO DotStar pin is `adafruit/Adafruit DotStar@^1.2.5` (registry has no 1.6.4).
- No general-purpose onboard LED on the old board; serial `ACK:` remains the feedback.
- Defaults: STT = `faster-whisper`, LLM = Ollama `format: json`.

## Open questions

1. **ESP32-S3 pin map** — verify the proposed GPIO allocation against the actual S3 board.
2. **TTS engine** — pick `edge-tts` (network, high quality) vs `piper` (offline neural).

## Next steps (in order)

1. Flash the ESP32-S3; confirm native USB CDC + `READY`/`PONG`.
2. Verify the S3 pin map; wire the 2× SG90 pan-tilt and confirm gestures.
3. Wire the DotStar and confirm `EXP:` faces.
4. Wire INMP441 + MAX98357A; build the binary audio framing (laptop + firmware).
5. LCD captions, then body/polish.
