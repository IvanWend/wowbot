# WowBot — Progress

Last updated: 2026-09-23

Running record. Phases mirror `ROADMAP.md` (v4 — standalone cloud robot); the contract
lives in `ARCHITECTURE.md`.

## Status at a glance

| Phase | Status | Result |
|---|---|---|
| — laptop brain (legacy) | ✅ Preserved | voice loop + serial link verified; kept as offline fallback |
| 0 — preserve (git) | ✅ Done | initial commit `052ef62` |
| 1 — power (bank) | ⬜ Pending | |
| 2 — self-hosted server | ✅ Done | running locally; DeepSeek verified live |
| 3 — firmware talking MVP | ⬜ Core | the go/no-go gate |
| 4 — DotStar face | ⬜ Custom | |
| 5 — servo pan-tilt | ⬜ Custom | |
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
- [ ] Install ESP-IDF v5.x toolchain
- [ ] Clone `xiaozhi-esp32`, select N16R8 board config (PSRAM on)
- [ ] Set WebSocket URI to the server
- [ ] Wire INMP441 (WS4/SCK5/SD6) + MAX98357A (DIN7/BCLK15/LRC16)
- [ ] Flash; verify wake-word → STT → LLM → TTS end-to-end

### Phase 4 — DotStar face
- [ ] Custom APA102 driver
- [ ] Expression states (idle/blink/listening/thinking/speaking)
- [ ] Brightness cap verified

### Phase 5 — servo pan-tilt
- [ ] LEDC driver; port keyframes from the legacy servo controller
- [ ] State→motion mapping verified

## Key decisions
- xiaozhi-esp32 + self-hosted xiaozhi-esp32-server; custom DotStar face; power bank; LCD 2004 dropped.
- API keys on the server, not firmware.

## Open questions
1. Server host (VPS vs local box). 2. Exact Kimi/SenseVoice config keys. 3. Type-C breakout CC resistors.

## Next steps (in order)
3 (talking MVP) → 4 (face) → 5 (motion) → 6 (polish).
