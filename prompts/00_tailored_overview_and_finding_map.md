# LDC1614 Tailored Industry-Readiness Hardening Prompt Sequence

This bundle is tailored directly from `docs/LDC1614_INDUSTRY_READINESS_EXPLORATION_REPORT.md`.

The exploration report classified the repo as **Engineering-grade with major gaps**, not architecture-blocked. It identified these concrete blockers:

- **H1**: Unsupported production/industry-grade wording without hardware validation.
- **H2**: Multi-register writes can leave partial hardware state without dirty/sync contract.
- **H3**: Blocking API latency contracts are incomplete.
- **H4**: ESP-IDF path is native but not production-ready.
- **M1**: Raw register diagnostics are not variant/access-type aware.
- **M2**: `dataReady()` hides transport/status errors.
- **M3**: Copy/move, thread-safety, and ISR-safety contracts are too weak.
- **M4**: Native tests do not cover enough negative/variant/fault cases.
- **M5**: Datasheet ambiguity and documentation nits need explicit treatment.
- **M6**: INTB handling in IDF example conflicts with datasheet wording.
- **M7**: Example labels and production guidance are too informal.
- **L1/L2/L3**: Reproducible version metadata, `probe()` docs, and IDF target metadata need cleanup.

Use the prompts in this exact order:

1. `01_core_contracts_partial_state.md`
   - Fixes H2, M1, M3, M5, part of L2.
   - Core dirty/sync contract, partial-write diagnostics, copy/move policy, raw diagnostics, datasheet notes.

2. `02_timing_status_freshness.md`
   - Fixes H3, M2, autoscan/freshness partials.
   - Blocking timebase contract, validation-before-wait, latency tables, `readDataReady()` guidance, UNREADCONV/fresh sample semantics.

3. `03_esp_idf_port_hardening.md`
   - Fixes H4, M6, L3.
   - Native IDF example quality, no `std::string` in IDF path, nonblocking/bounded CLI loop, locking, error mapping, INTB config, CI IDF build setup.

4. `04_native_tests_fault_injection_ci.md`
   - Fixes M4 and strengthens all prior chunks.
   - Fake-bus fault injection, nth-call failures, NACK/data NACK/bus errors, variant tests, identity mismatch, coverage/guards.

5. `05_docs_hil_release_final_report.md`
   - Fixes H1, M7, L1/L2 docs, and hardware/HIL evidence gaps.
   - Honest wording, hardware integration checklist, HIL runner/procedure, evidence matrix, final comprehensive hardening report.

After prompt 5, the repo can be considered a software-hardening candidate. It still must not be called field-proven/industry-grade until the HIL and physical fault matrix is actually run on real hardware.
