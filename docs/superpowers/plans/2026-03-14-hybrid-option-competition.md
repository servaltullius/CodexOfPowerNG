# Hybrid Weak Option Competition Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a first-wave hybrid rebalance for weak threshold-based build options so they compete within their themes without becoming new universal meta picks.

**Architecture:** Keep the existing weighted build-point model and slot-compatibility rules, but convert a small set of weak single-stat options into two-part bundle effects. Implement the change through catalog effect keys, runtime bundle resolution, and explicit payload/UI formatting for composite current and next-tier text.

**Tech Stack:** C++ build catalog/runtime/payload code, Prisma UI JS modules, Node UI tests, WSL/native CTest suite

---

## File Map

- Modify: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
  - Retune `reserve`, `magicka`, `meditation`, and `hauler` to hybrid bundle effect keys and new starting magnitudes.
- Modify: [src/BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp)
  - Resolve new bundle keys into concrete actor-value deltas.
- Modify: [src/PrismaUIPayloadsBuild.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/PrismaUIPayloadsBuild.cpp)
  - Format player-facing current and next-tier text for the new bundle options.
- Modify: [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)
  - Add localized strings for the new bundle effect labels.
- Test: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)
  - Lock in new effect keys and bundle magnitudes.
- Test: [tests/build_effect_runtime.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_effect_runtime.test.cpp)
  - Verify bundle totals resolve to the expected actor values.
- Test: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
  - Verify Build panel text shows both halves of the hybrid effect.
- Test: [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)
  - Verify new localized strings exist in both languages.

## Chunk 1: Lock The Hybrid Contract

### Task 1: Add failing tests for the new bundle option contract

**Files:**
- Modify: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)
- Modify: [tests/build_effect_runtime.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_effect_runtime.test.cpp)
- Modify: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
- Modify: [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)

- [ ] **Step 1: Add failing catalog assertions for hybrid effect keys**

Cover:
- `build.attack.reserve -> reserve_bundle`
- `build.utility.magicka -> magicka_well_bundle`
- `build.utility.meditation -> meditation_bundle`
- `build.utility.hauler -> hauler_bundle`

- [ ] **Step 2: Add failing catalog assertions for first-pass bundle magnitudes**

Cover:
- `Reserve`: primary `8`, support `8`
- `Magicka`: primary `12`, support `4`
- `Meditation`: primary `8`, support `4`
- `Hauler`: primary `8`, support `8`

Note:
- if the bundle representation uses one magnitude channel plus encoded formatting metadata, assert the chosen representation exactly instead of approximating it in prose.

- [ ] **Step 3: Add failing runtime assertions for concrete actor-value totals**

Cover:
- `Reserve` adds both `Stamina` and `StaminaRate`
- `Magicka` adds both `Magicka` and `MagickaRate`
- `Meditation` adds both `MagickaRate` and `Magicka`
- `Hauler` adds both `Stamina` and `CarryWeight`

- [ ] **Step 4: Add failing UI assertions for hybrid text rendering**

Cover:
- selected option detail shows both current effect parts
- next-tier text shows both upgraded effect parts
- Korean and English strings both exist

- [ ] **Step 5: Run targeted tests to confirm failure**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_option_catalog_contract CodexOfPowerNG_build_effect_runtime
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_option_catalog_contract|build_effect_runtime)$'
node --test tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs
```

Expected: FAIL on missing bundle keys or missing UI formatting.

## Chunk 2: Implement The Bundle Pass

### Task 2: Add new hybrid bundle keys and runtime resolution

**Files:**
- Modify: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
- Modify: [src/BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp)

- [ ] **Step 1: Retune `build.attack.reserve` to a bundle identity**

Use:
- bundle key: `reserve_bundle`
- base output: `+8 Stamina`, `+8% Stamina Rate`
- per tier output: `+2 Stamina`, `+4% Stamina Rate`

- [ ] **Step 2: Retune `build.utility.magicka` to a pool-first bundle**

Use:
- bundle key: `magicka_well_bundle`
- base output: `+12 Magicka`, `+4% Magicka Rate`
- per tier output: `+4 Magicka`, `+2% Magicka Rate`

- [ ] **Step 3: Retune `build.utility.meditation` to a regen-first bundle**

Use:
- bundle key: `meditation_bundle`
- base output: `+8% Magicka Rate`, `+4 Magicka`
- per tier output: `+4% Magicka Rate`, `+2 Magicka`

- [ ] **Step 4: Retune `build.utility.hauler` to an expedition bundle**

Use:
- bundle key: `hauler_bundle`
- base output: `+8 Stamina`, `+8 Carry Weight`
- per tier output: `+2 Stamina`, `+2 Carry Weight`

- [ ] **Step 5: Extend runtime delta resolution for the four bundle keys**

Implementation rule:
- resolve each bundle key to a fixed list of concrete actor-value deltas
- keep all arithmetic deterministic and driven by the already scaled option magnitude

- [ ] **Step 6: Re-run targeted native tests**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_option_catalog_contract CodexOfPowerNG_build_effect_runtime
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_option_catalog_contract|build_effect_runtime)$'
```

Expected: PASS

- [ ] **Step 7: Commit**

```bash
git add src/BuildOptionCatalog.cpp src/BuildEffectRuntime.cpp tests/build_option_catalog_contract.test.cpp tests/build_effect_runtime.test.cpp
git commit -m "feat: add hybrid bundle tuning for weak build options"
```

## Chunk 3: Expose Hybrid Effects Cleanly In The Build UI

### Task 3: Add explicit payload and localization support for bundle text

**Files:**
- Modify: [src/PrismaUIPayloadsBuild.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/PrismaUIPayloadsBuild.cpp)
- Modify: [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)
- Modify: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
- Modify: [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)

- [ ] **Step 1: Add localized labels/templates for the four hybrid bundles**

Need both:
- current effect text
- next-tier effect text

Keep phrasing short enough for the right-side focus panel.

- [ ] **Step 2: Add payload formatting branches for the new bundle keys**

Cover:
- row preview text
- selected detail current effect text
- selected detail next-tier effect text

- [ ] **Step 3: Verify supported-first rendering still works with bundle rows**

Focus on:
- selected theme rows
- detail panel
- tier preview

- [ ] **Step 4: Run focused JS tests**

Run:
```bash
node --test tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs
```

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/PrismaUIPayloadsBuild.cpp PrismaUI/views/codexofpowerng/ui_i18n.js tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs
git commit -m "feat: describe hybrid build option bundles in ui"
```

## Chunk 4: Full Validation

### Task 4: Run the complete regression and release build

**Files:**
- No new product files
- Validation only

- [ ] **Step 1: Run the full JS suite**

Run:
```bash
node --test tests/*.test.cjs
```

Expected: PASS

- [ ] **Step 2: Run the full native WSL suite**

Run:
```bash
ctest --test-dir build/wsl-debug --output-on-failure
```

Expected: PASS

- [ ] **Step 3: Run the host-safe regression gate**

Run:
```bash
scripts/test.sh
```

Expected: PASS

- [ ] **Step 4: Build and install release artifacts**

Run:
```bash
cmake --build --preset wsl-release
cmake --install build/wsl-release
```

Expected: release DLL and Prisma UI assets updated in `dist/CodexOfPowerNG`

- [ ] **Step 5: Commit any final test-only adjustments**

```bash
git add .
git commit -m "test: finalize hybrid build option competition pass"
```
