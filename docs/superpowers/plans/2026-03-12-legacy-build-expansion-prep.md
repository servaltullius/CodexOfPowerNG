# Legacy Build Expansion Prep Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Produce the preparation artifacts needed to expand the Build catalog from the current minimal set to a broad legacy-effect catalog without destabilizing runtime behavior or UI structure.

**Architecture:** Do not add broad new effects to the runtime yet. First create a single source of truth inventory that classifies each legacy AV/modifier reward by discipline, theme, hierarchy, runtime applicability, and migration mapping. Use that inventory to split the later implementation into `supported` and `needs-runtime` batches.

**Tech Stack:** Markdown/JSON contract docs, existing C++ build catalog/runtime sources, Node-based contract tests, git

---

## File Map

- Create: [docs/contracts/build-legacy-expansion-inventory.json](/home/kdw73/Codex%20of%20Power%20NG/docs/contracts/build-legacy-expansion-inventory.json)
  - Single inventory matrix for legacy AV/modifier rewards
- Create: [docs/superpowers/specs/2026-03-12-legacy-build-expansion-prep-design.md](/home/kdw73/Codex%20of%20Power%20NG/docs/superpowers/specs/2026-03-12-legacy-build-expansion-prep-design.md)
  - Design record for the preparation phase
- Create: [docs/superpowers/plans/2026-03-12-legacy-build-expansion-prep.md](/home/kdw73/Codex%20of%20Power%20NG/docs/superpowers/plans/2026-03-12-legacy-build-expansion-prep.md)
  - This implementation plan
- Modify: [tests/build_legacy_theme_inventory.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_legacy_theme_inventory.test.cjs)
  - Extend taxonomy checks from theme inventory to preparation inventory rules
- Reference only:
  - [src/RewardsRandomTables.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/RewardsRandomTables.cpp)
  - [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)
  - [src/BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp)

## Chunk 1: Define The Inventory Contract

### Task 1: Add the inventory schema and fail when required fields are missing

**Files:**
- Create: [docs/contracts/build-legacy-expansion-inventory.json](/home/kdw73/Codex%20of%20Power%20NG/docs/contracts/build-legacy-expansion-inventory.json)
- Modify: [tests/build_legacy_theme_inventory.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_legacy_theme_inventory.test.cjs)

- [ ] **Step 1: Write the failing test**

Add assertions that every inventory row contains:
- `legacyKey`
- `sourceGroup`
- `label`
- `discipline`
- `theme`
- `hierarchy`
- `effectType`
- `runtimeKey`
- `magnitude`
- `runtimeStatus`
- `migrationFrom`
- `notes`

- [ ] **Step 2: Run test to verify it fails**

Run:
```bash
node --test tests/build_legacy_theme_inventory.test.cjs
```

Expected: FAIL because the new inventory file or fields do not exist yet.

- [ ] **Step 3: Write the minimal inventory scaffold**

Create the new JSON file with an empty or minimal valid structure that the test can load.

- [ ] **Step 4: Run test to verify it passes**

Run:
```bash
node --test tests/build_legacy_theme_inventory.test.cjs
```

Expected: PASS for schema existence.

- [ ] **Step 5: Commit**

```bash
git add docs/contracts/build-legacy-expansion-inventory.json tests/build_legacy_theme_inventory.test.cjs
git commit -m "test: define legacy build expansion inventory contract"
```

## Chunk 2: Populate Legacy AV/Modifier Inventory

### Task 2: Inventory all non-skill legacy rewards

**Files:**
- Modify: [docs/contracts/build-legacy-expansion-inventory.json](/home/kdw73/Codex%20of%20Power%20NG/docs/contracts/build-legacy-expansion-inventory.json)
- Reference: [src/RewardsRandomTables.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/RewardsRandomTables.cpp)

- [ ] **Step 1: Enumerate eligible legacy rewards**

Include only AV/modifier/utility-style effects from the legacy reward table. Exclude skill rewards such as:
- one-handed
- two-handed
- archery
- block
- heavy/light armor
- alchemy
- lockpicking
- pickpocket

- [ ] **Step 2: Assign discipline/theme/hierarchy**

For each row, assign exactly one:
- discipline
- theme
- hierarchy

Use only the approved themes:
- attack: `devastation / precision / fury`
- defense: `guard / bastion / resistance`
- utility: `livelihood / exploration / finesse`

- [ ] **Step 3: Record notes for ambiguous effects**

When an effect could plausibly fit multiple themes, keep one primary theme and explain the choice in `notes`.

- [ ] **Step 4: Run the test suite for taxonomy drift**

Run:
```bash
node --test tests/build_legacy_theme_inventory.test.cjs
```

Expected: PASS with no invalid themes or missing hierarchy.

- [ ] **Step 5: Commit**

```bash
git add docs/contracts/build-legacy-expansion-inventory.json
git commit -m "docs: inventory legacy build expansion candidates"
```

## Chunk 3: Classify Runtime Applicability

### Task 3: Mark each legacy effect as supported, needs-runtime, or deferred

**Files:**
- Modify: [docs/contracts/build-legacy-expansion-inventory.json](/home/kdw73/Codex%20of%20Power%20NG/docs/contracts/build-legacy-expansion-inventory.json)
- Reference: [src/BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp)

- [ ] **Step 1: Define failing test coverage for runtime status**

Extend the inventory test so each row must use only:
- `supported`
- `needs-runtime`
- `deferred`

Also assert that excluded skill rewards do not appear in the inventory.

- [ ] **Step 2: Mark currently supported runtime keys**

Use the current runtime implementation to mark directly supported rows, including:
- `attack_damage_mult`
- `weapon_speed_mult`
- `critical_chance`
- `damage_resist`
- `block_power_modifier`
- `health`
- `speed_mult`
- `carry_weight`
- `speechcraft_modifier`

- [ ] **Step 3: Mark rows that need runtime work**

Typical candidates:
- elemental/magic/disease/poison resist
- regen effects
- stealth / lockpicking / pickpocket modifiers
- crafting/alchemy/enchanting modifiers
- shout recovery
- absorb chance

- [ ] **Step 4: Mark deferred rows**

Use `deferred` only where:
- the effect is intentionally out of scope now
- the effect is too ambiguous
- the effect is not worth carrying forward into Build

- [ ] **Step 5: Re-run inventory tests**

Run:
```bash
node --test tests/build_legacy_theme_inventory.test.cjs
```

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add docs/contracts/build-legacy-expansion-inventory.json tests/build_legacy_theme_inventory.test.cjs
git commit -m "docs: classify legacy build effects by runtime readiness"
```

## Chunk 4: Lock Migration Rules From The Current Minimal Catalog

### Task 4: Capture mapping from the shipped minimal catalog to the expanded taxonomy

**Files:**
- Modify: [docs/contracts/build-legacy-expansion-inventory.json](/home/kdw73/Codex%20of%20Power%20NG/docs/contracts/build-legacy-expansion-inventory.json)
- Reference: [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)

- [ ] **Step 1: Add failing checks for migration mapping presence**

The inventory test should require that rows corresponding to current shipped options carry valid `migrationFrom` metadata.

- [ ] **Step 2: Record mappings for the current 3x3 catalog**

At minimum cover:
- `build.attack.ferocity`
- `build.attack.precision`
- `build.attack.vitals`
- `build.defense.guard`
- `build.defense.bastion`
- `build.defense.endurance`
- `build.utility.cache`
- `build.utility.barter`
- `build.utility.mobility`

- [ ] **Step 3: Re-run inventory tests**

Run:
```bash
node --test tests/build_legacy_theme_inventory.test.cjs
```

Expected: PASS

- [ ] **Step 4: Commit**

```bash
git add docs/contracts/build-legacy-expansion-inventory.json tests/build_legacy_theme_inventory.test.cjs
git commit -m "docs: map current build catalog into expansion taxonomy"
```

## Chunk 5: Produce Execution Guidance For The Next Implementation Phase

### Task 5: Split follow-up implementation into safe batches

**Files:**
- Modify: [docs/contracts/build-legacy-expansion-inventory.json](/home/kdw73/Codex%20of%20Power%20NG/docs/contracts/build-legacy-expansion-inventory.json)
- Optionally append a short note to this plan if needed

- [ ] **Step 1: Identify the first `supported` batch**

Pick the smallest coherent implementation set that can be shipped first, for example:
- attack precision/devastation expansions
- defense guard/bastion expansions
- utility livelihood/exploration expansions

- [ ] **Step 2: Identify the first `needs-runtime` batch**

Group by shared runtime work instead of by raw theme size.

Examples:
- resistance AV mapping
- regen AV mapping
- stealth/economy modifier mapping

- [ ] **Step 3: Ensure no theme sprawl slipped in**

Verify that no new themes were introduced just to absorb awkward rows.

- [ ] **Step 4: Commit**

```bash
git add docs/contracts/build-legacy-expansion-inventory.json docs/superpowers/plans/2026-03-12-legacy-build-expansion-prep.md
git commit -m "docs: stage legacy build expansion implementation batches"
```

## Chunk 6: Validation And Handoff

### Task 6: Validate the prep artifacts and hand off to the expansion phase

**Files:**
- Reuse updated inventory test above

- [ ] **Step 1: Run focused inventory validation**

```bash
node --test tests/build_legacy_theme_inventory.test.cjs
```

- [ ] **Step 2: Run the full JS suite**

```bash
node --test tests/*.test.cjs
```

- [ ] **Step 3: Review the inventory manually against current runtime support**

Cross-check:
- [src/RewardsRandomTables.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/RewardsRandomTables.cpp)
- [src/BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp)
- [src/BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)

- [ ] **Step 4: Confirm handoff readiness**

The prep phase is complete only if:
- all non-skill legacy AV/modifier rewards are inventoried
- each row has one discipline/theme/hierarchy
- runtime status is assigned
- current shipped options are mapped into the taxonomy
- first implementation batches are obvious from the inventory

- [ ] **Step 5: Commit final prep adjustments**

```bash
git add docs/contracts/build-legacy-expansion-inventory.json tests/build_legacy_theme_inventory.test.cjs
git commit -m "docs: finalize legacy build expansion preparation"
```
