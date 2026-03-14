# Defense Balance Pass Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Lower overperforming defense scaling in the fixed build-point model and expose per-item build-point weight in the quick register UI.

**Architecture:** Keep the weighted build-point system, but retune the defense lane by lowering armor-derived build points, reducing the slope of the defense sustain/resistance options, and surfacing row-level point gain through the inventory payload and quick-register renderer. Preserve raw record counts for motivation while making the point math explicit.

**Tech Stack:** C++20, SKSE plugin runtime, Prisma UI JavaScript, native/Node contract tests

---

### Task 1: Lock failing regression tests

**Files:**
- Modify: `tests/build_migration_rules.test.cpp`
- Modify: `tests/build_effect_runtime.test.cpp`
- Modify: `tests/build_option_catalog_contract.test.cpp`
- Modify: `tests/build_batch_register_flow.test.cjs`
- Modify: `tests/inventory_payload_contract.test.cjs`

- [ ] **Step 1: Write failing tests for lower defense point gain**
- [ ] **Step 2: Run targeted native tests and confirm expected failures**
- [ ] **Step 3: Write failing tests for quick-register point visibility**
- [ ] **Step 4: Run targeted Node tests and confirm expected failures**

### Task 2: Retune defense progression and option values

**Files:**
- Modify: `src/BuildProgression.cpp`
- Modify: `src/BuildOptionCatalog.cpp`

- [ ] **Step 1: Lower armor and defense fallback build-point contribution**
- [ ] **Step 2: Reduce defense sustain/resistance option base and per-tier magnitudes**
- [ ] **Step 3: Re-run targeted native tests until green**

### Task 3: Surface build-point gain in quick register UI

**Files:**
- Modify: `include/CodexOfPowerNG/Registration.h`
- Modify: `src/RegistrationQuickListBuilder.cpp`
- Modify: `src/PrismaUIPayloadsInventory.cpp`
- Modify: `PrismaUI/views/codexofpowerng/ui_register_batch_panel.js`
- Modify: `PrismaUI/views/codexofpowerng/ui_i18n.js`

- [ ] **Step 1: Carry per-row build-point gain through the quick-list payload**
- [ ] **Step 2: Render row and summary point gain in the quick-register panel**
- [ ] **Step 3: Re-run targeted Node tests until green**

### Task 4: Verify end-to-end

**Files:**
- Modify: none

- [ ] **Step 1: Run focused native + Node regression commands**
- [ ] **Step 2: Run full `node --test tests/*.test.cjs`**
- [ ] **Step 3: Run full `ctest --test-dir build/wsl-debug --output-on-failure`**
- [ ] **Step 4: Run `scripts/test.sh`**
