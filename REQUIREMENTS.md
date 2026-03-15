# REQUIREMENTS
> **ProWin** — Feature Requirements Specification  
> GitHub: https://github.com/AwesomeSno/ProWin.git

---

## REQ-1: Execution Engine

### REQ-1.1 PE Binary Loader
- Must load standard Windows PE32 and PE32+ (`.exe`, `.dll`) files
- Must resolve import address tables (IAT) against the built-in DLL stub library
- Must support both x86_64 and i386 PE binaries
- Must handle ASLR (Address Space Layout Randomisation) by remapping base addresses into macOS virtual address space

### REQ-1.2 System Call Translation
- Must intercept and translate the following Win32 API categories:
  - Process and thread management (`CreateProcess`, `CreateThread`, `WaitForSingleObject`, etc.)
  - File I/O (`CreateFile`, `ReadFile`, `WriteFile`, `DeleteFile`, etc.)
  - Memory management (`VirtualAlloc`, `VirtualFree`, `HeapAlloc`, etc.)
  - Synchronisation primitives (`CreateMutex`, `CreateEvent`, `CreateSemaphore`, etc.)
  - Window management (`CreateWindowEx`, `SendMessage`, `DefWindowProc`, etc.)
- Must return correct HRESULT / Win32 error codes to maintain application compatibility

### REQ-1.3 DLL Stub Library
- Must provide stubs for at minimum: `kernel32.dll`, `user32.dll`, `gdi32.dll`, `ntdll.dll`, `advapi32.dll`, `shell32.dll`, `ole32.dll`, `msvcrt.dll`
- Stubs must be versioned and hot-patchable without restarting ProWin

---

## REQ-2: Graphics Subsystem

### REQ-2.1 DirectX Translation
- Must translate DirectX 9, 10, 11 calls to Metal via a DXMT-compatible layer
- Must support shader compilation from HLSL to Metal Shading Language (MSL) at runtime
- Must handle swap chain creation and present calls with vsync support

### REQ-2.2 Display
- Must support Retina (HiDPI) displays with automatic scaling
- Must allow Windows app windows to be resized, minimised, and maximised from the macOS window manager
- Must support fullscreen mode with macOS Spaces integration

---

## REQ-3: Storage and Registry
- Must maintain a per-app virtual registry backed by SQLite
- Must map `HKEY_LOCAL_MACHINE` and `HKEY_CURRENT_USER` to isolated containers
- Must redirect `C:\Windows` and `C:\Program Files` reads to bundled stubs
- Must redirect `C:\Users\<user>\...` writes to a macOS Application Support sandbox

---

## REQ-4: Networking
- Must proxy Windows socket calls (Winsock2) to BSD sockets
- Must respect macOS network privacy permissions (`LocalNetwork` entitlement)
- Must support IPv4 and IPv6

---

## REQ-5: User Interface

### REQ-5.1 App Library
- Must display installed Windows apps with name, icon, and last-run date
- Must allow drag-and-drop of `.exe` files to add new apps
- Must allow right-click context menu with: Launch, Show in Finder, Remove, Properties

### REQ-5.2 Performance Dashboard
- Must display live per-process metrics: CPU %, GPU %, RAM (MB), Disk read/write (MB/s)
- Must provide a 60-second rolling graph for each metric
- Must allow the user to set performance presets: Balanced, Performance, Battery Saver

### REQ-5.3 Settings
- Must include toggles for: GPU acceleration, Retina scaling, Auto-launch on login, Notifications
- Must include a Compatibility Mode selector (per app): Default, Legacy (Win7 mode), High-Performance

---

## REQ-6: GitHub Versioning (MANDATORY)

> ⚠️ **CRITICAL:** Every single code change — no matter how small — must be committed and pushed to GitHub before moving to the next task.

- Git remote must be set to: `https://github.com/AwesomeSno/ProWin.git`
- Every commit must follow Conventional Commits format: `feat:`, `fix:`, `perf:`, `refactor:`, `docs:`, `chore:`
- Version numbers must follow Semantic Versioning (SemVer): `MAJOR.MINOR.PATCH`
- A `CHANGELOG.md` must be maintained at the root of the repository
- Each version bump must update: `CHANGELOG.md`, the Xcode project version (`CFBundleShortVersionString`), and the build number (`CFBundleVersion`)

```bash
git add . && git commit -m "feat(engine): add PE loader base v0.2.0" && git push origin main
```

---

*ProWin — REQUIREMENTS | github.com/AwesomeSno/ProWin*
