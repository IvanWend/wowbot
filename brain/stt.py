"""Speech-to-text.

Default is local ``faster-whisper`` (CTranslate2). A cloud OpenAI Whisper path can be
dropped in here later without touching the rest of the brain — only this module
speaks the STT engine's API.
"""
from __future__ import annotations

import numpy as np


class Transcriber:
    def __init__(self, cfg):
        self.cfg = cfg.stt
        self._model = None

    def _load_model(self):
        from faster_whisper import WhisperModel  # lazy: downloads on first use

        print(f"[stt] loading faster-whisper model '{self.cfg.model}' "
              f"({self.cfg.device}) — first run downloads it...")
        self._model = WhisperModel(self.cfg.model, device=self.cfg.device, compute_type="int8")

    def transcribe(self, pcm: bytes) -> str:
        """Convert int16 mono bytes to text."""
        if self._model is None:
            self._load_model()

        audio = np.frombuffer(pcm, dtype=np.int16).astype(np.float32) / 32768.0
        segments, _info = self._model.transcribe(
            audio,
            language=self.cfg.language,
            beam_size=5,
            vad_filter=True,
        )
        return "".join(seg.text for seg in segments).strip()
