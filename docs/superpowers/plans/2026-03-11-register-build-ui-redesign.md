# Register/Build UI Redesign Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Redesign the Quick Register and Build tabs so the register screen reads as a codex-style work surface and the build screen reads as a shrine-like build assembly surface without changing the underlying gameplay contracts.

**Architecture:** Keep the existing Prisma UI module split intact and treat this as a presentation-layer rework. Push the shell/layout work into `index.html`, keep screen composition in `ui_rendering.js`, keep grouped register markup in `ui_register_batch_panel.js`, and keep build markup in `ui_build_panel.js`. Do not change native payload shape unless a specific presentation field is proven missing.

**Tech Stack:** Prisma UI web overlay, embedded CSS in `index.html`, modular JS renderers, Node-based UI regression tests.

---

## File Structure

- Modify: `PrismaUI/views/codexofpowerng/index.html`
  Responsibility: shared shell layout, embedded CSS tokens, quick/build section markup, and wiring of renderer refs.
- Modify: `PrismaUI/views/codexofpowerng/ui_rendering.js`
  Responsibility: compose screen-level rendering, quick/build shell metadata updates, and DOM fallback safety.
- Modify: `PrismaUI/views/codexofpowerng/ui_register_batch_panel.js`
  Responsibility: discipline section headers, compact codex row markup, state-tag hierarchy, and batch summary markup.
- Modify: `PrismaUI/views/codexofpowerng/ui_build_panel.js`
  Responsibility: shrine/altar build layout, score summary bar, active-slot cluster, option list cards, and detail panel markup.
- Modify: `PrismaUI/views/codexofpowerng/ui_i18n.js`
  Responsibility: strings for new codex/shrine copy, refined labels, and any new state text.
- Test: `tests/ui_rendering_module.test.cjs`
  Responsibility: verify renderer composition, required DOM refs, and quick/build shell assumptions.
- Test: `tests/build_batch_register_flow.test.cjs`
  Responsibility: verify grouped register markup, discipline section semantics, and disabled-state rendering.
- Test: `tests/build_ui_rendering_module.test.cjs`
  Responsibility: verify build summary/slot/detail structure and discipline-specific output.
- Test: `tests/ui_i18n_module.test.cjs`
  Responsibility: verify new labels exist in both locales and old copy does not regress required meaning.

## Chunk 1: Shared Shell and Quick Register Surface

### Task 1: Lock in shared shell direction and required refs

**Files:**
- Modify: `tests/ui_rendering_module.test.cjs`
- Modify: `PrismaUI/views/codexofpowerng/index.html`

- [ ] **Step 1: Write the failing renderer shell assertions**

Add assertions that:

```js
assert.match(html, /refs:\s*\{[\s\S]*quickBody,[\s\S]*regBody,/);
assert.match(html, /class="quickBatchSummary"/);
assert.match(html, /id="tabBuild"/);
assert.match(html, /id="buildPanel"/);
```

- [ ] **Step 2: Run the test to verify it fails**

Run:

```bash
node --test tests/ui_rendering_module.test.cjs
```

Expected: FAIL if required shell refs/markup are missing.

- [ ] **Step 3: Update `index.html` to match the redesigned shell contract**

Implement the minimum shell changes:

- preserve `quickBody`, `regBody`, and `buildPanel` refs in the renderer input
- keep Quick Register and Build as distinct section roots
- keep the quick summary/footer container stable for later styling work

- [ ] **Step 4: Re-run the test**

Run:

```bash
node --test tests/ui_rendering_module.test.cjs
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add tests/ui_rendering_module.test.cjs PrismaUI/views/codexofpowerng/index.html
git commit -m "test: lock quick and build shell refs"
```

### Task 2: Introduce the codex visual system for Quick Register

**Files:**
- Modify: `tests/build_batch_register_flow.test.cjs`
- Modify: `PrismaUI/views/codexofpowerng/ui_register_batch_panel.js`
- Modify: `PrismaUI/views/codexofpowerng/index.html`

- [ ] **Step 1: Write the failing grouped-row presentation test**

Add assertions that grouped quick rows emit:

```js
assert.match(html, /sectionRow/);
assert.match(html, /colSelect/);
assert.match(html, /colGroup/);
assert.match(html, /itemName/);
assert.match(html, /quest_protected|favorite_protected|not_actionable/);
```

Also add a focused assertion for discipline-specific presentation hooks such as:

```js
assert.match(html, /attack|defense|utility/);
```

- [ ] **Step 2: Run the test to verify it fails**

Run:

```bash
node --test tests/build_batch_register_flow.test.cjs
```

Expected: FAIL until row/section markup is updated.

- [ ] **Step 3: Rework grouped register markup into the codex row structure**

Update `ui_register_batch_panel.js` so each row reads as:

```html
<tr class="dataRow ...">
  <td class="colSelect">...</td>
  <td class="colGroup"><span class="pill ...">...</span></td>
  <td>
    <div class="itemName">...</div>
    <div class="small mono">...</div>
    <div class="small">state tags...</div>
    <div class="small">reason...</div>
  </td>
  <td class="mono colCount">safe/total</td>
  <td class="colAction"><button>Register</button></td>
</tr>
```

Then adjust the embedded CSS in `index.html` so:

- Attack / Defense / Utility each have unique accent tokens
- section headers use stronger separators
- row height is reduced
- state tags are quieter than item names
- default row buttons are less dominant than the batch footer action

- [ ] **Step 4: Re-run the targeted test**

Run:

```bash
node --test tests/build_batch_register_flow.test.cjs tests/ui_rendering_module.test.cjs
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add tests/build_batch_register_flow.test.cjs tests/ui_rendering_module.test.cjs PrismaUI/views/codexofpowerng/ui_register_batch_panel.js PrismaUI/views/codexofpowerng/index.html
git commit -m "feat: redesign quick register as codex list"
```

### Task 3: Tighten quick-screen composition and density

**Files:**
- Modify: `tests/ui_rendering_module.test.cjs`
- Modify: `PrismaUI/views/codexofpowerng/ui_rendering.js`
- Modify: `PrismaUI/views/codexofpowerng/index.html`

- [ ] **Step 1: Write the failing composition test**

Add a focused test that verifies:

```js
assert.match(quickBody.innerHTML, /Iron Sword|ok/);
assert.equal(quickBody.getAttribute("data-virtual-mode"), "grouped");
assert.match(refs.invMetaEl.textContent, /Inventory|showing/i);
```

- [ ] **Step 2: Run the test to verify it fails**

Run:

```bash
node --test tests/ui_rendering_module.test.cjs
```

Expected: FAIL until `renderQuick()` and the quick toolbar/footer layout are aligned with the new surface.

- [ ] **Step 3: Refine `renderQuick()` and quick-shell composition**

Make the minimum changes:

- keep grouped rows authoritative for Quick Register
- preserve the quick-body fallback lookup
- keep batch summary/footer copy aligned with the denser codex presentation
- visually separate toolbar controls from footer actions in `index.html`

- [ ] **Step 4: Re-run the test**

Run:

```bash
node --test tests/ui_rendering_module.test.cjs
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add tests/ui_rendering_module.test.cjs PrismaUI/views/codexofpowerng/ui_rendering.js PrismaUI/views/codexofpowerng/index.html
git commit -m "feat: tighten quick register shell composition"
```

## Chunk 2: Build Shrine / Altar Surface

### Task 4: Add the build summary bar and altar-centered slot layout

**Files:**
- Modify: `tests/build_ui_rendering_module.test.cjs`
- Modify: `PrismaUI/views/codexofpowerng/ui_build_panel.js`
- Modify: `PrismaUI/views/codexofpowerng/index.html`

- [ ] **Step 1: Write the failing build shell assertions**

Add assertions that the build markup contains:

```js
assert.match(renderedHtml, /Active Slots/);
assert.match(renderedHtml, /Attack|Defense|Utility/);
assert.match(renderedHtml, /slot/i);
assert.match(renderedHtml, /summary|score/i);
```

- [ ] **Step 2: Run the test to verify it fails**

Run:

```bash
node --test tests/build_ui_rendering_module.test.cjs
```

Expected: FAIL until the build panel exposes the new summary/altar structure.

- [ ] **Step 3: Rework the build shell into shrine layout**

Update `ui_build_panel.js` so the rendered structure has:

- a summary bar for discipline scores
- a visually central slot cluster for the six active slots
- supporting left/right panels rather than a flat neutral layout

Keep the underlying slot/action wiring unchanged.

- [ ] **Step 4: Re-run the test**

Run:

```bash
node --test tests/build_ui_rendering_module.test.cjs
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add tests/build_ui_rendering_module.test.cjs PrismaUI/views/codexofpowerng/ui_build_panel.js PrismaUI/views/codexofpowerng/index.html
git commit -m "feat: redesign build screen as shrine layout"
```

### Task 5: Clarify build card/detail hierarchy and slot-state styling

**Files:**
- Modify: `tests/build_ui_rendering_module.test.cjs`
- Modify: `PrismaUI/views/codexofpowerng/ui_build_panel.js`
- Modify: `PrismaUI/views/codexofpowerng/index.html`

- [ ] **Step 1: Write the failing card/detail assertions**

Extend build tests with checks like:

```js
assert.match(renderedHtml, /Locked|Unlocked|Active/);
assert.match(renderedHtml, /Requires|Need .* Score/);
assert.match(renderedHtml, /Activate|Deactivate|Swap/);
```

If the current tests already cover the raw words, add stricter structural assertions for:

```js
assert.match(renderedHtml, /buildOptionDetail|buildSlotCluster|buildScorePill/);
```

- [ ] **Step 2: Run the test to verify it fails**

Run:

```bash
node --test tests/build_ui_rendering_module.test.cjs
```

Expected: FAIL until the card/detail hierarchy exposes explicit shrine-state structure.

- [ ] **Step 3: Rework build cards and detail panel**

Implement:

- clearer active/locked/unlocked card states
- discipline-colored slot borders
- stronger detail panel emphasis for the selected option
- restrained but readable action buttons

Do not change native build semantics.

- [ ] **Step 4: Re-run the test**

Run:

```bash
node --test tests/build_ui_rendering_module.test.cjs
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add tests/build_ui_rendering_module.test.cjs PrismaUI/views/codexofpowerng/ui_build_panel.js PrismaUI/views/codexofpowerng/index.html
git commit -m "feat: refine build card and slot hierarchy"
```

## Chunk 3: Copy, Polish, and Verification

### Task 6: Update copy and translation coverage for the new tone

**Files:**
- Modify: `tests/ui_i18n_module.test.cjs`
- Modify: `PrismaUI/views/codexofpowerng/ui_i18n.js`
- Modify: `PrismaUI/views/codexofpowerng/index.html`

- [ ] **Step 1: Write the failing i18n assertions**

Add assertions that both locales include the new or updated UI language for:

```js
assert.match(source, /quick\.help/);
assert.match(source, /build\.help/);
assert.match(source, /quick\.actionableOnly/);
```

Then verify updated copy intent in both languages:

```js
assert.match(koSource, /공격|방어|유틸/);
assert.match(enSource, /Attack|Defense|Utility/);
```

- [ ] **Step 2: Run the test to verify it fails**

Run:

```bash
node --test tests/ui_i18n_module.test.cjs
```

Expected: FAIL until the updated copy exists in both locales.

- [ ] **Step 3: Update copy and help text**

Refine strings so:

- Quick Register reads like a record-book work surface
- Build reads like a current build assembly surface
- labels remain short enough for dense rows

- [ ] **Step 4: Re-run the test**

Run:

```bash
node --test tests/ui_i18n_module.test.cjs
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add tests/ui_i18n_module.test.cjs PrismaUI/views/codexofpowerng/ui_i18n.js PrismaUI/views/codexofpowerng/index.html
git commit -m "docs: refine codex and shrine UI copy"
```

### Task 7: Final regression sweep and manual package refresh

**Files:**
- Modify: `docs/superpowers/specs/2026-03-11-register-build-ui-redesign-design.md` (only if implementation notes changed)
- Modify: `docs/contracts/prismaui-js-api.md` (only if any UI contract fields changed)

- [ ] **Step 1: Run the JS regression sweep**

Run:

```bash
node --test tests/*.test.cjs
```

Expected: PASS.

- [ ] **Step 2: Refresh installed UI assets**

Run:

```bash
cmake --install build/wsl-release
rsync -a --delete --exclude='meta.ini' dist/CodexOfPowerNG/ /mnt/g/TAKEALOOK/mods/Codex.of.Power.NG-v1.2.0-rc.1/
```

Expected: updated Prisma UI assets in the test mod folder.

- [ ] **Step 3: Manual smoke test checklist**

Verify in game:

- Quick Register shows more rows per screen than before
- Attack / Defense / Utility are visually distinct
- protected rows stay readable and obviously blocked
- Build reads as slot-centered rather than table-centered
- high-DPI layout still fits without clipping

- [ ] **Step 4: Commit**

```bash
git add docs/superpowers/specs/2026-03-11-register-build-ui-redesign-design.md docs/contracts/prismaui-js-api.md
git commit -m "chore: finalize ui redesign verification notes"
```
