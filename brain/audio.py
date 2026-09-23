"""Microphone capture via ``sounddevice`` (PortAudio).

Kept separate from VAD so the two concerns (device I/O vs. speech detection) don't
tangle. ``sounddevice`` is imported lazily so the rest of the brain is importable
without audio hardware or the dependency installed.
"""
from __future__ import annotations

import queue


class AudioStream:
    """Streams fixed-size int16 mono blocks from the default (or chosen) mic."""

    def __init__(self, sample_rate: int, chunk_ms: int = 30, device=None):
        self.sample_rate = sample_rate
        self.chunk_ms = chunk_ms
        self.block_samples = int(sample_rate * chunk_ms / 1000)
        self.device = device
        self._q: "queue.Queue[bytes]" = queue.Queue()
        self._stream = None

    def __enter__(self) -> "AudioStream":
        self.start()
        return self

    def __exit__(self, *exc):
        self.stop()

    def start(self):
        import sounddevice as sd

        def _callback(indata, frames, time_info, status):
            if status:
                print(f"[audio] stream status: {status}")
            self._q.put(indata.copy().tobytes())

        self._stream = sd.InputStream(
            samplerate=self.sample_rate,
            channels=1,
            dtype="int16",
            blocksize=self.block_samples,
            device=self.device,
            callback=_callback,
        )
        self._stream.start()

    def read_block(self, timeout: float = 1.0) -> bytes | None:
        """Return the next int16 block, or ``None`` on timeout/underflow."""
        try:
            return self._q.get(timeout=timeout)
        except queue.Empty:
            return None

    def stop(self):
        if self._stream is not None:
            self._stream.stop()
            self._stream.close()
            self._stream = None


def list_devices() -> str:
    """Return a human-readable list of input devices (for ``--list-devices``)."""
    import sounddevice as sd

    lines = []
    try:
        devices = sd.query_devices()
        hostapis = sd.query_hostapis()
        default_in = sd.default.device[0]
    except Exception as exc:  # no audio backend
        return f"Could not query audio devices: {exc}"

    lines.append(f"Default input device: {default_in}")
    for i, dev in enumerate(devices):
        if dev["max_input_channels"] <= 0:
            continue
        hostapi = hostapis[dev["hostapi"]]["name"]
        lines.append(f"  [{i}] {dev['name']} — {hostapi}, {dev['default_samplerate']:.0f} Hz")
    return "\n".join(lines) if lines else "No input devices found."
