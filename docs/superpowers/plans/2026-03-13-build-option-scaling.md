# Build Option Scaling Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace baseline milestone growth with discipline-score-based scaling on slotted options so recorded progress directly strengthens equipped build choices.

**Architecture:** Keep the existing unlock-and-slot system, but remove baseline milestone bonuses as a player-facing growth path. Each slotted option will expose a base magnitude and per-tier gain, where `discipline score / 10` determines the current tier. Runtime application, payload generation, and Build UI all need to derive current values from that same formula so growth stays coherent.

**Tech Stack:** C++ build catalog/runtime/serialization, Prisma UI JS view layer, Node JS tests, CMake host/native tests

---

## File Map

- Modify: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
  - Remove baseline milestone usage and define per-option scaling metadata
- Modify if needed: [include/CodexOfPowerNG/BuildTypes.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildTypes.h)
  - Add scaling metadata to option definitions
- Modify: [src/BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp)
  - Apply scaled magnitudes instead of static option magnitudes + baseline deltas
- Modify: [src/BuildProgression.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildProgression.cpp)
  - Remove any player-facing dependence on baseline milestone counts if still used
- Modify: [src/PrismaUIPayloadsBuild.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/PrismaUIPayloadsBuild.cpp)
  - Emit current effect, current tier, next tier preview, and remaining score instead of baseline summaries
- Modify serialization/state only if needed:
  - [include/CodexOfPowerNG/State.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/State.h)
  - [src/SerializationSave.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationSave.cpp)
  - [src/SerializationLoad.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationLoad.cpp)
  - [include/CodexOfPowerNG/SerializationStateStore.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/SerializationStateStore.h)
  - Only to remove or neutralize obsolete baseline persistence
- Modify UI:
  - [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
  - [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)
  - [PrismaUI/views/codexofpowerng/interop_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/interop_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_state_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_state_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js)
  - [PrismaUI/views/codexofpowerng/ui_state.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_state.js)
- Modify tests:
  - [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)
  - [tests/build_effect_runtime.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_effect_runtime.test.cpp)
  - [tests/build_native_bridge_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_native_bridge_module.test.cjs)
  - [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
  - [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)
  - [tests/serialization_load_atomicity.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/serialization_load_atomicity.test.cjs) if baseline serialization removal changes load assumptions

## Chunk 1: Replace Baseline Metadata With Option Scaling

### Task 1: Add per-option scaling metadata and retire baseline milestones

**Files:**
- Modify: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
- Modify if needed: [include/CodexOfPowerNG/BuildTypes.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildTypes.h)
- Test: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)

- [ ] **Step 1: Write failing catalog tests for scaling metadata**

Add expectations that each slotted option exposes:
- base magnitude
- per-tier magnitude
- tier interval rule of 10 discipline score

Also add expectations that baseline milestones are no longer emitted as the primary growth model.

- [ ] **Step 2: Run targeted catalog tests to verify failure**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_option_catalog_contract_test
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_option_catalog_contract)$'
```

Expected: FAIL on missing scaling metadata or obsolete baseline assumptions.

- [ ] **Step 3: Add scaling metadata to the catalog**

Implement minimal schema changes so every option defines:
- base magnitude
- per-tier magnitude
- scaling discipline score interval (`10`)

Retire baseline milestone definitions from player-facing growth.

- [ ] **Step 4: Re-run targeted catalog tests**

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add include/CodexOfPowerNG/BuildTypes.h src/BuildOptionCatalog.cpp tests/build_option_catalog_contract.test.cpp
git commit -m "refactor: replace baseline milestones with option scaling metadata"
```

## Chunk 2: Runtime Recalculation Uses Scaled Option Values

### Task 2: Recompute active option effects from score-derived tiers

**Files:**
- Modify: [src/BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp)
- Modify: [src/BuildProgression.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildProgression.cpp) if it still exposes obsolete baseline concepts
- Test: [tests/build_effect_runtime.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_effect_runtime.test.cpp)

- [ ] **Step 1: Add failing runtime tests for tier scaling**

Cover:
- score 0/10/20/30 produces stepped option growth
- different disciplines scale only their own options
- baseline bonuses are not added separately anymore

- [ ] **Step 2: Run targeted runtime tests to verify failure**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_effect_runtime_test
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_effect_runtime)$'
```

Expected: FAIL on static-magnitude behavior.

- [ ] **Step 3: Implement scaled runtime application**

Compute:
- current tier = `disciplineScore / 10`
- final magnitude = `base + (tier * perTier)`

Apply only scaled option values. Do not apply separate baseline deltas.

- [ ] **Step 4: Re-run targeted runtime tests**

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/BuildEffectRuntime.cpp src/BuildProgression.cpp tests/build_effect_runtime.test.cpp
git commit -m "feat: scale active build options by discipline score"
```

## Chunk 3: Remove Obsolete Baseline Persistence

### Task 3: Clean up baseline-related persistence and migration behavior

**Files:**
- Modify as needed:
  - [include/CodexOfPowerNG/State.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/State.h)
  - [src/SerializationSave.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationSave.cpp)
  - [src/SerializationLoad.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationLoad.cpp)
  - [include/CodexOfPowerNG/SerializationStateStore.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/SerializationStateStore.h)
  - [src/SerializationStateStore.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/SerializationStateStore.cpp) if touched by the above
- Test: [tests/serialization_load_atomicity.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/serialization_load_atomicity.test.cjs) or matching runtime/load tests

- [ ] **Step 1: Add a failing test or assertion for legacy baseline carry-over**

Cover that load should recompute option values from score and active slots, not from persisted baseline totals.

- [ ] **Step 2: Run the targeted load/serialization test**

Expected: FAIL if old baseline data still affects outcome.

- [ ] **Step 3: Remove or neutralize baseline persistence**

Keep backward compatibility where necessary, but ensure old baseline fields no longer produce additive growth.

- [ ] **Step 4: Re-run targeted load tests**

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add include/CodexOfPowerNG/State.h include/CodexOfPowerNG/SerializationStateStore.h src/SerializationSave.cpp src/SerializationLoad.cpp src/SerializationStateStore.cpp tests/serialization_load_atomicity.test.cjs
git commit -m "refactor: remove persisted baseline build bonuses"
```

## Chunk 4: Build Payload And UI Shift To Current/Next Tier

### Task 4: Emit current scaled values and next-tier preview in build payloads

**Files:**
- Modify: [src/PrismaUIPayloadsBuild.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/PrismaUIPayloadsBuild.cpp)
- Modify bridge/state files if payload normalization changes:
  - [PrismaUI/views/codexofpowerng/interop_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/interop_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_state_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_state_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js)
  - [PrismaUI/views/codexofpowerng/ui_state.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_state.js)
- Test: [tests/build_native_bridge_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_native_bridge_module.test.cjs)

- [ ] **Step 1: Add failing payload tests**

Expect payload fields such as:
- currentMagnitude
- currentTier
- nextTierMagnitude
- scoreToNextTier

And confirm baseline summary fields are absent or deprecated.

- [ ] **Step 2: Run targeted JS payload tests**

Expected: FAIL on missing fields or old baseline payload shape.

- [ ] **Step 3: Implement payload changes**

Build payloads should expose score-driven option state directly instead of separate baseline effect summaries.

- [ ] **Step 4: Re-run targeted payload tests**

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/PrismaUIPayloadsBuild.cpp PrismaUI/views/codexofpowerng/interop_bridge.js PrismaUI/views/codexofpowerng/native_state_bridge.js PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js PrismaUI/views/codexofpowerng/ui_state.js tests/build_native_bridge_module.test.cjs
git commit -m "feat: expose scaled build option payloads"
```

### Task 5: Rework Build UI copy around current and next tier

**Files:**
- Modify: [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
- Modify: [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)
- Test:
  - [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
  - [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)

- [ ] **Step 1: Add failing UI tests for new growth copy**

Assert:
- top cards show score plus next-tier progress
- option rows show current scaled value
- detail panel shows current effect, current tier, next-tier effect, and remaining score
- baseline labels are removed

- [ ] **Step 2: Run targeted UI tests to verify failure**

Run:
```bash
node --test tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs
```

Expected: FAIL on obsolete baseline strings.

- [ ] **Step 3: Implement UI rendering and copy changes**

Keep option descriptions pure, but present scaled current value and next-tier preview in dedicated UI fields.

- [ ] **Step 4: Re-run targeted UI tests**

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add PrismaUI/views/codexofpowerng/ui_build_panel.js PrismaUI/views/codexofpowerng/ui_i18n.js tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs
git commit -m "feat: show build option scaling tiers in ui"
```

## Chunk 5: Full Verification

### Task 6: Run regression gates and rebuild installable assets

**Files:**
- Reuse updated files above

- [ ] **Step 1: Run focused JS tests**

```bash
node --test tests/build_native_bridge_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs
```

- [ ] **Step 2: Run the full JS suite**

```bash
node --test tests/*.test.cjs
```

- [ ] **Step 3: Run the fast host-safe gate**

```bash
scripts/test.sh
```

- [ ] **Step 4: Run focused native tests**

```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_option_catalog_contract_test CodexOfPowerNG_build_effect_runtime_test CodexOfPowerNG
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_option_catalog_contract|build_effect_runtime)$'
```

- [ ] **Step 5: Rebuild release assets**

```bash
cmake --build --preset wsl-release
cmake --install build/wsl-release
```

- [ ] **Step 6: Manual in-game checkpoints**

- Build top cards show score progression instead of baseline bonus summary
- Selecting an option shows current tier and next-tier preview
- Increasing a discipline score actually strengthens currently slotted options
- No leftover baseline growth is visible after loading an existing save

- [ ] **Step 7: Commit any final verification-only fixups**

```bash
git add .
git commit -m "test: finalize build option scaling migration"
```

## Notes

- This plan intentionally standardizes scaling interval to `10 discipline score` for all options.
- Option differentiation should come from `baseMagnitude` and `perTierMagnitude`, not from custom interval rules.
- If some effects still cannot expose intuitive numeric values, keep player-facing result-oriented copy while still computing scaling numerically under the hood.
