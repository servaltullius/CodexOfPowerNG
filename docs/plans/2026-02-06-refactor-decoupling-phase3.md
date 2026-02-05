# Refactor + Decoupling Phase 3 Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Decouple registration domain logic from raw runtime state storage by introducing a state-access port and centralizing mutation/snapshot operations.

**Architecture:** Keep `Registration.cpp` focused on gameplay rules while moving all `GetState()` lock/map/set access into a dedicated adapter module (`RegistrationStateStore`). This keeps synchronization and storage details in one place and reduces duplication in registration workflows.

**Tech Stack:** C++23, CommonLibSSE-NG runtime state singleton, CMake (`wsl-release`), existing Node/C++ regression checks.

### Task 1: Add RegistrationStateStore Port

**Files:**
- Create: `include/CodexOfPowerNG/RegistrationStateStore.h`
- Create: `src/RegistrationStateStore.cpp`

**Step 1:** Define snapshot and mutation/query API for blocked/registered data.

**Step 2:** Implement thread-safe wrappers around `GetState()`.

**Step 3:** Preserve existing behavior (legacy registered fallback, blocked pair insert).

### Task 2: Rewire Registration Domain

**Files:**
- Modify: `src/Registration.cpp`

**Step 1:** Replace direct `GetState()` access with `RegistrationStateStore` API.

**Step 2:** Replace duplicated blocked insertion and registered count logic with single helper calls.

**Step 3:** Keep quick-list behavior and registration rules unchanged.

### Task 3: Build Graph + Verification

**Files:**
- Modify: `CMakeLists.txt`

**Step 1:** Add new port source/header to build graph.

**Step 2:** Run C++ regression test (`events_notify_gate`).

**Step 3:** Run Node tests (`keycodes`, `lang_ui`).

**Step 4:** Build with `cmake --build --preset wsl-release` (`VCPKG_ROOT=/mnt/c/Users/kdw73/vcpkg`).

### Task 4: Packaging

**Files:**
- Update artifact: `releases/Codex of Power NG.zip`

**Step 1:** Install latest build to `dist`.

**Step 2:** Repack release zip for MO2 test.

**Step 3:** Report updated artifact hash.
