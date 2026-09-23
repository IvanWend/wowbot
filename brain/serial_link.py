"""PySerial bridge to the ESP32.

A no-op while ``serial_enabled=false`` (Phase 0). Flip the flag in ``config.yaml``
and set ``serial.port`` in Phase 1. Port failures degrade to a warning rather than
crashing the voice loop — the robot should still talk without its body attached.
"""
from __future__ import annotations

import time

from .schema import RobotAction, to_tokens


class SerialLink:
    def __init__(self, cfg):
        self.cfg = cfg.serial
        self.enabled = cfg.app.serial_enabled
        self._port = None

    def __enter__(self) -> "SerialLink":
        self.open()
        return self

    def __exit__(self, *exc):
        self.close()

    def open(self):
        if not self.enabled:
            return
        import serial  # lazy

        try:
            self._port = serial.Serial(
                self.cfg.port, self.cfg.baud, timeout=self.cfg.timeout
            )
            print(f"[serial] connected to {self.cfg.port} @ {self.cfg.baud}")
        except Exception as exc:
            self._port = None
            print(f"[serial] could not open {self.cfg.port}: {exc} — continuing headless")

    def close(self):
        if self._port is not None:
            self._port.close()
            self._port = None

    @property
    def ready(self) -> bool:
        return self._port is not None and self._port.is_open

    def send(self, token: str):
        """Send one token, newline-terminated (per the serial contract)."""
        if not self.ready:
            return
        try:
            self._port.write((token + "\n").encode("utf-8"))
            self._port.flush()
        except Exception as exc:
            print(f"[serial] write failed: {exc}")

    def say(self, text: str, timeout: float | None = None) -> bool:
        """Speak ``text`` on the robot and block until it finishes.

        Sends ``SAY:<text>`` then reads lines until the firmware replies ``DONE:SAY``
        (the speech module reported playback complete). Returns True on success, False
        on timeout or if the link isn't up.
        """
        if not self.ready or not text:
            return False
        text = " ".join(text.split())
        if not text:
            return False
        if timeout is None:
            timeout = 3.0 + 0.3 * len(text)  # generous: ~300 ms/char + 3 s
        self.send("SAY:" + text)

        deadline = time.monotonic() + timeout
        buf = b""
        while time.monotonic() < deadline:
            b = self._port.read(1)  # uses cfg serial.timeout (0.1 s) per byte
            if not b:
                continue
            if b == b"\n":
                line = buf.decode("utf-8", "replace").strip()
                buf = b""
                if line == "DONE:SAY":
                    return True
            else:
                buf += b
        return False

    def send_action(self, action: RobotAction):
        for token in to_tokens(action):
            self.send(token)
