# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
