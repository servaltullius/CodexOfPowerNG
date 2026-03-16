# Build Slot Single-Line Cards Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Compress Build tab active-slot cards into a single-line summary layout without changing the surrounding right-rail information architecture.

**Architecture:** Keep the existing slot summary panel and wheel-routing contract, but change each slot card from a vertical stack to a horizontal three-part row. Update the JS rendering markup and CSS density rules together, then pin the new contract in source-level Node tests.

**Tech Stack:** Prisma UI HTML/CSS/JS, Node test runner

---

## File Map

- Modify: [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
  - Adjust slot-card markup and shorten empty-slot copy.
- Modify: [PrismaUI/views/codexofpowerng/index.html](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/index.html)
  - Replace the slot-card vertical layout rules with a single-line compact contract.
- Test: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
  - Lock the new slot-card markup and CSS contract.

## Chunk 1: Lock The Single-Line Contract

### Task 1: Write failing tests for the compact slot-card layout

**Files:**
- Modify: `tests/build_ui_rendering_module.test.cjs`

- [ ] **Step 1: Add failing source assertions**

Cover:
- `buildSlotMatrixCard` uses a horizontal grid or flex contract instead of the current stacked flow
- `buildSlotMatrixName` is configured for single-line ellipsis
- slot actions stay on one line and use tighter spacing
- empty-slot hint also has a one-line contract
- rendered HTML still includes `buildSlotMatrixScroller` and `data-wheel-surface="build-slots"`
- empty slots render the shortened hint copy

- [ ] **Step 2: Run the focused test to verify it fails**

Run:
```bash
node --test tests/build_ui_rendering_module.test.cjs
```

Expected: FAIL because the current source still reflects the older 3-row slot-card contract.

## Chunk 2: Implement The Compact Slot Cards

### Task 2: Switch slot cards to the approved single-line structure

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/ui_build_panel.js`
- Modify: `PrismaUI/views/codexofpowerng/index.html`
- Test: `tests/build_ui_rendering_module.test.cjs`

- [ ] **Step 1: Update slot-card markup**

In `ui_build_panel.js`:
- keep the slot scroller and panel structure unchanged
- keep the slot chip, option name, and action in each card
- shorten the empty-slot text so it fits cleanly on one line

- [ ] **Step 2: Update slot-card CSS**

In `index.html`:
- give `buildSlotMatrixCard` a horizontal compact layout
- add single-line ellipsis behavior to the option title
- add single-line ellipsis behavior to the empty-slot hint
- tighten padding, gap, and button sizing so six cards fit more naturally in the lower rail

- [ ] **Step 3: Run the focused test to verify it passes**

Run:
```bash
node --test tests/build_ui_rendering_module.test.cjs
```

Expected: PASS

## Chunk 3: Regression Verification

### Task 3: Confirm the broader JS surface still passes

**Files:**
- Validation only

- [ ] **Step 1: Run the full JS test suite**

Run:
```bash
node --test tests/*.test.cjs
```

Expected: PASS

- [ ] **Step 2: Sanity-check the final diff**

Run:
```bash
git diff -- PrismaUI/views/codexofpowerng/ui_build_panel.js PrismaUI/views/codexofpowerng/index.html tests/build_ui_rendering_module.test.cjs docs/superpowers/specs/2026-03-16-build-slot-single-line-cards-design.md docs/superpowers/plans/2026-03-16-build-slot-single-line-cards.md
```

Expected: only the approved slot-card compacting work and supporting docs are present.

- [ ] **Step 3: Perform a runtime viewport sanity check**

Verify in the fixed Build viewport that:
- all 6 slot cards and action buttons stay fully visible without clipping
- empty-slot hints remain single-line in the active-slot panel
- only the slot grid scrolls when space gets tight
