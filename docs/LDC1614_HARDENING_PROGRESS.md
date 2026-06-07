# LDC1614 Hardening Progress

## Prompt 01 - H2/M1/M3/M5 core contracts

### Findings addressed

- H2/H3 partial hardware state: configuration writes can leave hardware and cached configuration diverged.
- M1 raw diagnostics: raw register writes were not clearly documented as unsafe cache-desynchronizing diagnostics.
- M3 API contracts: driver copy/move, thread-safety, ISR-safety, and transport recursion contracts were too weak.
- M5 datasheet notes: deglitch 33 MHz ambiguity, IDRIVE table reference, reset/reapply timing assumptions, and `probe()` lifecycle caveat needed explicit documentation.

### Implemented changes

- Added `hardwareConfigDirty()` and `hardwareConfigDirtyError()` runtime diagnostics.
- Added dirty-state fields to `SettingsSnapshot`.
- Added `syncConfig()` to re-apply cached configuration without a reset.
- Dirty state is set for failed configuration writes that may have reached hardware, partial `_applyConfig()`, partial `setSingleChannelMode()`, partial `resetAndReapply()`, and public raw diagnostic writes.
- Dirty state is cleared only after full successful `begin()`, `syncConfig()`, `recover()` reapply, or `resetAndReapply()`.
- `softReset()` leaves dirty state intact until a later successful reinitialization or full sync path.
- `recover()` no longer treats identity-read success as configuration sync when dirty state is set; it reapplies cached configuration before returning success.
- Public raw register access remains available, but it is documented as diagnostic-only and not variant/access-type safe.
- Deleted driver copy/move construction and assignment.
- Added public Doxygen and README notes for thread/ISR safety, external serialization, and callback non-recursion.
- Documented the deglitch 33 MHz `b111` choice and the lack of local evidence for an arbitrary post-`RESET_DEV` delay.

### Public API changes

- Added `Err::CONFIG_DIRTY`.
- Added `LDC1614::hardwareConfigDirty()`.
- Added `LDC1614::hardwareConfigDirtyError()`.
- Added `LDC1614::syncConfig()`.
- Added `SettingsSnapshot::hardwareConfigDirty`.
- Added `SettingsSnapshot::hardwareConfigDirtyError`.
- Deleted copy/move operations for `LDC1614`.

### Tests added

- Copy/move deletion static assertions.
- Deglitch enum value regression test.
- Probe missing-callback behavior test.
- `_applyConfig()` partial failure dirty-state/detail test via `begin()`.
- Dirty clear/retain tests for `syncConfig()`.
- Dirty clear/retain tests for `recover()`.
- Raw diagnostic write dirty-policy tests.
- `setSingleChannelMode()` partial failure dirty-state/detail test.
- `resetAndReapply()` partial failure and success-clear tests.
- `softReset()` dirty-state retention until reinitialization.
- Existing single-register cache-failure test now also asserts dirty state.

### Commands run

- `python tools/check_core_timing_guard.py` -> `Core timing guard PASSED`.
- `python tools/check_cli_contract.py` -> `CLI contract PASSED`.
- `python tools/check_idf_example_contract.py` -> `IDF example contract PASSED`.
- `python scripts/generate_version.py check` -> `Up to date: C:\Users\Honza\Documents\Projects\LDC1614\include\LDC1614\Version.h`.
- `python -m platformio test -e native` -> passed; `84 test cases: 84 succeeded in 00:00:00.995`.
- `python -m platformio run -e esp32s3dev` -> passed; `esp32s3dev SUCCESS 00:00:05.195`.
- `python -m platformio run -e esp32s2dev` -> passed; `esp32s2dev SUCCESS 00:00:04.889`.
- `python -m platformio pkg pack` -> passed; wrote `LDC1614-1.0.0.tar.gz`, which was removed after packaging.

### Remaining related work

- Raw register access is still an expert diagnostic escape hatch, not a full metadata-enforced safe register API.
- H2 clock/divider validation and sleep-before-config sequencing are intentionally left for the next relevant hardening chunk.
- ESP-IDF pure-build validation and HIL/hardware validation remain unclaimed.

## Prompt 03 - H4/M6/L3 ESP-IDF hardening

### Findings addressed

- H4 ESP-IDF path: native example still compiled the shared heap-capable CLI and had weak build/guard coverage.
- M6 INTB handling: native IDF example configured INTB with a pull-up despite the documented push-pull device behavior.
- L3 component metadata: `idf_component.yml` did not state intended ESP-IDF targets.
- Related documentation: IDF example scope needed explicit diagnostic bring-up labeling and production ownership caveats.

### Implemented changes

- Removed `examples/common/Ldc1614Cli.cpp` from the ESP-IDF example compile path.
- Added `examples/esp_idf/basic/main/Ldc1614IdfCli.*`, a fixed-buffer native IDF diagnostic CLI with the limited safe command set: `help`, `version`, `probe`, `status`, `drv`, `cfg`, `read`, `readall`, `ready`, `recover`, `timing`, and `selftest`.
- Replaced blocking `getchar()` with a timed `select()`/`read()` console loop that regularly calls `device.tick()`.
- Converted IDF GPIO/I2C setup from `ESP_ERROR_CHECK` abort paths to `Status` reporting.
- Added an example-owned FreeRTOS mutex around native IDF I2C transport operations and reset paths.
- Kept IDF I2C error mapping bounded and conservative: timeout maps to `I2C_TIMEOUT`; invalid response/NACK stays phase-unspecified with raw `esp_err_t` in `detail`.
- Configured INTB as a plain input with pull-up disabled and documented that LDC1612/LDC1614 INTB is push-pull active-low/configurable.
- Added CMake-time `Version.h` generation for pure ESP-IDF builds from clean checkouts.

### IDF example/component changes

- `examples/esp_idf/basic/main/CMakeLists.txt` now compiles only native IDF example sources.
- `examples/esp_idf/basic/README.md`, `docs/IDF_PORT.md`, and `docs/IDF_PORT_IMPLEMENTATION.md` now label the IDF app as diagnostic bring-up and separate static checks, pure IDF builds, and hardware/HIL validation.
- `idf_component.yml` now lists intended targets `esp32s2` and `esp32s3` and avoids the previous production-grade wording.

### Tests/guards/CI added

- `tools/check_idf_example_contract.py` now parses the actual IDF `SRCS` list and scans compiled sources plus local headers for Arduino/`std::string` leakage, `getchar()`, and `ESP_ERROR_CHECK`.
- `tools/check_cli_contract.py` now preserves Arduino shared-CLI checks while enforcing a native fixed-buffer IDF CLI contract.
- GitHub Actions now runs the IDF source-contract guard and includes a pure ESP-IDF build matrix for `esp32s2` and `esp32s3`.

### Commands run

- `git status --short` -> clean at prompt start.
- `git branch --show-current` -> `hardening/ldc1614-industry-readiness`.
- `git checkout hardening/ldc1614-industry-readiness` -> already on branch.
- `python tools/check_core_timing_guard.py` -> `Core timing guard PASSED`.
- `python tools/check_cli_contract.py` -> `CLI contract PASSED`.
- `python tools/check_idf_example_contract.py` -> `IDF example contract PASSED`.
- `python scripts/generate_version.py check` -> `Up to date: C:\Users\Honza\Documents\Projects\LDC1614\include\LDC1614\Version.h`.
- `python -m platformio test -e native` -> passed; `84 test cases: 84 succeeded in 00:00:01.054`.
- `python -m platformio run -e esp32s3dev` -> passed; `esp32s3dev SUCCESS 00:00:07.827`.
- `python -m platformio run -e esp32s2dev` -> passed; `esp32s2dev SUCCESS 00:00:07.557`.
- `python -m platformio pkg pack` -> passed; wrote `LDC1614-1.0.0.tar.gz`, which was removed after packaging.
- `idf.py --version` -> failed locally: `idf.py` is not recognized as a cmdlet, function, script file, or operable program.
- `idf.py -C examples/esp_idf/basic set-target esp32s3 build` -> not run locally because `idf.py` is unavailable.
- `idf.py -C examples/esp_idf/basic set-target esp32s2 build` -> not run locally because `idf.py` is unavailable.

### Remaining related work

- Pure ESP-IDF build success remains unclaimed locally until `idf.py` or CI logs are available.
- The example mutex serializes native I2C transactions only; production applications with multiple tasks must still serialize public driver API calls.
- No hardware/HIL validation is claimed for INTB, SD, address variants, fault injection, sensor behavior, or soak stability.
