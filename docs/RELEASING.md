# Release procedure

`library.json` is the version source of truth. Release tags are annotated and
immutable. Run every command from the repository root on the intended release
commit; on Windows, invoke PlatformIO only through `scripts\pio.cmd`.

## 1. Validate the release candidate

For `v3.1.0`:

```powershell
git status --short --branch
git diff --check
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
```

Confirm `library.json`, `idf_component.yml`, `Doxyfile`, and
`include/LDC1614/Version.h` all report `3.1.0`. Review the validation boundary;
the retained no-sensor HIL evidence does not qualify sensor-equipped product
hardware.

## 2. Commit and push

Review exactly what will enter the release, then commit it:

```powershell
git status --short --branch
git diff --stat
git diff
git add -A
git diff --cached --check
git diff --cached --stat
git diff --cached
git commit -m "Prepare release v3.1.0"
git push origin main
```

Wait for every required GitHub Actions job on that `main` commit to pass. With
GitHub CLI installed, the status can also be inspected with:

```powershell
$releaseSha = git rev-parse HEAD
gh run list --commit $releaseSha --limit 5
$releaseRun = gh run list --workflow ci.yml --commit $releaseSha --limit 1 --json databaseId --jq '.[0].databaseId'
if (-not $releaseRun) { throw "CI run for $releaseSha is not visible yet; retry this block" }
gh run watch $releaseRun --exit-status
```

## 3. Create and push the annotated tag

Create the tag locally so it is annotated rather than a lightweight tag:

```powershell
git fetch --prune --tags
git pull --ff-only origin main
git status --short --branch
git rev-list --left-right --count HEAD...origin/main
git tag --list v3.1.0
$releaseSha = git rev-parse HEAD
gh run list --workflow ci.yml --commit $releaseSha --limit 1
git tag -a v3.1.0 -m "Release v3.1.0"
git show --stat v3.1.0
git push origin v3.1.0
```

The status must be clean, the existing-tag query must print nothing, and the
listed CI run must be complete and successful. The tagged commit must be the
reviewed green `main` commit. If any check differs, stop instead of moving or
replacing a tag.

## 4. Publish the GitHub release

In GitHub:

1. Open **Releases** and select **Draft a new release**.
2. Choose the existing tag `v3.1.0`; do not create a second tag in the UI.
3. Keep `main` as the target and use title `LDC1614 v3.1.0`.
4. Generate release notes, then reconcile them with the `3.1.0` section of
   `CHANGELOG.md`.
5. Keep the no-sensor HIL limitations visible and publish the release.

Equivalent GitHub CLI command:

```powershell
gh release create v3.1.0 --verify-tag --title "LDC1614 v3.1.0" --generate-notes
```

Verify the published objects:

```powershell
git ls-remote --tags origin refs/tags/v3.1.0 refs/tags/v3.1.0^{}
gh release view v3.1.0
```
