# Session Kickoff

This is the handoff file: **update it at the end of every session**, and paste the
"Kickoff prompt" block into a fresh Claude Code session to resume cleanly. It's meant
to get a new session up to speed in one shot — `PROGRESS.md` is the detailed checklist;
this file is the "where we are and what's next" summary.

---

## Kickoff prompt (paste this into a new session)

> You are resuming the **WowBot** project — a desktop robot: an ESP32-S3 body (DotStar
> LED-matrix face, 2-axis pan-tilt head, INMP441 mic + MAX98357A speaker, LCD 2004
> status display) driven by a laptop running a local voice loop (VAD → Whisper → Ollama
> → TTS → serial tokens + streamed audio).
>
> Before doing anything, read these in order:
> 1. `docs/PROGRESS.md` — what's done and checked off.
> 2. `docs/ROADMAP.md` — the full plan, pin map, and power budget.
> 3. `docs/ARCHITECTURE.md` — the serial token contract both sides implement.
>
> Then continue from the "Current state", "Key facts & gotchas", and "Next steps"
> sections of `docs/SESSION_KICKOFF.md`. Do NOT re-scaffold or re-verify work already
> marked done — pick up at the next unchecked item.

---

## Current state

- **Last updated:** 2026-09-18
- **Phase 0** (laptop voice loop): DONE — verified end-to-end.
- **Phase 1** (serial link): DONE on the Maker-ESP32 V1.8 (`PING`→`PONG`,
  `EXP:HAPPY`→`ACK:EXP:HAPPY`); **needs re-flashing to the ESP32-S3** (native USB CDC).
- **Hardware upgraded:** purchased ESP32-S3 (N16R8), INMP441 mic, MAX98357A amp + 3 W
  speaker, 2-axis pan-tilt (2× SG90), Mean Well LRS-35-5 (5 V / 7 A), MP1584EN buck,
  1000 µF caps. The Maker-ESP32 and KY-038 are retired; the JQ8400-FN is now optional.
- **Last action:** re-analyzed the audio path; bought the final hardware; docs updated.
- **Where we stopped:** firmware still targets the Maker-ESP32 (classic ESP32 pins) and
  still holds the stale DFRobot `speech_controller.*` (JQ8400/SAY) — both need updating
  for the S3 + streaming-audio architecture.

## Environment & tooling

- OS: Windows 11 Pro. Shell: Git Bash (POSIX syntax — use `/dev/null`, not `NUL`).
- Python 3.13; deps in `requirements.txt` are already installed.
- PlatformIO CLI: `pio` is on PATH. Firmware is the PlatformIO project in `firmware/`.
- Board: **ESP32-S3 DevKit (N16R8)** — native USB. (Old Maker-ESP32 V1.8 was COM3 @ 115200.)
- Not a git repo yet — recommend `git init` before further renames/deletions.
- Repo root: `C:\Users\Ivan\Desktop\materials\projects\wowbot`

## Key facts & gotchas

- **Brain = ESP32-S3 (N16R8):** 16 MB flash + 8 MB PSRAM, **native USB** (GPIO19/20 —
  reserved). >921600 baud carries bidirectional audio.
- **Audio is full-duplex streaming:** INMP441 (I2S0) up → Whisper; laptop TTS → bytes →
  MAX98357A (I2S1) down. Length-prefixed binary frames, distinct from the newline tokens.
- **`pyttsx3` can't stream to the robot** — it drives the Windows sound device directly.
  Swap to `edge-tts` (MP3) or `piper` (WAV) so the laptop can send audio bytes.
- **JQ8400-FN is now optional** — the MAX98357A streams all audio, so the MP3 module is
  redundant unless kept for zero-latency SFX. `speech_controller.*` still holds the
  wrong DFRobot `SAY:` code and should be removed/replaced.
- **Power:** feed the S3 either 5 V to VIN (onboard regulator) *or* 3.3 V to 3V3 from the
  MP1584 — never both, and never with USB 5 V connected at the same time.
- PlatformIO DotStar pin is `adafruit/Adafruit DotStar@^1.2.5` — **not** `^1.6.4`
  (doesn't exist on the registry → `UnknownPackageError`).
- The user's keyboard has a Cyrillic layout; typing commands in it garbles them
  (`ping` → `зштп`).
- Defaults: TTS = `pyttsx3` (→ swap), STT = `faster-whisper`, LLM = Ollama `format: json`.
- Local config overrides: copy `config.yaml` → `config.local.yaml` (git-ignored).

## Open questions

1. **ESP32-S3 pin map** — verify the proposed GPIO allocation against the actual board.
2. **TTS engine** — `edge-tts` (network, high quality) vs `piper` (offline neural).

## Next steps

1. Flash the ESP32-S3; confirm native USB CDC + `READY`/`PONG`.
2. Verify the S3 pin map; wire the 2× SG90 pan-tilt (GPIO13/14) and confirm gestures.
3. Wire the DotStar (SPI) and confirm `EXP:` faces.
4. Wire INMP441 + MAX98357A; build the length-prefixed audio framing (laptop + firmware).
5. LCD captions, then body/packaging + portfolio polish.

---

## End-of-session update checklist

Before finishing a session, update this file (and `docs/PROGRESS.md` to match):

- [ ] Bump the date in "Current state".
- [ ] Note what got done / verified.
- [ ] Update "Next steps" with the new order.
- [ ] Add any new gotchas to "Key facts & gotchas".
- [ ] Move any resolved "Open questions" out, or add new ones.
