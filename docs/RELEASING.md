# Release procedure

`library.json` is the version source of truth. Release tags must be annotated
and are immutable once pushed; the historical `v2.0.0` tag is lightweight and
is deliberately not rewritten. Run every command from the repository root on
the intended release commit; on Windows, invoke PlatformIO only through
`scripts\pio.cmd`.

## 1. Validate the release candidate

For `v3.1.0`, start from a clean working tree on that exact commit:

```powershell
git status --short --branch
git diff --check
```

Then run every command in the
[required software checks](VALIDATION_STATUS.md#required-software-checks) list,
which is maintained there as the single copy.

Confirm `library.json`, `idf_component.yml`, `Doxyfile`, and
`include/LDC1614/Version.h` all report `3.1.0`. `scripts/generate_version.py`
synchronises only the last three from `library.json`, so update the prose
copies by hand in the same commit: `README.md`, `docs/VALIDATION_STATUS.md`,
and every `3.1.0`/`v3.1.0` literal in this file. Review the validation
boundary; the retained no-sensor HIL evidence does not qualify sensor-equipped
product hardware.

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
git commit -m "Prepare release $tag"
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
$version = (Get-Content library.json -Raw | ConvertFrom-Json).version
$tag = "v$version"
git tag --list $tag
$releaseSha = git rev-parse HEAD
gh run list --workflow ci.yml --commit $releaseSha --limit 1
git tag -a $tag -m "Release $tag"
git show --stat $tag
git push origin $tag
```

The status must be clean, the existing-tag query must print nothing, and the
listed CI run must be complete and successful. The tagged commit must be the
reviewed green `main` commit. If any check differs, stop instead of moving or
replacing a tag.

## 4. Publish the GitHub release

In GitHub:

1. Open **Releases** and select **Draft a new release**.
2. Choose the tag you just pushed (`$tag`); do not create a second tag in
   the UI.
3. Keep `main` as the target and use title `LDC1614 $tag`.
4. Generate release notes, then reconcile them with the matching `$version`
   section of `CHANGELOG.md`.
5. Keep the no-sensor HIL limitations visible and publish the release.

Equivalent GitHub CLI command:

```powershell
gh release create $tag --verify-tag --title "LDC1614 $tag" --generate-notes
```

Verify the published objects:

```powershell
git ls-remote --tags origin "refs/tags/$tag" "refs/tags/$tag^{}"
gh release view $tag
```
