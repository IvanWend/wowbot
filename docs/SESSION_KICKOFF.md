# Session Kickoff

This is the handoff file: **update it at the end of every session**, and paste the
"Kickoff prompt" block into a fresh Claude Code session to resume cleanly. It's the
"where we are and what's next" summary — `PROGRESS.md` is the detailed checklist,
`ROADMAP.md` is the full plan + pin map, `ARCHITECTURE.md` is the firmware↔server↔cloud
contract.

---

## Kickoff prompt (paste this into a new session)

> You are resuming **WowBot** — a standalone, laptop-free desktop robot: an ESP32-S3
> (N16R8) running **xiaozhi-esp32** that talks over 2.4 GHz Wi-Fi to a **self-hosted
> xiaozhi-esp32-server**, which calls DashScope STT / Kimi·DeepSeek·Qwen LLM /
> Volcengine·CosyVoice TTS. Hardware: INMP441 mic + MAX98357A speaker, a custom
> DotStar 16×16 face, and a 2× SG90 pan-tilt head, powered by a USB power bank.
>
> Before doing anything, read these in order:
> 1. `docs/PROGRESS.md` — what's done and checked off.
> 2. `docs/ROADMAP.md` — the full plan, pin map, and power.
> 3. `docs/ARCHITECTURE.md` — the firmware ↔ server ↔ cloud contract.
>
> **Then enter plan mode. Do NOT start writing or editing code.** Read the "Current
> state", "Key facts & gotchas", and "Next steps" sections below, verify them against
> the repo, and *craft the next step as a plan* for approval — at the
> architecture/decision level — before any implementation. The user steers; you plan,
> then (only after approval) generate code.

---

## Current state

- **Last updated:** 2026-09-23
- **Pivot (planning done, implementation not started).** WowBot moved from a
  laptop-driven design (VAD → Whisper → Ollama → TTS → serial tokens) to a **standalone
  xiaozhi-esp32** robot. The laptop path (`brain/` + Arduino `firmware/`) is preserved as
  an offline fallback, not deleted.
- **Decisions locked:** self-host `xiaozhi-esp32-server`; **custom DotStar face** (APA102
  16×16); **USB power bank** instead of the mains PSU; LCD 2004 dropped.
- **Where we stopped:** the plan is captured (below + `ROADMAP.md`). Nothing on the new
  path has been cloned, flashed, or wired yet.

## Environment & tooling

- OS: Windows 11 Pro. Shell: Git Bash (POSIX) or PowerShell.
- **ESP-IDF v5.x** toolchain for xiaozhi-esp32 (VS Code + ESP-IDF extension, or `idf.py`
  CLI). PlatformIO is still present but only for the legacy `firmware/`.
- Repos to clone: `78/xiaozhi-esp32` (firmware) and `78/xiaozhi-esp32-server` (server).
- Board: **ESP32-S3 DevKit (N16R8)** — native USB; Wi-Fi **2.4 GHz only**.
- Power: Xiaomi 20000 mAh 22.5 W bank (5 V) → Type-C→XH2.54 breakout → MP1584EN buck → 3.3 V rail.
- Repo is a git repo (branch `master`) with uncommitted work — make an initial commit first.

## Key facts & gotchas

- **API keys live on the server, not the firmware.** The firmware only holds the server's
  WebSocket URL (compile-time); DashScope/Kimi/DeepSeek/Volcengine keys go in the server's
  `config.yaml` / `data/.config.yaml`.
- **DotStar (APA102), character LCDs, and servos are not native to xiaozhi** — all three
  are custom drivers. xiaozhi's display layer covers OLED (SSD1306) + graphical LCD
  (ST7789/GC9A01) only.
- **No-codec audio path** (`NoAudioCodecSimplex`) is purpose-built for INMP441 + MAX98357A:
  mic `WS=4/SCK=5/SD=6`; amp `DIN=7/BCLK=15/LRC=16`.
- **MAX98357A** `SD`→3.3 V, `GAIN`→GND; **INMP441** `L/R`→GND (left channel).
- **APA102 at 5 V** has a ~3.5 V logic-high threshold → run the DotStar at 3.3 V (dimmer)
  or level-shift, or 3.3 V data is marginal at 5 V.
- **PSRAM must be enabled** in the xiaozhi board config (N16R8) or the audio/AI stack
  won't fit.
- PlatformIO's `esp32-s3-devkitc-1` board is the **N8 (8 MB, no PSRAM)** variant — xiaozhi
  needs a true N16R8 config.
- **Power bank = Xiaomi 20000 mAh 22.5 W** (≈12,000 mAh @ 5 V). Feed from its highest-amp
  port and use the **double-press low-current/trickle mode** (if this model has it) to stop
  it auto-shutting-off at idle.
- **A bare 2-wire Type-C breakout won't output 5 V from a C-to-C cable** — USB-C sources
  energize VBUS only after the sink pulls CC1/CC2 down with 5.1 kΩ. Confirm the breakout has
  those resistors, or use the bank's USB-A port + A→C cable (always-on 5 V).
- Kimi rides the OpenAI-compatible provider; SenseVoice/Paraformer rides the FunASR provider.
- Legacy: the old `brain/` + `firmware/` still build (Arduino/PlatformIO) and can drive the
  serial-token path offline.

## Open questions

1. **Server host** — VPS vs always-on local box (unblocks Phase 2).
2. **Exact server config keys** for Kimi (LLM) and SenseVoice/Paraformer (STT).
3. **Type-C breakout CC resistors** — confirm the 5.1 kΩ CC1/CC2 pull-downs are on the
   breakout, or plan on the bank's USB-A port + A→C cable.

## Next steps

1. **Phase 0** — initial git commit of the repo (preserve the laptop fallback).
2. **Phase 2** — stand up the self-hosted server (host → clone → keys → WebSocket URL).
3. **Phase 3** — firmware talking MVP (ESP-IDF → clone xiaozhi-esp32 → N16R8 config →
   wire mic+amp → flash → verify wake-word→STT→LLM→TTS).
4. **Phase 4** — custom DotStar face driver.
5. **Phase 5** — servo pan-tilt driver.
6. **Phase 6** — polish.

---

## End-of-session update checklist

Before finishing a session, update this file (and `docs/PROGRESS.md` to match):

- [ ] Bump the date in "Current state".
- [ ] Note what got done / verified.
- [ ] Update "Next steps" with the new order.
- [ ] Add any new gotchas to "Key facts & gotchas".
- [ ] Move any resolved "Open questions" out, or add new ones.
