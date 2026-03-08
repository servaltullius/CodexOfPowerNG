# 2026-03-08 Parallel Improvement Pass 4

## Goal
- Extract the Quick Register candidate-building path into a dedicated module so cache management and eligibility logic can evolve independently.
- Move PrismaUI native state callback handling out of `index.html` into a dedicated JS module to reduce inline bootstrap coupling.

## Non-goals
- Gameplay rule changes in registration, rewards, or undo behavior
- New UI features or visual redesigns
- Serialization schema changes
- Full removal of the remaining inline rendering logic in `index.html`

## Affected files or modules
- Add: `src/RegistrationQuickListBuilder.cpp`
- Add: `src/RegistrationQuickListBuilder.h`
- Add: `PrismaUI/views/codexofpowerng/native_state_bridge.js`
- Add: `tests/quick_register_builder_module.test.cjs`
- Add: `tests/native_state_bridge_module.test.cjs`
- Add: `docs/plans/2026-03-08-parallel-improvement-pass4.md`
- Modify: `CMakeLists.txt`
- Modify: `src/RegistrationQuickRegister.cpp`
- Modify: `PrismaUI/views/codexofpowerng/index.html`
- Modify: `tests/quick_register_rules.test.cjs`
- Modify: `tests/virtual_refresh_resync.test.cjs`
- Modify: `tests/interop_bridge_module.test.cjs`

## Constraints
- Preserve existing quick-register filtering, quest protection, TCC gating, and stable sort order
- Keep the short-lived quick-register cache and generation invalidation behavior unchanged
- Preserve the native callback payload shapes and existing render/update ordering in the UI
- Avoid expanding scope into release packaging or runtime asset changes

## Milestones
1. Freeze pass-4 scope in a plan document
2. Extract the Quick Register eligibility scan into a builder module
3. Extract native state handlers into `native_state_bridge.js`
4. Update structure tests to pin the new module boundaries
5. Run targeted tests, the full test script, and the release build

## Validation steps
1. `node --test tests/quick_register_builder_module.test.cjs tests/native_state_bridge_module.test.cjs tests/quick_register_rules.test.cjs tests/virtual_refresh_resync.test.cjs tests/interop_bridge_module.test.cjs`
2. `bash scripts/test.sh`
3. `env VCPKG_ROOT=/mnt/c/Users/kdw73/vcpkg cmake --build --preset wsl-release`

## Risks
- Regex-based source tests need coordinated updates when logic moves across files
- Quick Register behavior is sensitive to ordering and exclusion semantics, so helper extraction must stay behavior-preserving
- The inline UI script still owns a large amount of rendering logic, so this pass focuses only on native callback decoupling

## Rollback notes
- `RegistrationQuickListBuilder` can be reverted independently by moving the scan loop back into `RegistrationQuickRegister.cpp`
- `native_state_bridge.js` can be rolled back by restoring the inline native callback functions in `index.html`
