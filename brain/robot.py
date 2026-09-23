"""Ties the laptop's voice to the robot's actuators (serial tokens).

Ordering is chosen so the robot *reacts first*, then speaks, then settles:

1. `EXP:` — the face fires immediately.
2. `MOV:` — a non-blocking gesture starts on the firmware and runs while we talk.
3. speak — the robot's speech module (`SAY:`) when attached, laptop TTS otherwise.
4. `MOV:NONE` — return to neutral after speaking.
"""
from __future__ import annotations

from .schema import RobotAction, Expression, Movement


class Robot:
    def __init__(self, tts, serial_link):
        self.tts = tts
        self.serial = serial_link

    def act(self, action: RobotAction):
        # 1. React instantly.
        self.serial.send(f"EXP:{action.expression.value.upper()}")

        # 2. Start a gesture (non-blocking on the firmware).
        if action.movement is not Movement.NONE:
            self.serial.send(f"MOV:{action.movement.value.upper()}")

        # 3. Speak. With the robot attached, the speech module talks (robot voice);
        #    otherwise fall back to laptop TTS so the loop still works headless.
        #    Non-ASCII falls back too — the speech module is English-only for now.
        if action.speech:
            if self.serial.ready and action.speech.isascii():
                self.serial.say(action.speech)
            else:
                self.tts.speak(action.speech)

        # 4. Settle back to neutral.
        if action.movement is not Movement.NONE:
            self.serial.send("MOV:NONE")
        self.serial.send(f"EXP:{Expression.NEUTRAL.value.upper()}")
