# Build Slot Overflow Layout Implementation Plan

> Supersedes the active-slot portion of [2026-03-15-build-detail-rail-layout.md](/home/kdw73/Codex%20of%20Power%20NG/docs/superpowers/plans/2026-03-15-build-detail-rail-layout.md). The focused-option expansion remains, but the lower rail contract now moves from fixed-ratio clipping to an internal slot-grid scroller.

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove clipping from the Build tab active-slot panel while keeping the focused-option panel as the dominant decision surface.

**Architecture:** Keep the existing three-column Build layout and fix the overflow behavior at the detail-rail level. Split the slot summary area into a fixed header plus a dedicated matrix scroller, then update source-level rendering tests to pin the new CSS and markup contract.

**Tech Stack:** Prisma UI HTML/CSS/JS, Node test runner

---

## File Map

- Modify: [PrismaUI/views/codexofpowerng/index.html](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/index.html)
  - Replace the fixed lower rail row contract, add slot-panel internal scroller styling, and tighten slot-card density.
- Modify: [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
  - Wrap the slot matrix in a dedicated scroller container under a fixed panel eyebrow.
- Test: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
  - Lock the new detail-rail CSS contract and slot-summary markup contract.

## Chunk 1: Lock The Overflow Fix Contract

### Task 1: Write failing tests for the new rail and slot-summary structure

**Files:**
- Modify: `tests/build_ui_rendering_module.test.cjs`

- [ ] **Step 1: Write the failing source assertions**

Cover:
- `buildDetailRail` no longer uses the old `minmax(..., 0.8fr)` lower row contract
- `buildSlotSummaryPanel` becomes an internal grid/flex wrapper rather than a hard-clipped box
- new slot scroller class exists in rendered HTML and source CSS
- rendered markup exposes `data-wheel-surface="build-slots"` on the slot scroller
- slot-card density values are reduced again

- [ ] **Step 2: Run the test to verify it fails**

Run:
```bash
node --test tests/build_ui_rendering_module.test.cjs
```

Expected: FAIL because the current source still exposes the clipped slot-panel contract.

## Chunk 2: Implement The Slot Overflow Fix

### Task 2: Replace fixed clipping with internal slot scrolling

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`
- Modify: `PrismaUI/views/codexofpowerng/ui_build_panel.js`
- Test: `tests/build_ui_rendering_module.test.cjs`

- [ ] **Step 1: Update the slot-summary markup**

In `ui_build_panel.js`:
- keep the existing panel eyebrow
- wrap `buildSlotMatrix` in a dedicated `buildSlotMatrixScroller` container
- add `data-wheel-surface="build-slots"` so wheel routing prefers the slot grid before catalog fallback

- [ ] **Step 2: Update the detail-rail CSS**

In `index.html`:
- replace the lower row fixed ratio with a content-based row contract
- make `buildSlotSummaryPanel` a two-part container (`header + scroller`)
- move overflow handling from the whole panel to the slot-matrix scroller

- [ ] **Step 3: Compress the slot-card density one more step**

Adjust:
- slot matrix gap
- card padding
- slot name line-height/font
- slot action button spacing/height

- [ ] **Step 4: Run the focused test to verify it passes**

Run:
```bash
node --test tests/build_ui_rendering_module.test.cjs
```

Expected: PASS

- [ ] **Step 5: Run the full JS suite**

Run:
```bash
node --test tests/*.test.cjs
```

Expected: PASS

- [ ] **Step 6: Perform a runtime clipping sanity check**

Verify in a fixed Build viewport that:
- all 6 slot cards and action buttons are fully visible without panel-edge clipping
- only the slot grid scrolls when space is tight
- the selected-option panel remains the dominant rail surface

## Chunk 3: Full Verification

### Task 3: Refresh the release artifact after the UI-only fix

**Files:**
- No product logic changes beyond the UI files above
- Validation only

- [ ] **Step 1: Reinstall the release artifact**

Run:
```bash
cmake --install build/wsl-release
```

Expected: release payload in `dist/CodexOfPowerNG` is refreshed with the new UI files.

- [ ] **Step 2: Optional deployment sync**

If the user asks for it, sync:
```bash
rsync -a --delete dist/CodexOfPowerNG/PrismaUI/ /mnt/g/TAKEALOOK/mods/Codex.of.Power.NG-v1.2.0-rc.1/PrismaUI/
```
