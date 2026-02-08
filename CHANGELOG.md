# Changelog

All notable changes to **Codex of Power NG (CodexOfPowerNG)** are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

> Codex of Power NG is a new mod line and is not compatible with Codex of Power / SVCollection.

## [Unreleased]

## [1.0.2] - 2026-02-08

### Changed
- README and Nexus description now include direct links to prerequisite mods for faster setup.
- README now explains the mod gameplay loop and scope in more detail (KR/EN).

### Fixed
- Quick Register now derives displayed item count from live inventory count (`GetItemCount`) instead of `countDelta` snapshots.
- Ammo items are now categorized under Weapons (instead of Misc) in discovery/quick-register grouping.
- Added regression test `tests/quick_register_rules.test.cjs` for ammo grouping and live count behavior.

## [1.0.1] - 2026-02-08

### Fixed
- Settings tab action controls are now always visible without scrolling via a sticky action bar.
- Added `Close` button next to `Reload`/`Save` inside Settings for faster exit while editing options.
- Added regression test `tests/settings_sticky_actions.test.cjs` to keep sticky Settings actions from regressing.

## [1.0.0] - 2026-02-08

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

[Unreleased]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.2...HEAD
[1.0.2]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/servaltullius/CodexOfPowerNG/releases/tag/v1.0.0
