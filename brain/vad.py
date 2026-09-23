"""Voice activity detection + utterance segmentation via ``webrtcvad``.

``webrtcvad`` wants exactly 10/20/30 ms frames of 16-bit mono PCM at 8/16/32/48 kHz.
We standardise on 30 ms @ 16 kHz (960 bytes/frame) to match ``audio.AudioStream``.
"""
from __future__ import annotations

FRAME_MS = 30
BYTES_PER_SAMPLE = 2  # int16


class SpeechSegmenter:
    """Feeds 30 ms frames; yields complete utterances as concatenated int16 bytes.

    An utterance starts on the first voiced frame and ends once we've heard
    ``silence_ms`` of trailing silence. Trailing silence is trimmed; a small pre/post
    roll is a later refinement, not a v1 concern.
    """

    def __init__(self, sample_rate: int = 16000, vad_mode: int = 1, silence_ms: int = 700):
        import webrtcvad  # lazy: keeps the package importable without the dep

        self.sample_rate = sample_rate
        self.frame_ms = FRAME_MS
        self.frame_bytes = int(sample_rate * BYTES_PER_SAMPLE * FRAME_MS / 1000)
        self.silence_ms = silence_ms
        self._vad = webrtcvad.Vad(vad_mode)

        self._frames: list[bytes] = []
        self._trailing_silence_ms = 0
        self._speaking = False

    def reset(self):
        self._frames.clear()
        self._trailing_silence_ms = 0
        self._speaking = False

    def process(self, pcm: bytes) -> bytes | None:
        """Consume one frame. Return a finished utterance (int16 bytes) or ``None``."""
        if len(pcm) != self.frame_bytes:
            raise ValueError(
                f"VAD expects {self.frame_bytes}-byte frames, got {len(pcm)}. "
                "Check audio.chunk_ms (30) and sample_rate (16000) in config.yaml."
            )

        voiced = self._vad.is_speech(pcm, self.sample_rate)

        if voiced:
            self._speaking = True
            self._trailing_silence_ms = 0
            self._frames.append(pcm)
            return None

        if not self._speaking:
            return None

        # Unvoiced frame while speaking: count trailing silence.
        self._trailing_silence_ms += self.frame_ms
        if self._trailing_silence_ms < self.silence_ms:
            self._frames.append(pcm)  # keep brief gaps inside the utterance
            return None

        utterance = b"".join(self._frames)
        self.reset()
        return utterance
