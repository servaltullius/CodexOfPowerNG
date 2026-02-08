# Changelog

All notable changes to **Codex of Power NG (CodexOfPowerNG)** are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

> Codex of Power NG is a new mod line and is not compatible with Codex of Power / SVCollection.

## [Unreleased]

## [0.1.0] - 2026-02-08

### Added
- Prisma UI-based in-game menu at `PrismaUI/views/codexofpowerng/index.html`.
- English/Korean UI text support with language override option.
- Runtime warning and on-screen notification when legacy `SVCollection` MCM residue is detected.

### Changed
- Inventory listing pipeline updated to pagination + virtualized table rendering for smoother cursor/hover behavior.
- High DPI support improved with input hitbox scaling (`Input scale`) and safer fallback controls (`[`, `]`, `0`, `ESC`, `F4`).
- UI presentation improved with auto scaling for high resolutions and manual UI scale control.

### Removed
- Legacy runtime assets removed from repository/runtime:
  - `MCM/Config/SVCollection/*`
  - `SKSE/Plugins/SVCollection/*`
  - `Scripts/Source/*` legacy Papyrus/JContainers/UIExtensions stack

### Fixed
- Reduced frame hitches by queueing settings writes and localization reloads off the hot path.
- Added defensive handling for stale legacy MCM/keybind files that can interfere with NG startup/runtime behavior.

[Unreleased]: https://github.com/servaltullius/CodexOfPowerNG/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/servaltullius/CodexOfPowerNG/releases/tag/v0.1.0
