# CONSTRAINTS
> **ProWin** — Technical Rules and Technology Mandates  
> GitHub: https://github.com/AwesomeSno/ProWin.git

---

## C-1: Platform Constraints
- Target OS: macOS 13 Ventura and later (macOS 14 Sonoma as the primary dev target)
- Supported hardware: Apple Silicon (M1/M2/M3/M4) and Intel x86_64 Macs
- Minimum Xcode version: Xcode 15
- Deployment target: macOS 13.0
- The app MUST be a universal binary (`arm64` + `x86_64`) unless Metal-only features prevent it

---

## C-2: Language and Framework Rules

> 🚫 **RULE:** No third-party UI frameworks. All UI must be built with SwiftUI or AppKit.

- Primary language: Swift 5.9+
- C/C++ (C++17) is allowed for the execution engine and low-level translation layers only
- Swift/C++ interop via Swift Package Manager bridging headers
- No Electron, no web views for primary UI surfaces
- Metal must be used for all GPU rendering — no OpenGL (deprecated on macOS)
- CoreData or SQLite (via GRDB.swift) for persistent storage — no third-party ORMs

---

## C-3: Performance Constraints
- The translation layer must add no more than **5% CPU overhead** versus native execution for CPU-bound tasks
- GPU call translation latency per frame must not exceed **2 ms** on Apple Silicon
- ProWin's own memory footprint (excluding guest app memory) must stay below **200 MB RSS**
- Startup time from cold launch to app-ready UI must be under **2 seconds**
- All UI rendering must be asynchronous — no blocking on the main thread for more than **16 ms**

---

## C-4: Security Constraints
- The app must be hardened-runtime signed for App Store and Gatekeeper compliance
- Each Windows app runs in its own XPC service with a restricted sandbox
- No kernel extensions (kexts) — use DriverKit or user-space alternatives
- No JIT compilation entitlements unless strictly necessary; if used, justify in a dedicated entitlements file
- All file system access must go through macOS Security-Scoped Bookmarks
- Network access must declare `com.apple.security.network.client` entitlement explicitly

---

## C-5: Code Quality Constraints
- All Swift code must pass SwiftLint with the project's `.swiftlint.yml` configuration (zero warnings policy)
- All C/C++ code must be analysed with Clang Static Analyser (zero critical warnings)
- Test coverage for the execution engine must be ≥80% (measured via Xcode Code Coverage)
- No force-unwrap (`!`) in Swift except in unit tests with clear justification comments
- All public APIs must have DocC documentation comments

---

## C-6: Version Control Constraints (MANDATORY)

> ⛔ **NON-NEGOTIABLE:** These rules are absolute. Every violation must be corrected immediately.

- Git remote: `https://github.com/AwesomeSno/ProWin.git`
- Default branch: `main`
- Feature branches: `feature/<short-description>`
- No force-pushes to `main`
- Every commit must include a version bump — even patch-level fixes must increment the PATCH number
- `CHANGELOG.md` must be updated in the same commit as the version bump
- Tag every release:
  ```bash
  git tag -a v1.2.3 -m "Release v1.2.3"
  ```

```
CFBundleShortVersionString = "1.0.0";  // Marketing version (SemVer)
CFBundleVersion = "1";                 // Build number (increment each push)
```

---

## C-7: Dependency Constraints
- Prefer Swift Package Manager (SPM) over CocoaPods or Carthage
- All dependencies must have an active open-source licence (MIT, Apache 2.0, BSD)
- No dependencies with known CVEs above CVSS 7.0 at time of integration
- Vendored C/C++ libraries (e.g., Mesa, LLVM IR) must be pinned to an exact commit hash in `Package.swift`

---

*ProWin — CONSTRAINTS | github.com/AwesomeSno/ProWin*
