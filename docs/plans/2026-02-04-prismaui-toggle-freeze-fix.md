# PrismaUI Toggle Freeze Fix Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Fix the in-game freeze/hang when toggling the PrismaUI view (F4) so the UI can be opened/closed safely at main menu + in-game.

**Architecture:** Make UI state transitions (create/show/focus/hide/unfocus) happen on a safe execution point (SKSE TaskInterface) instead of directly inside the input event sink or PrismaUI DOM-ready callback. Prefer PrismaUI’s `Show/Hide` API and make focus behavior configurable (disable FocusMenu by default).

**Tech Stack:** C++ (CommonLibSSE-NG), SKSE, PrismaUI, vcpkg manifest + CMake presets.

---

### Task 1: Audit current UI toggle flow + thread context

**Files:**
- Modify: `src/PrismaUIManager.cpp`
- Modify: `src/main.cpp`
- Modify: `src/InputEventSink.cpp` (or wherever the hotkey sink lives)

**Steps:**
1. Identify where `ToggleUI()` is called (input event sink vs task).
2. Identify what thread the PrismaUI DOM-ready callback runs on (assume NOT safe for game/UI ops).
3. Confirm current API usage (`SetVisible` vs `Show/Hide`, direct `Focus` calls, focus menu parameters).

**Verification:**
- Build: `cmake --build --preset wsl-release`

---

### Task 2: Make hotkey toggle non-reentrant (queue to SKSE TaskInterface)

**Files:**
- Modify: `src/InputEventSink.cpp` (or existing hotkey handler)
- Modify: `include/CodexOfPowerNG/PrismaUIManager.h`
- Modify: `src/PrismaUIManager.cpp`

**Steps:**
1. Change hotkey handler to only enqueue a toggle request (no direct Focus/Show calls).
2. Implement `PrismaUIManager::RequestToggle()`:
   - atomically store “toggle requested”
   - queue a single `SKSE::TaskInterface::AddTask()` job to service the request
3. Keep existing debounce logic, but ensure the actual toggle work is executed only from the queued task.

**Verification:**
- Build: `cmake --build --preset wsl-release`

---

### Task 3: Use PrismaUI `Show/Hide` and delay Focus to the next frame

**Files:**
- Modify: `src/PrismaUIManager.cpp`

**Steps:**
1. Replace `SetVisible(view, true/false)` with:
   - open: `Show(view)`
   - close: `Hide(view)` (note: Hide auto-unfocuses if focused)
2. When opening:
   - If the view is not created yet, call `CreateView(path, onDomReady)` and return.
   - In `onDomReady`, set `domReady=true` and **queue** a main-thread job that performs `Show` and then queues another job for `Focus`.
3. When focusing:
   - Default to `disableFocusMenu=true` (workaround for suspected FocusMenu-related hangs).
   - Keep `pauseGame=false` by default.

**Verification:**
- Build: `cmake --build --preset wsl-release`

---

### Task 4: Add runtime-configurable focus settings (no rebuild needed for testing)

**Files:**
- Modify: `src/Settings.cpp` (or equivalent)
- Modify: `SKSE/Plugins/CodexOfPowerNG/settings.json` template under `dist/` packaging flow

**Steps:**
1. Add settings:
   - `ui.disableFocusMenu` (default true)
   - `ui.pauseGame` (default false)
   - `ui.focusDelayMs` (default 0 or 1 frame)
2. Log the resolved focus settings when opening the UI.

**Verification:**
- Build: `cmake --build --preset wsl-release`

---

### Task 5: Package MO2-ready release zip

**Files:**
- Modify (generated): `dist/CodexOfPowerNG/**`
- Modify (generated): `releases/Codex of Power NG.zip`

**Steps:**
1. Install: `cmake --install build/wsl-release`
2. Zip: `cd dist/CodexOfPowerNG && zip -r -FS \"../../releases/Codex of Power NG.zip\" .`

**Verification:**
- Inspect: `unzip -l \"releases/Codex of Power NG.zip\" | rg \"SKSE/Plugins/CodexOfPowerNG\\.dll\"`

---

### Task 6: Update repo guidance (AGENTS + docs)

**Files:**
- Modify: `AGENTS.md`

**Steps:**
1. Document build + package commands (presets, install, zip).
2. Document runtime expectations:
   - No ESP required (pure SKSE plugin + PrismaUI view)
   - PrismaUI dependency + recommended keyboard/input fixes
   - Focus settings + how to tweak `settings.json` for debugging

**Verification:**
- N/A (documentation only)

