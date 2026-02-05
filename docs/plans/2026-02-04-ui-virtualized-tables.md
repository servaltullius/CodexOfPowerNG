# UI Virtualized Tables Implementation Plan

> **For Codex:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to implement this plan task-by-task.

**Goal:** Make the UI feel noticeably smoother while the menu is open (cursor/hover/scroll) by reducing DOM size and rerender cost for large tables.

**Architecture:** Keep `.root` as the only scroll container (it already has a smooth wheel handler). For large tables, render only the visible slice of rows and use top/bottom spacer rows inside `<tbody>` to preserve scroll height. Avoid `nth-child` striping (spacers break it) and use explicit row classes instead.

**Tech Stack:** PrismaUI view (Ultralight) HTML/CSS/JS.

---

### Task 1: Prepare table styling for virtualization

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`

**Steps:**
1. Replace `tbody tr:nth-child(...)` styling with `.dataRow` / `.rowOdd` selectors.
2. Add `tr.spacerRow` styles to avoid borders/padding and to disable pointer events.
3. Set fixed row heights for Quick/Registered tables (scaled by `--uiScale`).

---

### Task 2: Implement virtualized rendering for Quick Register

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`

**Steps:**
1. Refactor `renderQuick()` to:
   - compute and store the filtered list (by search text)
   - update `quickVisibleIds`
   - call `renderQuickVirtual(force=true)` rather than injecting all rows.
2. Implement `renderQuickVirtual()`:
   - compute visible row range from `.root.scrollTop` + tbody offset
   - render top spacer + visible slice rows + bottom spacer
   - set row classes: `dataRow`, `rowOdd`, and `selected` when applicable
3. Add scroll listener on `.root` that schedules virtual rerender via `requestAnimationFrame`.

---

### Task 3: Implement virtualized rendering for Registered list

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`

**Steps:**
1. Refactor `renderRegistered()` to compute a filtered list and call `renderRegisteredVirtual(force=true)`.
2. Implement `renderRegisteredVirtual()` (same spacer approach as Quick; different `colspan` and row height).

---

### Task 4: Keyboard navigation compatibility

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`

**Steps:**
1. Replace `scrollIntoView()` (may not exist for non-rendered rows) with a `scrollQuickIndexIntoView()` helper that adjusts `.root.scrollTop`.
2. Ensure `setTab()` triggers virtual rerender for the active table.

---

### Task 5: Build + repackage release zip

**Files:**
- Modify: `AGENTS.md` (document virtualization + fixed row heights)

**Steps:**
1. Build + install:
   - `cmake --build --preset wsl-release`
   - `cmake --install build/wsl-release`
2. Update the MO2-ready archive:
   - `cd dist/CodexOfPowerNG`
   - `zip -r -FS "../../releases/Codex of Power NG.zip" .`
3. Verify:
   - `ls -la "releases/Codex of Power NG.zip"`

