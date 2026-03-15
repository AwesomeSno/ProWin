# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
