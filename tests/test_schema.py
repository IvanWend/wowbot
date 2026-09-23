"""Contract tests for the laptop half of the token schema.

Run without hardware or third-party deps (``schema.py`` is stdlib-only):

    python -m tests.test_schema      # or: pytest tests/
"""
from brain.schema import (
    RobotAction,
    Expression,
    Movement,
    parse_llm_json,
    to_tokens,
)


def test_parse_clean():
    a = parse_llm_json('{"expression":"happy","movement":"wave","speech":"Hi!"}')
    assert a.expression is Expression.HAPPY
    assert a.movement is Movement.WAVE
    assert a.speech == "Hi!"


def test_parse_fenced_json():
    a = parse_llm_json('```json\n{"expression":"sad","speech":"oh no"}\n```')
    assert a.expression is Expression.SAD
    assert a.speech == "oh no"


def test_parse_with_surrounding_prose():
    a = parse_llm_json('Sure! Here you go: {"expression":"curious","speech":"huh"}')
    assert a.expression is Expression.CURIOUS


def test_parse_garbage_falls_back_to_neutral():
    a = parse_llm_json("not json at all")
    assert a.expression is Expression.NEUTRAL
    assert a.movement is Movement.NONE
    assert a.speech == ""


def test_unknown_enum_values_are_clamped():
    a = parse_llm_json('{"expression":"flamethrower","movement":"spin"}')
    assert a.expression is Expression.NEUTRAL
    assert a.movement is Movement.NONE


def test_tokens_basic():
    a = RobotAction(Expression.EXCITED, Movement.WAVE, "woo")
    assert to_tokens(a) == ["EXP:EXCITED", "MOV:WAVE"]


def test_tokens_omit_none_movement():
    a = RobotAction(Expression.NEUTRAL, Movement.NONE, "hi")
    assert to_tokens(a) == ["EXP:NEUTRAL"]


def test_tokens_omit_speech():
    a = RobotAction(Expression.EXCITED, Movement.WAVE, "woo")
    assert to_tokens(a) == ["EXP:EXCITED", "MOV:WAVE"]


if __name__ == "__main__":
    import traceback

    failures = 0
    tests = [(k, v) for k, v in sorted(globals().items())
             if k.startswith("test_") and callable(v)]
    for name, fn in tests:
        try:
            fn()
            print(f"PASS {name}")
        except AssertionError as exc:
            failures += 1
            print(f"FAIL {name}: {exc}")
            traceback.print_exc()
    print(f"\n{len(tests) - failures}/{len(tests)} passed")
    raise SystemExit(1 if failures else 0)
