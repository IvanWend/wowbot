# WowBot — Architecture (v4, standalone cloud robot)

The single source of truth for the firmware ↔ server ↔ cloud contract. The laptop-driven
serial-token design (v3) is retired to a fallback; this document replaces it.

## Data flow

```
ESP32-S3 (xiaozhi-esp32)                self-hosted server               cloud APIs
  ┌──────────────┐                       ┌─────────────────┐            ┌───────────────┐
  │ INMP441 mic  │── I2S0 ─┐             │ SileroVAD       │            │ DashScope STT │
  │ (I2S capture)│         │  WebSocket  │ FunASR (STT)    │── HTTPS ──▶│ (SenseVoice)  │
  │ MAX98357A    │◀─ I2S1 ─┤  (audio +   │ LLM (Kimi/      │── HTTPS ──▶│ Kimi/DeepSeek/│
  │ (I2S play)   │         │   events)   │  DeepSeek/Qwen) │            │ Qwen (LLM)    │
  │ DotStar face │◀ custom ┘             │ TTS (Volcengine/│── HTTPS ──▶│ Volcengine/   │
  │ SG90 pan-tilt│◀ custom               │  CosyVoice)     │            │ CosyVoice TTS │
  └──────────────┘                       └─────────────────┘            └───────────────┘
```

Two directions over one WebSocket:
- **Upstream:** mic PCM → server STT.
- **Downstream:** server TTS audio → amp.

## Where things live

- **Firmware (ESP32):** the server's WebSocket URL only (compile-time). **No API keys.**
- **Server (`xiaozhi-esp32-server`):** all provider keys in `config.yaml` /
  `data/.config.yaml`. Device identity = ESP32 MAC address.

## Custom drivers (xiaozhi has no native support)

- **DotStar (APA102) face** — a `Display` subclass; map device state → expression.
- **SG90 pan-tilt** — an LEDC driver + a `DeviceStateMachine` listener; map state → motion.
- xiaozhi's built-in display layer covers OLED (SSD1306) + graphical LCD (ST7789/GC9A01).

### Custom driver design

Both drivers live in `main/boards/wowbot-n16r8/` in the firmware repo. Board `.cc` files
are auto-globbed (`main/CMakeLists.txt`), so no CMake edit is needed.

**Face — hooks the `Display` abstraction.** The firmware's `main/application.cc` already
calls three virtuals on `board.GetDisplay()` at the right moments, so a
`DotStarFaceDisplay : public Display` (currently `NoDisplay`) is the only seam needed:

- `SetEmotion(emotion)` — expression, from server JSON / firmware hardcodes.
- `SetStatus(status)` — mode, matched against `Lang::Strings::{LISTENING, SPEAKING,
  STANDBY, CONNECTING}`.
- `SetChatMessage(role, content)` — speaking (drives the animated mouth).

APA102 is **not** supported by the `led_strip` component (`led_model_t` has no APA102
entry), so the face drives it with the raw **SPI master** driver on `SPI2_HOST`:
`MOSI=GPIO11`, `CLK=GPIO12`, `MISO=-1` (write-only). Framing: start `4×0x00`, per LED
`0xE0|brightness, B, G, R`, end `(N/16)+1 × 0xFF`.

**Emotion normalization.** The emotion string arrives in mixed case — uppercase FunASR
tags (`HAPPY/SAD/ANGRY/NEUTRAL/FEARFUL/DISGUSTED/SURPRISED`), lowercase emoji-path
strings, and firmware hardcodes (`neutral`). The driver lowercases and maps onto the
canonical `brain/schema.py` set: `neutral, happy, sad, curious, angry, surprised, sleepy,
thinking, excited`.

**Head — a `DeviceStateMachine` listener.** `main.cc` constructs `Application` (and its
state machine) *before* `Board::GetInstance()`, so the board constructor can safely call
`Application::GetInstance().AddStateChangeListener(...)` for a `ServoController`. Servos
run on LEDC (50 Hz, 13-bit, 500–2400 µs; `pan=GPIO13`, `tilt=GPIO14`), porting the
non-blocking keyframes from the legacy `firmware/src/servo_controller.cpp`.

## Pin map

See `ROADMAP.md`. Core: INMP441 `WS4/SCK5/SD6`; MAX98357A `DIN7/BCLK15/LRC16`; DotStar
`11/12`; servos `13/14`; avoid strapping pins `0/3/45/46` and native USB `19/20`.

## Reusable from the legacy design

- `brain/schema.py` enums (expressions/movements) — keep as the canonical names.
- `firmware/src/servo_controller.cpp` keyframes — port to the LEDC driver.
- `firmware/src/led_matrix.cpp` face art — port to the APA102 driver.
