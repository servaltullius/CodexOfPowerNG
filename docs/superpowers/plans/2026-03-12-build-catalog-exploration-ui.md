# Build Catalog Exploration UI Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rebuild the Build tab around a catalog-first exploration flow that can scale to most legacy rewards without collapsing into long, unstable scroll-heavy UI.

**Architecture:** Keep the existing shell and Quick Register flow. Replace the current Build tab information architecture with a fixed-frame layout: left discipline rail, center theme header plus dense catalog list, right detail-first support rail, and fixed six-slot summary. Expand the build metadata model so options can be grouped by discipline, theme, and hierarchy (`signpost / standard / special`) without changing the slot system.

**Tech Stack:** C++, SKSE/CommonLibSSE runtime state, Prisma UI JS view layer, Node-based JS tests, CMake host/native tests

---

## File Map

- Modify: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
  - Source catalog for disciplines, theme ids, hierarchy markers, and effect metadata
- Modify: [include/CodexOfPowerNG/BuildTypes.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildTypes.h)
  - Add explicit theme/hierarchy metadata only if absent
- Modify: [src/PrismaUIPayloadsBuild.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/PrismaUIPayloadsBuild.cpp)
  - Build grouped payloads for discipline, theme, row metadata, and right-rail detail state
- Modify: [PrismaUI/views/codexofpowerng/index.html](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/index.html)
  - Replace current Build layout with fixed-frame catalog-first board
- Modify: [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
  - Build view model for discipline/theme navigation, dense list rows, and right-rail state
- Modify: [PrismaUI/views/codexofpowerng/ui_rendering.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_rendering.js)
  - Wire fixed viewport behavior and active-tab layout sizing
- Modify: [PrismaUI/views/codexofpowerng/ui_interactions.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_interactions.js)
  - Handle discipline/theme switching, row selection, and slot preview interactions
- Modify: [PrismaUI/views/codexofpowerng/input_correction.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/input_correction.js)
  - Ensure Build wheel routing only targets the central catalog scroller
- Modify: [PrismaUI/views/codexofpowerng/ui_state.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_state.js)
  - Persist selected discipline/theme/row state if needed
- Modify: [PrismaUI/views/codexofpowerng/ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)
  - Add theme labels, hierarchy labels, and any new copy
- Modify tests:
  - [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)
  - [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
  - [tests/ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_rendering_module.test.cjs)
  - [tests/ui_interactions_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/ui_interactions_module.test.cjs)
  - [tests/input_correction_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/input_correction_module.test.cjs)

## Chunk 1: Expand Catalog Metadata

### Task 1: Add theme and hierarchy metadata to the build catalog

**Files:**
- Modify: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
- Modify only if needed: [include/CodexOfPowerNG/BuildTypes.h](/home/kdw73/Codex%20of%20Power%20NG/include/CodexOfPowerNG/BuildTypes.h)
- Modify: [tests/build_option_catalog_contract.test.cpp](/home/kdw73/Codex%20of%20Power%20NG/tests/build_option_catalog_contract.test.cpp)

- [ ] **Step 1: Lock expected theme model in tests**

Add failing expectations for:
- discipline -> theme mapping
- per-option theme id
- per-option hierarchy (`signpost / standard / special`)

- [ ] **Step 2: Implement minimal metadata expansion**

Add enough metadata to describe:
- `공격`: `파괴 / 정밀 / 격노`
- `방어`: `수호 / 보루 / 저항`
- `유틸`: `생활 / 탐색 / 기교`

Do not expand slot counts or unlock score rules here.

- [ ] **Step 3: Re-run focused catalog tests**

Run:
```bash
scripts/test.sh build_option_catalog_contract.test.cpp
```

Expected: PASS

## Chunk 2: Build Payloads Around Catalog Exploration

### Task 2: Emit grouped Build payloads for discipline, theme, and row state

**Files:**
- Modify: [src/PrismaUIPayloadsBuild.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/PrismaUIPayloadsBuild.cpp)
- Modify: build payload type definitions if present nearby
- Add/modify JS tests that assert payload shape

- [ ] **Step 1: Define the payload contract**

The Build payload should expose:
- selected discipline
- available themes for that discipline
- current theme rows
- row fields: `name / oneLineEffect / unlockScore / compatibleSlots / state / hierarchy`
- right-rail selected option detail
- fixed six-slot summary

- [ ] **Step 2: Add failing tests for grouped payload structure**

Cover:
- theme headers exist
- only active theme rows are emitted into the central list payload
- selected option detail and slot summary are emitted separately

- [ ] **Step 3: Implement the grouped payload**

Keep state derivation minimal. Do not change actual activation rules.

- [ ] **Step 4: Re-run targeted JS/native payload tests**

Run the smallest tests that exercise the new payload contract.

## Chunk 3: Rebuild Build Tab Information Architecture

### Task 3: Replace the current Build screen with the catalog-first fixed frame

**Files:**
- Modify: [PrismaUI/views/codexofpowerng/index.html](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/index.html)
- Modify: [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
- Modify: [PrismaUI/views/codexofpowerng/ui_rendering.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_rendering.js)
- Modify: [PrismaUI/views/codexofpowerng/ui_state.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_state.js)

- [ ] **Step 1: Add failing rendering tests for the new structure**

Assert presence of:
- left discipline rail
- center theme header
- center dense list scroller
- right detail-first rail
- right bottom six-slot matrix

- [ ] **Step 2: Implement the new frame**

Rules:
- fixed overall board
- no root scroll for Build
- central list is the only scrolling surface
- right rail stays visible and stable

- [ ] **Step 3: Keep copy density practical**

List rows should show only:
- option name
- one-line effect
- unlock score
- compatible slots
- current state

- [ ] **Step 4: Re-run Build rendering tests**

Run:
```bash
node --test tests/build_ui_rendering_module.test.cjs tests/ui_rendering_module.test.cjs
```

Expected: PASS

## Chunk 4: Interaction Model And Wheel Routing

### Task 4: Make Build interactions match the new mental model

**Files:**
- Modify: [PrismaUI/views/codexofpowerng/ui_interactions.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_interactions.js)
- Modify: [PrismaUI/views/codexofpowerng/input_correction.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/input_correction.js)
- Modify interaction tests

- [ ] **Step 1: Add failing tests for discipline/theme selection**

Cover:
- switching discipline updates theme set
- switching theme updates central list
- row click updates right detail rail

- [ ] **Step 2: Add wheel routing tests**

Build wheel should target only the central list scroller.

- [ ] **Step 3: Implement interactions**

Do not add new activation rules. Only change navigation and focus behavior.

- [ ] **Step 4: Re-run interaction tests**

Run:
```bash
node --test tests/ui_interactions_module.test.cjs tests/input_correction_module.test.cjs
```

Expected: PASS

## Chunk 5: Fit Legacy Reward Expansion Into The New Structure

### Task 5: Prepare the catalog for broad legacy reward intake without UI drift

**Files:**
- Modify catalog and any supporting mapping files discovered during implementation
- Update i18n if theme labels or hierarchy labels are surfaced

- [ ] **Step 1: Create a legacy-to-theme inventory**

For each legacy effect candidate, assign:
- discipline
- theme
- hierarchy
- notes if the effect is ambiguous

This can start as a checked-in mapping table or structured comments near the catalog source, whichever best fits the codebase.

- [ ] **Step 2: Reject theme sprawl**

Do not add new themes unless an effect set clearly fails all current themes.
Prefer:
- primary theme assignment
- optional secondary tag

- [ ] **Step 3: Add regression coverage for theme stability**

Tests should fail if future catalog edits accidentally create unbounded new theme groups or bypass hierarchy tags.

## Chunk 6: Validation And Review Gate

### Task 6: Verify behavior, performance-sensitive layout, and regression safety

**Files:**
- Reuse updated tests above

- [ ] **Step 1: Run focused JS tests**

```bash
node --test tests/build_ui_rendering_module.test.cjs tests/ui_rendering_module.test.cjs tests/ui_interactions_module.test.cjs tests/input_correction_module.test.cjs
```

- [ ] **Step 2: Run the full JS suite**

```bash
node --test tests/*.test.cjs
```

- [ ] **Step 3: Run the fast host-safe regression gate**

```bash
scripts/test.sh
```

- [ ] **Step 4: Rebuild installable assets**

```bash
cmake --build --preset wsl-release
cmake --install build/wsl-release
```

- [ ] **Step 5: Review in-game with these checkpoints**

- Build tab has no root scroll
- center list is the only scroll surface
- discipline/theme switching is immediate
- right rail remains readable while browsing long catalogs
- six-slot matrix remains stable even with many rows

- [ ] **Step 6: Prepare follow-up list**

Capture anything deferred, especially:
- ambiguous legacy effects
- hierarchy tuning
- sorting/filtering refinements
- whether some themes need signpost options before full effect import
