# Resistance Theme Redesign Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace narrow resistance rows with broader bundled resistance packages so the defense resistance theme becomes slot-competitive.

**Architecture:** Keep the catalog UI shape intact, but shrink the resistance theme from many narrow rows to a smaller set of broad rows. Introduce synthetic bundled runtime keys that still use `BuildEffectType::ActorValue` but fan out to multiple concrete actor values in `BuildEffectRuntime`.

**Tech Stack:** C++ build catalog definitions, build runtime mapping, Prisma UI i18n/rendering tests, Node regression tests, C++ contract/runtime tests

---

## Chunk 1: Catalog And Copy Redesign

### Task 1: Replace narrow resistance rows with bundled catalog rows

**Files:**
- Modify: `src/BuildOptionCatalog.cpp`
- Test: `tests/build_option_catalog_contract.test.cpp`

- [ ] **Step 1: Write/adjust failing contract expectations for the new resistance rows**
- [ ] **Step 2: Run the targeted contract test to confirm it fails**
- [ ] **Step 3: Replace `fireward/frostward/stormward/antidote/purity` with bundled rows**
- [ ] **Step 4: Run the targeted contract test to confirm it passes**
- [ ] **Step 5: Commit**

### Task 2: Update player-facing copy for the redesigned resistance theme

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/ui_i18n.js`
- Test: `tests/build_ui_rendering_module.test.cjs`

- [ ] **Step 1: Write/adjust failing UI expectations for the new resistance labels/descriptions**
- [ ] **Step 2: Run the targeted UI test to confirm it fails**
- [ ] **Step 3: Update English/Korean titles and descriptions for bundled rows**
- [ ] **Step 4: Run the targeted UI test to confirm it passes**
- [ ] **Step 5: Commit**

## Chunk 2: Bundled Runtime Support

### Task 3: Add composite resistance runtime mappings

**Files:**
- Modify: `src/BuildEffectRuntime.cpp`
- Modify: `include/CodexOfPowerNG/BuildEffectRuntime.h` if helper declarations are needed
- Test: `tests/build_effect_runtime.test.cpp`

- [ ] **Step 1: Write/adjust failing runtime tests for bundled resistance application**
- [ ] **Step 2: Run the targeted runtime test to confirm it fails**
- [ ] **Step 3: Add bundled runtime-key handling for magic/elemental/status resistance**
- [ ] **Step 4: Run the targeted runtime test to confirm it passes**
- [ ] **Step 5: Commit**

## Chunk 3: Migration And Inventory Follow-through

### Task 4: Record migration and inventory consequences

**Files:**
- Modify: `docs/contracts/build-legacy-expansion-inventory.json`
- Modify: `src/BuildProgression.cpp` or migration helpers if active-slot remapping is implemented there
- Test: `tests/build_legacy_theme_inventory.test.cjs`
- Test: `tests/build_effect_runtime.test.cpp` or an existing migration-focused test if remap coverage fits better there

- [ ] **Step 1: Write/adjust failing tests for inventory notes and any required active-slot remapping**
- [ ] **Step 2: Run targeted tests to confirm they fail**
- [ ] **Step 3: Add bundled-row inventory notes and active-slot remapping rules**
- [ ] **Step 4: Run targeted tests to confirm they pass**
- [ ] **Step 5: Commit**

## Verification

- [ ] `node --test tests/build_ui_rendering_module.test.cjs tests/build_legacy_theme_inventory.test.cjs`
- [ ] `node --test tests/*.test.cjs`
- [ ] `scripts/test.sh`
- [ ] `cmake --build build/wsl-debug --target CodexOfPowerNG_build_option_catalog_contract_test CodexOfPowerNG_build_effect_runtime_test`
- [ ] `ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_option_catalog_contract|build_effect_runtime)$'`
- [ ] `cmake --build build/wsl-debug --target CodexOfPowerNG`

## Notes

- This plan intentionally does not redesign the overall build UI layout.
- This plan intentionally keeps resistance as one theme; it broadens rows instead of creating more subthemes.
- If composite runtime keys prove too awkward, stop and write a focused follow-up design instead of forcing partial per-row hacks.
