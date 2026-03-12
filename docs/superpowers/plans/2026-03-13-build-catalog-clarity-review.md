# Build Catalog Clarity Review Plan

## Goal

Apply the reviewed clarity fixes to the current 32-option build catalog without expanding scope into new legacy effect batches.

## Phase 1: Naming Pass

1. Rename the four approved option labels in localization and any test fixtures:
   - `정밀` -> `속공`
   - `수호` -> `철갑`
   - `보루` -> `철벽`
   - `파괴` -> `파괴숙련`
2. Verify the Build UI no longer shows theme/option collisions.

## Phase 2: Description Pass

1. Rewrite magic-school cost reduction rows to player-facing wording.
2. Rewrite crafting/trickery rows to outcome-based wording.
3. Rewrite resource/regen rows to “max/recovery/cooldown” wording.
4. Rewrite vague special rows (`반격`, `메아리`, `주문흡수`) so they describe the gameplay effect first.
5. Update JS tests that assert title/description strings.

## Phase 3: Role Separation Pass

1. Redesign `수호 / 방벽` so they are no longer armor-rating duplicates.
2. Redesign `정조준 / 급소` so they are no longer critical-chance duplicates.
3. Update catalog contract tests and runtime tests for the new role split.
4. Re-run Build UI rendering checks to confirm the new rows still read correctly inside the catalog-first layout.

## Phase 4: Review Gate

1. Re-review the catalog for:
   - theme identity
   - slot competition
   - readability in the grouped catalog UI
2. Only after that, start a separate pass for unlock pacing and balance.

## Verification

Minimum verification for this plan:

- `node --test tests/ui_i18n_module.test.cjs tests/build_ui_rendering_module.test.cjs`
- `node --test tests/*.test.cjs`
- relevant catalog/runtime C++ tests if effect semantics change

## Notes

- This plan intentionally does not add new catalog rows.
- This plan intentionally does not include score rebalance.
- If role-separation changes require new runtime mappings, that work should be split into a separate follow-up plan instead of being folded into the naming/clarity pass.
