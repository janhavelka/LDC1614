# Release procedure

`library.json` is the version source of truth. Release tags must be annotated
and are immutable once pushed; the historical `v2.0.0` tag is lightweight and
is deliberately not rewritten. Run every command from the repository root on
the intended release commit; on Windows, invoke PlatformIO only through
`scripts\pio.cmd`.

## 1. Prepare the release candidate

Start from a clean working tree on the intended `main` branch. Fetch remotes
and fast-forward to its upstream before editing; stop on dirty, divergent, or
conflicted state instead of overwriting work.

```powershell
git status --short --branch
git diff --check
```

Choose the version under `AGENTS.md` and use
`python scripts/generate_version.py bump patch`, `bump minor`, or `bump major`
as appropriate. Confirm `library.json`, `idf_component.yml`, `Doxyfile`, and
`include/LDC1614/Version.h` all report the new version. Update the current
version prose in `README.md` and `docs/VALIDATION_STATUS.md`, and promote the
completed `CHANGELOG.md` entries to a dated version section. Keep historical
version/tag references intact and this procedure parameterised.

Review the validation boundary; the retained no-sensor HIL evidence does not
qualify sensor-equipped product hardware or later behavior-changing firmware.
Derive the release variables from the prepared metadata:

```powershell
$version = (Get-Content library.json -Raw | ConvertFrom-Json).version
$tag = "v$version"
```

## 2. Commit and validate the exact candidate

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
if ($LASTEXITCODE -ne 0) { throw "Release commit failed" }
if (git status --porcelain) { throw "Release candidate must be clean" }
$releaseSha = git rev-parse HEAD
```

Run every command in the
[required software checks](VALIDATION_STATUS.md#required-software-checks) list,
which is maintained there as the single copy, from this clean commit. Every
command must succeed and leave the tree clean. If a correction is needed,
commit it, capture the new `$releaseSha`, and repeat validation before pushing.

## 3. Push and wait for CI

```powershell
if ((git rev-parse HEAD) -ne $releaseSha -or (git status --porcelain)) {
    throw "Release candidate changed; repeat validation"
}
git push origin main
if ($LASTEXITCODE -ne 0) { throw "Release push failed" }
```

Wait for every required GitHub Actions job on that `main` commit to pass. With
GitHub CLI installed, the status can also be inspected with:

```powershell
gh run list --commit $releaseSha --limit 5
$releaseRun = gh run list --workflow ci.yml --commit $releaseSha --limit 1 --json databaseId --jq '.[0].databaseId'
if (-not $releaseRun) { throw "CI run for $releaseSha is not visible yet; retry this block" }
gh run watch $releaseRun --exit-status
```

## 4. Create and push the annotated tag

Create the tag locally so it is annotated rather than a lightweight tag:

```powershell
git fetch --prune --tags
if ($LASTEXITCODE -ne 0) { throw "Release fetch failed" }
git status --short --branch
git rev-list --left-right --count HEAD...origin/main
if ((git status --porcelain) -or (git rev-parse HEAD) -ne $releaseSha -or
    (git rev-parse origin/main) -ne $releaseSha) {
    throw "Release candidate changed; repeat validation and CI before tagging"
}
git tag --list $tag
if (git tag --list $tag) { throw "Release tag already exists; never replace it" }
$releaseRuns = gh run list --workflow ci.yml --commit $releaseSha --limit 1 --json status,conclusion | ConvertFrom-Json
if ($LASTEXITCODE -ne 0 -or -not $releaseRuns -or
    $releaseRuns[0].status -ne 'completed' -or
    $releaseRuns[0].conclusion -ne 'success') {
    throw "The exact release commit must have successful CI before tagging"
}
git tag -a $tag $releaseSha -m "Release $tag"
if ($LASTEXITCODE -ne 0) { throw "Annotated release tag creation failed" }
git show --stat $tag
git push origin $tag
if ($LASTEXITCODE -ne 0) { throw "Release tag push failed" }
```

The status must be clean, the existing-tag query must print nothing, and the
listed CI run must be complete and successful. The tagged commit must be the
reviewed green `main` commit. If any check differs, stop instead of moving or
replacing a tag.

## 5. Publish the GitHub release

In GitHub:

1. Open **Releases** and select **Draft a new release**.
2. Choose the tag you just pushed (`$tag`); do not create a second tag in
   the UI.
3. Keep `main` as the target and use title `LDC1614 $tag`.
4. Generate release notes, then reconcile them with the matching `$version`
   section of `CHANGELOG.md`.
5. Keep the no-sensor HIL limitations visible and publish the release.

With GitHub CLI, generate notes into a temporary file outside the repository:

```powershell
$releaseNotes = Join-Path $env:TEMP "LDC1614-$tag-release-notes.md"
gh api repos/janhavelka/LDC1614/releases/generate-notes -f "tag_name=$tag" -f "target_commitish=$releaseSha" --jq '.body' | Set-Content -Encoding utf8 $releaseNotes
```

Review and edit that file to include the matching changelog entries and the
current hardware-evidence limitations, then publish the reconciled notes:

```powershell
gh release create $tag --verify-tag --title "LDC1614 $tag" --notes-file $releaseNotes
```

Verify the published objects:

```powershell
git ls-remote --tags origin "refs/tags/$tag" "refs/tags/$tag^{}"
gh release view $tag
```
