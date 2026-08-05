# Contributing

Thank you for considering contributing to this project!

## Quick Start

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Make your changes
4. Run the [validation checklist](docs/VALIDATION_STATUS.md)
5. Commit with a clear message: `git commit -m "feat: add X"`
6. Push and open a Pull Request

## Guidelines

### Code Style
- Follow existing code style (see `.clang-format`)
- Use `constexpr` instead of macros for constants
- Prefer explicit over implicit
- No heap allocations in steady-state library code

### Commits
- Use [Conventional Commits](https://www.conventionalcommits.org/) format:
  - `feat:` new feature
  - `fix:` bug fix
  - `docs:` documentation only
  - `refactor:` code change that neither fixes a bug nor adds a feature
  - `test:` adding or updating tests
  - `chore:` maintenance tasks

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
- Hardware claims require a retained structured result and its raw transcript
  with matching firmware-reported revision and source status.
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
