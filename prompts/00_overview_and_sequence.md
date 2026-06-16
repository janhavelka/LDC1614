# LDC1614 Industry-Readiness Hardening — Chunked Prompt Set

> Historical artifact: this was an execution prompt for a past hardening pass.
> It is preserved for traceability and is not current repository guidance.
> Current instructions live in `AGENTS.md`; current docs are indexed from
> `docs/README.md`.

Use these prompts one by one, in order. Each prompt is intentionally scoped so the coding agent can complete a coherent pass, commit it, push/sync it, and then wait for the next prompt.

Target repository: the currently opened LDC1614/LDC1612 repository.

Overall objective: move this library toward an industry-standard, production-ready I2C LDC1612/LDC1614 driver with framework-neutral core, strong ESP-IDF/Arduino examples, deterministic API contracts, good tests, honest docs, and a final report that clearly separates software readiness from hardware validation gaps.

Known audit themes to address:

- Core architecture appears strong and callback/transport based.
- Main known software gaps are around ESP-IDF example idiom/parity, component metadata, documentation honesty, test coverage, latency contracts, and validation evidence.
- IDF example reportedly pulls shared/common CLI code using `std::string`; this weakens the “native fixed-buffer ESP-IDF” claim.
- `idf_component.yml` reportedly lacks explicit target metadata.
- Hardware validation is still not proven for real LDC1614 behavior, DRDY/INTB, channel sequencing, coil/sensor limits, fault/unplug behavior, and full HIL stress.

Device facts the coder must keep in mind:

- LDC1612 has 2 channels; LDC1614 has 4 channels.
- I2C address is selected by ADDR pin: 0x2A when low, 0x2B when high.
- The device is configured over 400 kbit/s I2C.
- It measures LC resonator frequency and reports 28-bit conversion results split across MSB/LSB registers.
- DATAx_MSB must be read before DATAx_LSB for coherent channel data.
- RCOUNTx controls conversion time; SETTLECOUNTx controls sensor startup/settling time.
- INTB can be configured for conversion/status events and is push-pull, not open-drain.
- SD pin controls hardware shutdown/inactive mode and must not float.

Commit discipline:

After each prompt:

```bash
git diff --check
git status --short
python -m platformio test -e native || true
python -m platformio run -e esp32s3dev || true
python -m platformio run -e esp32s2dev || true
```

Run additional repo-specific guards if they exist. Then commit and sync:

```bash
git add -A
git commit -m "<clear scoped commit message>"
git push -u origin HEAD
```

If a command cannot run because tools are missing, record that exact fact in the prompt’s report section. Do not invent validation results.
