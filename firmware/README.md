# WowBot Firmware (ESP32-S3) — LEGACY

> ⚠️ **This is the legacy firmware** from the retired laptop-driven design (serial-token
> sketch: `PING`/`ACK:`/`EXP:`/`MOV:`). It is **preserved as an offline fallback, not the
> active path.** The active firmware is `xiaozhi-esp32` (ESP-IDF), documented in
> `../docs/ROADMAP.md` and `../docs/ARCHITECTURE.md`.

PlatformIO project for the ESP32-S3 side. Target board is the **ESP32-S3 DevKitC-1
(N16R8)** — 16 MB flash + 8 MB OPI PSRAM, **native USB** (GPIO19/20 = the laptop link).
See `../docs/ROADMAP.md` for the pin map. **Verify the pins against your board's
silkscreen with a multimeter before wiring.**

## Build & flash

```bash
pio run -t upload          # build + flash over the native USB port
pio device monitor         # 115200 (virtual on USB CDC)
```

If you prefer the Arduino IDE: board = `ESP32S3 Dev Module` (or `ESP32-S3-DevKitC-1`),
USB CDC On Boot = Enabled. Install `ESP32Servo` and `Adafruit DotStar` from the Library
Manager, copy `include/*.h` + `src/*.cpp` (rename `src/main.cpp` to the sketch).

## What each phase expects

| Phase | Flash this | Visible result |
|---|---|---|
| 1 | full sketch | `READY` at boot; `PING`→`PONG`; `ACK:` to `EXP:`/`MOV:`/`BRIGHT:` |
| 2 | attach 2× SG90 to GPIO13/14 | `MOV:NOD`/`SHAKE`/`LOOK_LEFT` drive pan-tilt |
| 2.5 | set `ARM_ENABLED=1`, attach TD-811MG (own ≥6 V rail) | heavy joint moves |
| 3 | wire DotStar to GPIO11/12 | `EXP:` sets the face, `BRIGHT:` scales it |
| 6 | wire INMP441 (I2S0) + MAX98357A (I2S1) | mic PCM up, TTS audio down |

## Manual smoke test (no laptop)

Open the serial monitor at 115200 and paste one line at a time:

```
PING          -> PONG
EXP:HAPPY     -> ACK:EXP:HAPPY  (face fires once wired)
MOV:NOD       -> ACK:MOV:NOD    (pan-tilt gesture once servos are wired)
BRIGHT:20     -> ACK:BRIGHT:20
```

## Power warnings (repeated from the ROADMAP)

- DotStar 16×16 full white ≈ 15 A. Brightness is **capped in software**
  (`DOTSTAR_BRIGHTNESS`); size the supply and fusing for the cap, not the demo.
- TD-811MG stall current up to 3.4 A. Do **not** share a rail with the ESP32 or matrix.
- Put a ~1000 µF cap near the matrix power pins and near the servo power pins.
- Random resets usually mean a power brown-out, not a firmware bug.
