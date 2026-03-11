# Build Character Silhouette Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reintroduce the existing character image into the Build screen as a low-contrast altar silhouette behind the active slot cluster without restoring the legacy reward orbit.

**Architecture:** Treat this as a focused presentation-layer enhancement on top of the existing Build redesign. Keep the asset path and fallback behavior aligned with the legacy reward image pipeline, but move the rendered image into the current Build shrine structure so slot cards and detail panels remain the foreground UI.

**Tech Stack:** Prisma UI web overlay, embedded CSS in `index.html`, modular JS renderers, Node-based UI regression tests.

---

## File Structure

- Modify: `PrismaUI/views/codexofpowerng/index.html`
  Responsibility: add the Build-screen silhouette markup, shared CSS layering, and DOM refs.
- Modify: `PrismaUI/views/codexofpowerng/ui_build_panel.js`
  Responsibility: preserve slot cluster structure while cooperating with the new silhouette container.
- Modify: `PrismaUI/views/codexofpowerng/ui_rendering.js`
  Responsibility: sync the reused character-image load/fallback state against the new Build-screen DOM location.
- Test: `tests/build_ui_rendering_module.test.cjs`
  Responsibility: verify Build markup and style hooks for the silhouette layer.
- Test: `tests/ui_rendering_module.test.cjs`
  Responsibility: verify DOM refs and fallback wiring for the reintroduced image.

## Chunk 1: Restore the Build-Screen Character Layer

### Task 1: Lock the new Build silhouette contract with tests

**Files:**
- Modify: `tests/build_ui_rendering_module.test.cjs`
- Modify: `tests/ui_rendering_module.test.cjs`

- [ ] **Step 1: Write the failing tests**

Add assertions that the Build view source contains:

```js
assert.match(html, /buildCharacterStage/);
assert.match(html, /buildCharacterSilhouette/);
assert.match(html, /buildCharacterShade/);
```

Add assertions that the renderer shell still wires:

```js
assert.match(html, /rewardCharacterImgEl/);
assert.match(html, /rewardImageFallbackEl/);
```

- [ ] **Step 2: Run the tests to verify they fail**

Run:

```bash
node --test tests/build_ui_rendering_module.test.cjs tests/ui_rendering_module.test.cjs
```

Expected: FAIL until the Build markup and shell contract include the silhouette layer.

- [ ] **Step 3: Commit**

```bash
git add tests/build_ui_rendering_module.test.cjs tests/ui_rendering_module.test.cjs
git commit -m "test: lock build silhouette contract"
```

### Task 2: Add the silhouette layer to the Build shrine

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`
- Modify: `PrismaUI/views/codexofpowerng/ui_build_panel.js`

- [ ] **Step 1: Implement the minimum DOM changes**

Update the Build screen so the slot cluster owns a non-interactive character stage:

- add a dedicated silhouette container behind the slot cards
- place the existing character image element inside that stage
- keep the slot grid and panel layout unchanged in front of it
- keep the fallback node available for missing-image cases

- [ ] **Step 2: Add the minimum CSS layering**

Implement:

- low-contrast image opacity
- vignette / shade overlay to protect readability
- explicit z-index ordering so slots and text stay above the image
- Build-only styling so Quick Register and other tabs remain untouched

- [ ] **Step 3: Re-run the targeted tests**

Run:

```bash
node --test tests/build_ui_rendering_module.test.cjs tests/ui_rendering_module.test.cjs
```

Expected: PASS.

- [ ] **Step 4: Commit**

```bash
git add PrismaUI/views/codexofpowerng/index.html PrismaUI/views/codexofpowerng/ui_build_panel.js tests/build_ui_rendering_module.test.cjs tests/ui_rendering_module.test.cjs
git commit -m "feat: restore build altar silhouette"
```

## Chunk 2: Reuse Fallback Wiring Safely

### Task 3: Route image-state syncing through the new Build location

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/ui_rendering.js`
- Modify: `tests/ui_rendering_module.test.cjs`

- [ ] **Step 1: Write the failing fallback test**

Add a focused assertion that the rendering layer can still resolve and synchronize the character image + fallback state when the image now lives in the Build shrine.

- [ ] **Step 2: Run the test to verify it fails**

Run:

```bash
node --test tests/ui_rendering_module.test.cjs
```

Expected: FAIL until the new DOM location is handled.

- [ ] **Step 3: Implement the minimal renderer update**

Keep the legacy helper path if present, but ensure the Build screen refs now point at the new image/fallback nodes and that missing-image behavior remains safe.

- [ ] **Step 4: Re-run the test**

Run:

```bash
node --test tests/ui_rendering_module.test.cjs
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add PrismaUI/views/codexofpowerng/ui_rendering.js tests/ui_rendering_module.test.cjs
git commit -m "fix: sync build silhouette image fallback"
```

## Chunk 3: Final Verification

### Task 4: Verify the complete UI contract

**Files:**
- Modify: none unless verification exposes a gap

- [ ] **Step 1: Run the focused Build/UI regression suite**

Run:

```bash
node --test tests/build_ui_rendering_module.test.cjs tests/ui_rendering_module.test.cjs tests/build_batch_register_flow.test.cjs
```

Expected: PASS.

- [ ] **Step 2: Run the full JS regression suite**

Run:

```bash
node --test tests/*.test.cjs
```

Expected: PASS with `0` failures.

- [ ] **Step 3: Install updated assets for game-side verification**

Run:

```bash
cmake --install build/wsl-release
rsync -a --delete --exclude='meta.ini' dist/CodexOfPowerNG/ /mnt/g/TAKEALOOK/mods/Codex.of.Power.NG-v1.2.0-rc.1/
```

Expected: install + sync complete with no rsync errors.

- [ ] **Step 4: Commit**

```bash
git add PrismaUI/views/codexofpowerng/index.html PrismaUI/views/codexofpowerng/ui_build_panel.js PrismaUI/views/codexofpowerng/ui_rendering.js tests/build_ui_rendering_module.test.cjs tests/ui_rendering_module.test.cjs
git commit -m "feat: restore build character silhouette"
```
