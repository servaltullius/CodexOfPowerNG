# Refactor + Decoupling Phase 2 Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Split registration map I/O/parsing from registration domain logic so map-loading behavior is easier to test and maintain without gameplay changes.

**Architecture:** Introduce a dedicated `RegistrationMaps` module that owns JSON file parsing and map assembly, while `Registration.cpp` keeps runtime decisions (discoverability, registration, inventory filtering). The new module receives a resolver callback so form-resolution remains runtime-specific and replaceable in tests.

**Tech Stack:** C++23, CommonLibSSE-NG, nlohmann/json, CMake presets (`wsl-release`), Node test harness + lightweight C++ assertions.

### Task 1: Add RegistrationMaps Decoupling Module

**Files:**
- Create: `include/CodexOfPowerNG/RegistrationMaps.h`
- Create: `src/RegistrationMaps.cpp`

**Step 1:** Define map-loading API (`Paths`, `Data`, resolver callback type).

**Step 2:** Implement exclude/variant JSON parsers in `RegistrationMaps.cpp`.

**Step 3:** Keep error handling non-throwing and preserve warning logs on malformed JSON.

### Task 2: Rewire Registration Runtime to New Module

**Files:**
- Modify: `src/Registration.cpp`

**Step 1:** Replace inline JSON file loading helpers with `RegistrationMaps::LoadFromDisk(...)`.

**Step 2:** Keep runtime resolver in `Registration.cpp` (TESDataHandler + local form-id conversion).

**Step 3:** Preserve behavior for exclude patches (`exclude_patch_01..N`), variant chains, and logs.

### Task 3: Build Graph + Verification

**Files:**
- Modify: `CMakeLists.txt`

**Step 1:** Add `src/RegistrationMaps.cpp` and header to build sources.

**Step 2:** Run C++ regression test: `events_notify_gate`.

**Step 3:** Run Node tests: `tests/keycodes.test.cjs`, `tests/lang_ui.test.cjs`.

**Step 4:** Run `cmake --build --preset wsl-release` with `VCPKG_ROOT=/mnt/c/Users/kdw73/vcpkg`.

### Task 4: Packaging

**Files:**
- Update artifact: `releases/Codex of Power NG.zip`

**Step 1:** Run install step if needed.

**Step 2:** Repack `dist/CodexOfPowerNG` into MO2-ready zip.

**Step 3:** Report changed files and verification evidence.
