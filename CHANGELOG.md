# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.21.0] - 2026-03-17

### Added
- **Native Frame Sync**: Upgraded synchronization from `CADisplayLink` to `CVDisplayLink` for professional-grade timing on macOS.
- **Hardware Callback**: Implemented C-style display link callback to handle periodic engine state polling.
- **Improved UI Reactivity**: Refactored `isRunning` as `@Published` for better integration with SwiftUI views.

### Changed
- Refactored `GameLoop.swift` to use `CoreVideo` instead of `QuartzCore` for main loop timing.

## [0.20.0] - 2026-03-17

### Added
- **Frame-Accurate UI**: Replaced `Timer` with `CADisplayLink` for smoother 60fps rendering.
- **Hardware Bridge**: Established connection between native macOS Input/Audio and Windows stubs.
- **XInput Engine**: Real controller state is now propagated from `GCController` to `XInputGetState`.
- **Audio Hook**: Added groundwork for `XAudio2` and `DirectSound` telemetry to native `AudioManager`.

### Fixed
- **Stub Stability**: Fixed missing include in `StubManager.cpp` causing build failures.

## [0.19.0] - 2026-03-17

### Added
- **x64 Calling Convention**: Redefined stub architecture to use `CPUContext` for argument passing (RCX, RDX, R8, R9).
- **Return Value Handling**: Stubs now return values in `RAX` via the `CPUContext`.
- **XInput Support**: Implemented `XInputGetState` with correct memory clearing for `XINPUT_STATE`.
- **Stub Orchestrator**: Added `StubManager::callStub` for centralized stub execution.

### Changed
- Refactored `StubFunction` signature to be `Win64Stub` compatible.

## [0.18.0] - 2026-03-17

### Added
- **IAT Resolution**: Implemented dynamic Import Address Table resolution in `PEImage`.
- **Stub Fallbacks**: Added `getOrCreateStub` with "null stub" logic to prevent crashes on unimplemented APIs.
- **Dynamic Linking**: Engine now correctly walks name and ordinal thunks to bind DLL imports.
- **Header Improvements**: Added x64-specific thunk structures and corrected packing pragmas.

### Fixed
- **Loader Crash**: Removed `fatalError` from `PELoader.swift`, enabling binaries with imports to load.

## [0.17.0] - 2026-03-17

### Added
- **Conditional Jumps**: Implemented `rflags`-based branching for `JZ`, `JNZ`, `JL`, `JGE`, `JLE`, and `JG`.
- **PE Protection Pass**: Added per-section memory protections (`mprotect`) based on PE section characteristics.
- **Unified Loader**: Consolidated `PELoader` logic into a single atomic `load(from:)` call returning `PELoadResult`.
- **CPU Flags Engine**: Implemented `ALU.h` with `updateFlags64` for tracking ZF, SF, CF, and OF.

### Fixed
- **Double-Increment Bug**: Fixed `RIP` advancing twice by centralizing incrementing logic in `InstructionDispatcher`.
- **VRAM Safety**: Added robust `mmap` error handling for VRAM allocation with UI alert propagation.
- **Relocation Timing**: Ensured memory protections are applied *after* relocations are performed.

## [0.16.0] - 2026-03-15

### Added
- **GRAPHICS BRIDGE**: Implemented virtual VRAM (framebuffer) that emulated x64 code can write to directly.
- **Metal Rendering**: Added a blitting system to copy virtual VRAM pixels to a macOS Metal window.
- **Loop Support**: Added `DEC`, `JNZ`, and `STOSD` instruction support to the interpreter.
- Optimized `DisplayManager` for Apple Silicon 16KB memory pages.

## [0.15.0] - 2026-03-15

### Added
- **COMPUTE SUPPORT**: Implemented `MOV` and `ADD` instruction decoding and execution.
- **CPU STATE TRACKING**: Added a live "Register State" view to the UI to monitor emulation progress.
- **REX/ModRM Support**: Foundation for complex x64 addressing modes.
- Enhanced `CPUContext` with indexed GPR access.

## [0.14.1] - 2026-03-15

### Fixed
- Fixed UI state stuck on "Executing..." by adding `@ObservedObject` for `GameLoop`.
- Added dynamic "Execution Finished" status message when the engine terminates.

## [0.14.0] - 2026-03-15

### Added
- **SUCCESSFUL SMOKE TEST**: The engine can now load, map, and emulate x64 instructions from a PE file.
- Instruction support for `NOP` and `RET`.

### Fixed
- Fixed "Permission Denied" memory error by removing unnecessary `PROT_EXEC` from the interpreter memory range.

## [0.13.9] - 2026-03-15

### Fixed
- Fixed "Failed to read section data" error by clearing C++ file stream state after reading headers.
- Implemented file-size-aware header reading to prevent EOF errors on files smaller than the 16KB system page size.

## [0.13.8] - 2026-03-15

### Fixed
- Fixed "0x00 byte" mapping issue by implementing recursive page alignment for `mprotect` in `MemoryManager`.
- Added strict error checking to `PEImage` loading sequence.
- Resolved permission conflicts on Apple Silicon (16KB pages) by using appropriate protection masks for overlapping code/data pages.
- Updated test generator to create a verifiable instruction sequence (`90 90 90 C3`).

## [0.13.7] - 2026-03-15

### Fixed
- Resolved `SIGABRT` crash when restarting the smoke test by correctly joining the engine thread.
- Added hex-byte logging for unknown opcodes to identify mapping issues.
- Fixed UI bridge race condition when starting/monitoring the engine.
- Cleaned up persistent `#pragma pack` warning.

## [0.13.6] - 2026-03-15

### Fixed
- Replaced `std::cout` with `printf` and manual `fflush(stdout)` across all C++ modules to ensure logs appear in Xcode's console.
- Added deeper logging to the `EngineBridge` and `EngineOrchestrator` start sequence.
- Verified and enforced Sandbox-disabled state for local development.

## [0.13.5] - 2026-03-15

### Fixed
- Disabled App Sandbox to allow ProWin to read the test executable from the Documents folder.
- Added detailed C++ logging to the PE Parser to track file access and magic number verification.

## [0.13.4] - 2026-03-15

### Fixed
- Fixed "LoaderError error 1" by using the absolute path to `test.exe` in `ContentView.swift`.
- Added file existence check to provide better error messages in the UI.
- Cleaned up unused `chrono` include in `EngineOrchestrator.cpp`.

## [0.13.3] - 2026-03-15

### Fixed
- Replaced `CADisplayLink` with `Timer` in `GameLoop` to fix "unavailable in macOS" build error.
- Corrected `MACOSX_DEPLOYMENT_TARGET` in project settings.

## [0.13.2] - 2026-03-15

### Fixed
- Resolved `ObservableObject` and `@Published` errors by adding `Combine` imports.
- Resolved `CADisplayLink` errors by adding `QuartzCore` import.
- Fixed protocol conformance issues in `GraphicsManager` and `InputManager`.

## [0.13.1] - 2026-03-15

### Fixed
- Resolved "Unable to find module dependency: 'XCTest'" build error by removing unused Test directory from the main app target.

## [0.13.0] - 2026-03-15

### Added
- `InstructionDispatcher` (C++) for executing decoded x64 instructions
- Implementation of `NOP` and `RET` execution logic
- High-performance Interpreter loop in `EngineOrchestrator`
- "Run Smoke Test" button in the SwiftUI UI
- End-to-end integration: Swift UI -> Objective-C++ Bridge -> C++ Interpreter
- Real-time logging of emulated instruction execution

## [0.12.0] - 2026-03-15

### Added
- `CPUContext` structure for tracking x64 General Purpose and XMM registers
- `InstructionDecoder` (C++) for translating x64 machine code to internal IR
- Initial decoding support for common x64 opcodes (NOP, PUSH, POP, CALL, RET, SYSCALL)
- `EngineOrchestrator` integration with `CPUContext` for maintaining execution state
- Foundation for the dynamic binary translation (DBT) engine

## [0.11.0] - 2026-03-15

### Added
- `GameLoop` (Swift) for high-level synchronization and frame updates
- `EngineOrchestrator` (C++) for background execution of Windows binary
- `EngineBridge` (Objective-C++) to control the engine thread from Swift
- Integrated `CADisplayLink` for smooth 60 FPS UI/Graphics sync
- Updated `ProWin-Bridging-Header.h` with all bridge components

## [0.10.0] - 2026-03-15

### Added
- `AudioManager` (Swift/AVFoundation) for native macOS audio playback
- `AudioBridge` (Objective-C++) for C++ to AVAudioEngine communication
- XAudio2 stubbing (`xaudio2_9.dll`) for modern Windows game audio
- DirectSound stubbing (`dsound.dll`) for legacy Windows game audio support
- Foundation for PCM buffer queuing in the audio pipeline

## [0.9.0] - 2026-03-15

### Added
- `InputManager` (Swift/GameController) for native controller support
- `InputBridge` (Objective-C++) for C++ to GameController communication
- XInput stubbing (`xinput1_4.dll`) for Windows game controller support
- Mapping of macOS controller buttons and sticks to XInput state

## [0.8.0] - 2026-03-15

### Added
- `GraphicsManager` (Swift/Metal) for native rendering foundation
- `MetalViewRepresentable` and `GameContentView` for displaying game output
- `GraphicsBridge` (Objective-C++) for C++ to Metal communication
- Initial DirectX (DXGI) and GDI stubs in `StubManager`
- Support for frame presentation logic entry points

## [0.7.0] - 2026-03-15

### Added
- Implementation of PE Base Relocation engine (ASLR support)
- Support for `IMAGE_REL_BASED_DIR64` relocation types
- Automatic coordinate patching when binary is loaded at a non-preferred base address
- Enhanced `PEParser` to extract Relocation Data Directory (DataDirectory[5])
- Pivoted engine development focus towards Windows Game Execution (DirectX/Metal focus)

## [0.5.0] - 2026-03-15

### Added
- `MemoryManager` class for handles virtual memory reservation and protection (mprotect/mmap)
- `PEImage` class for loading PE sections into mapped memory
- Implementation of memory-mapped execution foundation
- Support for copying section raw data and zero-filling BSS-like areas

## [0.4.0] - 2026-03-15

### Added
- Implementation of Windows PE section header parsing
- `PESectionWrapper` for Objective-C++ bridge
- Inclusion of section metadata in Swift `PEBinary` model
- Support for extracting section names, virtual addresses, and sizes

## [0.3.2] - 2026-03-15

### Changed
- Moved `Sources`, `Tests`, and bridging header into the `ProWin` directory to enable automatic Xcode synchronization
- Configured `SWIFT_OBJC_BRIDGING_HEADER` in project build settings

## [0.3.1] - 2026-03-15

### Changed
- Connected `PELoader` to `PEEngineWrapper` to support basic PE metadata loading in Swift

## [0.3.0] - 2026-03-15

### Added
- C++ `PEParser` for reading Windows PE headers
- Objective-C++ `PEEngineWrapper` for Swift interop
- ProWin bridging header
- Implementation of basic PE identification (MZ and PE signatures)

## [0.2.2] - 2026-03-15

### Fixed
- Resolved lint error in `PEStructures.h` regarding packing pragmas

## [0.2.1] - 2026-03-15

### Changed
- Added `.gitignore` and moved project documentation files out of git tracking

## [0.2.0] - 2026-03-15

### Added
- PELoader Swift interface stubs
- Project directory structure for Engine, UI, and Common components

## [0.1.0] - 2026-03-15

### Added
- Initial project scaffold
- Basic Xcode project for ProWin
- Directory structure: Sources, Tests, Resources, Docs, Scripts
