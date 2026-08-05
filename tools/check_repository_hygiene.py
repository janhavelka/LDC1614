#!/usr/bin/env python3
"""Reject stale generated files, broken local links, and duplicate HIL evidence."""

from __future__ import annotations

import hashlib
import json
import re
import subprocess
from pathlib import Path
from typing import List, Set

ROOT = Path(__file__).resolve().parent.parent
MARKDOWN_LINK = re.compile(r"!?\[[^\]]*\]\(([^)]+)\)")
FORBIDDEN_PREFIXES = ("docs/doxygen/", "prompts/")
FORBIDDEN_SUFFIXES = (".runner.md",)


def tracked_files() -> Set[str]:
    result = subprocess.run(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=True,
    )
    candidates = {
        line.strip().replace("\\", "/") for line in result.stdout.splitlines()
    }
    return {path for path in candidates if (ROOT / path).is_file()}


def check_tracked_artifacts(tracked: Set[str]) -> List[str]:
    errors: List[str] = []
    for path in sorted(tracked):
        if path.startswith(FORBIDDEN_PREFIXES):
            errors.append(f"tracked generated/one-time path: {path}")
        if path.endswith(FORBIDDEN_SUFFIXES):
            errors.append(f"tracked generated report: {path}")
    return errors


def check_markdown_links(tracked: Set[str]) -> List[str]:
    errors: List[str] = []
    for relative in sorted(path for path in tracked if path.endswith(".md")):
        source = ROOT / relative
        text = source.read_text(encoding="utf-8", errors="replace")
        for line_number, line in enumerate(text.splitlines(), start=1):
            for match in MARKDOWN_LINK.finditer(line):
                target = match.group(1).strip().strip("<>").split("#", 1)[0]
                if not target or "://" in target or target.startswith("mailto:"):
                    continue
                target = target.split(maxsplit=1)[0]
                if not (source.parent / target).resolve().exists():
                    errors.append(f"{relative}:{line_number}: missing local link: {target}")
    return errors


def check_hil_artifacts(tracked: Set[str]) -> List[str]:
    errors: List[str] = []
    report_json = sorted(
        path for path in tracked
        if path.startswith("docs/reports/") and path.endswith(".json")
    )
    report_raw = {
        path for path in tracked
        if path.startswith("docs/reports/") and path.endswith(".serial.txt")
    }
    referenced_raw: Set[str] = set()

    for relative in report_json:
        path = ROOT / relative
        try:
            result = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            errors.append(f"{relative}: invalid JSON: {exc}")
            continue

        if "transcript" in result:
            errors.append(f"{relative}: embeds duplicate transcript text")

        metadata = result.get("raw_transcript")
        if not isinstance(metadata, dict):
            errors.append(f"{relative}: missing raw_transcript metadata")
            continue
        raw_name = metadata.get("file")
        if not isinstance(raw_name, str) or Path(raw_name).name != raw_name:
            errors.append(f"{relative}: invalid raw transcript filename")
            continue

        raw_relative = (Path(relative).parent / raw_name).as_posix()
        referenced_raw.add(raw_relative)
        raw_path = ROOT / raw_relative
        if raw_relative not in report_raw or not raw_path.is_file():
            errors.append(f"{relative}: missing tracked raw transcript {raw_relative}")
            continue

        raw_bytes = raw_path.read_bytes()
        if metadata.get("bytes") != len(raw_bytes):
            errors.append(f"{relative}: raw transcript byte count mismatch")
        if metadata.get("sha256") != hashlib.sha256(raw_bytes).hexdigest():
            errors.append(f"{relative}: raw transcript SHA-256 mismatch")

    for relative in sorted(report_raw - referenced_raw):
        errors.append(f"{relative}: raw transcript has no structured JSON index")
    return errors


def main() -> int:
    tracked = tracked_files()
    errors = [
        *check_tracked_artifacts(tracked),
        *check_markdown_links(tracked),
        *check_hil_artifacts(tracked),
    ]
    if errors:
        print("Repository hygiene check FAILED:")
        for error in errors:
            print(f"  {error}")
        return 1
    print("Repository hygiene check PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
