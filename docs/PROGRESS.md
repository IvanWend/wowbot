# WowBot — Progress

Last updated: 2026-09-24

Running record. Phases mirror `ROADMAP.md` (v4 — standalone cloud robot); the contract
lives in `ARCHITECTURE.md`.

## Status at a glance

| Phase | Status | Result |
|---|---|---|
| — laptop brain (legacy) | ✅ Preserved | voice loop + serial link verified; kept as offline fallback |
| 0 — preserve (git) | ✅ Done | initial commit `052ef62` |
| 1 — power (bank) | ⬜ Pending | |
| 2 — self-hosted server | ✅ Done | running locally; DeepSeek verified live |
| 3 — firmware talking MVP | ✅ Done | end-to-end verified; only amp decoupling cap left |
| 4 — DotStar face | 📝 Designed | full-animated APA102 driver spec in ROADMAP/ARCHITECTURE; not implemented |
| 5 — servo pan-tilt | 📝 Designed | LEDC keyframe driver spec captured; not implemented |
| 6 — polish | ⬜ Pending | |

## Checklist

### Legacy (laptop path — preserved, not active)
- [x] Phase 0 laptop voice loop (VAD→Whisper→Ollama→TTS) — verified
- [x] Phase 1 serial link (PING→PONG, ACK:) on Maker-ESP32; firmware ported to S3 (builds)
- [ ] (optional) keep as an offline fallback if ever needed

### Phase 0 — preserve
- [x] Initial git commit of the repo (`052ef62`)

### Phase 2 — self-hosted server
- [x] Choose host (local laptop for now; VPS later)
- [x] Clone `xinnan-tech/xiaozhi-esp32-server`; install deps (Python 3.10 venv via uv + native ffmpeg/opus); run
- [x] Configure `data/.config.yaml`: SileroVAD, FunASR (SenseVoice), LLM (DeepSeek), TTS (EdgeTTS for now) + keys
- [x] Note WebSocket URL `ws://192.168.10.36:8000/xiaozhi/v1/`; device auth (disabled for dev)
- [ ] (later) swap TTS EdgeTTS → CosyVoice/Volcengine; migrate server to VPS

### Phase 3 — firmware talking MVP
- [x] Install ESP-IDF **v6.1** toolchain via EIM (`winget install Espressif.EIM-CLI` → `eim install -i v6.1 -t esp32s3`)
- [x] Clone `xiaozhi-esp32`; custom board `wowbot-n16r8` (no-codec simplex, NoDisplay; 16 MB flash + 8 MB PSRAM are S3 defaults)
- [x] Point firmware at the server via `CONFIG_OTA_URL` (board `config.json` → `http://192.168.10.36:8003/xiaozhi/ota/`)
- [x] Wire INMP441 (WS4/SCK5/SD6) + MAX98357A (DIN7/BCLK15/LRC16)
- [x] Flash (CH340 COM7); verify wake-word → STT → LLM → TTS end-to-end
- [x] Fix no-display LVGL crash — call `lv_init()` in the board constructor
- [ ] Add 100–470 µF decoupling cap across MAX98357A VIN/GND (clears the class-D squeal)

### Phase 4 — DotStar face
- [x] Design captured (ROADMAP/ARCHITECTURE): `DotStarFaceDisplay : Display`, SPI2 FSPI
  (MOSI=11/CLK=12/MISO=-1), APA102 framing, full-animated (blink/listening/speaking/
  thinking/crossfade), brightness cap, mixed-case emotion normalization
- [ ] Implement `main/boards/wowbot-n16r8/dotstar_face_display.{h,cc}` (future session)
- [ ] Flash + verify on the matrix

### Phase 5 — servo pan-tilt
- [x] Design captured: `ServoController` (LEDC 50 Hz, pan=13/tilt=14), port legacy
  keyframes, `DeviceStateMachine` listener + emotion→gesture mapping
- [ ] Implement `main/boards/wowbot-n16r8/servo_controller.{h,cc}` (future session)
- [ ] Flash + verify motion

## Key decisions
- xiaozhi-esp32 + self-hosted xiaozhi-esp32-server; custom DotStar face; power bank; LCD 2004 dropped.
- API keys on the server, not firmware.
- Bilingual: STT = SenseVoice (auto zh/en); LLM prompt follows the user's language;
  TTS = EdgeTTS **multilingual** voice `en-US-AvaMultilingualNeural` (English-first, also speaks Chinese —
  a plain English voice errors on Chinese text).

## Open questions
1. Type-C breakout CC resistors (Phase 1 power, later).
2. TTS upgrade: EdgeTTS → CosyVoice/Volcengine for natural code-switching (later).

## Next steps (in order)
1. Add 100–470 µF decoupling cap across MAX98357A VIN/GND → full voice loop complete.
2. Implement Phase 4 face driver (`dotstar_face_display.{h,cc}`) per the captured design.
3. Implement Phase 5 servo driver (`servo_controller.{h,cc}`).
4. Phase 6 polish.
