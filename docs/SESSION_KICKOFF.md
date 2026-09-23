# Session Kickoff

This is the handoff file: **update it at the end of every session**, and paste the
"Kickoff prompt" block into a fresh Claude Code session to resume cleanly. It's meant
to get a new session up to speed in one shot — `PROGRESS.md` is the detailed checklist;
this file is the "where we are and what's next" summary.

---

## Kickoff prompt (paste this into a new session)

> You are resuming the **WowBot** project — a standalone, laptop-free desktop robot:
> an ESP32-S3 (N16R8) running **xiaozhi-esp32** over 2.4 GHz Wi-Fi, talking to a
> **self-hosted xiaozhi-esp32-server** that calls DashScope STT / Kimi·DeepSeek·Qwen
> LLM / Volcengine·CosyVoice TTS. Custom drivers are needed for the DotStar 16×16
> face (APA102) and the 2× SG90 pan-tilt — xiaozhi supports neither natively.
>
> Before doing anything, read these in order:
> 1. `docs/PROGRESS.md` — what's done and checked off.
> 2. `docs/ROADMAP.md` — the full plan, component reality-check, pin map, and power budget.
> 3. `docs/ARCHITECTURE.md` — the firmware ↔ server ↔ cloud contract.
>
> Then continue from the "Current state", "Key facts & gotchas", and "Next steps"
> sections of `docs/SESSION_KICKOFF.md`. Do NOT re-scaffold or re-verify work already
> marked done — pick up at the next unchecked item.

---

## Current state

- **Last updated:** 2026-09-23
- **Pivot to standalone:** DONE (decision made) — the laptop-driven design (voice loop
  + serial link) is **preserved as a legacy offline fallback**, not the active path.
- **Phase 0 (preserve, git):** DONE — initial commit (`052ef62`) on `master`.
- **Planning:** DONE — see `ROADMAP.md` (v4) and `ARCHITECTURE.md` (v4).
- **Phase 2 (self-hosted server):** DONE — running locally on the laptop.
  - Repo: `C:\Users\Ivan\Desktop\materials\projects\xiaozhi-esp32-server` (sibling of wowbot).
  - DeepSeek (LLM) verified live; FunASR/SenseVoice + SileroVAD + EdgeTTS all load.
  - WebSocket `ws://192.168.10.36:8000/xiaozhi/v1/`; start via `bash run-server.sh`.
- **Next:** Phase 3 — firmware talking MVP (the go/no-go gate).

## Environment & tooling

- OS: Windows 11 Pro. Shell: PowerShell (primary) or Git Bash (POSIX).
- Git repo on `master` — commit early and often; don't leave work uncommitted.
- PlatformIO CLI (`pio`) is on PATH — but that's for the **legacy** firmware only.
  The active path is **ESP-IDF v5.x** (not yet installed) for `xiaozhi-esp32`.
- Repo root: `C:\Users\Ivan\Desktop\materials\projects\wowbot`
- Server (separate clone): `C:\Users\Ivan\Desktop\materials\projects\xiaozhi-esp32-server`
  — run with `bash run-server.sh`; uses a uv-managed **Python 3.10** venv and native
  `ffmpeg` + `opus.dll` staged in `tools/bin/`.

## Key facts & gotchas

- **Keys live on the server** (`config.yaml` / `data/.config.yaml`), never in firmware —
  the firmware holds only the server's WebSocket URL (compile-time).
- **No-codec audio path** (`NoAudioCodecSimplex`): INMP441 `WS=4/SCK=5/SD=6`;
  MAX98357A `DIN=7/BCLK=15/LRC=16`, `SD→3.3V`, `GAIN→GND`.
- **PSRAM must be enabled** (N16R8) in the xiaozhi board config. PlatformIO's
  `esp32-s3-devkitc-1` board is the N8 variant (8 MB, no PSRAM) — **wrong** for this
  board; the standalone path uses ESP-IDF + a N16R8 board config, not PlatformIO.
- **APA102 logic threshold:** at 5 V the DotStar's logic-high is ~3.5 V → run the face
  at 3.3 V or level-shift the DATA/CLK lines.
- **Power-bank gotchas:** Xiaomi 20000 mAh 22.5 W bank auto-shuts-off at low idle
  current — need sustained ≥2.4 A @ 5 V and/or the double-press low-current/trickle mode.
- **Local server gotchas:** needs **Python 3.10** (3.13 fails — torch 2.2.2 has no 3.13
  wheels); run with `PYTHONUTF8=1` (Windows cp1252 console chokes on Chinese log lines);
  `opuslib_next` needs `opus.dll` + pydub needs `ffmpeg` on `PATH` (both in `tools/bin/`).

## Open questions

1. **Server host** — local laptop for dev/testing (current); VPS is the later upgrade.
2. ~~Config keys~~ resolved: DeepSeek is native (`DeepSeekLLM`, `type: openai`); FunASR local.
3. **Type-C breakout** — confirm the 5.1 kΩ CC1/CC2 pull-downs (else USB-A port).

## Next steps

1. Phase 3 — firmware talking MVP (ESP-IDF → clone `xiaozhi-esp32` → N16R8/PSRAM →
   point WebSocket URL at `ws://<laptop-ip>:8000/xiaozhi/v1/` → wire INMP441 + MAX98357A →
   verify wake-word → STT → LLM → TTS end-to-end).
2. Phase 4 / 5 — custom DotStar face + SG90 pan-tilt drivers.
3. (later) swap TTS EdgeTTS → CosyVoice; migrate server to VPS.

---

## End-of-session update checklist

Before finishing a session, update this file (and `docs/PROGRESS.md` to match):

- [ ] Bump the date in "Current state".
- [ ] Note what got done / verified.
- [ ] Update "Next steps" with the new order.
- [ ] Add any new gotchas to "Key facts & gotchas".
- [ ] Move any resolved "Open questions" out, or add new ones.
