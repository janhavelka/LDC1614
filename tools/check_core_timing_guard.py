#!/usr/bin/env python3
from __future__ import annotations

import pathlib
import re
import sys
from typing import Dict

ROOT = pathlib.Path(__file__).resolve().parents[1]
SCAN_DIRS = ("src", "include")
VALID_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp"}

FORBIDDEN_CALLS = {
    "millis": re.compile(r"\bmillis\s*\("),
    "micros": re.compile(r"\bmicros\s*\("),
    "delay": re.compile(r"\bdelay\s*\("),
    "delayMicroseconds": re.compile(r"\bdelayMicroseconds\s*\("),
    "yield": re.compile(r"\byield\s*\("),
    "printf": re.compile(r"\bprintf\s*\("),
    "malloc": re.compile(r"\bmalloc\s*\("),
    "calloc": re.compile(r"\bcalloc\s*\("),
    "realloc": re.compile(r"\brealloc\s*\("),
    "free": re.compile(r"\bfree\s*\("),
    "vTaskDelay": re.compile(r"\bvTaskDelay\s*\("),
    "xSemaphoreTake": re.compile(r"\bxSemaphoreTake\s*\("),
    "xSemaphoreGive": re.compile(r"\bxSemaphoreGive\s*\("),
    "xSemaphoreCreateMutex": re.compile(r"\bxSemaphoreCreateMutex\s*\("),
}

FORBIDDEN_TOKENS = {
    "Wire": re.compile(r"\bWire\b"),
    "TwoWire": re.compile(r"\bTwoWire\b"),
    "Serial": re.compile(r"\bSerial\b"),
    "Arduino String": re.compile(r"\bString\b"),
    "std::string": re.compile(r"\bstd::string\b"),
    "std::vector": re.compile(r"\bstd::vector\b"),
    "new": re.compile(r"\bnew\b"),
    "throw": re.compile(r"\bthrow\b"),
    "try": re.compile(r"\btry\b"),
    "catch": re.compile(r"\bcatch\s*\("),
    "ESP_LOG": re.compile(r"\bESP_LOG[A-Z_]*\b"),
    "esp_": re.compile(r"\besp_[A-Za-z0-9_]*\b"),
    "FreeRTOS": re.compile(r"\bFreeRTOS\b"),
}

FORBIDDEN_INCLUDES = {
    "Arduino.h": re.compile(r'^\s*#\s*include\s*[<"]Arduino\.h[>"]', re.MULTILINE),
    "Wire.h": re.compile(r'^\s*#\s*include\s*[<"]Wire\.h[>"]', re.MULTILINE),
    "<string>": re.compile(r'^\s*#\s*include\s*<string>', re.MULTILINE),
    "<vector>": re.compile(r'^\s*#\s*include\s*<vector>', re.MULTILINE),
    "ESP-IDF driver header": re.compile(r'^\s*#\s*include\s*[<"]driver/[^>"]+[>"]', re.MULTILINE),
    "ESP-IDF freertos header": re.compile(r'^\s*#\s*include\s*[<"]freertos/[^>"]+[>"]', re.MULTILINE),
    "ESP-IDF esp header": re.compile(r'^\s*#\s*include\s*[<"]esp_[^>"]+[>"]', re.MULTILINE),
}

BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
LINE_COMMENT_RE = re.compile(r"//[^\n]*")
STRING_RE = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')


def strip_non_code(text: str) -> str:
    text = BLOCK_COMMENT_RE.sub("", text)
    text = LINE_COMMENT_RE.sub("", text)
    return STRING_RE.sub('""', text)


def collect_sources() -> list[pathlib.Path]:
    files: list[pathlib.Path] = []
    for dirname in SCAN_DIRS:
        root = ROOT / dirname
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if path.is_file() and path.suffix.lower() in VALID_SUFFIXES:
                files.append(path)
    return files


def main() -> int:
    observed_calls: Dict[str, Dict[str, int]] = {}
    observed_includes: Dict[str, Dict[str, int]] = {}
    observed_tokens: Dict[str, Dict[str, int]] = {}

    for path in collect_sources():
        rel = path.relative_to(ROOT).as_posix()
        raw = path.read_text(encoding="utf-8", errors="replace")
        code = strip_non_code(raw)

        call_counts: Dict[str, int] = {}
        for call_name, pattern in FORBIDDEN_CALLS.items():
            count = len(pattern.findall(code))
            if count > 0:
                call_counts[call_name] = count
        if call_counts:
            observed_calls[rel] = call_counts

        include_counts: Dict[str, int] = {}
        for label, pattern in FORBIDDEN_INCLUDES.items():
            count = len(pattern.findall(raw))
            if count > 0:
                include_counts[label] = count
        if include_counts:
            observed_includes[rel] = include_counts

        token_counts: Dict[str, int] = {}
        for label, pattern in FORBIDDEN_TOKENS.items():
            count = len(pattern.findall(code))
            if count > 0:
                token_counts[label] = count
        if token_counts:
            observed_tokens[rel] = token_counts

    errors: list[str] = []

    for rel, counts in observed_calls.items():
        errors.append(f"forbidden timing calls in core: {rel} -> {counts}")

    for rel, counts in observed_includes.items():
        errors.append(f"forbidden framework/STL includes in core: {rel} -> {counts}")

    for rel, counts in observed_tokens.items():
        errors.append(f"forbidden framework/allocation/logging tokens in core: {rel} -> {counts}")

    if errors:
        print("Core timing/framework guard FAILED:")
        for err in errors:
            print(f"- {err}")
        return 1

    print("Core timing/framework guard PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
