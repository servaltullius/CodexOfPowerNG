# Build Detail Rail Layout Implementation Plan

> Partially superseded on 2026-03-15 by [2026-03-15-build-slot-overflow-layout.md](/home/kdw73/Codex%20of%20Power%20NG/docs/superpowers/plans/2026-03-15-build-slot-overflow-layout.md) for the active-slot panel overflow fix.

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Expand the focused-option panel and compress the active-slot panel in the Build tab detail rail without changing overall information architecture.

**Architecture:** Keep the existing `ui_build_panel.js` markup and make the change primarily in `index.html` CSS. Update the rendering contract tests so the new ratio and compact slot density are pinned in source-level assertions.

**Tech Stack:** Prisma UI HTML/CSS/JS, Node test runner

---

## Chunk 1: Layout Contract

### Task 1: Document the new desktop ratio

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`
- Test: `tests/build_ui_rendering_module.test.cjs`

- [ ] **Step 1: Write the failing test**

Add source assertions for the new `buildDetailRail` row ratio and compact slot card rules.

- [ ] **Step 2: Run test to verify it fails**

Run: `node --test tests/build_ui_rendering_module.test.cjs`
Expected: FAIL because the CSS still exposes the previous detail-rail sizing.

- [ ] **Step 3: Write minimal implementation**

Update the desktop-only Build CSS:

- `buildDetailRail` uses an explicit weighted split favoring the focused panel
- slot card padding/gap/font/button sizes are reduced
- `@media (max-width: 980px)` remains intact

- [ ] **Step 4: Run test to verify it passes**

Run: `node --test tests/build_ui_rendering_module.test.cjs`
Expected: PASS

- [ ] **Step 5: Verify full UI regression**

Run: `node --test tests/*.test.cjs`
Expected: PASS
