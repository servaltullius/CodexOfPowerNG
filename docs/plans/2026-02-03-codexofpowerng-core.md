# CodexOfPowerNG Core Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to implement this plan task-by-task.

**Goal:** Implement Codex of Power NG’s core gameplay loop (register/record/reward) fully in an SKSE DLL (CommonLibSSE-NG flatrim) with a Prisma UI (HTML/JS) frontend, including persistence, safety filters, excludes, and reward refund.

**Architecture (high-level):**
- **Runtime:** SKSE DLL (C++23) owns all logic/state; no Papyrus/JContainers/MCM.
- **UI:** One PrismaView (`CreateView("codexofpowerng/index.html")`) with a JSON-based async bridge.
- **Persistence:**
  - **Per-save:** SKSE co-save (registered, blocked, notified, reward totals).
  - **Global:** JSON settings file under `Data/SKSE/Plugins/CodexOfPowerNG/settings.json`.
- **Safety:** never remove equipped / hotkeyed items (when protection enabled), hard-block keys/lockpicks/gold/claws, auto-block on remove failure, merge exclude files.

**Tech Stack:**
- CMake + vcpkg manifest mode
- CommonLibSSE (`commonlibsse-ng-flatrim`)
- Prisma UI plugin API (`external/PrismaUI/PrismaUI_API.h`)
- JSON parsing via `nlohmann-json` (vcpkg)

---

### Task 1: Add settings + localization (global files)

**Files:**
- Modify: `vcpkg.json`
- Create: `include/CodexOfPowerNG/Config.h`
- Create: `src/Config.cpp`
- Create: `include/CodexOfPowerNG/L10n.h`
- Create: `src/L10n.cpp`
- Create: `SKSE/Plugins/CodexOfPowerNG/settings.json`
- Create: `SKSE/Plugins/CodexOfPowerNG/lang/en.json`
- Create: `SKSE/Plugins/CodexOfPowerNG/lang/ko.json`

**Steps:**
1. Add `nlohmann-json` to vcpkg dependencies.
2. Implement `Settings` load/save with defaults and sane clamping.
3. Implement `L10n` that loads `en.json`/`ko.json`, supports `auto` mode, and provides `T("path.key", fallback)`.
4. Make hotkey configurable (default F4) and use it in input sink.

**Verify:**
- `cmake --preset wsl-release && cmake --build --preset wsl-release` succeeds.

---

### Task 2: Expand runtime state + co-save schema

**Files:**
- Modify: `include/CodexOfPowerNG/State.h`
- Modify: `src/State.cpp`
- Modify: `include/CodexOfPowerNG/Constants.h`
- Modify: `include/CodexOfPowerNG/Serialization.h`
- Modify: `src/Serialization.cpp`

**Steps:**
1. Replace `registeredItems` set with a map `FormID -> group`.
2. Add `blockedItems`, `notifiedItems`, and `rewardTotals` (by ActorValue) to state.
3. Add record types and versioning for new data; keep backward-compatible read within NG if possible.
4. Apply `ResolveFormID` remapping for all stored FormIDs on load.

**Verify:**
- Build succeeds; serialization callbacks registered.

---

### Task 3: Implement inventory scanning + safe removal

**Files:**
- Create: `include/CodexOfPowerNG/Inventory.h`
- Create: `src/Inventory.cpp`
- Create: `include/CodexOfPowerNG/Registration.h`
- Create: `src/Registration.cpp`

**Steps:**
1. Implement `GetDiscoveryGroup(TESBoundObject*)` and hard-block checks (keys/lockpick/gold/claws).
2. Implement exclude/variant map loading:
   - `exclude_map.json`, `exclude_user.json`, `exclude_patch_01..32.json`
   - `variant_map.json` (optional)
3. Implement “safe stack” selection by scanning `InventoryEntryData::extraLists` and skipping worn/hotkey stacks.
4. Implement `TryRegister(FormID)`:
   - Compute register key (optional normalization).
   - Validate not excluded/unnamed/already registered.
   - Remove 1 item (with ExtraDataList when possible).
   - Auto-block on removal failure.
   - Persist in state.

**Verify:**
- Build succeeds.

---

### Task 4: Rewards + refund

**Files:**
- Create: `include/CodexOfPowerNG/Rewards.h`
- Create: `src/Rewards.cpp`

**Steps:**
1. Port the reward tables and weights from `NEXUS_DESCRIPTION.md` exactly (ActorValue deltas).
2. Apply reward every N registrations with multiplier scaling and ShoutRecoveryMult clamping.
3. Record totals per ActorValue and expose for UI.
4. Implement “Reset rewards (refund)” that subtracts totals and clears only reward state.

**Verify:**
- Build succeeds.

---

### Task 5: Loot notification event sink

**Files:**
- Create: `include/CodexOfPowerNG/Events.h`
- Create: `src/Events.cpp`
- Modify: `src/main.cpp`

**Steps:**
1. Add `TESContainerChangedEvent` sink.
2. On item added to player: if discoverable + unregistered + not notified → `DebugNotification(...)`.
3. Respect settings toggles and avoid repeated notifications using `notifiedItems`.

**Verify:**
- Build succeeds.

---

### Task 6: Prisma UI bridge + UI view

**Files:**
- Modify: `docs/contracts/prismaui-js-api.md`
- Modify: `src/PrismaUIManager.cpp`
- Modify: `include/CodexOfPowerNG/PrismaUIManager.h`
- Modify: `PrismaUI/views/codexofpowerng/index.html`

**Steps:**
1. Define async JS API:
   - JS→C++: `copng_requestState`, `copng_requestInventory`, `copng_registerItem`, `copng_requestRewards`, `copng_saveSettings`, `copng_refundRewards`
   - C++→JS: `copng_setState`, `copng_setInventory`, `copng_setRewards`, `copng_toast`
2. Implement JSON payload validation and error reporting.
3. Build a functional UI with tabs: Quick Register / Registered / Rewards / Settings.

**Verify:**
- `cmake --install build/wsl-release` produces `dist/CodexOfPowerNG/` with DLL + Prisma view + `SKSE/Plugins/CodexOfPowerNG/*`.

---

### Task 7: Packaging + AGENTS.md update

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `AGENTS.md`

**Steps:**
1. Ensure `install()` includes `SKSE/Plugins/CodexOfPowerNG/` (settings/lang/exclude/variant).
2. Document reproducible build steps (Windows + WSL cross) and note the lld-link runtime caveat from CommonLib docs.

**Verify:**
- Fresh build + install works; `AGENTS.md` contains exact commands and expected outputs.
