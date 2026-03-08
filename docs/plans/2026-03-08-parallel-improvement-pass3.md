# 2026-03-08 Parallel Improvement Pass 3

## Goal
- Isolate the remaining high-traffic shared-state seams in `Events` and `Serialization`.
- Replace direct `RuntimeState` access in these paths with dedicated store APIs so the next parallel pass can focus on domain stores instead of raw state mutation.

## Non-goals
- Full removal of `RuntimeState`
- New serialization schema/version changes
- Gameplay behavior changes in loot notification or load/save semantics
- Release packaging updates

## Affected files or modules
- Add: `include/CodexOfPowerNG/NotifiedStateStore.h`
- Add: `src/NotifiedStateStore.cpp`
- Add: `include/CodexOfPowerNG/SerializationStateStore.h`
- Add: `src/SerializationStateStore.cpp`
- Add: `tests/notified_state_store_module.test.cjs`
- Add: `tests/serialization_state_store_module.test.cjs`
- Add: `docs/plans/2026-03-08-parallel-improvement-pass3.md`
- Modify: `CMakeLists.txt`
- Modify: `src/Events.cpp`
- Modify: `src/SerializationSave.cpp`
- Modify: `src/SerializationLoad.cpp`
- Modify: `tests/serialization_load_atomicity.test.cjs`

## Constraints
- Preserve existing serialized payload contents and load ordering semantics
- Keep the staging-before-commit behavior on load
- Keep loot notification throttling and already-notified behavior unchanged
- Do not expand scope into packaging or runtime UI behavior

## Milestones
1. Add the pass-3 plan and freeze scope
2. Introduce notified/serialization state access modules
3. Migrate `Events` and `Serialization` off direct `RuntimeState` access
4. Update regression tests to pin the new boundaries
5. Run tests and release build again

## Validation steps
1. `node --test tests/notified_state_store_module.test.cjs tests/serialization_state_store_module.test.cjs tests/serialization_load_atomicity.test.cjs`
2. `bash scripts/test.sh`
3. `env VCPKG_ROOT=/mnt/c/Users/<user>/vcpkg cmake --build --preset wsl-release`

## Risks
- Source-structure tests are regex-based and will need coordinated updates when boundaries move
- `SerializationStateStore` is still transitional and may be replaced by per-domain load/save later
- Any mistake in state replacement order could affect load integrity, so staging semantics must stay explicit

## Rollback notes
- `NotifiedStateStore` can be reverted independently from `SerializationStateStore`
- If serialization regressions appear, restore direct assignments in `SerializationLoad.cpp` and direct snapshots in `SerializationSave.cpp`
