# WowBot — Build Plan v4 (standalone cloud robot)

Supersedes v3. The robot no longer needs a laptop: the ESP32-S3 runs **xiaozhi-esp32**
and talks over Wi-Fi to a self-hosted **xiaozhi-esp32-server** that calls domestic Chinese
cloud APIs. Custom work = DotStar face + servo pan-tilt (xiaozhi has neither natively).

## Why xiaozhi-esp32 (not the only option)

Chosen because it already solves the hard 80% — I2S audio pipeline, VAD + wake word, the
WebSocket protocol, and native Chinese cloud providers. Alternatives considered and set
aside: ESP-ADF (building blocks; you assemble the loop yourself), ESPHome/Home Assistant
(wrong ecosystem), roll-your-own ESP-IDF + direct cloud calls (reinvents xiaozhi), and
on-device AI (the S3 is too small for conversational ASR+LLM+TTS). xiaozhi's board/driver
layer is built to be extended, which is where the custom face and servos plug in.

## Component reality check

| Function | Component | Notes |
|---|---|---|
| Brain | ESP32-S3 DevKit (N16R8) | 16 MB flash + 8 MB PSRAM, native USB, Wi-Fi 2.4 GHz |
| Firmware | xiaozhi-esp32 (ESP-IDF) | standalone voice assistant; no-codec audio path |
| Server | xiaozhi-esp32-server (self-hosted) | holds API keys; VPS or always-on local box |
| Mic | INMP441 I2S | 3.3 V, L/R→GND |
| Speaker | MAX98357A amp + 3 W speaker | VIN 5 V, SD→3.3 V, GAIN→GND |
| Face | DotStar 16×16 (APA102) | **custom driver** (not native); 3.3 V or level-shift |
| Head | 2× SG90 pan-tilt | **custom LEDC driver** |
| Power | Xiaomi 20000 mAh 22.5 W bank + Type-C→XH2.54 breakout + MP1584EN buck | ~12,000 mAh @ 5 V; watch auto-shutoff + CC resistors |
| (dropped) | LCD 2004 | redundant — a supported OLED/LCD gives captions for free |

## Architecture

ESP32-S3 (xiaozhi-esp32) —WebSocket/Wi-Fi→ self-hosted server —HTTPS→ DashScope STT ·
Kimi/DeepSeek/Qwen LLM · Volcengine/CosyVoice TTS. **API keys live on the server only**;
the firmware holds just the server's WebSocket URL.

## Pin map (xiaozhi no-codec default)

| Function | Pins |
|---|---|
| INMP441 mic | WS=GPIO4, SCK=GPIO5, SD=GPIO6 (3.3 V) |
| MAX98357A amp | DIN=GPIO7, BCLK=GPIO15, LRC=GPIO16 (5 V VIN) |
| DotStar face | DATA=GPIO11, CLK=GPIO12 |
| SG90 pan / tilt | GPIO13 / GPIO14 |
| Native USB | GPIO19/20 (reserved) |
| **Avoid** | GPIO0, GPIO3, GPIO45, GPIO46 (strapping) |

## Power (portable)

Xiaomi 20000 mAh 22.5 W bank (≈12,000 mAh / ~60 Wh @ 5 V) 5 V → main bus → servos +
MAX98357A directly; bus → MP1584EN → 3.3 V rail for S3 + INMP441 + DotStar. Feed from the
highest-amp port and use the bank's double-press low-current/trickle mode (if present) to
stop auto-shutoff at idle. **Gotcha:** a bare 2-wire Type-C breakout only yields 5 V if it
has the 5.1 kΩ CC1/CC2 pull-downs — otherwise use the bank's USB-A port + A→C cable.
1000 µF caps across servo power. Never feed the S3 both VIN and 3V3 at once.

## Phases

- **Phase 0 — preserve.** Initial git commit (legacy laptop path stays as fallback).
- **Phase 1 — power.** Power bank + buck + caps (user, physical).
- **Phase 2 — server.** Clone `xiaozhi-esp32-server`; in `data/.config.yaml`: SileroVAD,
  FunASR (SenseVoice/Paraformer), LLM (Kimi/DeepSeek/Qwen `type: openai`), TTS
  (Volcengine/CosyVoice) + keys; note the WebSocket URL and MAC-based device auth.
- **Phase 3 — firmware talking MVP.** ESP-IDF → clone `xiaozhi-esp32` → N16R8 board config
  (PSRAM on) → no-codec audio → wire mic+amp → flash → verify wake-word→STT→LLM→TTS.
- **Phase 4 — DotStar face.** Custom APA102 driver (full animated face); see
  "Custom driver design" below.
- **Phase 5 — servo pan-tilt.** LEDC driver; port keyframes from the legacy
  `firmware/src/servo_controller.cpp`; map device state → motion.
- **Phase 6 — polish.** Body, README, demo, latency benchmark.

## Custom driver design (Phase 4–5)

Both drivers live in `main/boards/wowbot-n16r8/` — the firmware globs
`boards/${BOARD_DIR}/*.cc` (`main/CMakeLists.txt` line ~906), so **new `.cc` files in the
board dir compile with no CMake edit**.

### Face — DotStar (APA102), full animated

- **Hook:** `DotStarFaceDisplay : public Display` (`main/display/display.h`), returned by
  the board's `GetDisplay()`. The app already calls `SetEmotion()`, `SetStatus()`,
  `SetChatMessage()` on every state/emotion/text change — no other wiring needed.
- **Transport:** APA102 is *not* in the `led_strip` driver's `led_model_t`
  (`WS2812/SK6812/WS2811/WS2816` only), so drive it with the **SPI master** driver on
  `SPI2_HOST` (FSPI): `MOSI=GPIO11`, `CLK=GPIO12`, `MISO=-1` (write-only — this keeps
  GPIO13 free for the pan servo).
- **Framing:** start `4×0x00`; per LED `0xE0|brightness, B, G, R` (APA102 native BRG
  order, 5-bit global-brightness byte); end `(N/16)+1 × 0xFF`.
- **Emotions:** normalize case-insensitively, then map onto the canonical set from
  `brain/schema.py`: `neutral/happy/sad/curious/angry/surprised/sleepy/thinking/excited`.
  The server sends **uppercase** FunASR tags (`HAPPY/SAD/ANGRY/NEUTRAL/FEARFUL/DISGUSTED/
  SURPRISED`) and **lowercase** emoji-path strings; firmware hardcodes `neutral`.
- **Animations:** idle blink (3–6 s), listening pulse, speaking mouth (from
  `SetChatMessage`/`SetStatus(SPEAKING)`), thinking dots, expression crossfade — a ~25 fps
  FreeRTOS render task with a mutex around the state shared with the app loop.
- **Brightness:** cap the 5-bit global byte to bound full-white current (legacy used
  40/255).
- Port the 16×16 eyes+mouth art from legacy `firmware/src/led_matrix.cpp` and expand.

### Head — SG90 pan-tilt

- **Hook:** `ServoController` owned by the board; it registers a `DeviceStateMachine`
  listener (safe — `Application` is constructed before `Board::GetInstance()` in
  `main.cc`).
- **Transport:** LEDC (reference idiom: `main/boards/electron-bot/oscillator.cc`): 50 Hz,
  13-bit duty, 500–2400 µs; `pan=GPIO13`, `tilt=GPIO14`.
- **Gestures:** port the non-blocking keyframes from legacy
  `firmware/src/servo_controller.cpp` (`WAVE/NOD/SHAKE/LOOK_LEFT/LOOK_RIGHT/TILT/DANCE`);
  `update()` runs from the face's render task.
- **Mapping:** device-state listener (listening→attentive, speaking→subtle motion,
  idle→occasional look-around) + emotion→gesture (happy→nod, surprised→tilt-up,
  angry→shake, sad→tilt-down).

## Suggested order

0 (commit) → 2 (server) → 3 (talking MVP — the go/no-go gate) → 4 (face) → 5 (motion) → 6 (polish).
