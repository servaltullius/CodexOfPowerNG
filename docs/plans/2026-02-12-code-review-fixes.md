# Code Review Fixes Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Fix all code review findings except I2 (reward caps — intentional by design).

**Architecture:** C++23 SKSE plugin. All changes are in C++ source files. No local compilation available (MSVC build on Windows); changes must be reviewed manually for correctness. JS tests verify UI-side behavior.

**Tech Stack:** C++23, CommonLibSSE-NG, nlohmann/json, spdlog, SKSE serialization API

---

## Constraint: No Local C++ Build

This project cannot compile locally. Every C++ change must be:
1. Written with exact code (no "add something here")
2. Self-reviewed against compiler rules (type mismatches, include order, scope)
3. Verified via `node tests/*.test.cjs` for any JS-side impact

---

### Task 1: C2 — Fix Serialization Partial Save

**Severity:** Critical

**Problem:** In `Save()`, each `return` on write failure exits the entire function. If REGI fails to write partway through, BLCK/NTFY/RWDS records are never saved — losing data that was perfectly valid.

**Files:**
- Modify: `src/Serialization.cpp:41-125`

**Step 1: Refactor each record block into a lambda**

Replace the `Save` function body with per-record lambdas so `return` only exits the current record, not the whole function:

```cpp
void Save(SKSE::SerializationInterface* a_intfc) noexcept
{
    auto& state = GetState();
    std::scoped_lock lock(state.mutex);

    // Each lambda writes one record type. A `return` inside only aborts
    // that record — the remaining records still get a chance to save.

    auto writeRegistered = [&]() {
        if (!a_intfc->OpenRecord(kRecordRegisteredItems, kSerializationVersion)) {
            SKSE::log::error("Failed to open co-save record REGI");
            return;
        }

        const std::uint32_t count = static_cast<std::uint32_t>(state.registeredItems.size());
        if (!a_intfc->WriteRecordData(count)) {
            SKSE::log::error("Failed to write registered count");
            return;
        }

        for (const auto& [formId, group] : state.registeredItems) {
            if (!a_intfc->WriteRecordData(formId) || !a_intfc->WriteRecordData(group)) {
                SKSE::log::error("Failed to write registered entry");
                return;
            }
        }
    };

    auto writeBlocked = [&]() {
        if (!a_intfc->OpenRecord(kRecordBlockedItems, kSerializationVersion)) {
            SKSE::log::error("Failed to open co-save record BLCK");
            return;
        }
        const std::uint32_t count = static_cast<std::uint32_t>(state.blockedItems.size());
        if (!a_intfc->WriteRecordData(count)) {
            SKSE::log::error("Failed to write blocked count");
            return;
        }
        for (auto formId : state.blockedItems) {
            if (!a_intfc->WriteRecordData(formId)) {
                SKSE::log::error("Failed to write blocked formId");
                return;
            }
        }
    };

    auto writeNotified = [&]() {
        if (!a_intfc->OpenRecord(kRecordNotifiedItems, kSerializationVersion)) {
            SKSE::log::error("Failed to open co-save record NTFY");
            return;
        }
        const std::uint32_t count = static_cast<std::uint32_t>(state.notifiedItems.size());
        if (!a_intfc->WriteRecordData(count)) {
            SKSE::log::error("Failed to write notified count");
            return;
        }
        for (auto formId : state.notifiedItems) {
            if (!a_intfc->WriteRecordData(formId)) {
                SKSE::log::error("Failed to write notified formId");
                return;
            }
        }
    };

    auto writeRewards = [&]() {
        if (!a_intfc->OpenRecord(kRecordRewards, kSerializationVersion)) {
            SKSE::log::error("Failed to open co-save record RWDS");
            return;
        }
        const std::uint32_t count = static_cast<std::uint32_t>(state.rewardTotals.size());
        if (!a_intfc->WriteRecordData(count)) {
            SKSE::log::error("Failed to write reward count");
            return;
        }

        for (const auto& [av, total] : state.rewardTotals) {
            const auto avRaw = static_cast<std::uint32_t>(av);
            if (!a_intfc->WriteRecordData(avRaw) || !a_intfc->WriteRecordData(total)) {
                SKSE::log::error("Failed to write reward entry");
                return;
            }
        }
    };

    writeRegistered();
    writeBlocked();
    writeNotified();
    writeRewards();
}
```

**Step 2: Self-review**

Verify:
- Each lambda captures `&` (correct — `a_intfc` and `state` are both in outer scope)
- `return` inside a lambda only exits the lambda, not the enclosing function
- No change to the Load function (it already handles partial/corrupt records gracefully)

**Step 3: Commit**

```bash
git add src/Serialization.cpp
git commit -m "fix(serialization): continue saving remaining records on partial write failure

C2: Each record type now writes inside its own lambda so a failure in one
record (e.g. REGI) does not prevent the remaining records (BLCK, NTFY, RWDS)
from being saved."
```

---

### Task 2: C1 — Fix Detached Threads in PrismaUIManager

**Severity:** Critical

**Problem:** Three `.detach()` calls (lines 241, 412, 462) create fire-and-forget threads. If the DLL unloads while any are still running, the process crashes from accessing freed memory.

**Files:**
- Modify: `src/PrismaUIManager.cpp:41-55` (add thread storage)
- Modify: `src/PrismaUIManager.cpp:188-242` (QueueSaveSettingsToDisk)
- Modify: `src/PrismaUIManager.cpp:355-413` (QueueCloseRetry)
- Modify: `src/PrismaUIManager.cpp:436-463` (QueueDelayedFocusAndState)
- Modify: `src/PrismaUIManager.cpp:906-918` (OnPostLoad — add shutdown join)
- Modify: `include/CodexOfPowerNG/PrismaUIManager.h` (add Shutdown declaration)

**Step 1: Add thread storage and JoinWorkers helper**

In the anonymous namespace (after `g_shuttingDown`), add:

```cpp
std::mutex g_workerMutex;
std::thread g_settingsSaveThread;
std::thread g_closeRetryThread;
std::thread g_focusDelayThread;

void JoinIfJoinable(std::thread& t) noexcept
{
    if (t.joinable()) {
        t.join();
    }
}

void JoinAllWorkers() noexcept
{
    std::scoped_lock lock(g_workerMutex);
    JoinIfJoinable(g_settingsSaveThread);
    JoinIfJoinable(g_closeRetryThread);
    JoinIfJoinable(g_focusDelayThread);
}
```

**Step 2: Replace `.detach()` with stored threads**

In `QueueSaveSettingsToDisk` (line ~200), replace:
```cpp
// OLD:
std::thread([]() { ... }).detach();

// NEW:
{
    std::scoped_lock lock(g_workerMutex);
    JoinIfJoinable(g_settingsSaveThread);
    g_settingsSaveThread = std::thread([]() { ... });
}
```

In `QueueCloseRetry` (line ~359), replace:
```cpp
// OLD:
std::thread([...]() { ... }).detach();

// NEW:
{
    std::scoped_lock lock(g_workerMutex);
    JoinIfJoinable(g_closeRetryThread);
    g_closeRetryThread = std::thread([...]() { ... });
}
```

In `QueueDelayedFocusAndState` (line ~443), replace:
```cpp
// OLD:
std::thread([delayMs]() { ... }).detach();

// NEW:
{
    std::scoped_lock lock(g_workerMutex);
    JoinIfJoinable(g_focusDelayThread);
    g_focusDelayThread = std::thread([delayMs]() { ... });
}
```

**Step 3: Add Shutdown function**

Add to `PrismaUIManager.h`:
```cpp
void Shutdown() noexcept;
```

Implement in `PrismaUIManager.cpp` (in the public namespace):
```cpp
void Shutdown() noexcept
{
    g_shuttingDown.store(true, std::memory_order_relaxed);
    JoinAllWorkers();
}
```

**Step 4: Call Shutdown from OnPreLoadGame**

Update `OnPreLoadGame`:
```cpp
void OnPreLoadGame() noexcept
{
    Shutdown();
    g_toggleAllowed.store(false, std::memory_order_relaxed);
    g_toggleAllowedAtMs.store(0, std::memory_order_relaxed);
}
```

**Step 5: Self-review**

Verify:
- No deadlock: `JoinAllWorkers` acquires `g_workerMutex`; the worker threads do NOT acquire `g_workerMutex` (they don't need to)
- The workers check `g_shuttingDown` and exit promptly; `join()` will return quickly
- `JoinIfJoinable` before assignment prevents leaking old thread handles
- `g_settingsSaveWorkerRunning` / `g_focusDelayArmed` guards still prevent concurrent launches correctly

**Step 6: Commit**

```bash
git add src/PrismaUIManager.cpp include/CodexOfPowerNG/PrismaUIManager.h
git commit -m "fix(ui): replace detached threads with joinable workers

C1: Store worker threads (settings save, close retry, focus delay) and join
them on pre-load/shutdown. Prevents crashes from accessing freed DLL memory
if a background thread is still running during unload."
```

---

### Task 3: I1 + I7 — Stabilize Quick-Register Pagination and Fix Total Count

**Severity:** Important

**Problem:** `BuildQuickRegisterList` paginates based on unstable `entryList` iteration order (I1). Also, `total` is 0 when `hasMore=true`, which is useless for UI pagination (I7).

**Files:**
- Modify: `src/Registration.cpp:327-468` (BuildQuickRegisterList)

**Step 1: Collect all eligible items, sort, then paginate**

Replace the current single-pass + post-sort approach with: collect all → sort → slice.

```cpp
QuickRegisterList BuildQuickRegisterList(std::size_t offset, std::size_t limit)
{
    QuickRegisterList result{};

    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return result;
    }

    const auto settings = GetSettings();

    EnsureMapsLoaded();

    const auto quickListState = RegistrationStateStore::SnapshotQuickList();
    const auto& blocked = quickListState.blockedItems;
    const auto& registered = quickListState.registeredKeys;

    const auto isExcludedFast = [&](const RE::TESForm* item) noexcept -> bool {
        if (!item) {
            return true;
        }

        const auto id = item->GetFormID();
        if (g_excluded.contains(id) || blocked.contains(id)) {
            return true;
        }

        if (RegistrationRules::IsIntrinsicExcluded(item)) {
            return true;
        }

        return false;
    };

    const auto getDiscoveryGroupFast = [&](const RE::TESForm* item) noexcept -> std::uint32_t {
        if (!item) {
            return 255;
        }

        if (isExcludedFast(item)) {
            return 255;
        }

        return RegistrationRules::GroupFromFormType(item->GetFormType());
    };

    // Phase 1: collect ALL eligible items (stable pagination requires full scan)
    std::vector<ListItem> allEligible;

    if (auto* changes = player->GetInventoryChanges(); changes && changes->entryList) {
        for (auto* entry : *changes->entryList) {
            if (!entry) {
                continue;
            }

            if (entry->IsQuestObject()) {
                continue;
            }

            auto* obj = entry->GetObject();
            if (!obj) {
                continue;
            }

            auto* regKey = GetRegisterKey(obj, settings);
            if (!regKey) {
                continue;
            }

            const auto group = getDiscoveryGroupFast(regKey);
            if (group > 5) {
                continue;
            }

            if (isExcludedFast(obj) || (regKey != obj && isExcludedFast(regKey))) {
                continue;
            }

            const auto regKeyId = regKey->GetFormID();
            const auto objId = obj->GetFormID();
            if (registered.contains(regKeyId) || registered.contains(objId)) {
                continue;
            }

            const auto totalCount = player->GetItemCount(obj);
            if (totalCount <= 0) {
                continue;
            }

            const auto removal =
                Inventory::SelectSafeRemoval(entry, totalCount, settings.protectFavorites);
            const auto safeCount = removal.safeCount;
            if (safeCount <= 0) {
                continue;
            }

            ListItem li{};
            li.formId = objId;
            li.regKey = regKeyId;
            li.group = group;
            li.totalCount = totalCount;
            li.safeCount = safeCount;
            li.excluded = false;
            li.registered = false;
            li.blocked = false;
            li.name = BestItemName(regKey, obj);
            if (li.name.empty()) {
                li.name = L10n::T("ui.unnamed", "(unnamed)");
            }

            allEligible.push_back(std::move(li));
        }
    }

    // Phase 2: sort ALL eligible items for stable ordering
    std::sort(allEligible.begin(), allEligible.end(), [](const ListItem& a, const ListItem& b) {
        if (a.group != b.group) {
            return a.group < b.group;
        }
        return a.name < b.name;
    });

    // Phase 3: paginate from sorted result
    const auto totalEligible = allEligible.size();
    const auto clampedOffset = (std::min)(offset, totalEligible);
    const auto remaining = totalEligible - clampedOffset;
    const auto pageSize = (std::min)(limit, remaining);

    result.total = totalEligible;
    result.hasMore = (clampedOffset + pageSize) < totalEligible;
    result.items.assign(
        std::make_move_iterator(allEligible.begin() + static_cast<std::ptrdiff_t>(clampedOffset)),
        std::make_move_iterator(allEligible.begin() + static_cast<std::ptrdiff_t>(clampedOffset + pageSize)));

    return result;
}
```

**Step 2: Self-review**

Verify:
- `allEligible` collects every eligible item (no early break at limit)
- Sort is deterministic: (group, name) — stable pagination
- `result.total` is always the true total, fixing I7
- Move iterators avoid unnecessary copies
- `<algorithm>` and `<iterator>` includes already present

**Step 3: Commit**

```bash
git add src/Registration.cpp
git commit -m "fix(registration): stabilize quick-register pagination order

I1+I7: Collect all eligible items before sorting, then paginate from the
sorted list. This guarantees stable page boundaries across calls.
Also fixes total field to always reflect true eligible count."
```

---

### Task 4: I5 — Fix SettingsEquivalent to Compare Clamped Values

**Severity:** Important

**Problem:** `SettingsFromJson` returns unclamped values. `SettingsEquivalent(current, next)` compares clamped `current` with unclamped `next`, treating identical-after-clamping values as "changed" and triggering unnecessary disk writes.

**Files:**
- Modify: `include/CodexOfPowerNG/Config.h` (add `ClampSettings` declaration)
- Modify: `src/Config.cpp` (expose `Clamp` as `ClampSettings`)
- Modify: `src/PrismaUIManager.cpp:739-775` (use clamped comparison)

**Step 1: Expose ClampSettings in Config.h**

Add to `Config.h`:
```cpp
// Clamps setting values to their valid ranges.
[[nodiscard]] Settings ClampSettings(const Settings& settings);
```

**Step 2: Implement ClampSettings in Config.cpp**

In Config.cpp, rename the anonymous `Clamp` → call the new public function, or have the anonymous `Clamp` delegate:

After the anonymous namespace's `Clamp` function, in the public namespace section, add:
```cpp
Settings ClampSettings(const Settings& settings)
{
    return Clamp(settings);
}
```

**Step 3: Clamp before comparison in OnJsSetSettings**

In `PrismaUIManager.cpp`, update `OnJsSetSettings`:
```cpp
const auto current = GetSettings();
const auto next = ClampSettings(SettingsFromJson(payload, current));
if (SettingsEquivalent(current, next)) {
```

**Step 4: Self-review**

Verify:
- `ClampSettings` is a thin wrapper around the existing `Clamp`
- The `Clamp` function remains in the anonymous namespace (no breaking change)
- `#include "CodexOfPowerNG/Config.h"` already exists in PrismaUIManager.cpp

**Step 5: Commit**

```bash
git add include/CodexOfPowerNG/Config.h src/Config.cpp src/PrismaUIManager.cpp
git commit -m "fix(settings): compare clamped values in SettingsEquivalent

I5: Expose ClampSettings() and apply it before equality check to avoid
spurious 'settings changed' saves when out-of-range values clamp to the
same effective value."
```

---

### Task 5: M1 — Extract Duplicated NowMs to Shared Header

**Severity:** Minor

**Problem:** Identical `NowMs()` helper defined in both `PrismaUIManager.cpp:78` and `Events.cpp:25`.

**Files:**
- Create: `include/CodexOfPowerNG/Util.h`
- Modify: `src/PrismaUIManager.cpp` (remove local `NowMs`, include `Util.h`)
- Modify: `src/Events.cpp` (remove local `NowMs`, include `Util.h`)

**Step 1: Create Util.h**

```cpp
#pragma once

#include <chrono>
#include <cstdint>

namespace CodexOfPowerNG
{
	[[nodiscard]] inline std::uint64_t NowMs() noexcept
	{
		using namespace std::chrono;
		return static_cast<std::uint64_t>(
			duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
	}
}
```

**Step 2: Update PrismaUIManager.cpp**

- Add `#include "CodexOfPowerNG/Util.h"` to the include block
- Remove the anonymous `NowMs()` definition (lines 78-83)
- Replace calls: `NowMs()` → `NowMs()` (same name, now from outer namespace; accessible inside the anonymous namespace since it's in the enclosing `CodexOfPowerNG` namespace)

**Step 3: Update Events.cpp**

- Add `#include "CodexOfPowerNG/Util.h"` to the include block
- Remove the anonymous `NowMs()` definition (lines 25-29)
- Same namespace resolution applies

**Step 4: Commit**

```bash
git add include/CodexOfPowerNG/Util.h src/PrismaUIManager.cpp src/Events.cpp
git commit -m "refactor: extract shared NowMs() helper to Util.h

M1: Eliminates duplicated steady_clock millisecond helper from
PrismaUIManager.cpp and Events.cpp."
```

---

### Task 6: I3 — Document Write-Once Map Invariant

**Severity:** Important (but actually safe — documentation fix)

**Problem:** `g_excluded` and `g_variantBase` are populated once under `g_mapsMutex` then read without the lock. This looks like a data race but is actually safe: every read path calls `EnsureMapsLoaded()` first, which acquires `g_mapsMutex`, establishing a happens-before relationship with the initial write.

**Files:**
- Modify: `src/Registration.cpp:31-37` (add comments)

**Step 1: Add documentation comments**

```cpp
// --- Write-once maps (populated by EnsureMapsLoaded, then immutable) ---
// Safety: every public function that reads these maps calls EnsureMapsLoaded()
// first. The mutex acquire/release in EnsureMapsLoaded() provides a
// happens-before relationship with the initial write, so subsequent
// lock-free reads are safe. Do NOT modify these maps after initialization.
std::unordered_set<RE::FormID>            g_excluded{};
std::unordered_map<RE::FormID, RE::FormID> g_variantBase{};
```

**Step 2: Commit**

```bash
git add src/Registration.cpp
git commit -m "docs(registration): document write-once map thread-safety invariant

I3: g_excluded and g_variantBase are safely read without locks because every
read path calls EnsureMapsLoaded() first, providing memory synchronization."
```

---

### Task 7: I6 — Replace Spin-Wait with Promise/Future in QueueCloseRetry

**Severity:** Important

**Problem:** `QueueCloseRetry` uses a background thread with `atomic_bool` flags and 5ms sleep polling to wait for UI task completion. This is fragile and wastes CPU.

**Files:**
- Modify: `src/PrismaUIManager.cpp:355-413` (QueueCloseRetry)

**Step 1: Replace atomic polling with promise/future**

```cpp
void QueueCloseRetry(PrismaView view, bool destroyOnClose, std::uint32_t /*attempt*/) noexcept
{
    constexpr std::uint32_t kMaxAttempts = 20;

    auto worker = [view, destroyOnClose, kMaxAttempts]() {
        for (std::uint32_t attempt = 1; attempt <= kMaxAttempts; ++attempt) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            if (g_shuttingDown.load(std::memory_order_relaxed)) return;

            enum class CloseResult { kDone, kRetry, kQueueFailed };
            auto promise = std::make_shared<std::promise<CloseResult>>();
            auto future = promise->get_future();

            if (!QueueUITask([&, promise, view, destroyOnClose, attempt]() {
                if (g_openRequested.load(std::memory_order_relaxed)) {
                    SKSE::log::info("Close: aborted (re-open requested)");
                    promise->set_value(CloseResult::kDone);
                    return;
                }

                auto* api = g_prismaAPI.load(std::memory_order_acquire);
                const auto activeView = g_view.load(std::memory_order_acquire);
                if (!api || view == 0 || activeView == 0 || activeView != view || !api->IsValid(view)) {
                    promise->set_value(CloseResult::kDone);
                    return;
                }

                if (api->HasFocus(view)) {
                    SKSE::log::warn("Close: still focused (attempt {}); retrying Unfocus/Hide", attempt);
                    api->Unfocus(view);
                    api->Hide(view);
                    QueueForceHideFocusMenu();
                    QueueHideSkyrimCursor();
                    promise->set_value(CloseResult::kRetry);
                    return;
                }

                if (destroyOnClose) {
                    api->Destroy(view);
                    SKSE::log::info("PrismaView destroyed: {}", view);
                    ResetViewStateOnUIThread();
                }
                promise->set_value(CloseResult::kDone);
            })) {
                return;
            }

            // Block until UI task completes (no spin-wait)
            const auto result = future.get();
            if (result == CloseResult::kDone) return;
            // kRetry → loop continues
        }

        SKSE::log::error("Close: focus did not clear after {} attempts; leaving view {}", kMaxAttempts, view);
        QueueForceHideFocusMenu();
        QueueHideSkyrimCursor();
    };

    {
        std::scoped_lock lock(g_workerMutex);
        JoinIfJoinable(g_closeRetryThread);
        g_closeRetryThread = std::thread(std::move(worker));
    }
}
```

Note: Add `#include <future>` to the include block.

**Step 2: Self-review**

Verify:
- `promise->set_value()` is called exactly once in every code path
- `future.get()` blocks cleanly (no busy loop)
- The `[&, promise, ...]` capture: `promise` is by value (shared_ptr copy), atomics are global so `&` is safe
- Actually, be careful: `attempt` is a loop variable captured by value — correct since it's a `std::uint32_t`
- The stored thread pattern (from Task 2) is used

**Step 3: Commit**

```bash
git add src/PrismaUIManager.cpp
git commit -m "refactor(ui): replace spin-wait with promise/future in close retry

I6: QueueCloseRetry now uses std::promise/std::future instead of atomic
flags with 5ms sleep polling. Cleaner synchronization with no busy-wait."
```

---

### Task 8: M2/M6 — Fix Registration.h Indentation

**Severity:** Minor

**Problem:** `Registration.h` has inconsistent indentation — some struct members use extra tabs, namespace content has mixed indentation levels.

**Files:**
- Modify: `include/CodexOfPowerNG/Registration.h`

**Step 1: Normalize to project conventions**

Project convention: namespace content at 1 tab, struct members at 1 tab inside struct (2 tabs total for nested).

Write the corrected file (using tabs):

```cpp
#pragma once

#include <RE/Skyrim.h>

#include <cstdint>
#include <string>
#include <vector>

namespace CodexOfPowerNG::Registration
{
	struct ListItem
	{
		RE::FormID      formId{ 0 };
		RE::FormID      regKey{ 0 };
		std::uint32_t   group{ 255 };
		std::int32_t    totalCount{ 0 };
		std::int32_t    safeCount{ 0 };
		bool            excluded{ false };
		bool            registered{ false };
		bool            blocked{ false };
		std::string     name;
	};

	struct QuickRegisterList
	{
		bool                  hasMore{ false };
		std::size_t           total{ 0 };
		std::vector<ListItem> items;
	};

	struct RegisterResult
	{
		bool            success{ false };
		std::string     message;
		RE::FormID      regKey{ 0 };
		std::uint32_t   group{ 255 };
		std::size_t     totalRegistered{ 0 };
	};

	[[nodiscard]] std::uint32_t GetDiscoveryGroup(const RE::TESForm* item) noexcept;
	[[nodiscard]] std::string   GetDiscoveryGroupName(std::uint32_t group);

	[[nodiscard]] RE::FormID GetRegisterKeyId(RE::FormID formId) noexcept;

	[[nodiscard]] bool IsRegistered(RE::FormID formId) noexcept;

	[[nodiscard]] bool IsExcluded(RE::FormID formId) noexcept;

	[[nodiscard]] bool IsDiscoverable(RE::FormID formId) noexcept;

	[[nodiscard]] QuickRegisterList BuildQuickRegisterList(std::size_t offset, std::size_t limit);

	[[nodiscard]] std::vector<ListItem> BuildRegisteredList();

	[[nodiscard]] RegisterResult TryRegisterItem(RE::FormID formId);

	void Warmup() noexcept;
}
```

**Step 2: Commit**

```bash
git add include/CodexOfPowerNG/Registration.h
git commit -m "style(registration): normalize header indentation

M2/M6: All struct members and declarations now use consistent 1-tab
indentation inside the namespace."
```

---

### Task 9: Run JS Tests + Final Verification

**Files:**
- Test: `tests/*.test.cjs`

**Step 1: Run all JS tests**

```bash
node tests/lang_ui.test.cjs && node tests/keycodes.test.cjs && node tests/settings_sticky_actions.test.cjs && node tests/quick_register_rules.test.cjs
```

Expected: All pass (these are JS-side tests; C++ changes should not affect them).

**Step 2: Review all changed files**

```bash
git diff --stat HEAD~8
git log --oneline -8
```

Verify 8 commits, all changes aligned with plan.

---

## Summary of Changes

| ID | Severity | Fix | Files |
|----|----------|-----|-------|
| C2 | Critical | Serialization: lambda per record, `return` no longer aborts all | `Serialization.cpp` |
| C1 | Critical | Joinable threads with shutdown | `PrismaUIManager.cpp`, `PrismaUIManager.h` |
| I1 | Important | Full-scan + sort before paginate | `Registration.cpp` |
| I7 | Important | `total` always reflects true count | (same as I1) |
| I5 | Important | Clamp before SettingsEquivalent | `Config.h`, `Config.cpp`, `PrismaUIManager.cpp` |
| M1 | Minor | Extract NowMs to `Util.h` | `Util.h`, `PrismaUIManager.cpp`, `Events.cpp` |
| I3 | Important | Document write-once invariant | `Registration.cpp` |
| I6 | Important | promise/future replaces spin-wait | `PrismaUIManager.cpp` |
| M2/M6 | Minor | Normalize Registration.h indent | `Registration.h` |
| I2 | — | **SKIP** (intentional design) | — |
