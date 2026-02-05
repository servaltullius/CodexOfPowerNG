# Inventory UI Performance Implementation Plan

> **For Codex:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to implement this plan task-by-task.

**Goal:** Remove multi-second stutters when opening/refreshing the inventory list in the PrismaUI view.

**Architecture:** Keep all Skyrim object access on the main thread, but avoid building/rendering a full inventory table in one shot. Add pagination + lightweight rendering in JS, and replace `PlayerCharacter::GetInventory()` (expensive copy) with `InventoryChanges::VisitInventory()` iteration.

**Tech Stack:** C++ (CommonLibSSE-NG), SKSE TaskInterface, PrismaUI InteropCall, HTML/JS view.

---

### Task 1: Add basic instrumentation for inventory refresh

**Files:**
- Modify: `src/PrismaUIManager.cpp`

**Steps:**
1. Add timing logs around inventory list build and JSON send (count + elapsed ms).
2. Build: `cmake --build --preset wsl-release`

**Expected:** Logs show `inventory: built N items in Xms` so we can correlate stutters.

---

### Task 2: Add paginated inventory API (C++ ↔ JS contract)

**Files:**
- Modify: `src/PrismaUIManager.cpp`
- Modify: `include/CodexOfPowerNG/Registration.h`
- Modify: `src/Registration.cpp`
- Modify: `PrismaUI/views/codexofpowerng/index.html`

**Steps:**
1. Extend `copng_requestInventory` payload to accept `{ page, pageSize }`.
2. Change `copng_setInventory` payload to return `{ page, pageSize, total, items: [...] }`.
3. Update JS to render only the current page, and add Prev/Next controls.
4. Build + install:
   - `cmake --build --preset wsl-release`
   - `cmake --install build/wsl-release`

**Expected:** No large DOM table generation; inventory refresh is fast and stable.

---

### Task 3: Optimize inventory enumeration (avoid `GetInventory()` map copy)

**Files:**
- Modify: `src/Registration.cpp`

**Steps:**
1. Replace `PlayerCharacter::GetInventory()` usage with `InventoryChanges::VisitInventory(visitor)`.
2. Compute total count using `countDelta + sum(extraList->GetCount())`.
3. Reduce per-item locking by snapshotting `registeredItems` / `blockedItems` into local sets before iteration.
4. Build + install:
   - `cmake --build --preset wsl-release`
   - `cmake --install build/wsl-release`

**Expected:** Inventory scan time drops significantly, even with large inventories.

---

### Task 4: Repackage release + update repo instructions

**Files:**
- Modify: `AGENTS.md` (document pagination + perf notes)

**Steps:**
1. Update the MO2-ready archive:
   - `cd dist/CodexOfPowerNG`
   - `zip -r -FS "../../releases/Codex of Power NG.zip" .`
2. Verify archive path exists:
   - `ls -la "releases/Codex of Power NG.zip"`

**Expected:** `releases/Codex of Power NG.zip` installs in MO2 and UI no longer causes stutter.

