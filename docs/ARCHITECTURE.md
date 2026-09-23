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

- **DotStar (APA102) face** — add a driver; map device state → expression.
- **SG90 pan-tilt** — add an LEDC driver; map state → motion.
- xiaozhi's built-in display layer covers OLED (SSD1306) + graphical LCD (ST7789/GC9A01).

## Pin map

See `ROADMAP.md`. Core: INMP441 `WS4/SCK5/SD6`; MAX98357A `DIN7/BCLK15/LRC16`; DotStar
`11/12`; servos `13/14`; avoid strapping pins `0/3/45/46` and native USB `19/20`.

## Reusable from the legacy design

- `brain/schema.py` enums (expressions/movements) — keep as the canonical names.
- `firmware/src/servo_controller.cpp` keyframes — port to the LEDC driver.
- `firmware/src/led_matrix.cpp` face art — port to the APA102 driver.
