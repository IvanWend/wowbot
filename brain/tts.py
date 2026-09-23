"""Text-to-speech.

Default is ``pyttsx3`` (Windows SAPI5): offline and zero-setup. The interface is a
single ``speak`` method so swapping in ``edge-tts`` (network, great quality) or
``piper`` (offline neural) touches only this module.
"""
from __future__ import annotations


class TTS:
    def speak(self, text: str) -> None:
        raise NotImplementedError


class Pyttsx3TTS(TTS):
    def __init__(self, rate: int | None = None, voice: str | None = None):
        self.rate = rate
        self.voice = voice
        self._engine = None

    def _ensure_engine(self):
        if self._engine is None:
            import pyttsx3  # lazy: init is slow, only pay it on first speech

            self._engine = pyttsx3.init()
            if self.rate is not None:
                self._engine.setProperty("rate", self.rate)
            if self.voice is not None:
                self._engine.setProperty("voice", self.voice)

    def speak(self, text: str) -> None:
        if not text:
            return
        self._ensure_engine()
        self._engine.say(text)
        self._engine.runAndWait()  # blocking — fine for a first draft


def make_tts(cfg) -> TTS:
    engine = cfg.tts.engine
    if engine == "pyttsx3":
        return Pyttsx3TTS(rate=cfg.tts.rate, voice=cfg.tts.voice)
    raise ValueError(f"Unknown TTS engine '{engine}' (see config.yaml)")
