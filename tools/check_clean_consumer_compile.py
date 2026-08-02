#!/usr/bin/env python3
"""Compile a clean consumer translation unit against the public headers."""

from __future__ import annotations

import os
import shutil
import subprocess
import sys
import tarfile
import tempfile
from pathlib import Path
from typing import Optional


ROOT = Path(__file__).resolve().parent.parent
VERSION_HEADER = ROOT / "include" / "LDC1614" / "Version.h"


def fail(message: str) -> int:
    print(f"Clean consumer compile guard FAILED: {message}", file=sys.stderr)
    return 1


def find_compiler() -> Optional[str]:
    candidates = []
    env_cxx = os.environ.get("CXX", "").strip()
    if env_cxx:
        candidates.append(env_cxx)
    candidates.extend(["c++", "g++", "clang++"])
    for candidate in candidates:
        if shutil.which(candidate):
            return candidate
    return None


def ensure_version_header_tracked() -> int:
    if not VERSION_HEADER.exists():
        return fail(f"missing generated public header: {VERSION_HEADER.relative_to(ROOT)}")

    if not (ROOT / ".git").exists() or shutil.which("git") is None:
        return 0

    result = subprocess.run(
        ["git", "ls-files", "--error-unmatch", "include/LDC1614/Version.h"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        return fail("include/LDC1614/Version.h exists but is not tracked by git")
    return 0


def run_command(command: list[str], cwd: Path) -> subprocess.CompletedProcess[str]:
    return subprocess.run(command, cwd=cwd, capture_output=True, text=True, check=False)


def pack_library(temp: Path) -> Optional[Path]:
    output_dir = temp / "dist"
    output_dir.mkdir()
    result = run_command(
        [sys.executable, "-m", "platformio", "pkg", "pack", str(ROOT), "-o", str(output_dir)],
        ROOT,
    )
    if result.returncode != 0:
        print(result.stdout, end="")
        print(result.stderr, end="", file=sys.stderr)
        fail("PlatformIO package pack failed")
        return None

    archives = sorted(output_dir.glob("*.tar.gz"))
    if len(archives) != 1:
        fail(f"expected one package archive in {output_dir}, found {len(archives)}")
        return None
    return archives[0]


def extract_archive(archive: Path, temp: Path) -> Path:
    extract_root = temp / "extracted"
    extract_root.mkdir()
    with tarfile.open(archive, "r:gz") as tar:
        if sys.version_info >= (3, 12):
            tar.extractall(extract_root, filter="data")
        else:
            tar.extractall(extract_root)

    entries = [path for path in extract_root.iterdir()]
    if len(entries) == 1 and entries[0].is_dir():
        return entries[0]
    return extract_root


def compile_consumer(compiler: str, package_root: Path, temp: Path) -> int:
    include_dir = package_root / "include"
    source_file = package_root / "src" / "LDC1614.cpp"
    version_header = include_dir / "LDC1614" / "Version.h"
    library_json = package_root / "library.json"
    documentation = (
        package_root / "CONTRIBUTING.md",
        package_root / "Doxyfile",
        package_root / "docs" / "README.md",
        package_root / "docs" / "HARDWARE_INTEGRATION.md",
        package_root / "docs" / "HIL_VALIDATION.md",
        package_root / "docs" / "I2C_INTEGRATION.md",
        package_root / "docs" / "IDF_PORT.md",
        package_root / "docs" / "VALIDATION_STATUS.md",
        package_root / "docs" / "reference" / "LDC1614_datasheet.pdf",
    )
    for required in (include_dir, source_file, version_header, library_json, *documentation):
        if not required.exists():
            return fail(f"packed package is missing {required.relative_to(package_root)}")

    consumer = temp / "consumer.cpp"
    consumer.write_text(
        """#include <type_traits>
#include "LDC1614/LDC1614.h"

static_assert(!std::is_copy_constructible<LDC1614::LDC1614>::value,
              "driver instances are intentionally non-copyable");
static_assert(LDC1614::VERSION_CODE >= 10000U,
              "Version.h must be available to public consumers");

int main() {
  LDC1614::Config cfg;
  cfg.i2cAddress = LDC1614::I2cAddress::ADDR_GND;
  LDC1614::LDC1614 device;
  LDC1614::Status st = device.bind(cfg);
  return st.is(LDC1614::Err::INVALID_CONFIG) ? 0 : 1;
}
""",
        encoding="utf-8",
        newline="\n",
    )
    output = temp / "consumer"
    if os.name == "nt":
        output = output.with_suffix(".exe")

    command = [
        compiler,
        "-std=c++17",
        "-I",
        str(include_dir),
        str(consumer),
        str(source_file),
        "-o",
        str(output),
    ]
    result = run_command(command, temp)
    if result.returncode != 0:
        print(result.stdout, end="")
        print(result.stderr, end="", file=sys.stderr)
        return fail("packed package consumer compile failed")
    return 0


def main() -> int:
    status = ensure_version_header_tracked()
    if status != 0:
        return status

    compiler = find_compiler()
    if compiler is None:
        return fail("no C++ compiler found; set CXX or install c++/g++/clang++")

    with tempfile.TemporaryDirectory(prefix="ldc1614_consumer_") as temp_root:
        temp = Path(temp_root)
        archive = pack_library(temp)
        if archive is None:
            return 1
        package_root = extract_archive(archive, temp)
        status = compile_consumer(compiler, package_root, temp)
        if status != 0:
            return status

    print("Clean package consumer compile guard PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
