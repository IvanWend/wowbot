"""WowBot Phase 0 — the laptop voice loop.

    python -m brain                       # full mic loop
    python -m brain --text "hello there"  # one turn, no mic
    python -m brain --list-devices        # pick a mic
"""
from __future__ import annotations

import argparse
import sys

from . import config
from .audio import AudioStream, list_devices
from .vad import SpeechSegmenter
from .stt import Transcriber
from .llm import LLM, fallback_action
from .tts import make_tts
from .serial_link import SerialLink
from .robot import Robot


def build_robot(cfg):
    """Wire the real (single) instances together."""
    stt = Transcriber(cfg)
    llm = LLM(cfg)
    tts = make_tts(cfg)
    serial = SerialLink(cfg)
    robot = Robot(tts, serial)
    return stt, llm, tts, serial, robot


def one_turn(text: str, cfg, stt, llm, robot) -> None:
    print(f"  you   : {text}")
    try:
        action = llm.generate(text)
    except Exception:
        action = fallback_action()
    print(f"  wowbot: {action.as_dict()}")
    robot.act(action)


def run_mic_loop(cfg, stt, llm, robot) -> None:
    segmenter = SpeechSegmenter(
        sample_rate=cfg.audio.sample_rate,
        vad_mode=cfg.vad.mode,
        silence_ms=cfg.vad.silence_ms,
    )

    print("Listening — speak, then pause. Ctrl+C to quit.")
    with AudioStream(cfg.audio.sample_rate, cfg.audio.chunk_ms, cfg.audio.input_device) as stream:
        while True:
            block = stream.read_block()
            if block is None:
                continue
            utterance = segmenter.process(block)
            if utterance is None:
                continue

            text = stt.transcribe(utterance)
            if not text:
                continue
            one_turn(text, cfg, stt, llm, robot)


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(prog="wowbot", description="WowBot voice loop")
    parser.add_argument("--config", help="path to config.yaml (default: repo root)")
    parser.add_argument("--list-devices", action="store_true", help="list mic devices and exit")
    parser.add_argument("--text", help="run a single turn on this text instead of the mic")
    args = parser.parse_args(argv)

    if args.list_devices:
        print(list_devices())
        return 0

    cfg = config.load(args.config)

    if args.text:
        _stt, _llm, _tts, _serial, robot = build_robot(cfg)
        with _serial:
            one_turn(args.text, cfg, _stt, _llm, robot)
        return 0

    stt, llm, tts, serial, robot = build_robot(cfg)
    print(f"WowBot {cfg.app.name} — serial_enabled={cfg.app.serial_enabled}, "
          f"stt={cfg.stt.model}, llm={cfg.llm.model}, tts={cfg.tts.engine}")

    with serial:
        try:
            run_mic_loop(cfg, stt, llm, robot)
        except KeyboardInterrupt:
            print("\nBye.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
