# Refactor + Decoupling Phase 1 Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Reduce coupling and maintenance risk in core runtime modules without changing gameplay/UI behavior.

**Architecture:** Keep current runtime behavior intact while extracting pure payload mapping helpers, removing duplicated inventory-safe-removal logic, and introducing a task scheduling port layer (`ITaskScheduler`) so SKSE task calls are not hardwired throughout feature code.

**Tech Stack:** C++23, CommonLibSSE-NG, SKSE task interface, nlohmann/json, Node test harness + lightweight C++ assertions.

### Task 1: Remove Safe-Removal Duplication in Registration

**Files:**
- Modify: `src/Registration.cpp`
- Reuse: `src/Inventory.cpp`, `include/CodexOfPowerNG/Inventory.h`

**Steps:**
1. Replace duplicated safe-count/extra-list traversal in quick register list builder with `Inventory::SelectSafeRemoval`.
2. Preserve current filtering behavior (`safeCount > 0` gate, protect favorites semantics).
3. Build and run existing tests to confirm no behavior drift.

### Task 2: Extract Prisma UI Payload Mapping (Refactor)

**Files:**
- Create: `src/PrismaUIPayloads.h`
- Create: `src/PrismaUIPayloadsInventory.cpp`
- Create: `src/PrismaUIPayloadsRewards.cpp`
- Modify: `src/PrismaUIManager.cpp`

**Steps:**
1. Move inventory/registered/reward payload JSON builders out of `PrismaUIManager.cpp` into dedicated helpers.
2. Keep function signatures simple (plain values + domain list structs).
3. Update manager call sites only; do not alter JS contract keys.

### Task 3: Introduce Task Scheduler Port (Decoupling)

**Files:**
- Create: `include/CodexOfPowerNG/TaskScheduler.h`
- Create: `src/TaskScheduler.cpp`
- Modify: `src/PrismaUIManager.cpp`
- Modify: `src/Events.cpp`

**Steps:**
1. Add `ITaskScheduler` interface + default SKSE-backed implementation.
2. Add `QueueMainTask` / `QueueUITask` helpers and test override hook.
3. Replace direct `SKSE::GetTaskInterface()` usage in `PrismaUIManager.cpp` and `src/Events.cpp` with scheduler port helpers while preserving current fallback behavior.

### Task 4: Verification and Regression Guard

**Files:**
- Existing test: `tests/events_notify_gate.test.cpp`
- Modify: `CMakeLists.txt` (new source files)

**Steps:**
1. Ensure new sources are included in build graph.
2. Run C++ regression assertion test (`events_notify_gate`).
3. Run Node tests (`tests/keycodes.test.cjs`, `tests/lang_ui.test.cjs`).
4. Run `cmake --build --preset wsl-release`.

## Status Update (2026-02-17)
- Task 2 payload split was implemented as two concrete modules:
  - `src/PrismaUIPayloadsInventory.cpp` (inventory/registered payload builders)
  - `src/PrismaUIPayloadsRewards.cpp` (reward totals/formatting payload builders)
- Additional follow-up modularization also landed:
  - Registration internals: `src/RegistrationInternalMaps.cpp`, `src/RegistrationInternalTcc.cpp`
  - Serialization flow: `src/SerializationSave.cpp`, `src/SerializationLoad.cpp` with `src/Serialization.cpp` as callback install entrypoint
