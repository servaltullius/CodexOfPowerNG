# Build Catalog Curation Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Curate the first-pass build catalog so the current 3-per-discipline option set is clear, non-overlapping, and uses concrete player-facing descriptions.

**Architecture:** Keep the existing `3 x 3` build catalog and slot structure. First verify that slotted build options actually drive runtime gameplay effects; then replace the ambiguous three options, update localized strings, and add regression coverage around the curated catalog contract.

**Tech Stack:** C++, SKSE/CommonLibSSE runtime state, Prisma UI JS view layer, Node-based JS tests, CMake host/native tests

---

## File Map

- Modify: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
  - Source of truth for build option ids, disciplines, unlock scores, and effect keys
- Modify: [include/CodexOfPowerNG/BuildTypes.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildTypes.h)
  - Only if new effect type metadata is required; avoid changing unless runtime audit proves necessary
- Modify: [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)
  - Korean/English titles and descriptions for curated options
- Modify: [src/PrismaUIPayloadsBuild.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/PrismaUIPayloadsBuild.cpp)
  - Only if effect labels or payload metadata need to expose updated keys
- Modify: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)
  - Native contract test for curated ids, disciplines, unlock scores, and effect keys
- Modify: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
  - View-level contract for updated build copy
- Modify: [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)
  - Locale string coverage for new titles/descriptions
- Optional modify: runtime build effect application files after audit
  - Candidate files: [src/BuildStateStore.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildStateStore.cpp), [src/BuildProgression.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildProgression.cpp), reward/build runtime glue if effect application is currently missing

## Chunk 1: Audit Runtime Effect Application

### Task 1: Verify whether active build slots currently affect gameplay

**Files:**
- Inspect: [src/BuildStateStore.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildStateStore.cpp)
- Inspect: [src/BuildProgression.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildProgression.cpp)
- Inspect: runtime reward/build integration files discovered during audit

- [ ] **Step 1: Write down the audit question**

Question: “When a build option is activated into a slot, which runtime code applies its effect to the player?”

- [ ] **Step 2: Search for slot effect usage**

Run:
```bash
rg -n "GetActiveSlot\\(|activeBuildSlots\\[|effectType|effectKey|vendor_price_bonus|speed_mult|critical|attack_damage_mult|damage_resist" src include
```

Expected: concrete runtime usage path, or clear evidence that only UI/state paths exist.

- [ ] **Step 3: Record audit result in working notes**

If runtime application path exists:
- note the exact files/functions
- continue to Chunk 2 without widening scope

If runtime application path does not exist:
- explicitly mark that the current catalog copy is descriptive-only and misleading
- widen Chunk 2 to include minimal runtime application for curated effects

- [ ] **Step 4: Commit audit checkpoint if scope changes**

```bash
git add docs/superpowers/specs/2026-03-12-build-catalog-curation-design.md docs/superpowers/plans/2026-03-12-build-catalog-curation.md
git commit -m "docs: capture build catalog curation audit scope"
```

## Chunk 2: Curate the 1st-Pass Catalog

### Task 2: Lock the curated option set in tests first

**Files:**
- Modify: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)

- [ ] **Step 1: Update expected catalog entries**

Replace the current expectations so the catalog becomes:

- Attack
  - `build.attack.ferocity`
  - `build.attack.precision`
  - `build.attack.vitals`
- Defense
  - `build.defense.guard`
  - `build.defense.bastion`
  - `build.defense.endurance`
- Utility
  - `build.utility.cache`
  - `build.utility.barter`
  - `build.utility.mobility`

Keep unlock scores `5 / 15 / 30`.

- [ ] **Step 2: Run the contract test and verify failure**

Run:
```bash
scripts/test.sh build_option_catalog_contract.test.cpp
```

Expected: FAIL because the source catalog still contains the old ids/effect keys.

- [ ] **Step 3: Update [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)**

Apply the curated mapping:

- `build.attack.momentum` -> `build.attack.vitals`
  - effect key: legacy critical chance style key
- `build.defense.stalwart` -> `build.defense.endurance`
  - effect key: health
- `build.utility.scout` -> `build.utility.mobility`
  - effect key: speed mult

Do not change slot compatibility or unlock scores.

- [ ] **Step 4: Re-run the contract test**

Run:
```bash
scripts/test.sh build_option_catalog_contract.test.cpp
```

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add tests/build_option_catalog_contract.test.cpp src/BuildOptionCatalog.cpp
git commit -m "feat: curate first-pass build option catalog"
```

### Task 3: Make player-facing copy concrete and consistent

**Files:**
- Modify: [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)
- Modify: [tests/ui_i18n_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_i18n_module.test.cjs)
- Modify: [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)

- [ ] **Step 1: Write failing string expectations**

Add assertions for:
- new ids/titles
- concrete descriptions
- Korean and English coverage

Examples to assert:
- `슬롯 활성 시 공격 피해가 5% 증가합니다.`
- `슬롯 활성 시 최대 체력이 15 증가합니다.`
- `슬롯 활성 시 이동 속도가 3% 증가합니다.`

- [ ] **Step 2: Run JS tests and verify failure**

Run:
```bash
node --test tests/ui_i18n_module.test.cjs tests/build_ui_rendering_module.test.cjs
```

Expected: FAIL because the old strings still reference `가속 / 강건 / 정찰`.

- [ ] **Step 3: Update localized build strings**

In [ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js):
- rename titles
- replace ambiguous descriptions with explicit `슬롯 활성 시 ...` style wording
- make `흥정` wording explicit about shop prices

- [ ] **Step 4: Re-run JS tests**

Run:
```bash
node --test tests/ui_i18n_module.test.cjs tests/build_ui_rendering_module.test.cjs
```

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add PrismaUI/views/codexofpowerng/ui_i18n.js tests/ui_i18n_module.test.cjs tests/build_ui_rendering_module.test.cjs
git commit -m "feat: clarify curated build option copy"
```

## Chunk 3: Runtime Effect Gate

### Task 4: If needed, implement missing runtime application for curated effects

**Files:**
- Modify only if audit in Chunk 1 shows missing runtime path
- Candidate files:
  - [src/BuildStateStore.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildStateStore.cpp)
  - [src/BuildProgression.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildProgression.cpp)
  - build/reward runtime glue identified during audit
- Test:
  - native build/runtime tests near existing progression/state coverage

- [ ] **Step 1: Add a failing runtime regression test**

Create or extend a native test so that:
- activating a curated option records a slot
- the corresponding runtime effect path is observable

Use the smallest testable surface already present in the build/reward runtime.

- [ ] **Step 2: Run the targeted test and verify failure**

Run:
```bash
ctest --test-dir build/wsl-release --output-on-failure -R "build|registration"
```

Expected: FAIL showing the missing runtime effect path.

- [ ] **Step 3: Implement the minimal effect application path**

Only support the curated phase-1 effects:
- attack damage
- weapon speed
- critical chance
- damage resist
- block percent
- health
- carry weight
- vendor price bonus
- speed mult

Do not re-open the entire legacy reward random-table system.

- [ ] **Step 4: Re-run the targeted native tests**

Run:
```bash
ctest --test-dir build/wsl-release --output-on-failure -R "build|registration"
```

Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add <runtime files> <tests>
git commit -m "feat: apply curated build slot effects at runtime"
```

## Chunk 4: End-to-End Verification

### Task 5: Verify catalog, UI payload, and build screen all align

**Files:**
- Verify: [src/PrismaUIPayloadsBuild.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/PrismaUIPayloadsBuild.cpp)
- Verify: [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
- Verify: [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)

- [ ] **Step 1: Run the full JS suite**

Run:
```bash
node --test tests/*.test.cjs
```

Expected: PASS

- [ ] **Step 2: Run the fast project regression gate**

Run:
```bash
scripts/test.sh
```

Expected: PASS

- [ ] **Step 3: Build installable artifacts**

Run:
```bash
cmake --install build/wsl-release
```

Expected: install completes and updates `dist/CodexOfPowerNG`

- [ ] **Step 4: Sync the test mod folder if using the same local workflow**

Run:
```bash
rsync -a --delete --exclude='meta.ini' dist/CodexOfPowerNG/ /mnt/g/TAKEALOOK/mods/Codex.of.Power.NG-v1.2.0-rc.1/
```

Expected: game test folder matches the current curated build

- [ ] **Step 5: Commit final verification checkpoint**

```bash
git add .
git commit -m "test: verify curated build catalog end to end"
```
