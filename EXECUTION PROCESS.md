# EXECUTION PROCESS
> **ProWin** — How the AI Agent Should Plan, Build, Test, and Debug  
> GitHub: https://github.com/AwesomeSno/ProWin.git

---

## PHASE 0: Pre-Build Ritual (Always First)

> ⚠️ **RULE:** Before writing a single line of code, complete ALL steps in this phase.

1. Read `PROJECT OBJECTIVE.md`, `REQUIREMENTS.md`, and `CONSTRAINTS.md` in full
2. Check if a ProWin Xcode project already exists. If not, create it:
   - File > New > Project > macOS > App
   - Name: `ProWin`, Bundle ID: `com.awesomesno.prowin`
3. Initialise git and connect to remote:
   ```bash
   git init
   git remote add origin https://github.com/AwesomeSno/ProWin.git
   ```
4. Create the initial file structure: `Sources/`, `Tests/`, `Resources/`, `Docs/`, `Scripts/`
5. Create `CHANGELOG.md` with version `v0.1.0` entry and push:
   ```bash
   git add .
   git commit -m "chore: initial project scaffold v0.1.0"
   git push -u origin main
   ```

---

## PHASE 1: Planning

### Step 1.1 — Task Decomposition
Before any coding session, decompose the current objective into atomic tasks. Each task must map to exactly one commit. Write the task list as a numbered checklist comment at the top of the relevant Swift/C++ file.

### Step 1.2 — Interface Design First
Design all public Swift interfaces and C++ header files before implementing them. Stub all functions with `fatalError("Not yet implemented")` or `assert(false)`. This gives the build a clean compile baseline.

### Step 1.3 — Commit the Stubs
```bash
git add .
git commit -m "feat(engine): stub PE loader interface v0.2.0"
git push origin main
```

---

## PHASE 2: Building

### Step 2.1 — One Feature at a Time
Implement exactly one feature or function at a time. Do not start a second feature until the first compiles, passes its unit test, and has been pushed to GitHub.

### Step 2.2 — The Build-Commit Loop

> 🔁 **LOOP:** This loop must be followed for every single change without exception.

1. Write the implementation
2. Run `xcodebuild -scheme ProWin -configuration Debug build` — fix all errors and warnings
3. Run SwiftLint — fix all violations
4. Update `CHANGELOG.md` — add entry under the appropriate version heading
5. Increment version:
   - `PATCH +1` → fixes / minor additions
   - `MINOR +1` → new features
   - `MAJOR +1` → breaking changes
6. Update `CFBundleShortVersionString` and `CFBundleVersion` in project settings
7. Commit and push:
   ```bash
   git add .
   git commit -m "feat(loader): implement IAT resolution v0.3.0"
   git tag -a v0.3.0 -m "Release v0.3.0"
   git push origin main --tags
   ```

### Step 2.3 — CHANGELOG.md Format

Every entry must follow this exact format:

```markdown
## [1.2.3] - 2025-03-15

### Added
- PE loader now handles ASLR remapping

### Fixed
- Crash when DLL import table is empty

### Changed
- Increased registry cache size to 64 MB
```

---

## PHASE 3: Testing

### Step 3.1 — Unit Tests
- Every new public function must have at least one XCTest unit test before the feature is considered done
- Tests live in `ProWinTests/` and mirror the source structure
- Run with: `xcodebuild test -scheme ProWin -destination 'platform=macOS'`
- All tests must pass before committing — zero tolerance for skipped or xfail tests in `main`

### Step 3.2 — Integration Tests
- After completing each Phase 2 feature, run the integration test suite in `ProWinIntegrationTests/`
- Integration tests must launch a real `.exe` from `TestFixtures/` and assert on exit code, output, and performance

### Step 3.3 — Performance Benchmarks
- After every MINOR or MAJOR version bump, run the XCTest performance suite
- Benchmarks must be committed to `Docs/Benchmarks/<version>.json`
- If a benchmark regresses by more than 5%, the build is broken — do not push until fixed

---

## PHASE 4: Debugging Protocol

### Step 4.1 — Reproduce First
Never attempt a fix without first writing a failing test that reproduces the bug. Commit the failing test with prefix `test(failing):` before starting the fix.

### Step 4.2 — Bisect Before Fixing
If the root cause is unclear, use `git bisect` to identify the introducing commit. Document the bisect result in the commit message.

### Step 4.3 — Fix, Test, Push
1. Fix the root cause (not the symptom)
2. Confirm the previously failing test now passes
3. Run the full test suite to check for regressions
4. Update `CHANGELOG.md` under `### Fixed`
5. Increment PATCH version
6. Commit with prefix `fix:` and push

---

## PHASE 5: GitHub Push Rules (ABSOLUTE)

> ⛔ **ABSOLUTE RULE:** Push to GitHub after EVERY edit. No batch commits. No "I'll push later." Every change = one commit = one push.

- If the internet is unavailable, queue commits locally and push as soon as connectivity is restored — do not accumulate edits without committing
- Use `git status` before every commit to verify no untracked files are left behind
- Use `git log --oneline -5` after every push to confirm the remote received the commit
- If a push is rejected, do **not** force-push — instead:
  ```bash
  git pull --rebase origin main
  # resolve conflicts
  git push origin main --tags
  ```

**Mandatory post-edit checklist:**
```bash
git status                          # verify changes
git add .
git commit -m "<type>(<scope>): <description> v<X.Y.Z>"
git tag -a v<X.Y.Z> -m "Release v<X.Y.Z>"
git push origin main --tags
git log --oneline -3                # confirm push succeeded
```

---

## PHASE 6: Release Checklist

Before tagging any release (v1.0.0 and above):

- [ ] All unit and integration tests pass
- [ ] SwiftLint and Clang Static Analyser report zero warnings
- [ ] `CHANGELOG.md` is complete and accurate
- [ ] `CFBundleShortVersionString` matches the git tag
- [ ] Benchmarks have been run and committed to `Docs/Benchmarks/`
- [ ] A GitHub Release has been created with the CHANGELOG entry as release notes
- [ ] The Xcode archive has been created and notarised

---

*ProWin — EXECUTION PROCESS | github.com/AwesomeSno/ProWin*
