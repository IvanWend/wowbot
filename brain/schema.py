"""The shared contract: LLM JSON → ``RobotAction`` → serial tokens.

This is the laptop half of the contract documented in ``docs/ARCHITECTURE.md``.
Keep the enum *values* in sync with ``firmware/include/expressions.h`` and
``firmware/include/servo_controller.h`` (names are matched case-insensitively on
the ESP32, but the canonical spellings live here).
"""
from __future__ import annotations

import json
import re
from dataclasses import dataclass, field
from enum import Enum


class Expression(str, Enum):
    NEUTRAL = "neutral"
    HAPPY = "happy"
    SAD = "sad"
    CURIOUS = "curious"
    ANGRY = "angry"
    SURPRISED = "surprised"
    SLEEPY = "sleepy"
    THINKING = "thinking"
    EXCITED = "excited"


class Movement(str, Enum):
    NONE = "none"
    WAVE = "wave"
    NOD = "nod"
    SHAKE = "shake"
    LOOK_LEFT = "look_left"
    LOOK_RIGHT = "look_right"
    TILT = "tilt"
    DANCE = "dance"


@dataclass
class RobotAction:
    """A validated response from the LLM, ready to act on."""

    expression: Expression = Expression.NEUTRAL
    movement: Movement = Movement.NONE
    speech: str = ""

    def as_dict(self) -> dict:
        return {
            "expression": self.expression.value,
            "movement": self.movement.value,
            "speech": self.speech,
        }


# --- LLM-facing -----------------------------------------------------------------

SYSTEM_PROMPT = """\
You are WowBot, a small, cheerful desktop robot companion with a screen face, \
a pan-tilt head, and a voice. Reply to the user in ONE short spoken line (1-2 \
sentences), warm and a little playful. No emoji. Do not narrate your own actions.

Respond with a single JSON object and nothing else, in exactly this shape:
{
  "expression": "<one of: neutral, happy, sad, curious, angry, surprised, sleepy, thinking, excited>",
  "movement": "<one of: none, wave, nod, shake, look_left, look_right, tilt, dance>",
  "speech": "<what you say out loud>"
}
Pick the expression and movement that match the emotional tone of your reply.\
"""

# JSON Schema for the response (used for validation and, with Ollama's newer
# structured-output support, to force the shape at generation time).
LLM_JSON_SCHEMA: dict = {
    "type": "object",
    "properties": {
        "expression": {
            "type": "string",
            "enum": [e.value for e in Expression],
        },
        "movement": {
            "type": "string",
            "enum": [m.value for m in Movement],
        },
        "speech": {"type": "string"},
    },
    "required": ["expression", "speech"],
    "additionalProperties": False,
}


# --- Parsing --------------------------------------------------------------------

_FENCE_RE = re.compile(r"```(?:json)?\s*(.*?)```", re.DOTALL)
_BRACE_RE = re.compile(r"\{.*\}", re.DOTALL)


def _coerce_enum(value: str | None, enum_cls, default):
    if value is None:
        return default
    try:
        return enum_cls(str(value).strip().lower())
    except ValueError:
        return default


def parse_llm_json(text: str) -> RobotAction:
    """Turn raw LLM output into a validated ``RobotAction``.

    Tolerant of code fences, surrounding prose, and missing/unknown fields — the
    model's schema is a suggestion, so we clamp rather than crash.
    """
    cleaned = _FENCE_RE.sub(r"\1", text)
    match = _BRACE_RE.search(cleaned)
    raw = match.group(0) if match else cleaned

    try:
        data = json.loads(raw)
    except json.JSONDecodeError:
        data = {}

    if not isinstance(data, dict):
        data = {}

    return RobotAction(
        expression=_coerce_enum(data.get("expression"), Expression, Expression.NEUTRAL),
        movement=_coerce_enum(data.get("movement"), Movement, Movement.NONE),
        speech=str(data.get("speech") or "").strip(),
    )


# --- Serial encoding ------------------------------------------------------------

def to_tokens(action: RobotAction) -> list[str]:
    """Encode a ``RobotAction`` into newline-delimited serial tokens.

    ``speech`` is deliberately NOT encoded here — it is spoken by the laptop's TTS
    (Phases 0–5) and, in Phase 6, streamed to the robot as audio bytes. Either way it
    never becomes a control token.
    """
    tokens = [f"EXP:{action.expression.value.upper()}"]
    if action.movement is not Movement.NONE:
        tokens.append(f"MOV:{action.movement.value.upper()}")
    return tokens
