# SRC KNOWLEDGE BASE

## OVERVIEW
`src/` contains runtime C++ implementation: SKSE lifecycle, Prisma UI bridge, registration flow, rewards, and serialization.

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Load/bootstrap + hotkey | `src/main.cpp` | `SKSEPluginLoad`, message listener, input sink |
| UI manager public flow | `src/PrismaUIManager.cpp` | post-load setup, toggle, shutdown |
| UI lifecycle workers | `src/PrismaUIViewLifecycle.cpp`, `src/PrismaUIViewWorkers.cpp` | create/focus/hide/destroy sequencing |
| JS request handlers | `src/PrismaUIRequestOps.cpp`, `src/PrismaUIRequests.cpp` | register/undo/refund/inventory/rewards IPC |
| Registration core | `src/Registration*.cpp` | quick list build, cache, undo policy, TCC gate |
| Reward engine | `src/Rewards*.cpp` | reward grant, caps, post-load resync scheduling |
| Save/load state | `src/Serialization*.cpp`, `src/State.cpp` | co-save read/write and runtime hydration |
| Runtime settings + l10n | `src/Config.cpp`, `src/L10n.cpp` | layered settings and language selection |
| Task dispatch | `src/TaskScheduler.cpp` | main task vs UI task queue wrappers |

## CONVENTIONS
- Keep tab indentation consistent with neighboring code blocks.
- Route game object access through main task queue paths.
- Route Prisma UI API and `SendJS`/`CallJS` style work through UI queue paths.
- Keep quick-register inventory path pagination-aware (`page`, `pageSize`, `total`, `hasMore`).
- Invalidate quick-register cache after mutating registration state.

## ANTI-PATTERNS
- Do not execute Prisma UI operations synchronously when UI queue dispatch fails (`ToggleUI` intentionally refuses fallback).
- Do not move inventory listing back to full-map `GetInventory()` scans in quick-register path.
- Do not remove load-boundary scheduler resets (`ResetSyncSchedulersForLoad`, toggle policy reset).
- Do not bypass newest-only undo semantics in registration undo flow.
- Do not mix reward-state sync logic into unrelated modules; keep sync in `Rewards*.cpp`.
