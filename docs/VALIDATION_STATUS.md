# LDC1614 validation status

This page separates repeatable software checks from retained physical evidence.
It is not a release certificate, and a build or host test is never presented as
hardware validation.

## Release state

`library.json` is the version source of truth. Version `3.1.0` is published:
the annotated tag `v3.1.0` points at commit `9a86034`, matching the `3.1.0`
entry in `CHANGELOG.md`. Never move or reinterpret an existing tag. The next
publication requires a new version in `library.json` (for example through
`python scripts/generate_version.py bump patch`), a reviewed commit that passes
CI, and its own annotated tag; `docs/RELEASING.md` holds the procedure.

No retained run qualifies the exact release commit on a sensor-equipped target.
The strongest current hardware evidence used clean firmware commit `e4d0436` on
an ESP32-S2/LDC1614 no-sensor fixture. Later repository changes concern release
documentation, host validation, evidence storage, and behavior-preserving CLI
cleanup, but that distinction must remain visible.

## Required software checks

Run these from a clean checkout of the proposed release commit:

```powershell
python tools/check_core_timing_guard.py
python tools/check_cli_contract.py
python tools/check_idf_example_contract.py
python tools/check_readiness_claims.py
python tools/check_repository_hygiene.py
python tools/test_ldc1614_hil_runner.py
python scripts/generate_version.py check
python tools/check_clean_consumer_compile.py
.\scripts\pio.cmd test -e native
.\scripts\pio.cmd run -e esp32s3dev
.\scripts\pio.cmd run -e esp32s2dev
doxygen Doxyfile
git diff --check
```

CI additionally builds the native ESP-IDF diagnostic for ESP32-S2 and ESP32-S3
with ESP-IDF 6.0.2 and verifies Doxygen with warnings treated as errors. The HIL
runner host suite tests parsing, fail-fast behavior, and no-port classification;
it does not operate hardware unless an explicit serial fixture is supplied.

## Retained hardware evidence

The canonical JSON/transcript pairs are indexed in the repository-only
[Retained HIL evidence](https://github.com/janhavelka/LDC1614/blob/main/docs/reports/README.md).

| Boundary | Retained result | Meaning |
| --- | --- | --- |
| Non-reset matrix | Clean `e4d0436`: 187/187 commands passed after a rail cycle | Supports identity, complete replay/readback, four-channel configuration boundaries, invalid-input fences, protocol stress, and 100/400 kHz re-admission on the named no-sensor fixture. |
| No-reset soak | Clean `e4d0436`: 3,600.0 seconds, 2,926 cycles, 32,186 commands, zero failures/unknowns/resets, 32 ms worst latency | Supports bounded controller reconstruction, applied-state invalidation, full initialization/replay, identity, STATUS, and sleep/wake behavior. It is not a sensor-cadence or software-reset test. |
| Software reset | Clean `5e3199e`: reset write succeeded; the following MANUFACTURER_ID combined read failed with raw detail `259` | `RESET_DEV` remains unqualified. The failure retained partial-write and dirty applied-state provenance. |
| MCU-only upload/reboot | Clean `e4d0436`: a previously readable energized LDC refused the first combined read after the upload/reboot interval; a true rail cycle restored it | Qualify MCU-only reset and defined SD/power behavior on every product. The initiating electrical edge was not captured. |
| Wire-level discriminator | Temporary direct reader observed ACK for `0x2A` write and pointer `0x7E`, then NACK for the repeated-start read address; all ACKs and `0x5449` returned after a rail cycle | Narrows the persistent condition but does not identify its initiating cause or provide release acceptance. |

Every retained run above used the Arduino platform pinned in `platformio.ini`
(pioarduino `55.03.311`, ESP-IDF 5.5.5), which reports every synchronous
transaction that does not reach its internal `DONE` state as
`ESP_ERR_INVALID_STATE` (`259`), including an ordinary NACK. Raw detail `259`
therefore does not prove a stuck shared bus. No retained run exercises the
natively supported ESP-IDF 6.x generation, whose transaction error codes
differ; re-qualify the raw detail before relying on it there. Controller
reconstruction is not device admission; complete identity plus replay remains
required.

## Hardware evidence still required

Before claiming suitability for a specific deployment, retain evidence for:

- the exact board, release firmware, device variant, address strap, populated
  channels, reference clock, complete production profile, and full identity
  and configuration readback;
- sensor-attached DATA MSB-before-LSB ordering under active conversions,
  STATUS/UNREAD/INTB/error-channel side effects, freshness, delayed-read data
  loss, frequency bounds, under-range, over-range, watchdog, zero-count, and
  amplitude conditions where electrically safe, and physical response on every
  selected channel;
- coil tuning, target range, deglitch choice, drive current, and calibration;
- correlated initialization, apply, acquisition, active cancellation, deadline
  cancellation followed by a clean new operation, invalidation, and
  exactly-once terminal-result behavior;
- controlled NACK/timeout, unplug/replug or power loss, bounded owner recovery,
  full replay, and continued operation of another shared-bus device;
- MCU-only reset, `RESET_DEV` if exposed by the product, INTB, and SD behavior;
- LDC1612 and `0x2B` fixtures when those variants are supported; and
- a bounded soak at the production channel mask, cadence, clock profile, and
  exact application-owned ESP32 backend.

Use [HIL validation](HIL_VALIDATION.md) for collection rules. Without real
hardware plus matching firmware-reported revision and source status, the runner
result is `NOT_RUN`, not a pass.
