# WowBot

A standalone, laptop-free desktop robot: an **ESP32-S3 (N16R8)** body running
**xiaozhi-esp32** over Wi-Fi, with a DotStar LED-matrix face and a 2-axis pan-tilt head.

```
ESP32-S3 (xiaozhi-esp32) ──WebSocket/Wi-Fi──▶ self-hosted xiaozhi-esp32-server ──HTTPS──▶
  INMP441 mic (I2S in)                        SileroVAD · FunASR STT                  DashScope STT
  MAX98357A amp (I2S out)                     Kimi/DeepSeek/Qwen LLM                  Kimi/DeepSeek/Qwen
  DotStar face (custom APA102 driver)         Volcengine/CosyVoice TTS                Volcengine/CosyVoice TTS
  SG90 pan-tilt (custom LEDC driver)
```

**API keys live on the server only** — the firmware holds just the server's WebSocket URL.

- `docs/ROADMAP.md` — the full plan, component reality-check, pin map, and power budget.
- `docs/ARCHITECTURE.md` — the firmware ↔ server ↔ cloud contract.
- `docs/PROGRESS.md` — what's done and checked off.
- `docs/SESSION_KICKOFF.md` — the resume/handoff file.

## Status

- **Pivot to standalone: decided.** The earlier laptop-driven design (a Python voice loop
  `VAD → Whisper → Ollama → TTS` driving the ESP32 over a serial link) is **preserved as a
  legacy offline fallback**, not the active path.
- **Phase 0 (preserve, git): done** — initial commit.
- **Planning: done.** Implementation (server → firmware-talking MVP → face → motion) not
  started yet.

## Layout

```
wowbot/
├── brain/            # LEGACY laptop software (Python): VAD → STT → LLM → TTS → serial
├── firmware/         # LEGACY ESP32 firmware (PlatformIO / Arduino serial-token sketch)
├── docs/             # ROADMAP + ARCHITECTURE + PROGRESS + session kickoff
├── assets/           # (legacy) mp3 clip folder, superseded
├── tests/            # (legacy) laptop-side unit tests
├── config.yaml       # (legacy) laptop config
└── requirements.txt  # (legacy) laptop Python deps
```

The active firmware will live in its own `xiaozhi-esp32` (ESP-IDF) checkout; the server in
its own `xiaozhi-esp32-server` checkout — neither is in this tree yet.

## Hardware

| Function | Component | Notes |
|---|---|---|
| Brain | ESP32-S3 DevKit (N16R8) | 16 MB flash + 8 MB PSRAM, native USB, 2.4 GHz Wi-Fi |
| Firmware | xiaozhi-esp32 (ESP-IDF) | no-codec audio path (`NoAudioCodecSimplex`) |
| Server | xiaozhi-esp32-server (self-hosted) | holds API keys; VPS or always-on local box |
| Mic | INMP441 I2S | 3.3 V, L/R→GND |
| Speaker | MAX98357A amp + 3 W speaker | VIN 5 V, SD→3.3 V, GAIN→GND |
| Face | DotStar 16×16 (APA102) | **custom driver**; 3.3 V or level-shift |
| Head | 2× SG90 pan-tilt | **custom LEDC driver** |
| Power | Xiaomi 20000 mAh 22.5 W bank + MP1584EN buck | watch auto-shutoff + CC resistors |

See `docs/ROADMAP.md` for the pin map and the power warnings that matter most: the
ESP32-S3 board config must be the **N16R8 variant with PSRAM on** (PlatformIO's
`esp32-s3-devkitc-1` is the wrong N8 board), and the APA102 logic-high threshold means the
face runs at 3.3 V or through level shifters.

## Open questions

1. Server host — VPS vs always-on local box.
2. Exact `xiaozhi-esp32-server` config keys for Kimi (OpenAI-compatible) and
   SenseVoice/Paraformer (FunASR).
3. Type-C→XH2.54 breakout — confirm the 5.1 kΩ CC1/CC2 pull-downs (else use USB-A).
