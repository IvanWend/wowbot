# WowBot — Architecture & Serial Contract

The single source of truth for the laptop ↔ ESP32-S3 interface. Both sides implement
against this document. `docs/ROADMAP.md` holds the hardware plan and phases.

## Two channels over one USB link

The ESP32-S3's native USB carries **two protocols** over the same physical link:

1. **Control tokens** — newline-terminated text (below). Low bandwidth, for face/motion/status.
2. **Audio stream** — length-prefixed binary frames, full-duplex: mic PCM up, TTS audio
   down. High bandwidth (the reason for the S3's native USB).

## Data flow

```
              laptop (brain/)                         ESP32-S3 (firmware/)
   ┌─────────┐   ┌──────────┐   ┌─────┐   ┌──────────────┐
   │   VAD   │──▶│  Whisper │──▶│Ollama│──▶│ TTS (voice)  │
   │segmenter│   │  (STT)   │   │(JSON)│   │ → audio down │
   └────▲────┘   └──────────┘   └──┬──┘   └──────┬───────┘
        │ PCM up                   │ RobotAction  │
        │                          ▼              │
   ┌────┴──────────────────────────────┐          │
   │   native USB CDC: control tokens  │◀─────────┘
   │          + binary audio           │
   └────┬──────────────────────────────┘
        │ EXP:/MOV:/BRIGHT:  +  audio bytes
   ┌────▼──────────────────────────────────────────────┐
   │                   ESP32-S3 (N16R8)                 │
   │  I2S0 RX ← INMP441    I2S1 TX → MAX98357A → spkr   │
   │  SPI → DotStar face   LEDC → 2× SG90 pan-tilt      │
   │  I2C → LCD 2004 (caption)                          │
   └─────────────────────────────────────────────────────┘
```

Three layers of the contract, in order of specificity:

1. **LLM JSON** — what Ollama is prompted to emit (`{expression, movement, speech}`).
2. **`RobotAction`** — the validated, canonical object the laptop code works with.
3. **control tokens** — newline-terminated strings sent to the ESP32-S3.

## Control protocol (native USB CDC, newline-terminated)

Laptop → ESP32-S3:

| Token | Example | Phase | Meaning |
|---|---|---|---|
| `EXP:<EXPR>` | `EXP:HAPPY` | 1 (ACK) / 3 (face) | set face expression |
| `MOV:<MOVE>` | `MOV:NOD` | 1 (ACK) / 2 (head) | trigger a non-blocking gesture |
| `BRIGHT:<0-255>` | `BRIGHT:40` | 3 | set DotStar brightness |
| `PING` | `PING` | 1 | link test |

ESP32-S3 → laptop:

| Token | Meaning |
|---|---|
| `READY` | sent once at boot |
| `PONG` | reply to `PING` |
| `ACK:<cmd>` | a recognized command was dispatched (e.g. `ACK:EXP:HAPPY`) |
| `ERR:<msg>` | unrecognized command |

`EXP`/`MOV` names are case-insensitive on the firmware.

### Planned control tokens

- `LCD:<line1\|line2\|line3\|line4>` — caption/status text on the 2004 LCD (Phase 4).

### Optional legacy token

- `SFX:<n>` — play JQ8400-FN flash clip #n. The JQ8400 is owned but **redundant** now
  that the MAX98357A streams all audio; keep this only if you want zero-latency clips.

## Audio protocol (native USB, length-prefixed binary)

Not newline text — a framing layer: `[len][payload]` chunks. Full-duplex:

- **Upstream (mic):** INMP441 → I2S0 → 16 kHz / 16-bit mono PCM → laptop → Whisper.
- **Downstream (voice):** laptop TTS → audio bytes → I2S1 → MAX98357A → 3 W speaker.

Native USB at 921600+ carries raw 16 kHz / 16-bit mono (~32 KB/s) each way without
compression. **The laptop TTS must emit audio bytes**, so `pyttsx3` (which drives the
Windows sound device directly) is replaced by `edge-tts` (MP3) or `piper` (WAV).

## Canonical enums

Expressions (`EXP:`) and movements (`MOV:`) — defined identically in
`brain/schema.py` and `firmware/include/{expressions,servo_controller}.h`.

Expressions: `neutral, happy, sad, curious, angry, surprised, sleepy, thinking, excited`

Movements: `none, wave, nod, shake, look_left, look_right, tilt, dance`

`none` never produces a `MOV:` token (laptop drops it). `wave` needs an arm servo; the
2-axis head covers nod/shake/look/tilt.

## Laptop module responsibilities

| Module | Phase | Responsibility |
|---|---|---|
| `config.py` | 0 | load `config.yaml` into nested namespaces |
| `schema.py` | 0 | enums, `RobotAction`, JSON parsing, token encoding |
| `vad.py` | 0 | `webrtcvad` + utterance segmentation |
| `stt.py` | 0 | faster-whisper transcription (fed by the robot's mic PCM) |
| `llm.py` | 0 | Ollama client → `RobotAction` |
| `tts.py` | 6 | TTS → **audio bytes** (edge-tts / piper), streamed to the robot |
| `audio.py` | 6 | mic PCM — via serial (robot mic) with `sounddevice` fallback |
| `serial_link.py` | 1 | native-USB bridge: control tokens + binary audio framing |
| `robot.py` | 1 | `RobotAction` → `EXP:`/`MOV:` tokens + audio playback |

## Firmware module responsibilities

| Module | Phase | Responsibility |
|---|---|---|
| `config_pins.h` | 1 | ESP32-S3 pin map (verify against the board) |
| `tokens.h` | 1 | parse control lines into `Command` |
| `expressions.h` | 1 | expression enum |
| `servo_controller.*` | 2 | non-blocking `millis()` pan-tilt gesture state machine |
| `led_matrix.*` | 3 | DotStar face, brightness cap, static patterns |
| `lcd_controller.*` | 4 | I2C 2004 caption/status text |
| `i2s_audio.*` | 6 | I2S0 mic capture + I2S1 speaker playback + binary framing |
| `main.cpp` | 1 | setup/loop, serial dispatch, ACK replies |

## TTS / STT alternatives

- **TTS** — for on-robot voice the engine must emit audio bytes: `edge-tts` (network,
  MP3, very good quality) or `piper` (offline neural WAV). `pyttsx3` is retained only as
  a headless laptop-speaker fallback.
- **STT** — `faster-whisper` (local). Cloud fallback to the OpenAI Whisper API is a
  one-file swap in `stt.py`.

The `tts.py`/`stt.py` interfaces are the only places that care, so swapping is contained.
