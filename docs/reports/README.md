# Retained HIL evidence

This directory contains the minimum reviewed hardware evidence needed to support
the library's current validation statements. It is not a release certificate;
the remaining hardware gates are listed in
[Validation status](../VALIDATION_STATUS.md).

## Fixture and scope

The retained runs used an ESP32-S2 on COM8, an LDC1614 at `0x2A`, pioarduino
`55.03.311` / ESP-IDF 5.5.5, 400 kHz I2C, and no LC sensor. They support
identity, register/configuration, cooperative lifecycle, and no-reset transport
claims only. They do not validate sensor physics, `0x2B`, LDC1612, INTB, SD,
drive-current tuning, or an ESP32-S3 product bus owner.

## Canonical artifacts

| Artifact | Status | Retained observation |
| --- | --- | --- |
| [`comprehensive-no-sensor-5e3199e.json`](20260804/comprehensive-no-sensor-5e3199e.json) | `FAIL` | Clean firmware completed discovery, initialization, apply, and wake. The confirmed software-reset write was followed by a failed MANUFACTURER_ID combined read (`code=14`, raw detail `259`) with partial-write/dirty-state provenance. |
| [`nonreset-exhaustive-e4d0436.json`](20260804/nonreset-exhaustive-e4d0436.json) | `FAIL` | Clean replacement firmware was flashed while the LDC remained powered. Its first combined read failed before matrix traffic; a true rail cycle restored reads. This bounds the transition to the upload/reboot interval without assigning an electrical cause. |
| [`transient-direct-identity-discriminator.json`](20260804/transient-direct-identity-discriminator.json) | Diagnostic `FAIL` | Temporary direct open-drain instrumentation observed idle-high lines and ACKs for the write address and register pointer, followed by NACK on the repeated-start read address. The dirty diagnostic lacks release identity and is not acceptance evidence. |
| [`cold-start-direct-discriminator.json`](20260804/cold-start-direct-discriminator.json) | Diagnostic `FAIL` | After both rails were removed for ten seconds, corrected instrumentation received every ACK and read MANUFACTURER_ID `0x5449`. The dirty diagnostic intentionally remains non-acceptance evidence. |
| [`nonreset-exhaustive-post-rail-v2-e4d0436.json`](20260804/nonreset-exhaustive-post-rail-v2-e4d0436.json) | `PASS` | Clean firmware passed 187/187 non-reset commands after a rail cycle, including replay/readback, four-channel profile boundaries, invalid-input fences, acquisition/mixed/identity stress, and alternating 100/400 kHz re-admission. |
| [`one-hour-nonreset-e4d0436.json`](20260804/one-hour-nonreset-e4d0436.json) | `PASS` | Clean firmware completed 3,600.0 seconds, 2,926 controller-reconstruction/re-admission cycles, and 32,186 commands with zero failures, unknowns, or resets and 32 ms worst command latency. |

Every JSON file is paired with the same-basename `.serial.txt` capture. The
compact JSON stores parsed command results plus the raw filename, byte count,
and SHA-256 digest instead of embedding a second copy of the complete
transcript. `python tools/check_repository_hygiene.py` verifies every pair.

## Retention policy

Superseded parser experiments, repeated failed recovery attempts, generated
Markdown reports, and earlier v2/v3 debug bundles are intentionally absent from
the current tree. Git history through commit `d3b434a` retains them if a future
investigation needs the chronology. New evidence belongs here only when it has
an exact fixture description, firmware-reported Git identity/status, structured
result, raw capture, and a current claim that depends on it.
