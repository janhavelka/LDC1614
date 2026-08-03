# Arduino diagnostic bring-up CLI

This PlatformIO/Arduino example demonstrates the v3 cooperative owner contract.
It is diagnostic firmware, not a production bus manager or hardware-validation
claim.

The application owns the ESP-IDF new-master bus/device handles, pins,
serialization, the 64-bit monotonic time extension, absolute job deadlines,
transfer budget, and recovery policy. It first calls bus-silent `bind()`,
schedules `startInitialize()`, then advances the active job by at most one I2C
callback per `loop()` pass. Terminal results are consumed through
`takeResult()` exactly once.

The Arduino and native ESP-IDF diagnostics share
`examples/esp32/I2cMasterTransport.*`. A combined register read is one bounded
`i2c_master_transmit_receive()` transaction. Explicit owner recovery removes
the diagnostic's device handle, deletes/recreates its sole owned bus, and
then runs the ESP-IDF driver's bounded bus reset/line-clear before recreating
the device handle. It performs one bounded target-address probe and reports success
only on ACK; full `init` replay remains mandatory. The
repository pins pioarduino
`55.03.311`; COM8 reproduced the open ESP-IDF post-NACK
`ESP_ERR_INVALID_STATE` failure on this pin. Do not treat `busrecover` as
guaranteed recovery until the complete recovery/replay gate passes, and do not
upgrade without repeating combined-read and post-NACK tests on real LDC
hardware. A production shared-bus owner must coordinate and rebuild all of its
registered device handles; this single-device diagnostic cannot supply that
product policy.

The maintained ESP32-S2 PlatformIO profile uses its internal USB CDC upload
path: one automatic bootloader entry, port re-enumeration, no redundant
pre-flash reset, and return to the application. The upload command also
suppresses esptool's Unicode progress bar so Windows console encoding cannot
interrupt the flash. Routine firmware updates must not require operator reset
cycles.

Type `help` for the canonical, colored command reference. It is generated from
the same fixed command manifest used to check Arduino/ESP-IDF parity and groups
all 64 command families by purpose. Repository contract checks separately
associate each row with its execution class, safety class, fixture requirement,
and stable evidence keys. `color off` emits the same help without ANSI escapes
for logs and parsers; `verbose 1` adds bounded per-step diagnostics without
changing scheduling.

The surface covers:

- lifecycle and job control: `bind`, `end`, `init`, `apply`, `resetreapply`,
  `sleep`, `wake`, `cancel`, `job`, `result`, `invalidate`, and `busrecover`;
- readings: `read`, `last`, `watch`, `samplerate`, `ready`, `status`,
  `status_raw`, `intb`, and `initdrive`;
- the complete profile: `mode`, `refclk`, `deglitch`, `activation`, `timeout`,
  RP/auto-amplitude/high-current/INTB behavior, all error routes, and every
  per-channel RCOUNT, SETTLECOUNT, divider, OFFSET, IDRIVE, and sensor bound;
- registers and calculations: `probe`, `scan`, `dump`, `verify`, `reg`, `wreg`,
  `decode`, `freq`, `timing`, and `driveua`; and
- diagnostics: `drv`, `state`, `selftest`, `stress`, `stress_mix`, `soak`, and
  optional application-owned shutdown-pin control through `sd`.

Profile setters only change a fixed-size staged copy and perform no I2C. Use
`profile validate`, put verified hardware to sleep, then run
`profile commit confirm`; the commit changes desired state but still performs
no I2C. `apply` is the explicit full replay and verification step. `addr` and
`variant` are intentional build-profile facts: change `makeBoardConfig()`, end,
rebuild, and bind instead of changing the live transport identity from the CLI.
The accepted `error` fields are `data-under`, `data-over`, `data-watchdog`,
`data-amplitude-high`, `data-amplitude-low`, `status-under`, `status-over`,
`status-watchdog`, `status-amplitude-high`, `status-amplitude-low`,
`status-zero-count`, and `data-ready`; `errors show|all|none` handles the whole
set.

Commands that may mutate hardware, invalidate trust, reconstruct the bus, or
perform broad/raw destructive reads require the literal `confirm` token as
shown in `help`. The explicit semantic `status`, `status_raw`, `ready`, and
acquisition commands do not require confirmation; they retain and print their
destructive STATUS evidence. Raw register writes make the applied configuration
unknown and must be followed by `init` or a verified replay before trusted
acquisition.

Every multi-transfer core job and CLI diagnostic session is cooperative.
`service()` performs at most one physical I2C attempt per Arduino `loop()` pass;
`maxTransfers == 0` is never used to hide work. `cancel` is bus-silent, job
results are correlated by operation ID and consumed once, and fixed session
limits prevent unbounded loops. `watch`, `samplerate`, `stress`, `stress_mix`,
and `soak` report requested/completed/failure counts and terminal outcomes.
`samplerate` first performs at most one destructive STATUS readiness read per
pass and waits only until a fixed per-sample deadline. It accepts a sample only
when the requested channel is selected, valid, fresh, fault-free, not overrun,
and within the configured sensor-frequency bounds; its output retains readiness
snapshot and check-count evidence. It never busy-waits for DRDY.
Acquisition output explicitly says `SEQUENTIAL_READOUT`, includes both STATUS
snapshots and all masks, and never claims simultaneous samples.

The serial loop reads at most 32 characters and accepts at most one complete
command per pass. A prompt is printed only after immediate work or a scheduled
operation reaches a terminal result, so pasted commands cannot force multiple
I2C callbacks into one pass.

`scan` cooperatively probes the 112 usable addresses `0x08..0x77`; reserved I2C
address groups are excluded. Address NACK means no device and is counted as a
completed probe, while timeout or bus failure terminates with a non-OK status.
Scanning is diagnostic owner policy, never automatic startup behavior. Run
`busrecover confirm` and `init` after scan diagnostics when the controller or
attached devices require post-NACK recovery. If the backend remains in
`ESP_ERR_INVALID_STATE`, stop the run and physically cycle device/bus power;
repeated commands on the poisoned state are not recovery evidence.

The example profile values, GPIO8/GPIO9 pins, 43 MHz internal-clock estimate,
sensor-frequency bounds, and drive-current code are placeholders. Replace and
validate them against the exact board, LC tank, target, and clock plan.
