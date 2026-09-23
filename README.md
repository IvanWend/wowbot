# WowBot

A small desktop robot: an ESP32 "body" with a DotStar LED-matrix face, a servo
arm, and a speech-synthesis module (robot voice), driven by a laptop running a local
speech loop:

```
VAD (mic) → Whisper (STT) → Ollama (schema-constrained JSON) → TTS → serial tokens → ESP32
```

- **Phase 0** is laptop-only (no hardware) and is the first thing to build.
- The ESP32 firmware and the serial link come online in **Phase 1** and later.

Read `docs/ROADMAP.md` for the full plan, hardware reality-check, pin map, and
power budget. `docs/ARCHITECTURE.md` is the single source of truth for the serial
token contract both sides implement against. `docs/PROGRESS.md` tracks what's done.

## Layout

```
wowbot/
├── brain/            # Phase 0+ laptop software (Python)
├── firmware/         # ESP32 firmware (PlatformIO / Arduino, Phase 1+)
├── docs/             # ROADMAP + ARCHITECTURE + PROGRESS + session kickoff
├── assets/mp3/       # (superseded — the speech module synthesizes audio, no clips)
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

`faster-whisper` downloads its model (`small` by default) on first use. The TTS
engine is `pyttsx3` (Windows SAPI5, offline, zero setup) by default; see
`docs/ARCHITECTURE.md` for `edge-tts`/`piper` alternatives when you want a better voice.

## Hardware (Phase 1+)

See `docs/ROADMAP.md` for the full pin map and power warnings. The two things that
matter most:

- **Verify the Maker-ESP32 V1.8 pinout with a multimeter** before wiring anything —
  it's an unbranded DevKit clone and the silkscreen may lie.
- **Do not share one 5 V rail** across the DotStar matrix, the TD-811MG servo, and
  the ESP32. The matrix (up to 15 A full white) and servo (3.4 A stall) each need
  their own supply with a common ground.

## Configuration

All laptop settings live in `config.yaml`. Key knobs:

| Key | Phase 0 default | Meaning |
|---|---|---|
| `serial_enabled` | `false` | set `true` in Phase 1 to talk to the ESP32 |
| `stt.model` | `small` | faster-whisper size (`tiny`/`base`/`small`/`medium`) |
| `llm.model` | `llama3.2` | Ollama model to chat with |
| `tts.engine` | `pyttsx3` | TTS backend |
| `serial.port` | `COM3` | Windows serial port (Phase 1) |

## Open questions (unblock later phases)

1. **Mic model** — a KY-038 analog sensor keeps voice on the laptop (current default);
   an INMP441 I2S mic would move capture onto the robot. Send the part number to
   settle Phase 0's final architecture.
2. **Chinese speech** — the DFRobot speech module speaks Chinese too, but the firmware
   is English-ASCII only. Add UTF-16LE if the loop ever emits non-ASCII.
