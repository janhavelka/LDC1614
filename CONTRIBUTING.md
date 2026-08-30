# Contributing

Thank you for considering contributing to this project!

## Quick Start

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Make your changes
4. Run the [validation checklist](docs/VALIDATION_STATUS.md)
5. Commit with a clear, imperative message: `git commit -m "Add X"`
6. Push and open a Pull Request

## Guidelines

### Code Style
- Follow existing code style (see `.clang-format`)
- Use `constexpr` instead of macros for constants
- Prefer explicit over implicit
- No heap allocations in steady-state library code

### Commits
- Write a short imperative subject naming the change, matching the existing
  history (for example `Expand diagnostics and harden I2C recovery evidence`).
  No commit-message format is enforced by CI.
- [Conventional Commits](https://www.conventionalcommits.org/) prefixes
  (`feat:`, `fix:`, `docs:`, `refactor:`, `test:`, `chore:`) are accepted but
  not required.

### Pull Requests
- Keep PRs focused (one feature/fix per PR)
- Update documentation if needed
- Add changelog entry under `[Unreleased]`
- Ensure CI passes

### Documentation

- Keep public API comments, README examples, integration guides, and the
  changelog consistent in the same change.
- Run `doxygen Doxyfile`; warnings are errors.
- Run `python tools/check_repository_hygiene.py`; local documentation links and
  retained JSON/raw HIL evidence must remain consistent.
- Do not edit or commit `docs/doxygen/`. It is generated output.
- Keep hardware and CI claims evidence-based. A build or dry run is not HIL.
- On Windows, run PlatformIO only through `.\scripts\pio.cmd`; do not install a
  second PlatformIO Core for this repository.
- Update both diagnostic CLI implementations, the host CLI manifest, behavioral
  tests, and HIL parser together when the command or output contract changes.
- The per-class transfer maxima (13/23, 15/25, 16/26, and `2 + 2N` up to 10)
  are restated in `README.md`, `docs/I2C_INTEGRATION.md`, and the `start*()`
  doc comments in `include/LDC1614/LDC1614.h`. They derive from
  `configurationTransferCount()` in `src/LDC1614.cpp`; change all four
  together, together with the software-reset and destructive-read prose.
- Hardware claims require a retained structured result and its raw transcript
  with matching firmware-reported revision and source status.
- Retained HIL counts (matrix command totals, soak duration/cycles/commands)
  are restated in `README.md`, `docs/VALIDATION_STATUS.md`,
  `docs/HIL_VALIDATION.md`, `docs/reports/README.md`, and the released
  `CHANGELOG.md` entry. Update every current copy in the same change; never
  leave one describing a superseded run.
- Maintainers follow [the release procedure](https://github.com/janhavelka/LDC1614/blob/main/docs/RELEASING.md) and create a
  new annotated tag only after the exact release commit passes CI.

### What We Accept
- Bug fixes
- Documentation improvements
- Performance improvements (with benchmarks)
- New examples (if they demonstrate a common use case)

### What We Probably Won't Accept
- Breaking API changes without discussion
- Heavy dependencies
- Platform-specific code in the library core
- Features that add heap allocations in steady state

## Questions?

Open a GitHub Discussion or Issue for questions.
