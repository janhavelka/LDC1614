# COM8 I2C transport regression investigation

Date: 2026-07-22

Fixture: ESP32-S2 on COM8, LDC1614 at address `0x2A`, another responding
device at `0x3C`, and no LC sensor attached. The absence of a sensor permits
silicon conversion-condition flags; it does not permit transport, identity, or
nonzero-status failures.

## Result

The maintained Arduino target is pinned to pioarduino `53.03.13` (Arduino
3.1.3 with ESP-IDF 5.3 libraries). On this fixture, the `54.03.20` stack
(Arduino 3.2.0 with ESP-IDF 5.4 libraries) intermittently failed combined
write/read operations with `ESP_ERR_INVALID_STATE` and read-length mismatch.
Changing bus speed and removing the startup scan did not correct it. A full
power cycle was required before the comparison stack produced stable results.

This is a target transport-stack decision, not a dependency or workaround in
the framework-neutral LDC core. It does not prove that every ESP-IDF new-master
backend is affected or that the TunnelMonitor ESP32-S3 backend is safe.

## Observations

| Stack / condition | Observation |
| --- | --- |
| Clean v3.0.0 firmware, pioarduino `54.03.20`, 400 kHz | Scan found `0x2A` and `0x3C`; combined identity reads intermittently returned length mismatch, zero/`0xFFFF` words, and raw `ESP_ERR_INVALID_STATE` (`259`). The committed failing smoke artifact is listed below. |
| Same stack, 100 kHz | Same failure class; lowering frequency did not correct the state failure. |
| Same stack without startup scan | Same failure class; the diagnostic scan was not the root cause. |
| pioarduino `53.03.13`, 400 kHz, after full power cycle | 25 repeated groups of `probe`, `reg 0x7E`, and `reg 0x7F` completed 75/75 identity transactions with `0x5449` / `0x3055`. This targeted A/B console evidence selected the candidate stack; it is not a substitute for the clean candidate smoke and one-hour artifact. |
| Clean post-tag `bf44cf1`, pioarduino `53.03.13`, 400 kHz | The NACK-heavy startup scan found `0x2A` and `0x3C`, after which combined reads latched at zero-length while address probes and register writes still completed. The 39-command smoke correctly failed 24 commands. The MCU and serial CLI remained responsive; the missing capability was explicit application-owned controller reinitialization without an MCU reset. |
| Clean `ac710c1`, Wire teardown/rebegin after each scan | The ESP32-S2 remained responsive for all 200 commands. All 25 scans and recovery commands returned success, but 16/25 following initialization reads failed and only 9/25 succeeded. A recovery operation that reports success while the next combined read remains unusable is not a valid ownership contract; this run forced removal of the duplicate Wire backend. |
| Clean `05a71d7`, shared new-master delete/recreate after each scan | The raw backend error became explicit `ESP_ERR_INVALID_STATE` (`259`). Twenty of 25 following initialization reads failed, scans sometimes reported false devices or timed out, and delete/recreate did not reset controller state. This negative run selected the driver's dedicated `i2c_master_bus_reset()` operation for the next candidate. |
| 1 MHz | Deliberately not run. The LDC1614 I2C interface maximum is 400 kHz, so 1 MHz would be outside the device contract even if another shared-bus device supports it. |

The failing clean v3.0.0 evidence is retained in:

- `hil-validation-COM8-20260722-v3-smoke.runner.json`
- `hil-validation-COM8-20260722-v3-smoke.runner.md`
- `hil-validation-COM8-20260722-bf44cf1-smoke.runner.json`
- `hil-validation-COM8-20260722-bf44cf1-smoke.runner.md`
- `hil-validation-COM8-20260722-ac710c1-recovery-stress.runner.json`
- `hil-validation-COM8-20260722-ac710c1-recovery-stress.runner.md`
- `hil-validation-COM8-20260722-05a71d7-recovery-stress.runner.json`
- `hil-validation-COM8-20260722-05a71d7-recovery-stress.runner.md`

The Markdown artifacts embed their raw command transcripts. All are intentional
`FAIL` records and must not be cited as positive HIL acceptance.

The resulting example-owner correction removes the implicit startup scan,
limits diagnostic scans to usable addresses `0x08..0x77`, and consolidates both
maintained ESP32 diagnostics on one ESP-IDF new-master transport. `busrecover`
uses the driver's explicit controller reset, invalidates trusted applied state,
and requires complete `init` replay. The ESP32-S2 upload profile also uses
automatic internal-USB bootloader entry, waits for port re-enumeration, and
returns to the application after flashing. These are candidate changes until
clean-firmware HIL passes.

## Integration consequence

The failing Arduino artifacts used Wire. The corrected Arduino diagnostic and
the native ESP-IDF diagnostic now use the same ESP-IDF new-master API family as
TunnelMonitor, while TunnelMonitor still creates a device handle per transfer.
The ownership implementations are therefore closer but not identical. Espressif issue
<https://github.com/espressif/esp-idf/issues/14030> records a NACK followed by
persistent `ESP_ERR_INVALID_STATE`; the issue remained open when checked on
2026-07-22. A product-neutral ESP32-S3 TunnelMonitor HIL gate is therefore still
required: induce a confirmed absent-address NACK, execute repeated valid
combined reads, then exercise explicit owner recovery while capturing raw
backend codes.

## Acceptance boundary

The stack comparison and post-tag failure establish real negative regression
records and an explicit no-MCU-reset recovery design. Release-grade positive
evidence still requires a clean firmware identity match, the full no-sensor
command matrix, a one-hour bounded soak with no unexpected reset or ambiguous
response, and exact output artifacts. Sensor accuracy, channel physics, INTB,
SD, address `0x2B`, LDC1612, and TunnelMonitor ESP32-S3 backend behavior remain
outside this fixture.
