# UI Scale + List UX Implementation Plan

> **For Codex:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to implement this plan task-by-task.

**Goal:** Make the PrismaUI menu comfortably large on 4K (and other resolutions) and improve list readability without reintroducing stutters.

**Architecture:** Use a CSS-driven `--uiScale` variable (auto + manual override) to scale the whole UI, widen the main panel (menu-first UX), and improve table readability (sticky header, better spacing). Keep inventory requests paginated and further reduce per-entry cost by using `InventoryEntryData::countDelta` instead of `PlayerCharacter::GetItemCount()` inside the inventory scan.

**Tech Stack:** HTML/CSS/JS (PrismaUI view), C++ (CommonLibSSE-NG), SKSE TaskInterface, PrismaUI API (CreateView/Show/Focus).

---

### Task 1: Make auto UI scaling larger + safer clamps

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`

**Steps:**
1. Increase auto scale curve for high resolutions (e.g. 4K) while keeping 1080p ~1.0.
2. Keep manual scale override (1.0–3.0) as the escape hatch.
3. Build: `cmake --build --preset wsl-release`

**Expected:** 4K users see a noticeably larger UI by default; manual mode can increase further.

---

### Task 2: Increase panel sizing + table readability (menu-first)

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`

**Steps:**
1. Increase `.root` max width and use more of the viewport height.
2. Add sticky table header and slightly larger base sizing for readability.
3. Build: `cmake --build --preset wsl-release`

**Expected:** Menu uses most of the screen and lists are easier to scan without scrolling.

---

### Task 3: Reduce inventory scan cost (avoid `GetItemCount()` in loop)

**Files:**
- Modify: `src/Registration.cpp`

**Steps:**
1. Replace `player->GetItemCount(obj)` with `entry->countDelta` when scanning `InventoryChanges->entryList`.
2. Keep existing safe-count logic (extra lists + equipped/favorited protection).
3. Build + install:
   - `cmake --build --preset wsl-release`
   - `cmake --install build/wsl-release`

**Expected:** Opening inventory page causes less main-thread hitching on large inventories.

---

### Task 4: Repackage the MO2-ready archive

**Files:**
- (none)

**Steps:**
1. `cd dist/CodexOfPowerNG`
2. `zip -r -FS "../../releases/Codex of Power NG.zip" .`
3. `ls -la "releases/Codex of Power NG.zip"`

**Expected:** `releases/Codex of Power NG.zip` updated with latest DLL + view changes.

