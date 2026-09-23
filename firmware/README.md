# WowBot Firmware (ESP32)

PlatformIO project for the ESP32 side. Target board is the **Maker-ESP32 V1.8**, treated
as a 30-pin ESP32-WROOM-32 DevKit clone. **Verify the pinout with a multimeter before
wiring anything** — the silkscreen on unbranded boards is often wrong.

## Build & flash

```bash
# with PlatformIO CLI
pio run -t upload
pio device monitor          # 115200 baud

# or open firmware/ in PlatformIO for VS Code and hit Upload
```

If you prefer the Arduino IDE, the code is standard Arduino — install
`ESP32Servo` and `Adafruit DotStar` from the Library Manager, copy the `.cpp`/`.h`
files plus `main.cpp` (rename to `wowbot.ino`), and set the board to `ESP32 Dev Module`.

## What each phase expects

| Phase | Flash this | Visible result |
|---|---|---|
| 1 | full sketch | serial replies: `READY`, `PONG` to `PING`, `ACK:` to `EXP:`/`MOV:`/`SAY:`/`BRIGHT:` |
| 1.5 | wire speech module to GPIO16/17 | `SAY:<text>` speaks it on the robot |
| 2 | attach SG90 to GPIO13 | `MOV:WAVE` etc. drive the head servo |
| 2.5 | set `ARM_ENABLED=1`, attach TD-811MG (own 5 V/3 A+ rail) | heavy joint moves |
| 3 | wire DotStar to GPIO23/18 | `EXP:` sets the face, `BRIGHT:` scales it |

## Manual smoke test (no laptop)

Open the serial monitor at 115200 and paste one line at a time:

```
PING          -> PONG
EXP:HAPPY     -> ACK:EXP:HAPPY  (face fires once wired)
MOV:WAVE      -> ACK:MOV:WAVE    (head gesture once the servo is wired)
SAY:hello     -> ACK:SAY ... DONE:SAY  (speaks once the speech module is wired)
BRIGHT:20     -> ACK:BRIGHT:20
```

## Power warnings (repeated from the ROADMAP)

- DotStar 16×16 full white = ~15 A. Brightness is **capped in software**
  (`DOTSTAR_BRIGHTNESS`); size the supply and fusing for the cap, not the demo.
- TD-811MG stall current = up to 3.4 A. Do **not** share a rail with the ESP32 or matrix.
- Put a ~1000 µF cap near the matrix power pins and near the servo power pins.
- Random resets usually mean a power brown-out, not a firmware bug.
