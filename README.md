# ProWin

**ProWin** is an open-source, native macOS application that loads and emulates Windows x64 binaries (PE32+) directly on macOS. It provides a dynamic binary translation (DBT) engine written in C++, a Swift/SwiftUI front-end, and an Objective-C++ bridge layer that connects high-performance native macOS APIs — Metal for graphics, AVFoundation for audio, and GameController for input — to their Windows equivalents (DirectX, XAudio2, XInput).

> **Current version:** v0.15.0 — Active development. x64 instruction support and DirectX pipeline are work-in-progress.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Getting Started](#getting-started)
- [Running the Smoke Test](#running-the-smoke-test)
- [Supported x64 Instructions](#supported-x64-instructions)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [Changelog](#changelog)
- [License](#license)

---

## Overview

ProWin bridges the gap between the Windows binary ecosystem and macOS by:

1. **Parsing** Windows PE32/PE32+ (`.exe`) files and mapping their sections into virtual memory using `mmap`/`mprotect`.
2. **Emulating** the x64 CPU with a software fetch–decode–execute loop.
3. **Stubbing** Windows DLL imports (DirectX, XInput, XAudio2) so that Windows games and applications do not hard-crash on missing symbols.
4. **Translating** graphics, audio, and input calls to native macOS equivalents in real time.

The long-term goal is to run Windows DirectX games on Apple Silicon and Intel Macs without a virtual machine.

---

## Features

| Feature | Status |
|---|---|
| PE32/PE32+ binary parsing | ✅ Complete |
| Section loading & memory mapping | ✅ Complete |
| PE Base Relocation (ASLR) | ✅ Complete |
| x64 CPU context (all GPRs, XMM) | ✅ Complete |
| Instruction decoding (MOV, ADD, NOP, RET) | ✅ Complete |
| Fetch–Decode–Execute interpreter loop | ✅ Complete |
| Live register state view in UI | ✅ Complete |
| DirectX / DXGI / GDI stubs | 🔄 In Progress |
| XInput controller stub | 🔄 In Progress |
| XAudio2 / DirectSound stubs | 🔄 In Progress |
| Metal graphics backend | 🔄 In Progress |
| AVFoundation audio backend | 🔄 In Progress |
| Full x64 ISA coverage | 🔄 In Progress |
| Import Address Table (IAT) resolution | 🔄 In Progress |
| Exception / interrupt handling | 🔄 In Progress |

---

## Architecture

ProWin is organized into three layers that communicate via an Objective-C++ bridge:

```
┌─────────────────────────────────────────────┐
│               SwiftUI Layer                 │
│  ContentView · GameLoop · PELoader          │
│  GraphicsManager · InputManager · AudioManager │
└───────────────────┬─────────────────────────┘
                    │  Objective-C++ Bridge
┌───────────────────▼─────────────────────────┐
│             Bridge Layer (Obj-C++)           │
│  EngineBridge · PEEngineWrapper             │
│  GraphicsBridge · InputBridge · AudioBridge │
└───────────────────┬─────────────────────────┘
                    │  C++ API calls
┌───────────────────▼─────────────────────────┐
│              C++ Engine Core                │
│  EngineOrchestrator (fetch–decode–execute)  │
│  InstructionDecoder · InstructionDispatcher │
│  CPUContext · MemoryManager                 │
│  PEParser · PEImage · StubManager           │
└─────────────────────────────────────────────┘
```

### Key Components

| Component | Language | Description |
|---|---|---|
| `EngineOrchestrator` | C++ | Main execution loop; ties together all engine components |
| `InstructionDecoder` | C++ | Translates raw x64 bytes into an internal IR |
| `InstructionDispatcher` | C++ | Executes the decoded IR and updates `CPUContext` |
| `CPUContext` | C++ | Holds all x64 register state (RAX–R15, RIP, RFLAGS, XMM0–15) |
| `MemoryManager` | C++ | Reserves and maps virtual address space using `mmap`/`mprotect` |
| `PEParser` | C++ | Reads PE DOS/NT headers, section table, data directories |
| `PEImage` | C++ | Loads PE sections into mapped memory, applies relocations |
| `StubManager` | C++ | Registry of Windows DLL function stubs |
| `EngineBridge` | Obj-C++ | Starts/stops the engine thread; exposes register reads to Swift |
| `PEEngineWrapper` | Obj-C++ | Loads PE binaries and returns metadata to Swift |
| `GraphicsBridge` | Obj-C++ | Connects the C++ renderer to the Metal command queue |
| `InputBridge` | Obj-C++ | Maps `GCController` state to XInput structures |
| `AudioBridge` | Obj-C++ | Queues PCM audio buffers via `AVAudioEngine` |
| `GameLoop` | Swift | 60 FPS timer loop; syncs engine state to SwiftUI |
| `PELoader` | Swift | High-level API for loading and executing PE binaries |
| `GraphicsManager` | Swift | Metal device and command queue setup |
| `InputManager` | Swift | GameController session and button mapping |
| `AudioManager` | Swift | `AVAudioEngine` graph and format configuration |

---

## Project Structure

```
ProWin/
├── ProWin/                         # Application source
│   ├── ProWinApp.swift             # SwiftUI @main entry point
│   ├── ContentView.swift           # Main UI (smoke test button, register view)
│   ├── ProWin-Bridging-Header.h    # Swift → Obj-C++ bridge declarations
│   ├── Assets.xcassets/            # App icons and colour assets
│   └── Sources/
│       ├── Engine/                 # C++ emulation engine
│       │   ├── CPUContext.h
│       │   ├── InstructionDecoder.h / .cpp
│       │   ├── InstructionDispatcher.h / .cpp
│       │   ├── EngineOrchestrator.h / .cpp
│       │   ├── MemoryManager.h / .cpp
│       │   ├── PEParser.h / .cpp
│       │   ├── PEStructures.h
│       │   ├── PEImage.h / .cpp
│       │   └── StubManager.h / .cpp
│       └── UI/                     # Swift + Obj-C++ UI & bridge layer
│           ├── EngineBridge.h / .mm
│           ├── PEEngineWrapper.h / .mm
│           ├── GraphicsBridge.h / .mm
│           ├── InputBridge.h / .mm
│           ├── AudioBridge.h / .mm
│           ├── GameLoop.swift
│           ├── PELoader.swift
│           ├── GraphicsManager.swift
│           ├── InputManager.swift
│           ├── AudioManager.swift
│           └── GameContentView.swift
├── ProWin.xcodeproj/               # Xcode project
├── Scripts/
│   └── generate_test_pe.py         # Generates a minimal x64 PE for smoke testing
├── test.exe                        # Pre-built minimal test binary
├── test.asm                        # Assembly source for test binary
└── CHANGELOG.md                    # Full version history
```

---

## Requirements

| Requirement | Minimum Version |
|---|---|
| macOS | 13.0 Ventura |
| Xcode | 15.0 |
| Swift | 5.9 |
| Apple Silicon (arm64) or Intel (x86_64) | — |

No external package managers or third-party libraries are required. ProWin uses only Apple system frameworks (SwiftUI, Metal, MetalKit, AVFoundation, GameController) and the standard C++ library.

---

## Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/AwesomeSno/ProWin.git
cd ProWin
```

### 2. Open in Xcode

```bash
open ProWin.xcodeproj
```

### 3. Select a scheme and destination

- Scheme: **ProWin**
- Destination: **My Mac**

### 4. Build and run

Press **⌘R** or choose **Product → Run**.

> **Note:** The App Sandbox is disabled so that ProWin can read binaries from your Documents folder during development. Do not re-enable the sandbox without also adding the appropriate entitlements.

---

## Running the Smoke Test

The quickest way to verify your build is to use the built-in smoke test:

1. Generate (or use the pre-built) test binary:

   ```bash
   python3 Scripts/generate_test_pe.py
   # Writes a minimal PE to the path hard-coded in ContentView.swift
   ```

2. Launch the app and press **"Run Smoke Test"** in the main window.

3. The UI will display the current execution status and a live **Register State** panel showing the values of RAX, RCX, and RIP after each instruction.

The test binary executes:

```asm
mov rax, 0x0000000100000000   ; load a 64-bit immediate into RAX
mov rcx, 0x00000005           ; load a 32-bit immediate into RCX
add rax, rcx                  ; add RCX to RAX
ret                           ; return / halt emulation
```

A successful run will show RAX = `0x0000000100000005` in the register view.

---

## Supported x64 Instructions

| Opcode | Mnemonic | Notes |
|---|---|---|
| `0x90` | `NOP` | No operation |
| `0xC3` | `RET` | Halts the emulation loop |
| `0x48 0xB8` | `MOV RAX, imm64` | REX.W + opcode-reg encoding |
| `0x48 0xB9` | `MOV RCX, imm64` | REX.W + opcode-reg encoding |
| `0x48 0x01` | `ADD r/m64, r64` | ModRM reg-to-reg |
| `0x0F 0x05` | `SYSCALL` | Stubbed (no-op) |

Additional instructions (PUSH, POP, CALL, JMP, conditional branches, etc.) are decoded but not yet fully executed.

---

## Roadmap

- [ ] Full x64 general-purpose instruction set (arithmetic, bitwise, shifts, branches)
- [ ] Import Address Table (IAT) resolution and DLL loading
- [ ] DirectX 11/12 → Metal graphics translation layer
- [ ] XAudio2 → AVAudioEngine audio translation
- [ ] XInput → GameController full mapping
- [ ] Exception and structured-exception-handling (SEH) support
- [ ] Thread creation and synchronisation primitives
- [ ] Windows heap and CRT stubs
- [ ] Performance: JIT compilation backend

---

## Contributing

Contributions are welcome! To get started:

1. Fork the repository and create a feature branch.
2. Make your changes, following the existing three-layer architecture (Swift UI → Obj-C++ Bridge → C++ Engine).
3. Add or update the relevant section in `CHANGELOG.md`.
4. Open a pull request with a clear description of the change.

Please keep pull requests focused: one feature or fix per PR makes reviews easier.

---

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for the full version history.

---

## License

This project does not currently include a license file. All rights are reserved by the author unless stated otherwise.
