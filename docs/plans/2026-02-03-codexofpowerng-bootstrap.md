# CodexOfPowerNG Bootstrap Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to implement this plan task-by-task.

**Goal:** Scaffold CodexOfPowerNG as a new SKSE plugin + Prisma UI view mod (SE 1.5.97 + AE 1.6+ single DLL), with a minimal JS↔C++ bridge and co-save persistence skeleton.

**Architecture (high-level):**
- **Single PrismaView per plugin** (`CreateView("codexofpowerng/index.html")`)
- **JS → C++** via `RegisterJSListener(view, "copng_*", callback)` (payload is `const char*`, treat as JSON string)
- **C++ → JS** via `Invoke()` for flexible calls and `InteropCall()` for high-frequency calls (string arg only)
- **Persistence** via `SKSE SerializationInterface` co-save + `ResolveFormID` on load

**Tech Stack:**
- C++23, CMake, vcpkg (manifest mode)
- CommonLibSSE-NG (`commonlibsse-ng-flatrim`)
- Prisma UI Framework (runtime dependency) + `PrismaUI_API.h` for plugin API

---

### Task 1: Repo packaging skeleton

**Files:**
- Create: `AGENTS.md`
- Create: `PrismaUI/views/codexofpowerng/index.html`
- Create: `docs/contracts/prismaui-js-api.md`

**Steps:**
1. Create Prisma UI view folder under `PrismaUI/views/codexofpowerng/`.
2. Add a minimal `index.html` that can call `window.copng_*` functions.
3. Document the initial JS API contract and payload formats.

**Verify:**
- Manual: confirm Prisma UI base dir expectation: `Data/PrismaUI/views/<viewPath>`.

---

### Task 2: vcpkg manifest + CMake scaffolding (Windows)

**Files:**
- Create: `vcpkg.json`
- Create: `vcpkg-configuration.json`
- Create: `CMakeLists.txt`
- Create: `CMakePresets.json`

**Steps:**
1. Configure vcpkg registry (Color-Glass) with a known-good baseline.
2. Add dependencies: `commonlibsse-ng-flatrim` (+ explicit `spdlog`).
3. Create CMake target via `add_commonlibsse_plugin(...)`.
4. Add `install()` rules to build a distributable `Data/`-shaped folder.

**Verify (Windows):**
- Run: `cmake --preset windows-release`
- Run: `cmake --build --preset windows-release`
- Run: `cmake --install --preset windows-release`
- Expected: `dist/CodexOfPowerNG/SKSE/Plugins/CodexOfPowerNG.dll` and `dist/CodexOfPowerNG/PrismaUI/views/codexofpowerng/index.html`

---

### Task 3: SKSE plugin skeleton (Prisma UI bridge + hotkey)

**Files:**
- Create: `external/PrismaUI/PrismaUI_API.h`
- Create: `include/CodexOfPowerNG/Constants.h`
- Create: `include/CodexOfPowerNG/State.h`
- Create: `include/CodexOfPowerNG/PrismaUIManager.h`
- Create: `src/main.cpp`
- Create: `src/State.cpp`
- Create: `src/PrismaUIManager.cpp`

**Steps:**
1. Initialize logging (spdlog file sink).
2. Register SKSE Messaging listener:
   - On `kPostLoad`: request Prisma UI API via `PRISMA_UI_API::RequestPluginAPI()`.
   - On `kDataLoaded`: create view and register JS listeners.
   - On `kInputLoaded`: register an input event sink for hotkey toggle (default: F4).
3. Implement `Toggle()`:
   - If closed: `Show()` + `Focus()`
   - If open: `Unfocus()` + `Hide()`

**Verify:**
- Build succeeds.
- In-game: pressing F4 toggles UI if Prisma UI is installed.

---

### Task 4: Co-save serialization skeleton

**Files:**
- Create: `include/CodexOfPowerNG/Serialization.h`
- Create: `src/Serialization.cpp`

**Steps:**
1. Register callbacks: `SetUniqueID`, `SetSaveCallback`, `SetLoadCallback`, `SetRevertCallback`.
2. Save record(s) (v1):
   - Registered item FormIDs (`REGI`)
3. Load record(s):
   - Read FormIDs and remap via `ResolveFormID`.
4. Revert clears runtime state.

**Verify (in-game):**
- Save → exit → load and confirm state is restored (by asking JS to query state via `copng_requestState`).

---

## Notes / Dependencies (runtime)
- Prisma UI requires: SKSE, Address Library for SKSE Plugins, and Media Keys Fix SKSE (keyboard input).
- View base directory: `Skyrim/Data/PrismaUI/views/`

