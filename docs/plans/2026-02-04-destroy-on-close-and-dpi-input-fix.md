# Destroy-on-Close + DPI Input Fix Implementation Plan

> **For Codex:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to implement this plan task-by-task.

**Goal:** Stop FPS drops that persist after closing the UI by fully releasing the PrismaUI view when closed, and improve click/hitbox accuracy for high-DPI / Windows scaling (175–200%).

**Architecture:** Make view lifetime explicit: create lazily on first open, hide or destroy on close based on `settings.json` (`ui.destroyOnClose`). Guard against stale DOM-ready callbacks after destroy/recreate. Improve JS-side coordinate correction by trying both origin-relative and root-relative scaling (mul/div) and selecting the best target via `elementFromPoint` scoring.

**Tech Stack:** C++ (CommonLibSSE-NG), SKSE TaskInterface (AddUITask), Prisma UI API (CreateView/Show/Focus/Hide/Destroy), HTML/CSS/JS.

---

### Task 1: Implement `ui.destroyOnClose` (destroy view on close)

**Files:**
- Modify: `src/PrismaUIManager.cpp`

**Step 1: Add a safe “reset view state” helper**
- Reset: `g_view`, `g_domReady`, `g_openRequested`, `g_viewHidden`, `g_viewFocused`, `g_focusAttemptCount`
- Ensure the helper runs only on the PrismaUI/UI thread (call from `AddUITask`).

**Step 2: ToggleUI close path uses Destroy when enabled**
- In the “hide” branch:
  - If `GetSettings().uiDestroyOnClose == true`: call `api->Destroy(view)` and then reset state.
  - Else: keep `api->Hide(view)` (Hide auto-unfocuses).

**Step 3: Guard DOM-ready callback**
- In `OnDomReady(PrismaView view)`:
  - Ignore callbacks where `view != g_view` (stale callback after destroy/recreate).
  - Only set `g_domReady=true` for the active view.

**Step 4: Verification (build/install)**
- Run: `cmake --build --preset wsl-release`
- Run: `cmake --install build/wsl-release`
- Expected: build/install success, `dist/CodexOfPowerNG/SKSE/Plugins/CodexOfPowerNG.dll` updated.

---

### Task 2: Make DPI click correction more robust (root-relative + origin-relative candidates)

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`

**Step 1: Expand candidate coordinate mapping**
- For each trusted left-click:
  - Compute candidates using `inputScale`:
    - origin-mul / origin-div
    - root-relative-mul / root-relative-div (scale around `.root` bounding rect)
  - Score each `elementFromPoint()` result and redirect only when the corrected target is “better”.

**Step 2: Verification (static)**
- Ensure the UI still works with `inputScale=1.0` (no correction).
- Ensure `inputScale=2.0` does not disable all clicks (fallback keeps original click when correction is not better).

---

### Task 3: Improve auto UI size for high DPI when `devicePixelRatio` is unreliable

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`

**Steps:**
1. In auto UI scaling, treat `inputScale` as a fallback “effective DPR” when `devicePixelRatio` is ~1.0 and `inputScale` is >1.0.
2. Keep manual override (1.0–3.0) as escape hatch.

---

### Task 4: Repackage release zip + docs update

**Files:**
- Modify: `AGENTS.md`

**Steps:**
1. Update troubleshooting section:
   - `ui.destroyOnClose` behavior and why it helps (FPS drop when UI closed).
   - Revised DPI correction notes (root-relative candidates).
2. Repackage:
   - `cd dist/CodexOfPowerNG && zip -r -FS \"../../releases/Codex of Power NG.zip\" .`
3. Expected: `releases/Codex of Power NG.zip` updated for MO2.

