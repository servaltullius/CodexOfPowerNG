# Modpack Build Point Balance Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace score compression with a fixed build-point progression model that preserves record counts for motivation and uses weighted build points for unlocks, tiers, and active option magnitudes.

**Architecture:** Keep the current discipline record counts as the visible collection metric, add persisted build-point totals per discipline in centi-points, and route registration, undo, unlock checks, runtime scaling, payload generation, and Build UI through those point totals. Retire the temporary effective-score compression helper path once all callers have moved to the new linear point model.

**Tech Stack:** C++ gameplay progression/runtime/serialization/payload code, Prisma UI JS modules, Node tests, CMake host tests, native Skyrim/CommonLib regression tests

---

## File Map

- Modify: [include/CodexOfPowerNG/BuildTypes.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildTypes.h)
  - Rename or replace `unlockScore` with point-based unlock metadata that does not imply raw-count semantics.
- Modify: [include/CodexOfPowerNG/BuildOptionCatalog.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildOptionCatalog.h)
  - Add build-point helper declarations, point formatting helpers, and retire the temporary effective-score API.
- Modify: [include/CodexOfPowerNG/BuildStateStore.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildStateStore.h)
  - Add per-discipline build-point totals in centi-points and pure getter/setter helpers.
- Modify: [include/CodexOfPowerNG/SerializationStateStore.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/SerializationStateStore.h)
  - Persist build-point totals as part of the snapshot model.
- Modify: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
  - Replace compression helpers with fixed point thresholds and linear point-tier helpers.
- Modify: [src/BuildProgression.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildProgression.cpp)
  - Resolve form-type build-point gains, update register/undo accumulation, and add conservative migration fallbacks for unresolved historical registrations.
- Modify: [src/BuildStateStore.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildStateStore.cpp)
  - Store and mutate per-discipline build-point totals.
- Modify: [src/BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp)
  - Scale active option magnitudes from build-point tiers.
- Modify: [src/PrismaUIPayloadsBuild.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/PrismaUIPayloadsBuild.cpp)
  - Emit both record count and build-point progression metadata.
- Modify: [src/Serialization.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/Serialization.cpp)
  - Register snapshot fields and versioning for build-point totals.
- Modify: [src/SerializationLoad.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationLoad.cpp)
  - Restore or reconstruct build-point totals during load.
- Modify: [src/SerializationSave.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationSave.cpp)
  - Save build-point totals.
- Modify: [src/SerializationStateStore.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationStateStore.cpp)
  - Keep snapshot copy logic in sync.
- Modify if needed:
  - [PrismaUI/views/codexofpowerng/interop_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/interop_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_state_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_state_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js)
  - [PrismaUI/views/codexofpowerng/ui_state.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_state.js)
  - Normalize new payload fields without regressing old fallback handling.
- Modify: [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
  - Present record count and build points together.
- Modify: [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)
  - Update Build copy to point-based progression language.

- Test: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)
  - Lock in the linear point-tier helpers and unlock thresholds.
- Test: [tests/build_effect_runtime.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_effect_runtime.test.cpp)
  - Prove runtime totals use build-point tiers instead of compressed score helpers.
- Test: [tests/build_native_bridge_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_native_bridge_module.test.cjs)
  - Verify payloads expose record count and build-point fields.
- Test: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
  - Verify Build panel copy and point-based progress rendering.
- Test: [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)
  - Verify updated Build strings.
- Test: [tests/reward_orbit_layout.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/reward_orbit_layout.test.cjs)
  - Catch stale help text assumptions.
- Test: [tests/build_state_store_ops.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_state_store_ops.test.cpp)
  - Verify new point totals are stored and mutated safely.
- Test: [tests/build_migration_rules.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_migration_rules.test.cpp)
  - Verify migration rebuilds point totals from historical registrations.
- Test: [tests/serialization_state_store_ops.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/serialization_state_store_ops.test.cpp)
  - Verify snapshot state includes point totals.

## Chunk 1: Build Point Data Model

### Task 1: Add failing tests for build-point state and catalog semantics

**Files:**
- Modify: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)
- Modify: [tests/build_state_store_ops.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_state_store_ops.test.cpp)
- Modify if needed: [include/CodexOfPowerNG/BuildTypes.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildTypes.h)

- [ ] **Step 1: Add failing catalog assertions for linear build-point tier helpers**

Cover:
- `0.0 pt -> tier 0`
- `7.99 pt -> tier 0`
- `8.0 pt -> tier 1`
- `24.0 pt -> tier 3`

- [ ] **Step 2: Add failing catalog assertions for the approved unlock ladder**

Cover:
- old `5/10/15/20/25/30/35/40` rows now map to `4/8/12/16/20/24/28/32 pt`

- [ ] **Step 3: Add failing state-store assertions for persisted build-point totals**

Cover:
- Attack/Defense/Utility point totals can be set, read, incremented, and cleared independently of record counts

- [ ] **Step 4: Run targeted host-safe tests to verify failure**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_option_catalog_contract_test CodexOfPowerNG_build_state_store_ops_test
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_option_catalog_contract|build_state_store_ops)$'
```

Expected: FAIL on missing point-based APIs and state fields.

### Task 2: Implement point helpers and state fields

**Files:**
- Modify: [include/CodexOfPowerNG/BuildTypes.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildTypes.h)
- Modify: [include/CodexOfPowerNG/BuildOptionCatalog.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildOptionCatalog.h)
- Modify: [include/CodexOfPowerNG/BuildStateStore.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildStateStore.h)
- Modify: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
- Modify: [src/BuildStateStore.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildStateStore.cpp)

- [ ] **Step 1: Replace point-ambiguous names with explicit point terminology**

Use names such as:
- `unlockPointsCenti`
- `GetBuildPointsTier()`
- `GetNextBuildPointsThresholdCenti()`

- [ ] **Step 2: Add shared constants for fixed point progression**

Cover:
- `kBuildPointScale = 100`
- `kBuildPointsPerTierCenti = 800`

- [ ] **Step 3: Implement linear tier helpers and next-threshold helpers**

No compression logic should remain in the helper layer.

- [ ] **Step 4: Add per-discipline build-point storage to the state store**

Keep record counts unchanged.

- [ ] **Step 5: Re-run targeted host-safe tests**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_option_catalog_contract_test CodexOfPowerNG_build_state_store_ops_test
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_option_catalog_contract|build_state_store_ops)$'
```

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add include/CodexOfPowerNG/BuildTypes.h include/CodexOfPowerNG/BuildOptionCatalog.h include/CodexOfPowerNG/BuildStateStore.h src/BuildOptionCatalog.cpp src/BuildStateStore.cpp tests/build_option_catalog_contract.test.cpp tests/build_state_store_ops.test.cpp
git commit -m "feat: add fixed build point progression model"
```

## Chunk 2: Registration, Undo, And Migration

### Task 3: Add failing tests for weighted build-point gain and migration fallback

**Files:**
- Modify: [tests/build_migration_rules.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_migration_rules.test.cpp)
- Modify if needed: relevant progression tests that already cover discipline resolution
- Modify later: [src/BuildProgression.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildProgression.cpp)

- [ ] **Step 1: Add failing assertions for approved form-type weights**

Cover:
- `Weapon -> 0.80`
- `Ammo -> 0.35`
- `Armor -> 0.40`
- `AlchemyItem -> 0.25`
- `Ingredient -> 0.10`
- `Book -> 0.08`
- `Scroll -> 0.18`
- `SoulGem -> 0.30`
- `Misc -> 0.05`

- [ ] **Step 2: Add failing migration assertions for unresolved historical registrations**

Cover conservative fallback weights for:
- Attack
- Defense
- Utility

- [ ] **Step 3: Run the targeted native tests to verify failure**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_migration_rules_test
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_migration_rules)$'
```

Expected: FAIL on missing point totals and missing weight fallback logic.

### Task 4: Implement weighted registration and migration accumulation

**Files:**
- Modify: [src/BuildProgression.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildProgression.cpp)
- Modify if needed: [include/CodexOfPowerNG/SerializationStateStore.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/SerializationStateStore.h)

- [ ] **Step 1: Add a single helper that resolves fixed build-point gain from form type**

Keep discipline mapping and point mapping together so they cannot drift.

- [ ] **Step 2: Update registration and undo to mutate both record count and build points**

Every successful register/undo path must stay symmetric.

- [ ] **Step 3: Update migration replay to rebuild build points from historical registrations**

Use conservative discipline fallback weights only when the original form type cannot be resolved.

- [ ] **Step 4: Re-run targeted migration tests**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_migration_rules_test
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_migration_rules)$'
```

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/BuildProgression.cpp include/CodexOfPowerNG/SerializationStateStore.h tests/build_migration_rules.test.cpp
git commit -m "feat: weight build progression by item type"
```

## Chunk 3: Serialization And Runtime Scaling

### Task 5: Add failing tests for serialization and runtime point tiers

**Files:**
- Modify: [tests/serialization_state_store_ops.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/serialization_state_store_ops.test.cpp)
- Modify: [tests/build_effect_runtime.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_effect_runtime.test.cpp)
- Modify later:
  - [src/Serialization.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/Serialization.cpp)
  - [src/SerializationLoad.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationLoad.cpp)
  - [src/SerializationSave.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationSave.cpp)
  - [src/SerializationStateStore.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationStateStore.cpp)
  - [src/BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp)

- [ ] **Step 1: Add failing serialization assertions for point totals in snapshots**

Cover:
- snapshot copy
- save/load roundtrip
- clear/reset behavior

- [ ] **Step 2: Add failing runtime assertions for point-tier-based magnitudes**

Cover at least:
- one Attack row
- one Defense row
- one Utility row
- a high-record Utility case that now lands much lower because of low per-item weights rather than compression

- [ ] **Step 3: Run targeted tests to verify failure**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_effect_runtime_test CodexOfPowerNG_serialization_state_store_ops_test
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_effect_runtime|serialization_state_store_ops)$'
```

Expected: FAIL on missing point persistence and old raw/compression runtime semantics.

### Task 6: Implement point persistence and runtime scaling

**Files:**
- Modify: [src/Serialization.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/Serialization.cpp)
- Modify: [src/SerializationLoad.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationLoad.cpp)
- Modify: [src/SerializationSave.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationSave.cpp)
- Modify: [src/SerializationStateStore.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationStateStore.cpp)
- Modify: [src/BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp)

- [ ] **Step 1: Persist build-point totals in the snapshot versioned format**

Do not remove record counts.

- [ ] **Step 2: Make load reconstruct point totals when old saves do not contain the new fields**

Migration must be deterministic and safe.

- [ ] **Step 3: Route runtime scaling through build-point tiers only**

Delete the temporary effective-score compression call path once the new helpers are in place.

- [ ] **Step 4: Re-run targeted tests**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_effect_runtime_test CodexOfPowerNG_serialization_state_store_ops_test
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_effect_runtime|serialization_state_store_ops)$'
```

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/Serialization.cpp src/SerializationLoad.cpp src/SerializationSave.cpp src/SerializationStateStore.cpp src/BuildEffectRuntime.cpp tests/build_effect_runtime.test.cpp tests/serialization_state_store_ops.test.cpp
git commit -m "feat: persist and scale build progression from fixed points"
```

## Chunk 4: Payloads And Build UI

### Task 7: Add failing payload and UI tests for record-count plus build-point presentation

**Files:**
- Modify: [tests/build_native_bridge_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_native_bridge_module.test.cjs)
- Modify: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
- Modify: [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)
- Modify: [tests/reward_orbit_layout.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/reward_orbit_layout.test.cjs)
- Modify if needed: [tests/build_ui_state_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_state_module.test.cjs)

- [ ] **Step 1: Add failing payload expectations for `recordCount`, `buildPoints`, and point-based next-tier fields**

Cover:
- counts remain visible
- progress fields are point-based
- old compression helper names no longer appear in the native payload source

- [ ] **Step 2: Add failing rendering expectations for both metrics in the Build header/cards**

Cover:
- visible record count
- visible build points
- point-based next-upgrade text

- [ ] **Step 3: Update copy tests to remove any reference to hidden compression or `10 score` semantics**

- [ ] **Step 4: Run targeted JS tests to verify failure**

Run:
```bash
node --test tests/build_native_bridge_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs tests/reward_orbit_layout.test.cjs
```

Expected: FAIL on missing point fields and stale copy.

### Task 8: Implement payload and UI changes

**Files:**
- Modify: [src/PrismaUIPayloadsBuild.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/PrismaUIPayloadsBuild.cpp)
- Modify if needed:
  - [PrismaUI/views/codexofpowerng/interop_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/interop_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_state_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_state_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js)
  - [PrismaUI/views/codexofpowerng/ui_state.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_state.js)
- Modify: [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
- Modify: [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)

- [ ] **Step 1: Emit both record count and build-point progression metadata**

Payload rows should include explicit point unlock and next-tier values.

- [ ] **Step 2: Update state normalization to treat build points as the progression source of truth**

- [ ] **Step 3: Update Build panel rendering and copy**

Show both collection history and progression power without implying they are the same number.

- [ ] **Step 4: Re-run targeted JS tests**

Run:
```bash
node --test tests/build_native_bridge_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs tests/reward_orbit_layout.test.cjs
```

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/PrismaUIPayloadsBuild.cpp PrismaUI/views/codexofpowerng/interop_bridge.js PrismaUI/views/codexofpowerng/native_state_bridge.js PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js PrismaUI/views/codexofpowerng/ui_state.js PrismaUI/views/codexofpowerng/ui_build_panel.js PrismaUI/views/codexofpowerng/ui_i18n.js tests/build_native_bridge_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs tests/reward_orbit_layout.test.cjs tests/build_ui_state_module.test.cjs
git commit -m "feat: expose build points in payloads and build ui"
```

## Chunk 5: Final Verification

### Task 9: Run regression gates and package smoke checks

**Files:**
- No new source files

- [ ] **Step 1: Run the host-safe regression gate**

Run:
```bash
scripts/test.sh
```

Expected: PASS

- [ ] **Step 2: Run focused native tests for build progression**

Run:
```bash
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_effect_runtime|build_migration_rules|build_option_catalog_contract|build_state_store_ops|serialization_state_store_ops)$'
```

Expected: PASS

- [ ] **Step 3: Rebuild and install the release package**

Run:
```bash
export VCPKG_ROOT=/mnt/c/Users/kdw73/vcpkg
cmake --preset wsl-release
cmake --build --preset wsl-release
cmake --install build/wsl-release
```

Expected: install updates `dist/CodexOfPowerNG`

- [ ] **Step 4: Commit**

```bash
git add .
git commit -m "chore: finalize fixed build point balance pass"
```

Plan complete and saved to `docs/superpowers/plans/2026-03-14-modpack-build-balance.md`. Ready to execute?
