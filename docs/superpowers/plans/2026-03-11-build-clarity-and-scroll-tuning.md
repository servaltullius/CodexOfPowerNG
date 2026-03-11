# Scroll Profiles and Build Performance Tuning Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give Quick Register and Build different immediate wheel-scroll feels, and make the Build screen scroll more smoothly by trimming its heaviest visual effects.

**Architecture:** Keep one wheel handler, but choose a scroll profile from the active section instead of using one global tuning rule. Pair that with a conservative Build-screen CSS pass that reduces blur, glow, and layered background cost without changing the current shrine layout.

**Tech Stack:** Prisma UI embedded HTML/CSS, `input_correction.js`, Node-based UI regression tests.

---

## File Structure

- Modify: `PrismaUI/views/codexofpowerng/input_correction.js`
  Responsibility: choose per-screen wheel profiles and keep wheel behavior immediate.
- Modify: `PrismaUI/views/codexofpowerng/index.html`
  Responsibility: conservatively lighten Build-screen blur, shadow, and background layering.
- Test: `tests/input_correction_module.test.cjs`
  Responsibility: lock Quick/Register and Build to different immediate wheel profiles.
- Test: `tests/build_ui_rendering_module.test.cjs`
  Responsibility: lock the lighter Build-screen visual contract.
- Verify: `tests/ui_rendering_module.test.cjs`
  Responsibility: protect existing rendering ownership and layout assumptions.

## Chunk 1: Tab-Specific Wheel Profiles

### Task 1: Lock per-screen immediate wheel behavior

**Files:**
- Modify: `tests/input_correction_module.test.cjs`
- Modify: `PrismaUI/views/codexofpowerng/input_correction.js`

- [ ] **Step 1: Write a failing test for Quick Register using the larger immediate wheel step**
- [ ] **Step 2: Run `node --test tests/input_correction_module.test.cjs` and confirm RED**
- [ ] **Step 3: Add active-section profile selection in `installDirectWheelScroll()`**
- [ ] **Step 4: Implement immediate `Quick Register` normalization with a larger useful notch step**
- [ ] **Step 5: Re-run `node --test tests/input_correction_module.test.cjs` and confirm GREEN**

### Task 2: Lock Build to a smaller immediate wheel step

**Files:**
- Modify: `tests/input_correction_module.test.cjs`
- Modify: `PrismaUI/views/codexofpowerng/input_correction.js`

- [ ] **Step 1: Write a failing test for Build using a smaller immediate wheel step than Quick Register**
- [ ] **Step 2: Run `node --test tests/input_correction_module.test.cjs` and confirm RED**
- [ ] **Step 3: Implement the Build profile without RAF easing**
- [ ] **Step 4: Re-run `node --test tests/input_correction_module.test.cjs` and confirm GREEN**

## Chunk 2: Conservative Build-Screen Lightening

### Task 3: Lock the lighter Build visual contract

**Files:**
- Modify: `tests/build_ui_rendering_module.test.cjs`
- Modify: `PrismaUI/views/codexofpowerng/index.html`

- [ ] **Step 1: Write a failing test that captures reduced Build-screen blur / shadow / layer intensity**
- [ ] **Step 2: Run `node --test tests/build_ui_rendering_module.test.cjs` and confirm RED**
- [ ] **Step 3: Reduce the most expensive Build-screen effects while preserving the shrine composition**
- [ ] **Step 4: Keep the lower altar silhouette placement and current three-panel Build layout unchanged**
- [ ] **Step 5: Re-run `node --test tests/build_ui_rendering_module.test.cjs` and confirm GREEN**

### Task 4: Verify Build lightening does not break shared rendering behavior

**Files:**
- Modify: none unless verification reveals a regression
- Test: `tests/ui_rendering_module.test.cjs`

- [ ] **Step 1: Run `node --test tests/ui_rendering_module.test.cjs tests/build_ui_rendering_module.test.cjs`**
- [ ] **Step 2: If a shared renderer contract breaks, make the smallest targeted fix**
- [ ] **Step 3: Re-run the same command and confirm GREEN**

## Chunk 3: Full Verification and Sync

### Task 5: Verify the combined tuning pass and sync the game test folder

**Files:**
- Modify: none unless verification exposes a gap

- [ ] **Step 1: Run `node --test tests/input_correction_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/ui_rendering_module.test.cjs`**
- [ ] **Step 2: Run `node --test tests/*.test.cjs`**
- [ ] **Step 3: Run `cmake --install build/wsl-release`**
- [ ] **Step 4: Run `rsync -a --delete --exclude='meta.ini' dist/CodexOfPowerNG/ /mnt/g/TAKEALOOK/mods/Codex.of.Power.NG-v1.2.0-rc.1/`**
- [ ] **Step 5: Verify the synced file hashes for the changed assets before asking for in-game validation**
