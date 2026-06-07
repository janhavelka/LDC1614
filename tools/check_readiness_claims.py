#!/usr/bin/env python3
"""Guard maintained surfaces against unsupported readiness claims."""

from __future__ import annotations

import re
import sys
from pathlib import Path
from typing import Iterable, List

ROOT = Path(__file__).resolve().parent.parent

FORBIDDEN = [
    (re.compile(r"\bproduction-grade\b", re.IGNORECASE), "production-grade"),
    (re.compile(r"\bindustry-grade\b", re.IGNORECASE), "industry-grade"),
    (re.compile(r"\bfield-ready\b", re.IGNORECASE), "field-ready"),
    (re.compile(r"\brelease-ready\b", re.IGNORECASE), "release-ready"),
    (re.compile(r"\bproduction ready\b", re.IGNORECASE), "production ready"),
    (re.compile(r"\bhardware[- ]validated\b", re.IGNORECASE), "hardware validated"),
    (re.compile(r"\bcertified\b", re.IGNORECASE), "certified"),
    (re.compile(r"\bvalidated starting points?\b", re.IGNORECASE), "validated starting points"),
    (re.compile(r"\bstable production operation\b", re.IGNORECASE), "stable production operation"),
]

EXCLUDED_DIR_PARTS = {
    ".git",
    ".pio",
    "__pycache__",
    "pdf-extracted-md",
    "extracted-md",
}

EXCLUDED_FILENAMES = {
    "LDC1614_INDUSTRY_READINESS_IMPLEMENTATION_PLAN.md",
    "LDC1614_HARDENING_PROGRESS.md",
    "LDC1614_INDUSTRY_HARDENING_FINAL_REPORT.md",
    "LDC1614_INDUSTRY_READINESS_EXPLORATION_REPORT.md",
}


def maintained_files() -> Iterable[Path]:
    explicit = [
        ROOT / "README.md",
        ROOT / "library.json",
        ROOT / "idf_component.yml",
        ROOT / ".github" / "workflows" / "ci.yml",
        ROOT / "docs" / "IDF_PORT.md",
        ROOT / "docs" / "IDF_PORT_IMPLEMENTATION.md",
        ROOT / "docs" / "HARDWARE_INTEGRATION.md",
        ROOT / "docs" / "HIL_VALIDATION.md",
        ROOT / "docs" / "hil" / "README.md",
    ]
    for path in explicit:
        if path.exists():
            yield path

    for base, patterns in (
        (ROOT / "examples", ("*.md", "*.cpp", "*.h", "*.hpp")),
        (ROOT / "docs" / "application_notes", ("*.md",)),
    ):
        if not base.exists():
            continue
        for pattern in patterns:
            for path in base.rglob(pattern):
                if should_scan(path):
                    yield path


def should_scan(path: Path) -> bool:
    rel_parts = set(path.relative_to(ROOT).parts)
    if rel_parts & EXCLUDED_DIR_PARTS:
        return False
    if path.name in EXCLUDED_FILENAMES:
        return False
    return True


def check_file(path: Path) -> List[str]:
    text = path.read_text(encoding="utf-8", errors="replace")
    errors: List[str] = []
    for line_no, line in enumerate(text.splitlines(), start=1):
        for pattern, label in FORBIDDEN:
            if pattern.search(line):
                errors.append(f"{path.relative_to(ROOT)}:{line_no}: unsupported readiness wording: {label}")
    return errors


def main() -> int:
    seen = set()
    errors: List[str] = []
    for path in maintained_files():
        if path in seen or not should_scan(path):
            continue
        seen.add(path)
        errors.extend(check_file(path))

    if errors:
        print("Readiness claims guard FAILED:")
        for error in errors:
            print(f"  {error}")
        return 1

    print("Readiness claims guard PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
