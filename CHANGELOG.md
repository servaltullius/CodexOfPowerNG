# Changelog

All notable changes to **Codex of Power NG (CodexOfPowerNG)** are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

> Codex of Power NG is a new mod line and is not compatible with Codex of Power / SVCollection.

## [Unreleased]

## [1.0.8-rc.3] - 2026-02-22 (Pre-release)

### Fixed
- Settings save (`settings.user.json`) is now atomic via `.tmp` + rename with `.bak` backup retention, and startup repair for stale `.tmp/.bak` artifacts was added.
- Prisma UI interop now guards JSON serialization (`payload.dump()`) failures to avoid `noexcept` termination.
- Queued SKSE tasks are now wrapped with exception guards to prevent unhandled exceptions from killing the game process.
- PreLoadGame now forces Prisma UI cleanup (FocusMenu/cursor/view) to reduce stuck focus/cursor/pause cases across load boundaries.
- Exclude patch scanning now uses non-throwing `std::filesystem::exists(..., error_code)` to avoid unexpected exceptions.

### Changed
- Logging now flushes on `warn` (instead of `info`) to reduce potential I/O stalls under verbose logs.
- Prisma UI API can be acquired lazily if it was unavailable during PostLoad.

### Notes
- Detailed release note: `docs/releases/v1.0.8-rc.3.md`
- Save compatibility policy remains unchanged: Codex of Power / SVCollection is not compatible.

## [1.0.8-rc.2] - 2026-02-21 (Pre-release)

### Fixed
- Reward sync no longer applies downward "cap-guard" corrections for capped actor values when the observed cap overflow is caused by external buffs/gear (prevents unintended debuffs on e.g. frost resist / move speed).

### Notes
- Detailed release note: `docs/releases/v1.0.8-rc.2.md`
- Save compatibility policy remains unchanged: Codex of Power / SVCollection is not compatible.

## [1.0.8-rc.1] - 2026-02-21 (Pre-release)

### Fixed
- Reward sync now supports early convergence exit, reducing unnecessary post-load pass execution.
- Registration undo now records reward deltas from actual applied grant deltas (instead of full before/after snapshot diff), improving rollback consistency.
- Undo flow now warns when a rollback target existed but no actor-value rollback delta could be applied.
- Save writer flow now aggregates writer success/failure and emits a top-level failure log when co-save write is incomplete.

### Changed
- Quick-register list now uses a short-lived cache with generation-based invalidation to reduce repeated full-scan/sort cost.
- Quick-register cache is invalidated on registration/undo/load/revert/settings updates to keep list correctness.
- Added/updated regression tests for reward sync policy, undo rollback behavior, save writer aggregation, and quick-register cache invalidation.

### Notes
- Detailed release note: `docs/releases/v1.0.8-rc.1.md`
- Save compatibility policy remains unchanged: Codex of Power / SVCollection is not compatible.

## [1.0.7-rc.14] - 2026-02-19 (Pre-release)

### Fixed
- Added a reward cap for `AttackDamageMult` to prevent runaway physical damage scaling from reward accumulation.
- Added a non-converging sync guard in reward resync passes to stop repeated re-application on AV channels that do not reflect expected convergence after `ModActorValue`.
- Reduced risk of extreme early-game damage spikes caused by repeated sync deltas on multiplicative damage channels.

### Notes
- This pre-release focuses on addressing reported “damage spike / inflated hit numbers” behavior.
- Save compatibility policy remains unchanged: Codex of Power / SVCollection is not compatible.

## [1.0.7-rc.13] - 2026-02-19 (Pre-release)

### Fixed
- Carry weight reward sync now applies immediately when a non-trivial carry delta is detected during load-time sync passes.
- Added post-load/new-game carry-weight quick resync scheduling to reduce timing-sensitive load desync cases.
- Added regression coverage for the load scenario where state contains carry bonus but actor snapshots still report base values.

### Notes
- This pre-release specifically targets the reported “carry weight bonus not reflected after loading save” symptom.
- Save compatibility policy is unchanged: Codex of Power / SVCollection is not compatible.

## [1.0.7-rc.12] - 2026-02-19 (Pre-release)

### Fixed
- Reward sync for capped ActorValues was hardened to avoid over-cap drift and over-correction after load/reload.
- Added immediate cap-guard correction path when current value is detected above hard cap during sync.
- Fixed settings payload/UI save handling so `rewardMultiplier = 0` is preserved (no unintended fallback to `1.0`).
- Improved Prisma UI close retry behavior to continue cleanup/retry when UI task enqueue fails.
- Resolved WSL cross-build include path issue by ensuring multiarch glibc headers are available in generated clang-cl flags.

### Changed
- Added conservative capped sync helper and expanded carry-weight/capped sync test coverage.
- Split C++ test target build from release builds via `COPNG_BUILD_CPP_TEST_TARGETS` preset control.
- Updated runtime docs to match actual quick-register pagination/counting behavior.
- Added stricter JSON parse diagnostics in settings/request payload handlers.

### Notes
- This pre-release focuses on sync/cap correctness and build/release reliability.
- Save compatibility policy remains unchanged: Codex of Power / SVCollection is not compatible.

## [1.0.7-rc.11] - 2026-02-19 (Pre-release)

### Changed
- Default reward roll cadence was adjusted from `every: 10` to `every: 5` for new/default settings paths.
- Settings UI fallback for `rewardEvery` now defaults to `5` (instead of `10`) when no valid value exists.
- Prisma UI API contract examples were updated to reflect the new default reward cadence.

### Notes
- Existing users keep their configured `settings.user.json` value if already set.
- This pre-release focuses on reward default tuning only; no new runtime dependency was added.

## [1.0.7-rc.10] - 2026-02-18 (Pre-release)

### Changed
- Reward sync stabilization was iterated through `v1.0.7-rc.1` ~ `v1.0.7-rc.10` with focus on carry-weight reflection timing, cap normalization safety, and sync watchdog behavior.
- The temporary one-time carry recovery UI flow introduced in `v1.0.7-rc.9` was removed in `v1.0.7-rc.10` to keep runtime behavior simpler and avoid misleading recovery semantics.

### Notes
- Detailed per-RC notes are tracked in `docs/releases/v1.0.7-rc.*.md`.
- Latest pre-release in this track: `v1.0.7-rc.10`.

## [1.0.6] - 2026-02-17

### Added
- Added LOTD/TCC registration gate option (`registration.requireTccDisplayed`) with fail-closed behavior when required TCC lists are unavailable.
- Added LOTD gate warning UI elements (status pill, toast, settings banner) and related regression coverage.
- Added Rewards tab character image + orbit visualization.
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

[Unreleased]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.8-rc.3...HEAD
[1.0.8-rc.3]: https://github.com/servaltullius/CodexOfPowerNG/releases/tag/v1.0.8-rc.3
[1.0.8-rc.2]: https://github.com/servaltullius/CodexOfPowerNG/releases/tag/v1.0.8-rc.2
[1.0.8-rc.1]: https://github.com/servaltullius/CodexOfPowerNG/releases/tag/v1.0.8-rc.1
[1.0.7-rc.14]: https://github.com/servaltullius/CodexOfPowerNG/releases/tag/v1.0.7-rc.14
[1.0.7-rc.13]: https://github.com/servaltullius/CodexOfPowerNG/releases/tag/v1.0.7-rc.13
[1.0.7-rc.12]: https://github.com/servaltullius/CodexOfPowerNG/releases/tag/v1.0.7-rc.12
[1.0.7-rc.11]: https://github.com/servaltullius/CodexOfPowerNG/releases/tag/v1.0.7-rc.11
[1.0.7-rc.10]: https://github.com/servaltullius/CodexOfPowerNG/releases/tag/v1.0.7-rc.10
[1.0.6]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.5...v1.0.6
[1.0.5]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.4...v1.0.5
[1.0.4]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.3...v1.0.4
[1.0.2]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/servaltullius/CodexOfPowerNG/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/servaltullius/CodexOfPowerNG/releases/tag/v1.0.0
