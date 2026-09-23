"""Ollama client that returns a validated ``RobotAction``.

Speaks Ollama's local HTTP API directly (no extra dependency beyond ``requests``).
Uses ``format: json`` plus a strict system prompt, then clamps the result through
``schema.parse_llm_json`` so a malformed reply never crashes the loop.
"""
from __future__ import annotations

from .schema import (
    SYSTEM_PROMPT,
    LLM_JSON_SCHEMA,
    RobotAction,
    Expression,
    parse_llm_json,
)


class LLM:
    def __init__(self, cfg):
        self.cfg = cfg.llm
        self.persona = getattr(cfg, "persona", None)
        self._history: list[dict] = []

    def generate(self, user_text: str) -> RobotAction:
        messages = [{"role": "system", "content": SYSTEM_PROMPT}]
        messages.extend(self._history[-2 * self.cfg.history_turns :])
        messages.append({"role": "user", "content": user_text})

        content = self._call(messages)

        action = parse_llm_json(content)
        self._remember(user_text, action)
        return action

    def _call(self, messages: list[dict]) -> str:
        import requests  # lazy

        payload = {
            "model": self.cfg.model,
            "messages": messages,
            "stream": False,
            "format": "json",  # Ollama: enforce JSON output; also see structured outputs
            "options": {
                "temperature": self.cfg.temperature,
                "num_predict": self.cfg.max_tokens,
            },
        }
        # NOTE: Ollama >=0.5 supports `format: LLM_JSON_SCHEMA` for strict structured
        # output. That's more reliable than `"json"` + prompt; swap here once tested.
        try:
            resp = requests.post(
                f"{self.cfg.host}/api/chat", json=payload, timeout=60
            )
            resp.raise_for_status()
            return resp.json()["message"]["content"]
        except Exception as exc:  # network / model missing / bad JSON
            print(f"[llm] Ollama call failed: {exc}")
            raise

    def _remember(self, user_text: str, action: RobotAction):
        """Keep a short, plain-text memory so the model has conversational context.

        We store the *speech*, not the raw JSON, so history stays natural for the model.
        """
        if not self.cfg.history_turns:
            return
        self._history.append({"role": "user", "content": user_text})
        self._history.append({"role": "assistant", "content": action.speech})


def fallback_action() -> RobotAction:
    """Used when generation fails entirely — keep the robot responsive, not silent."""
    return RobotAction(
        expression=Expression.SURPRISED,
        speech="Hmm, I didn't quite catch that. Could you say it again?",
    )
