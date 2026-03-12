# Build Catalog Competition And Tempo Review Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rebalance slot competition and unlock pacing for the current build catalog without expanding runtime scope or adding new effect batches.

**Architecture:** Treat baseline milestones and slotted rows as one progression system. Adjust timing and pressure in small batches, starting with the most overloaded themes and only then revisiting weaker themes that currently lose slot competition.

**Tech Stack:** C++ build catalog definitions, Prisma UI copy/rendering tests, Node regression tests, C++ catalog/runtime contract tests

---

## Chunk 1: Utility Pressure Rebalance

### Task 1: Rebalance utility baseline milestones

**Files:**
- Modify: `src/BuildOptionCatalog.cpp`
- Test: `tests/build_option_catalog_contract.test.cpp`
- Test: `tests/build_ui_rendering_module.test.cjs`

- [ ] **Step 1: Write/adjust failing tests for utility baseline expectations**
- [ ] **Step 2: Run targeted tests to confirm they fail**
- [ ] **Step 3: Adjust utility baseline so livelihood is not reinforced twice in the early curve**
- [ ] **Step 4: Run targeted tests to confirm they pass**
- [ ] **Step 5: Commit**

### Task 2: Rebalance livelihood early unlock pressure

**Files:**
- Modify: `src/BuildOptionCatalog.cpp`
- Modify: `PrismaUI/views/codexofpowerng/ui_i18n.js` if labels/descriptions need sequencing hints
- Test: `tests/build_option_catalog_contract.test.cpp`
- Test: `tests/build_ui_rendering_module.test.cjs`

- [ ] **Step 1: Write/adjust failing tests for livelihood unlock ordering**
- [ ] **Step 2: Run targeted tests to confirm they fail**
- [ ] **Step 3: Move or retune `cache`, `barter`, and `hauler` so utility does not default to livelihood by timing alone**
- [ ] **Step 4: Run targeted tests to confirm they pass**
- [ ] **Step 5: Commit**

## Chunk 2: Attack Early Curve Rebalance

### Task 3: Rebalance attack baseline milestones

**Files:**
- Modify: `src/BuildOptionCatalog.cpp`
- Test: `tests/build_option_catalog_contract.test.cpp`
- Test: `tests/build_effect_runtime.test.cpp`

- [ ] **Step 1: Write/adjust failing tests for attack baseline expectations**
- [ ] **Step 2: Run targeted tests to confirm they fail**
- [ ] **Step 3: Reduce early generic attack-damage pressure so baseline does not overpower theme identity**
- [ ] **Step 4: Run targeted tests to confirm they pass**
- [ ] **Step 5: Commit**

### Task 4: Rebalance ferocity/vitals relative to fury

**Files:**
- Modify: `src/BuildOptionCatalog.cpp`
- Test: `tests/build_option_catalog_contract.test.cpp`
- Test: `tests/build_effect_runtime.test.cpp`

- [ ] **Step 1: Write/adjust failing tests for attack theme unlock pacing**
- [ ] **Step 2: Run targeted tests to confirm they fail**
- [ ] **Step 3: Retune unlock scores so fury becomes a real early alternative instead of a secondary sustain branch**
- [ ] **Step 4: Run targeted tests to confirm they pass**
- [ ] **Step 5: Commit**

## Chunk 3: Exploration Ordering Fix

### Task 5: Put the exploration signpost first

**Files:**
- Modify: `src/BuildOptionCatalog.cpp`
- Test: `tests/build_option_catalog_contract.test.cpp`
- Test: `tests/build_ui_rendering_module.test.cjs`

- [ ] **Step 1: Write/adjust failing tests for exploration theme ordering**
- [ ] **Step 2: Run targeted tests to confirm they fail**
- [ ] **Step 3: Move the signpost row ahead of or alongside the first standard row**
- [ ] **Step 4: Run targeted tests to confirm they pass**
- [ ] **Step 5: Commit**

## Chunk 4: Defense Pressure Review

### Task 6: Reduce guard density without touching runtime scope

**Files:**
- Modify: `src/BuildOptionCatalog.cpp`
- Test: `tests/build_option_catalog_contract.test.cpp`
- Test: `tests/build_effect_runtime.test.cpp`

- [ ] **Step 1: Write/adjust failing tests for guard unlock spacing**
- [ ] **Step 2: Run targeted tests to confirm they fail**
- [ ] **Step 3: Spread guard rows so guard no longer dominates the 20-30 band**
- [ ] **Step 4: Run targeted tests to confirm they pass**
- [ ] **Step 5: Commit**

### Task 7: Reassess resistance after spacing changes

**Files:**
- Modify: `docs/contracts/build-legacy-expansion-inventory.json` if the review conclusion changes
- Modify: `docs/superpowers/specs/2026-03-13-build-catalog-competition-tempo-review-design.md` if new constraints are discovered

- [ ] **Step 1: Re-review resistance after utility/attack/guard changes**
- [ ] **Step 2: If resistance still loses badly, record whether the next pass must broaden rows instead of just moving scores**
- [ ] **Step 3: Commit docs-only conclusion if needed**

## Verification

- [ ] `node --test tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs`
- [ ] `node --test tests/*.test.cjs`
- [ ] `scripts/test.sh`
- [ ] `cmake --build build/wsl-debug --target CodexOfPowerNG_build_option_catalog_contract_test CodexOfPowerNG_build_effect_runtime_test`
- [ ] `ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_option_catalog_contract|build_effect_runtime)$'`

## Notes

- This plan intentionally does not add new rows.
- This plan intentionally does not add new runtime keys.
- If resistance only becomes healthy by changing row breadth, stop score-only work there and write a dedicated resistance redesign follow-up instead of forcing it into this batch.
