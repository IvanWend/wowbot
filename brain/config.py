"""Configuration loading.

Reads ``config.yaml`` from the repo root (or ``$WOWBOT_CONFIG``), then exposes it as
nested ``SimpleNamespace`` objects so you can write ``cfg.audio.sample_rate`` instead
of dict indexing.
"""
from __future__ import annotations

import os
from pathlib import Path
from types import SimpleNamespace
from typing import Any

import yaml

ROOT = Path(__file__).resolve().parent.parent


def _to_namespace(obj: Any) -> Any:
    if isinstance(obj, dict):
        return SimpleNamespace(**{k: _to_namespace(v) for k, v in obj.items()})
    if isinstance(obj, list):
        return [_to_namespace(v) for v in obj]
    return obj


def load(path: str | Path | None = None) -> SimpleNamespace:
    """Load config, merging ``config.yaml`` over ``config.local.yaml`` if present."""
    path = Path(path or os.environ.get("WOWBOT_CONFIG", ROOT / "config.yaml"))
    if not path.exists():
        raise FileNotFoundError(
            f"Config not found at {path}. Copy config.yaml to config.local.yaml "
            "or set WOWBOT_CONFIG."
        )

    data: dict[str, Any] = yaml.safe_load(path.read_text(encoding="utf-8")) or {}

    # Local overrides (git-ignored) sit next to the base file.
    local = path.with_name(path.stem + ".local" + path.suffix)
    if local.exists():
        local_data = yaml.safe_load(local.read_text(encoding="utf-8")) or {}
        _deep_merge(data, local_data)

    return _to_namespace(data)


def _deep_merge(base: dict, override: dict) -> None:
    for key, value in override.items():
        if isinstance(value, dict) and isinstance(base.get(key), dict):
            _deep_merge(base[key], value)
        else:
            base[key] = value
