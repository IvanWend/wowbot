# Robot audio (superseded)

This folder was going to hold TF-card sound-effect clips for a DFPlayer Mini MP3
module. The audio module turned out to be a **DFRobot Gravity Speech Synthesis V2.0**
(SKU DFR0760), a text-to-speech chip — so the robot *synthesizes* speech rather than
playing clips. No files are needed here.

The speech path lives in `firmware/src/speech_controller.cpp` and the `SAY:` token in
`docs/ARCHITECTURE.md`.
