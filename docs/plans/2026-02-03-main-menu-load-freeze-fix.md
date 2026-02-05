# CodexOfPowerNG Main-Menu Load Freeze Fix — Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to implement this plan task-by-task.

**Goal:** Prevent freezes/hangs when clicking **Load** from the main menu with CodexOfPowerNG enabled (Skyrim SE 1.5.97 + AE 1.6+).

**Architecture (high-level):**
- **Pre-warm data:** Load/parse exclude + variant maps at `kDataLoaded` (main menu) so we don’t do JSON IO + `TESDataHandler::LookupForm` work during save-load or event storms.
- **Guard event sinks:** `TESContainerChangedEvent` processing should early-exit while the loading menu is open / game not active, and avoid expensive operations (e.g., `PlayerCharacter::GetInventory()`).
- **Optional debounce:** Ignore the first few seconds after a load/new game to avoid “inventory restore” storms.

**Tech Stack:**
- CommonLibSSE-NG (flatrim)
- SKSE MessagingInterface (`kDataLoaded`, `kPostLoadGame`, `kNewGame`)
- Prisma UI plugin API (no change required for this fix)

---

### Task 1: Pre-warm exclude/variant maps at `kDataLoaded`

**Files:**
- Modify: `include/CodexOfPowerNG/Registration.h`
- Modify: `src/Registration.cpp`
- Modify: `src/main.cpp`

**Step 1: Implement `Registration::Warmup()`**
- Implement as a thin wrapper around the existing internal map loader (idempotent).

**Step 2: Call `Registration::Warmup()` at `SKSE::MessagingInterface::kDataLoaded`**
- This should run at the main menu after all forms are loaded.

**Verify:**
- Run: `cmake --build --preset wsl-release`
- Expected: command succeeds (exit code 0)

---

### Task 2: Make `TESContainerChangedEvent` sink safe during load

**Files:**
- Modify: `src/Events.cpp`
- Modify: `src/main.cpp`

**Step 1: Add early guards**
- Skip processing if:
  - `RE::UI::GetSingleton()->IsMenuOpen(RE::LoadingMenu::MENU_NAME)` is true, OR
  - `RE::Main::GetSingleton()->gameActive` is false

**Step 2: Remove expensive quest-item check**
- Remove `player->GetInventory()` usage from the event handler (avoid deadlocks / re-entrancy during load).

**Step 3 (optional): Add small post-load debounce**
- When receiving `kPostLoadGame` / `kNewGame`, record a timestamp and ignore container events for ~3–5 seconds.

**Verify:**
- Run: `cmake --build --preset wsl-release`
- Expected: command succeeds (exit code 0)

---

### Task 3: Rebuild dist + update release zip

**Files:**
- (generated) `dist/CodexOfPowerNG/**`
- Modify: `releases/Codex of Power NG.zip`

**Steps:**
1. Install to dist:
   - Run: `cmake --install build/wsl-release`
2. Recreate/refresh the release zip from `dist/CodexOfPowerNG/`:
   - Run: `cd dist/CodexOfPowerNG && zip -r -FS \"../../releases/Codex of Power NG.zip\" .`

**Verify:**
- Run: `unzip -l \"releases/Codex of Power NG.zip\" | rg -n \"CodexOfPowerNG\\.dll|PrismaUI/views\"`
- Expected: shows the DLL under `SKSE/Plugins/` and the Prisma UI view under `PrismaUI/views/`

---

### Task 4: Document the fix + build steps in `AGENTS.md`

**Files:**
- Modify: `AGENTS.md`

**Steps:**
1. Add a troubleshooting note for “freeze on load” and what guards exist.
2. Ensure `wsl-release` build + install + zip commands are included and known-good.

**Verify:**
- `AGENTS.md` contains exact commands for:
  - configure/build
  - install to `dist/`
  - zip to `releases/`

