# Modpack Build Balance Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make build progression modpack-safe by separating raw unlock score from compressed effective scaling score and retuning the most inflation-sensitive option rows.

**Architecture:** Keep raw registration score as the only persisted progression source of truth. Add a shared helper layer that derives per-discipline effective score and next-tier thresholds from raw score, then route runtime scaling, payload generation, and Build UI through that helper. Apply a focused catalog retune on the most modpack-sensitive rows after the new score model is in place.

**Tech Stack:** C++ build progression/runtime/payload code, Prisma UI JS modules, Node JS tests, CMake host/native tests

---

## File Map

- Modify: [include/CodexOfPowerNG/BuildOptionCatalog.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildOptionCatalog.h)
  - Add helper declarations for effective-score conversion and next effective-tier thresholds if these belong with current scaling helpers.
- Modify: [include/CodexOfPowerNG/BuildTypes.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildTypes.h)
  - Keep raw score as the saved source of truth; only add constants if the compression curves need shared definitions.
- Modify: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
  - Implement raw-to-effective score helpers and apply first-pass option magnitude retunes.
- Modify: [src/BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp)
  - Scale active option magnitudes from effective score instead of raw score.
- Modify: [src/PrismaUIPayloadsBuild.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/PrismaUIPayloadsBuild.cpp)
  - Emit raw score, effective tier, and actual raw score needed to reach the next effective tier.
- Modify if payload normalization needs to change:
  - [PrismaUI/views/codexofpowerng/interop_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/interop_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_state_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_state_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js)
  - [PrismaUI/views/codexofpowerng/ui_state.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_state.js)
  - Preserve field names if possible, but make sure the normalized model does not assume the old linear `nextTierScore` semantics.
- Modify: [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
  - Render the new next-tier semantics without implying a permanent raw-10 cadence.
- Modify: [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)
  - Update copy where the current text assumes uniform `10 score = 1 tier`.
- Test: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)
  - Lock in the effective-score curves and catalog retune numbers.
- Test: [tests/build_effect_runtime.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_effect_runtime.test.cpp)
  - Prove runtime totals use compressed scaling and revised row magnitudes.
- Test: [tests/build_native_bridge_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_native_bridge_module.test.cjs)
  - Verify payload shape for score/tier metadata.
- Test: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
  - Verify Build panel copy and next-tier progress rendering.
- Test: [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)
  - Verify updated help/copy strings for compressed progression semantics.

## Chunk 1: Effective Score Model

### Task 1: Add failing catalog tests for raw-to-effective score compression

**Files:**
- Modify: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)
- Modify if needed: [include/CodexOfPowerNG/BuildOptionCatalog.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildOptionCatalog.h)

- [ ] **Step 1: Add failing assertions for Attack/Defense effective-score conversion**

Cover:
- raw `0 -> 0`
- raw `30 -> 30`
- raw `35 -> 32`
- raw `60 -> 45`
- raw `120 -> 65`

- [ ] **Step 2: Add failing assertions for Utility effective-score conversion**

Cover:
- raw `0 -> 0`
- raw `20 -> 20`
- raw `35 -> 26`
- raw `60 -> 36`
- raw `120 -> 48`

- [ ] **Step 3: Add failing assertions for next effective-tier threshold helpers**

Cover:
- the next raw score needed for the next effective tier after a compressed breakpoint
- Utility and Attack/Defense do not share identical late-game thresholds

- [ ] **Step 4: Run targeted catalog tests to verify failure**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_option_catalog_contract_test
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_option_catalog_contract)$'
```

Expected: FAIL on missing effective-score helpers and missing threshold semantics.

## Chunk 2: Implement Effective Score Helpers And Catalog Retune

### Task 2: Implement the compressed score helpers and row retunes

**Files:**
- Modify: [include/CodexOfPowerNG/BuildOptionCatalog.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildOptionCatalog.h)
- Modify if needed: [include/CodexOfPowerNG/BuildTypes.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildTypes.h)
- Modify: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
- Test: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)

- [ ] **Step 1: Declare helper APIs for effective score and next-tier raw thresholds**

Expose functions for:
- `GetEffectiveBuildScore(discipline, rawScore)`
- `GetEffectiveBuildScalingTier(discipline, rawScore)`
- `GetNextRawScoreForEffectiveBuildTier(discipline, rawScore)`
- `GetRawScoreToNextEffectiveBuildTier(discipline, rawScore)`

- [ ] **Step 2: Implement Attack/Defense and Utility compression curves**

Use the approved piecewise rules from the design doc.

- [ ] **Step 3: Update catalog row magnitudes to the approved first-pass retune values**

Apply the exact design-approved revisions to:
- `ferocity`
- `precision`
- `guard`
- `bulwark`
- `cache`
- `barter`
- `smithing`
- `alchemy`
- `enchanting`
- `mobility`
- `sneak`
- `lockpicking`
- `pickpocket`
- `conjuration`
- `illusion`
- `echo`

- [ ] **Step 4: Re-run targeted catalog tests**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_option_catalog_contract_test
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_option_catalog_contract)$'
```

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add include/CodexOfPowerNG/BuildOptionCatalog.h include/CodexOfPowerNG/BuildTypes.h src/BuildOptionCatalog.cpp tests/build_option_catalog_contract.test.cpp
git commit -m "feat: add modpack-safe build score scaling"
```

## Chunk 3: Runtime Uses Effective Score

### Task 3: Write failing runtime tests for compressed scaling

**Files:**
- Modify: [tests/build_effect_runtime.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_effect_runtime.test.cpp)
- Modify later: [src/BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp)

- [ ] **Step 1: Add a failing Attack/Defense runtime case across a compression boundary**

Example:
- a raw score that used to give tier `6` should now produce a lower tier after compression

- [ ] **Step 2: Add a failing Utility runtime case across a stronger compression boundary**

Example:
- raw Utility `120` should no longer produce the old linear tier result

- [ ] **Step 3: Add failing assertions for revised row magnitudes**

Cover at least:
- `cache`
- `mobility`
- one crafting modifier row

- [ ] **Step 4: Run targeted runtime tests to verify failure**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_effect_runtime_test
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_effect_runtime)$'
```

Expected: FAIL on the old linear scaling path.

### Task 4: Implement runtime scaling from effective score

**Files:**
- Modify: [src/BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp)
- Test: [tests/build_effect_runtime.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_effect_runtime.test.cpp)

- [ ] **Step 1: Route scaled magnitude lookup through the effective-score helper**

Active option recomputation must use effective score instead of raw score.

- [ ] **Step 2: Keep unlock checks based on raw score**

Do not change existing unlock eligibility behavior.

- [ ] **Step 3: Re-run targeted runtime tests**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_effect_runtime_test
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_effect_runtime)$'
```

Expected: PASS

- [ ] **Step 4: Commit**

```bash
git add src/BuildEffectRuntime.cpp tests/build_effect_runtime.test.cpp
git commit -m "feat: compress build runtime scaling for modpacks"
```

## Chunk 4: Payloads And UI Semantics

### Task 5: Add failing payload tests for compressed next-tier metadata

**Files:**
- Modify: [tests/build_native_bridge_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_native_bridge_module.test.cjs)
- Modify later: [src/PrismaUIPayloadsBuild.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/PrismaUIPayloadsBuild.cpp)
- Modify if needed:
  - [PrismaUI/views/codexofpowerng/interop_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/interop_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_state_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_state_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js)

- [ ] **Step 1: Add failing expectations for payload fields that distinguish raw score from effective tier**

Cover:
- raw score still present
- current tier derived from effective score
- next tier score derived from the real next raw threshold
- score-to-next-tier reflects the compressed curve

- [ ] **Step 2: Run targeted payload tests to verify failure**

Run:
```bash
node --test tests/build_native_bridge_module.test.cjs
```

Expected: FAIL on old payload semantics.

### Task 6: Implement payload and UI changes

**Files:**
- Modify: [src/PrismaUIPayloadsBuild.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/PrismaUIPayloadsBuild.cpp)
- Modify if needed:
  - [PrismaUI/views/codexofpowerng/interop_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/interop_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_state_bridge.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_state_bridge.js)
  - [PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js)
  - [PrismaUI/views/codexofpowerng/ui_state.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_state.js)
- Modify: [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
- Modify: [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)
- Modify: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
- Modify: [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)
- Test: [tests/build_native_bridge_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_native_bridge_module.test.cjs)

- [ ] **Step 1: Emit the new raw score / effective tier metadata from native payloads**

- [ ] **Step 2: Update Build panel rendering so next-tier messaging reflects the compressed raw threshold**

- [ ] **Step 3: Replace any copy that still implies a universal `10 raw score = 1 tier forever` rule**

- [ ] **Step 4: Run targeted JS tests**

Run:
```bash
node --test tests/build_native_bridge_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs
```

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/PrismaUIPayloadsBuild.cpp PrismaUI/views/codexofpowerng/interop_bridge.js PrismaUI/views/codexofpowerng/native_state_bridge.js PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js PrismaUI/views/codexofpowerng/ui_state.js PrismaUI/views/codexofpowerng/ui_build_panel.js PrismaUI/views/codexofpowerng/ui_i18n.js tests/build_native_bridge_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs
git commit -m "feat: expose modpack-safe build tier progress"
```

## Chunk 5: Regression Verification

### Task 7: Run regression gates after the balance refactor

**Files:**
- No code changes required

- [ ] **Step 1: Run targeted native build tests**

```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_option_catalog_contract_test CodexOfPowerNG_build_effect_runtime_test
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_option_catalog_contract|build_effect_runtime)$'
```

- [ ] **Step 2: Run targeted JS tests**

```bash
node --test tests/build_native_bridge_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs
```

- [ ] **Step 3: Run the repo fast regression gate**

```bash
scripts/test.sh
```

- [ ] **Step 4: Commit any final doc/test adjustments if needed**

```bash
git add -A
git commit -m "test: verify modpack-safe build balance changes"
```

## Notes

- This plan intentionally keeps raw score as the only saved progression value.
- This plan intentionally does not change discipline mapping yet.
- If Utility still dominates after this pass, create a second spec for category-weighted scoring instead of mixing that redesign into this implementation.
