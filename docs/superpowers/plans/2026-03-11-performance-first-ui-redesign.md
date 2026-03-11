# Performance-First UI Redesign Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Redesign the current Prisma UI presentation so Quick Register stays fast and list-oriented, while Build becomes a mostly fixed management screen that keeps mood with cheaper rendering.

**Architecture:** Keep the existing gameplay and JS module boundaries, but shift the visual contract from layered translucent web panels to more opaque, framed overlay panels. Treat Quick Register and Build as different performance surfaces: Quick Register remains the primary scrolling work list, while Build becomes a stable three-panel layout where only the option rail should feel scroll-heavy.

**Tech Stack:** Prisma UI embedded HTML/CSS/JS, modular renderers (`ui_rendering.js`, `ui_build_panel.js`, `ui_register_batch_panel.js`), input correction module, Node-based UI regression tests.

---

## File Structure

- Modify: `PrismaUI/views/codexofpowerng/index.html`
  Responsibility: update the shared visual contract toward more opaque, cheaper panels and reduced heavy effects.
- Modify: `PrismaUI/views/codexofpowerng/ui_build_panel.js`
  Responsibility: reshape Build markup so the layout behaves more like a fixed management surface.
- Modify: `PrismaUI/views/codexofpowerng/ui_rendering.js`
  Responsibility: keep Build render behavior aligned with the fixed-layout intent and preserve tab rendering.
- Modify: `PrismaUI/views/codexofpowerng/input_correction.js`
  Responsibility: preserve distinct wheel profiles once the new Build layout is in place.
- Test: `tests/build_ui_rendering_module.test.cjs`
  Responsibility: lock the lighter Build visual contract and fixed-layout markup assumptions.
- Test: `tests/ui_rendering_module.test.cjs`
  Responsibility: protect shared rendering ownership and Build rerender assumptions.
- Test: `tests/build_batch_register_flow.test.cjs`
  Responsibility: protect Quick Register grouped work-list behavior while the visual contract changes.

## Chunk 1: Build as a Fixed Management Surface

### Task 1: Lock the new Build layout contract

**Files:**
- Modify: `tests/build_ui_rendering_module.test.cjs`
- Modify: `PrismaUI/views/codexofpowerng/ui_build_panel.js`

- [ ] **Step 1: Write a failing test for a more fixed Build layout contract**
- [ ] **Step 2: Run `node --test tests/build_ui_rendering_module.test.cjs` and confirm RED**
- [ ] **Step 3: Update `ui_build_panel.js` so the center altar and right focus panel are treated as stable panels while the left option rail remains the main browsing surface**
- [ ] **Step 4: Re-run `node --test tests/build_ui_rendering_module.test.cjs` and confirm GREEN**

### Task 2: Lock the lighter Build visual contract

**Files:**
- Modify: `tests/build_ui_rendering_module.test.cjs`
- Modify: `PrismaUI/views/codexofpowerng/index.html`

- [ ] **Step 1: Write a failing test for more opaque Build panels and reduced heavy effects**
- [ ] **Step 2: Run `node --test tests/build_ui_rendering_module.test.cjs` and confirm RED**
- [ ] **Step 3: Reduce Build blur / glow / shadow / layered background usage with the smallest possible CSS changes**
- [ ] **Step 4: Preserve discipline colors and the lower altar silhouette**
- [ ] **Step 5: Re-run `node --test tests/build_ui_rendering_module.test.cjs` and confirm GREEN**

## Chunk 2: Quick Register as a Work Surface

### Task 3: Keep Quick Register grouped, dense, and cheaper to paint

**Files:**
- Modify: `tests/build_batch_register_flow.test.cjs`
- Modify: `PrismaUI/views/codexofpowerng/index.html`
- Modify: `PrismaUI/views/codexofpowerng/ui_register_batch_panel.js`

- [ ] **Step 1: Write a failing test for the expected grouped work-list structure if markup changes are needed**
- [ ] **Step 2: Run `node --test tests/build_batch_register_flow.test.cjs` and confirm RED**
- [ ] **Step 3: Reduce unnecessary decorative effects in Quick Register while preserving grouped codex rows, state tags, and batch summary behavior**
- [ ] **Step 4: Re-run `node --test tests/build_batch_register_flow.test.cjs` and confirm GREEN**

### Task 4: Preserve distinct wheel behavior under the new screen contracts

**Files:**
- Modify: `tests/input_correction_module.test.cjs`
- Modify: `PrismaUI/views/codexofpowerng/input_correction.js`

- [ ] **Step 1: Add or refine failing tests if the new layout changes the expected Quick vs Build wheel behavior**
- [ ] **Step 2: Run `node --test tests/input_correction_module.test.cjs` and confirm RED**
- [ ] **Step 3: Adjust screen detection or wheel profile constants only if needed**
- [ ] **Step 4: Re-run `node --test tests/input_correction_module.test.cjs` and confirm GREEN**

## Chunk 3: Shared Rendering Verification and Sync

### Task 5: Verify the renderer still owns the UI correctly

**Files:**
- Modify: none unless verification exposes a gap

- [ ] **Step 1: Run `node --test tests/ui_rendering_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/build_batch_register_flow.test.cjs tests/input_correction_module.test.cjs`**
- [ ] **Step 2: If a shared render contract breaks, make the smallest targeted fix in `ui_rendering.js` or `index.html`**
- [ ] **Step 3: Re-run the same command and confirm GREEN**

### Task 6: Final regression, install, and test-folder sync

**Files:**
- Modify: none unless verification reveals a regression

- [ ] **Step 1: Run `node --test tests/*.test.cjs`**
- [ ] **Step 2: Run `cmake --install build/wsl-release`**
- [ ] **Step 3: Run `rsync -a --delete --exclude='meta.ini' dist/CodexOfPowerNG/ /mnt/g/TAKEALOOK/mods/Codex.of.Power.NG-v1.2.0-rc.1/`**
- [ ] **Step 4: Verify hashes for the changed UI assets in the synced test folder**
- [ ] **Step 5: Ask for in-game validation before committing**
