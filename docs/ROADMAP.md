# WowBot — Build Plan v3 (final hardware)

Supersedes v2. Updated for the components actually purchased (2026-09-18).

## Component reality check

| Function | Component | Notes |
|---|---|---|
| Brain | ESP32-S3 DevKit (N16R8) | 16 MB flash + 8 MB PSRAM, **native USB** (→ high-bandwidth audio). Replaces the Maker-ESP32 V1.8 (retired to a test mule). |
| Mic | INMP441 I2S MEMS | Digital PCM → clean Whisper input. Retires the KY-038 (analog threshold only). |
| Speaker | MAX98357 I2S class-D amp + 3 W cavity speaker | Streams the laptop's TTS back to the robot body. Retires the laptop speaker. |
| Sound effects | JQ8400-FN MP3 player (owned) | **Optional now** — the MAX98357A streams all audio, so the JQ8400 is redundant unless you want zero-latency clips without a laptop. |
| Face | DotStar 16×16 (256 px, APA102) | Unchanged — the expression display. |
| Head motion | 2-axis pan-tilt bracket + 2× SG90 | Pan/tilt/nod/shake/look/tilt; "wave" needs an arm. |
| Arm (optional) | TD-811MG (owned) | Heavy joint, 15–20 kg·cm. Wants its own ≥6 V rail for full torque. |
| Text display | LCD 2004 (20×4, I2C) | Caption/status, complements the face. |
| Power | Mean Well LRS-35-5 (5 V / 7 A) + MP1584EN buck + 1000 µF caps | 7 A headroom absorbs servo stalls; 3.3 V logic rail isolated from motor noise. |

## Audio architecture (final)

Full-duplex audio over the ESP32-S3's native USB:

- **Upstream:** INMP441 → I2S0 → PCM (16 kHz / 16-bit mono) → USB → laptop → Whisper.
- **Downstream:** laptop TTS → audio bytes → USB → ESP32-S3 → I2S1 → MAX98357A → 3 W speaker.

Native USB at 921600+ carries raw 16 kHz / 16-bit mono (~32 KB/s) each way without
compression. The laptop TTS must emit audio **bytes** (a WAV/MP3 buffer), so `pyttsx3`
(which drives the Windows sound device directly) is replaced by `edge-tts` (MP3) or
`piper` (WAV).

## ESP32-S3 pin map (proposed — verify against your board's silkscreen)

| Function | Pins | Notes |
|---|---|---|
| Native USB to laptop | GPIO19 (D−), GPIO20 (D+) | fixed, reserved — don't repurpose |
| INMP441 mic | I2S0 — WS=GPIO4, SCK=GPIO5, SD=GPIO6 | RX |
| MAX98357A amp | I2S1 — BCLK=GPIO7, LRC=GPIO8, DIN=GPIO9 | TX |
| DotStar matrix | SPI — DATA=GPIO11, CLK=GPIO12 | fast refresh of 256 px |
| SG90 pan | GPIO13 | LEDC PWM |
| SG90 tilt | GPIO14 | LEDC PWM |
| LCD 2004 | I2C — SDA=GPIO17, SCL=GPIO18 | caption/status |
| TD-811MG arm (optional) | GPIO21 | LEDC PWM, own ≥6 V rail |
| JQ8400 (optional) | UART — TX=GPIO47, RX=GPIO48 | only if kept |
| **Avoid** | GPIO0, GPIO3, GPIO45, GPIO46 (strapping); GPIO19/20 (USB) | boot loops / USB breakage |

## Power budget

- Mean Well LRS-35-5 = 5 V / 7 A. Feeds: 2× SG90 (stall ~0.5 A each), DotStar (cap
  brightness 30–50 → well under 1 A), and the MP1584EN buck.
- MP1584EN bucks 5 V → 3.3 V for the ESP32-S3 logic + audio chips, isolating them from
  servo/motor noise. **Gotcha:** feed the S3 either 5 V to its VIN/5 V pin (onboard
  regulator) *or* 3.3 V to its 3V3 pin — never both, and never with USB 5 V connected
  at the same time.
- 1000 µF caps across the servo power terminals to absorb inrush/stall transients.
- TD-811MG (optional) wants its own ≥6 V rail for full torque; at 5 V it's slower/weaker.

## Phases

**Phase 0 — laptop voice loop.** DONE. (VAD → Whisper → Ollama → TTS, headless.)

**Phase 1 — serial link.** DONE (on the Maker-ESP32; port to the S3). `PING`→`PONG`,
`EXP:`/`MOV:`→`ACK:`.

**Phase 2 — pan-tilt head.** 2× SG90 on GPIO13/14, non-blocking `millis()` gestures
(`MOV:NOD`, `MOV:SHAKE`, `MOV:LOOK_LEFT`, …). "wave" waits on an arm servo.

**Phase 2.5 — TD-811MG arm (optional).** Own ≥6 V rail, common ground.

**Phase 3 — DotStar face.** SPI (GPIO11/12), brightness cap, static expression patterns.

**Phase 4 — LCD 2004 caption/status.** I2C (GPIO17/18), show what it heard / its reply.

**Phase 5 — body/packaging** (open-frame / 3D-printed shell).

**Phase 6 — bidirectional audio.** INMP441 mic + MAX98357A speaker over native USB:
length-prefixed binary framing, laptop TTS → audio bytes (edge-tts/piper). This is the
project's core value and can be tackled early (the S3 + parts are already in hand).

**Phase 7 — portfolio polish.** README, demo video, latency benchmark.

## Suggested order

1. Flash the ESP32-S3 (Blink + serial `READY`/`PONG`) — confirm native USB CDC and pin map.
2. Phase 2: wire the 2× SG90 pan-tilt, confirm `MOV:NOD`/`SHAKE` sweep.
3. Phase 3: wire the DotStar, confirm `EXP:` faces.
4. Phase 6: wire INMP441 + MAX98357A, build the binary audio framing both sides.
5. Phase 4: LCD captions. Then polish (5/7).
