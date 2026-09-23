# WowBot

A small desktop robot: an ESP32-S3 "body" with a DotStar LED-matrix face, a 2-axis
pan-tilt head, a mic, and a speaker, driven by a laptop running a local voice loop:

```
VAD (mic) → Whisper (STT) → Ollama (schema-constrained JSON) → TTS → serial tokens + audio → ESP32-S3
```

- **Phase 0** (laptop voice loop) and **Phase 1** (serial link) are done; the hardware
  is an ESP32-S3 (N16R8) with native USB for bidirectional audio.
- Read `docs/ROADMAP.md` for the full plan, hardware reality-check, pin map, and power
  budget. `docs/ARCHITECTURE.md` is the single source of truth for the serial-token +
  audio-framing contract. `docs/PROGRESS.md` tracks what's done; `docs/SESSION_KICKOFF.md`
  is the resume/handoff file.

## Layout

```
wowbot/
├── brain/            # laptop software (Python): VAD → STT → LLM → TTS → serial
├── firmware/         # ESP32-S3 firmware (PlatformIO / Arduino)
├── docs/             # ROADMAP + ARCHITECTURE + PROGRESS + session kickoff
├── assets/mp3/       # (superseded — audio now streams, no clip playback)
├── tests/            # laptop-side unit tests (no hardware needed)
├── config.yaml       # laptop config
└── requirements.txt
```

## Quickstart — Phase 0 (laptop only)

Requires Python 3.10+.

```bash
cd brain/..
python -m venv .venv
# Windows:
.venv\Scripts\activate
pip install -r requirements.txt

# Pull an Ollama model (adjust the name in config.yaml to match)
ollama pull llama3.2

# Sanity-check the pipeline without a mic:
python -m brain --text "what's your favorite color?"

# Or run the full mic loop:
python -m brain
```

`faster-whisper` downloads its model (`small` by default) on first use. The TTS engine
is `pyttsx3` (Windows SAPI5, offline) by default; Phase 6 swaps in `edge-tts`/`piper` so
the laptop can stream audio bytes to the robot's speaker.

## Hardware (Phase 1+)

See `docs/ROADMAP.md` for the full pin map and power warnings. The two things that matter
most:

- **Verify the ESP32-S3 pin map with a multimeter** before wiring — GPIO19/20 (USB) and
  the strapping pins (GPIO0/3/45/46) must stay clear.
- **Do not share one 5 V rail** across the DotStar matrix, the servos, and the ESP32-S3.
  The matrix (up to 15 A full white) and servos (stall transients) need their own supply
  with a common ground; the MP1584EN buck isolates the 3.3 V logic rail.

## Configuration

All laptop settings live in `config.yaml`. Key knobs:

| Key | Default | Meaning |
|---|---|---|
| `serial_enabled` | `false` | set `true` in Phase 1 to talk to the ESP32-S3 |
| `serial.port` | `COM3` | the S3's native-USB COM port (find via `python -m serial.tools.list_ports`) |
| `stt.model` | `small` | faster-whisper size (`tiny`/`base`/`small`/`medium`) |
| `llm.model` | `llama3.2` | Ollama model to chat with |
| `tts.engine` | `pyttsx3` | TTS backend (`edge-tts`/`piper` in Phase 6) |

## Open questions

1. **ESP32-S3 pin map** — verify the proposed GPIO allocation against the actual board.
2. **TTS engine** — `edge-tts` (network, high quality) vs `piper` (offline neural).
