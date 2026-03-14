# Hybrid Specialist Preservation Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Retune the first hybrid weak-option pass so specialist options keep their best-in-slot identity while hybrid options remain broader but less efficient.

**Architecture:** Reuse the existing hybrid bundle system introduced in the previous pass. Limit the corrective work to bundle magnitudes, localized descriptions, and the tests that lock runtime totals and Build panel text. Do not alter slot compatibility, unlock thresholds, or effect-system structure again.

**Tech Stack:** C++ build catalog/runtime/payload code, Prisma UI JavaScript modules, Node tests, WSL CTest suite

---

## File Map

- Modify: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
  - Reduce hybrid primary values in the catalog rows and hybrid support riders inside `GetResolvedBuildEffectBundle()`.
- Modify: [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)
  - Update hybrid option descriptions so they describe the reduced specialist-preserving values.
- Test: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)
  - Lock the new primary magnitudes for `reserve`, `magicka`, `meditation`, `hauler`.
- Test: [tests/build_effect_runtime.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_effect_runtime.test.cpp)
  - Lock the new actor-value totals after the reduced support riders.
- Test: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
  - Lock the Build panel current/next-tier text for the updated hybrid values.
- Test: [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)
  - Lock the localized descriptions for the specialist-preserving hybrid values.

## Chunk 1: Lock The New Competition Contract

### Task 1: Write the failing tests for specialist preservation

**Files:**
- Modify: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)
- Modify: [tests/build_effect_runtime.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_effect_runtime.test.cpp)
- Modify: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
- Modify: [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)

- [ ] **Step 1: Update catalog expectations for the new reduced hybrid magnitudes**

Cover:
- `build.attack.reserve`: primary `8`, per-tier `2`
- `build.utility.magicka`: primary `12`, per-tier `4`
- `build.utility.meditation`: primary `0.08`, per-tier `0.04`
- `build.utility.hauler`: primary `6`, per-tier `2`

- [ ] **Step 2: Add pairwise sanity assertions for specialist lane ownership**

Use dedicated snapshot comparisons and assert:
- `Reserve < Secondwind` in stamina regen at tier `0`, `2`, `3`
- `Reserve` still clears the stamina floor at tier `0`, `2`, `3`
- `Magicka Well < Meditation` in magicka regen at tier `0`, `2`, `3`
- `Magicka Well > Meditation` in magicka pool at tier `0`, `2`, `3`
- `Long Haul < Cache` in carry weight at tier `0`, `2`, `3`
- `Long Haul` still clears the expedition floor at tier `0`, `2`, `3`

- [ ] **Step 3: Keep the existing aggregate runtime snapshots but update them to the reduced values**

Use the existing targeted snapshots and assert:
- `Reserve` totals now resolve to weaker regen than before
- `Magicka Well` totals now resolve to weaker regen than before
- `Meditation` totals now resolve to a smaller magicka floor than before
- `Long Haul` totals now resolve to weaker carry support than before

- [ ] **Step 4: Update UI rendering expectations for the new hybrid totals**

Cover:
- current effect text
- next-tier effect text
- English wording in row/detail rendering

- [ ] **Step 5: Update localization expectations**

Cover:
- English and Korean descriptions for:
  - `build.attack.reserve`
  - `build.utility.magicka`
  - `build.utility.meditation`
  - `build.utility.hauler`

- [ ] **Step 6: Run focused tests to verify they fail**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_option_catalog_contract CodexOfPowerNG_build_effect_runtime
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_option_catalog_contract|build_effect_runtime)$'
node --test tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs
```

Expected: FAIL on the old hybrid values still present in source.

## Chunk 2: Retune The Hybrid Values

### Task 2: Trim hybrid riders so specialists remain best-in-slot

**Files:**
- Modify: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
- Test: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)
- Test: [tests/build_effect_runtime.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_effect_runtime.test.cpp)

- [ ] **Step 1: Reduce `Reserve` support regen**

Inside `GetResolvedBuildEffectBundle()` set:
- support base `0.04f`
- support per tier `0.02f`

Keep:
- primary `8.0f`
- primary per tier `2.0f`

- [ ] **Step 2: Reduce `Magicka Well` support regen**

Inside `GetResolvedBuildEffectBundle()` set:
- support base `0.02f`
- support per tier `0.01f`

Keep:
- primary `12.0f`
- primary per tier `4.0f`

- [ ] **Step 3: Reduce `Meditation` support pool**

Inside `GetResolvedBuildEffectBundle()` set:
- support base `2.0f`
- support per tier `1.0f`

Keep:
- primary `0.08f`
- primary per tier `0.04f`

- [ ] **Step 4: Reduce `Long Haul` primary and carry support**

In the catalog row and bundle resolver set:
- primary base `6.0f`
- primary per tier `2.0f`
- support base `4.0f`
- support per tier `1.0f`

- [ ] **Step 5: Run focused native tests**

Run:
```bash
cmake --build build/wsl-debug --target CodexOfPowerNG_build_option_catalog_contract CodexOfPowerNG_build_effect_runtime
ctest --test-dir build/wsl-debug --output-on-failure -R '^(build_option_catalog_contract|build_effect_runtime)$'
```

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add src/BuildOptionCatalog.cpp tests/build_option_catalog_contract.test.cpp tests/build_effect_runtime.test.cpp
git commit -m "feat: preserve specialist lanes in hybrid build options"
```

## Chunk 3: Align Build Copy With The Reduced Hybrid Pass

### Task 3: Update player-facing descriptions and Build panel expectations

**Files:**
- Modify: [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)
- Modify: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
- Modify: [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)

- [ ] **Step 1: Rewrite hybrid option descriptions to match the trimmed values**

Describe:
- `Reserve` as burst-first with a smaller regen rider
- `Magicka Well` as pool-first with a smaller regen rider
- `Meditation` as regen-first with a smaller pool rider
- `Long Haul` as expedition utility with weaker carry support than `Cache`

Also forbid copy that implies:
- better sustain than `Secondwind`
- better regen than `Meditation`
- better hauling than `Cache`

- [ ] **Step 2: Update Build panel rendering expectations**

Verify:
- row previews show the new current totals
- focused option current/next effect text show the new bundle values

- [ ] **Step 3: Run focused JS tests**

Run:
```bash
node --test tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs
```

Expected: PASS

- [ ] **Step 4: Commit**

```bash
git add PrismaUI/views/codexofpowerng/ui_i18n.js tests/build_ui_rendering_module.test.cjs tests/ui_i18n_module.test.cjs
git commit -m "feat: update hybrid build copy for specialist preservation"
```

## Chunk 4: Full Verification

### Task 4: Run the complete regression and release build

**Files:**
- No product file changes
- Validation only

- [ ] **Step 1: Run the full JS suite**

Run:
```bash
node --test tests/*.test.cjs
```

Expected: PASS

- [ ] **Step 2: Rebuild the full WSL debug test tree**

Run:
```bash
cmake --build build/wsl-debug
```

Expected: all debug test binaries are rebuilt with the latest source.

- [ ] **Step 3: Run the full WSL CTest suite**

Run:
```bash
ctest --test-dir build/wsl-debug --output-on-failure
```

Expected: PASS

- [ ] **Step 4: Run the host-safe regression gate**

Run:
```bash
scripts/test.sh
```

Expected: PASS

- [ ] **Step 5: Rebuild and install the release artifact**

Run:
```bash
cmake --build --preset wsl-release
cmake --install build/wsl-release
```

Expected: release build succeeds and `dist/CodexOfPowerNG` is refreshed.

- [ ] **Step 6: Optional deployment sync**

If the user asks for it, sync:
```bash
rsync -a --delete --exclude 'meta.ini' dist/CodexOfPowerNG/SKSE/ /mnt/g/TAKEALOOK/mods/Codex.of.Power.NG-v1.2.0-rc.1/SKSE/
rsync -a --delete dist/CodexOfPowerNG/PrismaUI/ /mnt/g/TAKEALOOK/mods/Codex.of.Power.NG-v1.2.0-rc.1/PrismaUI/
```

Then compare hashes:
```bash
sha256sum dist/CodexOfPowerNG/SKSE/Plugins/CodexOfPowerNG.dll /mnt/g/TAKEALOOK/mods/Codex.of.Power.NG-v1.2.0-rc.1/SKSE/Plugins/CodexOfPowerNG.dll
```
