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

---a

## Current state

- **Last updated:** 2026-09-24
- **Pivot to standalone:** DONE (decision made) — the laptop-driven design (voice loop
  + serial link) is **preserved as a legacy offline fallback**, not the active path.
- **Phase 0 (preserve, git):** DONE — initial commit (`052ef62`) on `master`.
- **Planning:** DONE — see `ROADMAP.md` (v4) and `ARCHITECTURE.md` (v4).
- **Phase 2 (self-hosted server):** DONE — running locally on the laptop.
  - Repo: `C:\Users\Ivan\Desktop\materials\projects\xiaozhi-esp32-server` (sibling of wowbot).
  - DeepSeek (LLM) verified live; FunASR/SenseVoice + SileroVAD + EdgeTTS all load.
  - WebSocket `ws://192.168.10.36:8000/xiaozhi/v1/`; start via `bash run-server.sh`.
- **Phase 3 (firmware talking MVP):** DONE — end-to-end verified.
  - ESP-IDF **v6.1** (via EIM); custom board `wowbot-n16r8` in `main/boards/` (no-codec simplex + `NoDisplay` + `lv_init()`).
  - Firmware repo: `C:\Users\Ivan\Desktop\materials\projects\xiaozhi-esp32` (sibling of wowbot).
  - Board flashed over CH340 **COM7**; MAC `1c:29:04:23:c7:08`; wake word `你好小智`.
  - Wake-word → STT → LLM → TTS all verified; only the amp decoupling cap (squeal) remains.
- **Phase 4 / 5 (custom drivers):** DESIGN CAPTURED, not implemented — the exact hook
  points and driver plan are in `ROADMAP.md` ("Custom driver design") and
  `ARCHITECTURE.md`. Both drivers go in `main/boards/wowbot-n16r8/` (auto-globbed, no
  CMake edit): a `DotStarFaceDisplay : Display` (SPI2 FSPI, full animated) and a
  `ServoController` (LEDC, `DeviceStateMachine` listener). Build next session — **no code
  was written this session by design.**

## Environment & tooling

- OS: Windows 11 Pro. Shell: PowerShell (primary) or Git Bash (POSIX).
- Git repo on `master` — commit early and often; don't leave work uncommitted.
- PlatformIO CLI (`pio`) is on PATH — legacy firmware only. The active path is
  **ESP-IDF v6.1**, installed via **EIM** (`winget install Espressif.EIM-CLI` → `eim install`).
  Run IDF commands from **PowerShell** with `eim run "<cmd>" v6.1` (MSys/Git Bash is unsupported by IDF v6).
- Firmware repo: `C:\Users\Ivan\Desktop\materials\projects\xiaozhi-esp32` (sibling of wowbot).
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
- **Firmware gotchas:** ESP-IDF **v6.1** (not v5). The firmware finds the server via
  **`CONFIG_OTA_URL`** (set in `main/boards/wowbot-n16r8/config.json` → `http://192.168.10.36:8003/xiaozhi/ota/`),
  not a hardcoded WebSocket URL — that IP is baked in, so it must change if the laptop's IP changes.
  **No-display boards must call `lv_init()`** in the board ctor (else a LoadProhibited crash when the font loads).
- **TTS voice:** must be a **multilingual** EdgeTTS voice (`en-US-AvaMultilingualNeural`) —
  English-only voices (`en-US-JennyNeural`) error on Chinese text; `zh-CN-XiaoxiaoNeural` speaks both but Chinese-first.
- **Audio hardware:** MAX98357A needs a **100–470 µF decoupling cap** across VIN/GND (class-D squeal without it);
  `SD→3.3V` (enable), `GAIN→GND`, VIN fed from the board's **5 V pin**, common ground with the board.
- **APA102 not in `led_strip`:** the ESP-IDF `led_strip` component's `led_model_t` is only
  `WS2812/SK6812/WS2811/WS2816` — drive the DotStar with the raw **SPI master** driver on
  `SPI2_HOST` (`MOSI=11`, `CLK=12`, `MISO=-1`), not `led_strip_spi`.
- **Board `.cc` files are auto-globbed:** `main/CMakeLists.txt` globs `boards/${BOARD_DIR}/*.cc`,
  so new drivers under `main/boards/wowbot-n16r8/` compile with **no CMake edit**. (Adding
  to `main/display/` *would* require editing the explicit `SOURCES` list.)
- **Emotion strings are mixed-case:** server FunASR tags are **uppercase**
  (`HAPPY/SAD/ANGRY/…`), the emoji path sends **lowercase**, firmware hardcodes `neutral`
  — the face driver must normalize case-insensitively.
- **Servo boot order is safe:** `main.cc` builds `Application` (state machine) *before*
  `Board::GetInstance()`, so the board ctor can register a `DeviceStateMachine` listener.
  LEDC servo reference: `main/boards/electron-bot/oscillator.cc` (50 Hz, 13-bit).

## Open questions

1. **Server host** — local laptop for dev/testing (current); VPS is the later upgrade.
2. ~~Config keys~~ resolved: DeepSeek is native (`DeepSeekLLM`, `type: openai`); FunASR local.
3. **Type-C breakout** — confirm the 5.1 kΩ CC1/CC2 pull-downs (else USB-A port).

## Next steps

1. Add the MAX98357A decoupling cap (100–470 µF across VIN/GND) → clear the squeal → voice loop complete.
2. Implement Phase 4 face driver `main/boards/wowbot-n16r8/dotstar_face_display.{h,cc}`
   per the captured design (ROADMAP "Custom driver design").
3. Implement Phase 5 servo driver `main/boards/wowbot-n16r8/servo_controller.{h,cc}`.
4. (later) swap TTS EdgeTTS → CosyVoice; migrate server to VPS; bake a stable IP/hostname.

---

## End-of-session update checklist

Before finishing a session, update this file (and `docs/PROGRESS.md` to match):

- [ ] Bump the date in "Current state".
- [ ] Note what got done / verified.
- [ ] Update "Next steps" with the new order.
- [ ] Add any new gotchas to "Key facts & gotchas".
- [ ] Move any resolved "Open questions" out, or add new ones.
