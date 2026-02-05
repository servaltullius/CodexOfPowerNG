# Refactor + Decoupling Phase 4 Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Continue priority refactoring by extracting registration rule logic into a dedicated module and making registration state access swappable via interface.

**Architecture:** Add `RegistrationRules` for reusable rule decisions (discovery group, intrinsic exclusion, variant-step validity). Upgrade `RegistrationStateStore` from function-only adapter to interface-backed port with default runtime implementation plus testing override hook. Keep runtime behavior unchanged.

**Tech Stack:** C++23, CommonLibSSE-NG types, RuntimeState singleton adapter, CMake `wsl-release`, existing C++/Node regression checks.

### Task 1: Extract RegistrationRules Module

**Files:**
- Create: `include/CodexOfPowerNG/RegistrationRules.h`
- Create: `src/RegistrationRules.cpp`
- Modify: `src/Registration.cpp`

**Step 1:** Add rule API for intrinsic exclusion, form-type→group mapping, and variant-step validity.

**Step 2:** Replace duplicated dragon claw/type switch checks in `Registration.cpp` with `RegistrationRules` calls.

**Step 3:** Keep numeric group semantics (`0..5`, unknown `255`) unchanged.

### Task 2: Interface-Backed RegistrationStateStore

**Files:**
- Modify: `include/CodexOfPowerNG/RegistrationStateStore.h`
- Modify: `src/RegistrationStateStore.cpp`

**Step 1:** Define `IRegistrationStateStore` interface.

**Step 2:** Implement default runtime-backed store and testing override setter.

**Step 3:** Keep existing wrapper function API stable to avoid broad call-site churn.

### Task 3: Build Graph + Verification

**Files:**
- Modify: `CMakeLists.txt`

**Step 1:** Add `RegistrationRules` source/header.

**Step 2:** Run C++ regression test: `events_notify_gate`.

**Step 3:** Run Node tests: `keycodes`, `lang_ui`.

**Step 4:** Build with `cmake --build --preset wsl-release` (`VCPKG_ROOT=/mnt/c/Users/kdw73/vcpkg`).

### Task 4: Packaging

**Files:**
- Update artifact: `releases/Codex of Power NG.zip`

**Step 1:** Install to `dist`.

**Step 2:** Repack MO2 zip.

**Step 3:** Report final artifact hash and changed files.
