# Corpse Explosion (On-Kill VFX Proc) Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to implement this plan task-by-task.

**Goal:** When the player kills an enemy, roll a configurable proc chance and, if it procs, play a **safe vanilla VFX** on the corpse (no damage/explosion gameplay).

**Architecture:** Register a `TESDeathEvent` sink via `RE::ScriptEventSourceHolder`. When `event->dead == true` and `actorKiller == player`, queue a main-thread task that applies a short-lived VFX to `actorDying` using `TESObjectREFR::InstantiateHitArt` (preferred) or `InstantiateHitShader` (fallback). Gate with `gameActive`, grace period after load, and cooldown to avoid spam/CTD risks.

**Tech Stack:** CommonLibSSE-NG + SKSE tasks, PrismaUI view (HTML/JS), JSON settings (`Data/SKSE/Plugins/CodexOfPowerNG/settings.json`).

---

## Decisions (locked)

- Trigger: **A) Player kill** (TESDeathEvent) ✅
- Visual-only: **No damage / no gameplay explosion** ✅

---

## Task 1: Add settings schema (C++ + JSON)

**Files:**
- Modify: `include/CodexOfPowerNG/Config.h`
- Modify: `src/Config.cpp`
- Modify: `src/PrismaUIManager.cpp`
- Modify: `SKSE/Plugins/CodexOfPowerNG/settings.json` (template shipped in dist)

**Steps:**
1. Add new settings fields (enable/chance/cooldown + VFX mode + optional editorID override).
2. Load/save to settings.json under a new object key (e.g. `corpseExplosion`).
3. Extend PrismaUI settings bridge (`SettingsToJson/SettingsFromJson/SettingsEquivalent`).

**Verification:**
- Build compiles.
- Open UI → Settings → values round-trip (get/set/save).

---

## Task 2: Add UI controls (Prisma UI view)

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`
- Modify: `SKSE/Plugins/CodexOfPowerNG/lang/en.json`
- Modify: `SKSE/Plugins/CodexOfPowerNG/lang/ko.json`

**UI Controls (minimum):**
- Toggle: Enable on-kill proc
- Slider/number: Chance (%)
- Number: Cooldown (ms)
- Dropdown: VFX mode (`auto|art|shader|none`)
- Optional text: VFX editorID override (advanced)

**Verification:**
- UI renders at 4K scaling (inherits existing scale system).
- Saving settings does not stutter (uses existing background save queue).

---

## Task 3: Implement TESDeathEvent sink + proc logic

**Files:**
- Modify: `src/Events.cpp`
- Modify (if needed): `include/CodexOfPowerNG/Events.h`

**Steps:**
1. Add `BSTEventSink<TESDeathEvent>` with strict guards:
   - `event && event->dead`
   - `g_gameReady == true`
   - `RE::Main::GetSingleton()->gameActive == true`
   - `actorKiller == player`
2. Debounce after load (reuse existing `kDebounceMs` pattern).
3. Apply chance + cooldown (store last-proc timestamp).
4. Queue main-thread task to apply VFX to the dying actor reference handle.

**Verification:**
- No main menu freeze (events gated by `gameActive` and debounce).
- Logging shows: proc rolled, proc fired, VFX applied/fallback used.

---

## Task 4: Safe vanilla VFX selection (auto + override)

**Files:**
- Modify: `src/Events.cpp` (or add `src/Vfx.cpp` + header if it grows)

**Approach:**
- Preferred: `BGSArtObject` via `TESForm::LookupByEditorID` when override provided.
- Auto mode: scan `TESForm::GetAllFormsByEditorID()` for `BGSArtObject` candidates whose editorID/model path looks explosion-like; cache first good match.
- Fallback: `TESEffectShader` selection with similar rules.
- Hard fallback: `DebugNotification` only (still indicates proc).

**Verification:**
- If override editorID is invalid, auto/fallback still works and does not crash.
- VFX is visible and clearly indicates “proc happened”.

---

## Task 5: Build + dist + release zip

**Files:**
- Build (WSL preset): `cmake --build --preset wsl-release`
- Install: `cmake --install build/wsl-release`
- Update zip: `cd dist/CodexOfPowerNG && zip -r -FS \"../../releases/Codex of Power NG.zip\" .`

**Verification:**
- `dist/CodexOfPowerNG/SKSE/Plugins/CodexOfPowerNG.dll` updated
- `releases/Codex of Power NG.zip` updated (MO2-ready)

