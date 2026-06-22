import json
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).parents[1]
sys.path.insert(0, str(ROOT / "bin" / "data"))

from multilateration import calculateSerialLine, parseSerialLine


CASES = json.loads((ROOT / "tests" / "data" / "multilateration.json").read_text())


class RandomOffsets:
    def __init__(self, offsets):
        self.offsets = iter(offsets)

    def uniform(self, minimum, maximum):
        offset = next(self.offsets)
        assert minimum <= offset <= maximum
        return offset


@pytest.mark.parametrize("case", CASES, ids=[case["input"] for case in CASES])
def test_serial_line_location(case):
    rng = RandomOffsets(case["random_offsets"]) if "random_offsets" in case else None
    actual = calculateSerialLine(case["input"], rng)
    expected = case["expected"]

    assert actual["side"] == expected["side"]
    assert actual["valid"] is expected["valid"]
    assert actual["x"] == pytest.approx(expected["x"], abs=1e-9)
    assert actual["y"] == pytest.approx(expected["y"], abs=1e-9)


@pytest.mark.parametrize("line", ["", "A 1 2 3", "B 1 2 3 4 5", "C 1 2 3", "D 1 100", "A one 2 3 4"])
def test_invalid_serial_line(line):
    with pytest.raises((ValueError, IndexError)):
        parseSerialLine(line)
