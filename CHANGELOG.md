# Changelog

All notable changes to **Codex of Power NG (CodexOfPowerNG)** are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

> Codex of Power NG is a new mod line and is not compatible with Codex of Power / SVCollection.

## [Unreleased]

## [1.0.6] - 2026-02-17

### Added
- Added LOTD/TCC registration gate option (`registration.requireTccDisplayed`) with fail-closed behavior when required TCC lists are unavailable.
- Added LOTD gate warning UI elements (status pill, toast, settings banner) and related regression coverage.
- Added host regression tests:
  - `tests/registration_formid_parse.test.cpp` (FormID parse/local-id boundary cases)
  - `tests/serialization_write_flow.test.cpp` (save-write flow should not short-circuit after earlier failures)

### Changed
- Internal C++ modules were decomposed for maintainability with no intended gameplay/UI behavior change:
  - Prisma UI payload builders split into `src/PrismaUIPayloadsInventory.cpp` and `src/PrismaUIPayloadsRewards.cpp`.
  - Registration internals split by concern (`src/RegistrationInternal.cpp`, `src/RegistrationInternalMaps.cpp`, `src/RegistrationInternalTcc.cpp`).
  - Serialization callbacks split by responsibility:
    - `src/Serialization.cpp` (install/entrypoint)
    - `src/SerializationSave.cpp` (save + revert)
    - `src/SerializationLoad.cpp` (load path)

## [1.0.5] - 2026-02-12

### Fixed
- Removed overly aggressive `IsQuestObject()` pre-check that blocked registration of items from large quest mods (e.g. Glenmoril). The actual `RemoveItem()` + count verification remains as the safety net.
- Fixed misleading "quest item" error message on excluded items; now correctly reports "excluded item".
- Removed unused `isQuestObject` field from `RemoveSelection`.
- Quick-register list now deduplicates items sharing the same registration key (quest-aliased copies no longer appear as separate rows).

## [1.0.4] - 2026-02-12

### Changed
- Mouse wheel scroll replaced with direct delta application for smoother feel across all tabs.
- Virtual table render throttled to ~30fps to reduce layout thrashing during rapid scroll.
- Virtual table overscan increased from 7 to 15 rows for fewer blank flashes.
- Quick-register pagination now sorts the full eligible list before slicing pages (stable page order).
- `QueueCloseRetry` spin-wait replaced with `std::promise`/`std::future` for cleaner thread synchronization.
- Duplicated `NowMs()` helper consolidated into shared `Util.h`.

### Fixed
- Co-save serialization no longer abandons remaining records (BLCK, NTFY, RWDS) when an earlier record write fails.
- Three detached background threads (settings save, close retry, focus delay) are now stored and joined on shutdown, preventing potential crashes during DLL unload.
- Settings "no changes" comparison now clamps values before comparing, avoiding spurious disk writes for out-of-range inputs.
- Quick-register `total` field now always reflects the true eligible item count (was 0 when `hasMore` was true).
- Normalized inconsistent indentation in `Registration.h`.

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

[Unreleased]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.6...HEAD
[1.0.6]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.5...v1.0.6
[1.0.5]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.4...v1.0.5
[1.0.4]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.3...v1.0.4
[1.0.2]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/servaltullius/CodexOfPowerNG/releases/tag/v1.0.0
