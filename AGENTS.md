# PROJECT KNOWLEDGE BASE

**Generated:** 2026-02-24 09:39:46 +0900
**Commit:** dad0d96
**Branch:** main
**Mode:** update

## OVERVIEW
CodexOfPowerNG is a Skyrim SKSE plugin with a Prisma UI web overlay for item registration/progression.  
This repo is a new mod line (no Codex/SVCollection backward compatibility, no save migration).

## STRUCTURE
```text
./
├── src/                           # Runtime logic, SKSE events, UI manager, rewards, serialization
├── include/CodexOfPowerNG/        # Public contracts and shared type definitions
├── PrismaUI/views/codexofpowerng/ # Single-page UI and JS modules (interop, input, virtual tables)
├── tests/                         # Node source-assert tests + standalone C++ host tests
├── SKSE/Plugins/CodexOfPowerNG/   # Runtime settings + localization assets packaged with DLL
├── cmake/                         # WSL clang-cl toolchains, custom triplet, vcpkg overlay port
├── docs/releases/                 # Release notes and release-note policy (KR)
└── scripts/                       # vibe helpers and local test orchestration
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Plugin bootstrap/hotkey | `src/main.cpp` | SKSE init, messaging hooks, legacy residue warning |
| UI lifecycle/toggle | `src/PrismaUIManager.cpp` | toggle policy, creation, shutdown, UI-thread gating |
| C++ <-> JS request handling | `src/PrismaUIRequestOps.cpp` | register/undo/refund/state payload dispatch |
| Registration rules/list | `src/Registration*.cpp` | eligibility, cache invalidation, pagination |
| Rewards sync/resync | `src/Rewards*.cpp`, `include/CodexOfPowerNG/RewardsResync.h` | post-load sync, quick carry-weight resync |
| Save/load flow | `src/Serialization*.cpp` | co-save callbacks and runtime state hydration |
| UI entry and event wiring | `PrismaUI/views/codexofpowerng/index.html`, `ui_wiring.js` | tabs, controls, native command dispatch |
| Build/cross toolchain | `CMakePresets.json`, `cmake/toolchains/wsl-clangcl-vcpkg.cmake` | WSL -> Windows clang-cl/lld-link path |
| Release note policy | `docs/releases/PATCH_NOTE_RULES.ko.md` | required KR sections, filename/tag rules |

## CODE MAP
| Symbol | Type | Location | Refs | Role |
|--------|------|----------|------|------|
| `SKSEPluginLoad` | Function | `src/main.cpp` | 1 | SKSE plugin entry |
| `OnSKSEMessage` | Function | `src/main.cpp` | 1 | Routes lifecycle messages to modules |
| `PrismaUIManager::ToggleUI` | Function | `src/PrismaUIManager.cpp` | 2 | Guarded UI toggle transition |
| `Registration::BuildQuickRegisterList` | Function | `src/RegistrationQuickRegister.cpp` | 3 | Eligible item page builder |
| `Registration::TryRegisterItem` | Function | `src/RegistrationQuickRegister.cpp` | 3 | Consume/register path |
| `Registration::TryUndoRegistration` | Function | `src/RegistrationUndo.cpp` | 3 | Latest-action undo policy |
| `Rewards::SyncRewardTotalsToPlayer` | Function | `src/Rewards.cpp` | 3 | Post-load actor-value reconciliation |
| `GetSettings` | Function | `src/Config.cpp` | 24 | Shared runtime config read path |

## CONVENTIONS
- First context command in this repo: `python3 scripts/vibe.py doctor --full`.
- C++ indentation is tab-based (anonymous namespace bodies tend to use deeper tab nesting).
- HTML uses 2 spaces; embedded JS style in `index.html` uses 6-space alignment.
- WSL build path is preset-driven (`wsl-release`), with `VCPKG_ROOT` required.
- Runtime settings are layered: base `settings.json` + user override `settings.user.json`.
- Release notes are Korean; `CHANGELOG.md` remains English.

## ANTI-PATTERNS (THIS PROJECT)
- Do not re-add removed legacy assets: `MCM/`, `Scripts/`, `SKSE/Plugins/SVCollection/`.
- Do not run repo-wide formatting or unrelated refactors while touching feature files.
- Do not claim compatibility/migration from legacy Codex/SVCollection runtime state.
- Do not write release notes with mismatched tag/version filenames.
- Do not claim tests/perf numbers you did not execute/measure.
- Do not describe unchanged areas as changed in release notes.

## UNIQUE STYLES
- Strict thread split: game-object access on main task queue; PrismaUI API/JS calls on UI queue.
- Toggle policy explicitly blocks menu/loading contexts and applies post-load grace windows.
- Inventory quick-register flow uses paged payloads and table virtualization for large inventories.
- UI modules are IIFE-style with both `module.exports` and `global.COPNG*` exports.
- High-DPI handling is a first-class behavior (`ui.inputScale`, perf mode, key-driven recovery).

## COMMANDS
```bash
python3 scripts/vibe.py doctor --full

export VCPKG_ROOT="/mnt/c/Users/<user>/vcpkg"
cmake --preset wsl-release
cmake --build --preset wsl-release
cmake --install build/wsl-release

cd dist/CodexOfPowerNG
zip -r -FS "../../releases/Codex of Power NG.zip" .

scripts/test.sh
node --test tests/*.test.cjs
```

## NOTES
- No `.github/workflows/*.yml` CI workflow is present in this repo snapshot.
- If `VCPKG_ROOT` mismatch errors appear, clear `build/wsl-release/` and reconfigure.
- TypeScript language server is not installed here; JS verification relies on Node tests.
