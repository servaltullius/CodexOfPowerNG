# Modpack Build Point Balance Design

## Summary

The current build system still ties practical power too closely to raw registration count.
That makes large curated modpacks such as Nolvus or LoreRim feel overtuned even after the first compression pass, especially in `Utility`.

The approved direction is to stop compressing late-game score curves and instead make each registered item contribute a fixed amount of build progression from the start.
This keeps every new registration valuable, avoids the feeling that progress is secretly throttled later, and lets the game stay readable in both vanilla-sized and modpack-sized item pools.

The new model separates:

- `recordCount`: honest registration count for each discipline
- `buildPoints`: weighted progression currency used for unlocks and active option scaling

`recordCount` preserves collection motivation and player-visible history.
`buildPoints` keeps unlock and effect growth stable across very different load orders.

## Goals

- Keep the registration loop honest: one successful registration still increases the visible record count.
- Remove the “late-game compression” feeling that weakens motivation in large saves.
- Make item value predictable from the first item to the last item.
- Reduce `Utility` overgrowth structurally instead of relying on late-game tapering.
- Preserve the current three-discipline model and the existing slot layout.
- Keep the first-pass option retunes already implemented unless a later balance pass proves they are still too high.

## Non-Goals

- Do not redesign the Attack / Defense / Utility discipline split.
- Do not introduce spendable currencies, perk trees, or respec points.
- Do not add per-item manual discipline assignment.
- Do not solve every balance issue through one data pass; this change targets progression shape first.
- Do not remove record count from the UI.

## Problem Statement

### 1. Raw count is stable for motivation, but unstable for balance

Raw registration count is a good player-facing metric because it clearly answers “how much have I collected?”
It is a bad universal balance metric because different load orders expose dramatically different item volumes.

When the same `1 registration = 1 score` rule drives both unlock cadence and active stat growth, modpacks produce much higher final power than the catalog was tuned for.

### 2. Compression fixes power, but weakens reward psychology

The previous approved design used `rawScore` and `effectiveScore`, with late-game piecewise compression.
That does solve runaway growth, but it creates a new problem:

- the player keeps registering items
- the displayed count keeps rising
- but each later item matters less than earlier items

That is mathematically safe, but it feels like diminishing returns were added after the fact.
The user explicitly rejected that tradeoff because it weakens the motivation to keep collecting.

### 3. Utility is structurally broader than the other disciplines

The current mapping in [BuildProgression.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildProgression.cpp) sends all of the following into `Utility`:

- `AlchemyItem`
- `Ingredient`
- `Book`
- `Scroll`
- `SoulGem`
- `Misc`

That mapping is still acceptable, but it cannot keep using a universal `1 item = 1 progression unit` rule.
The input pool is too broad.
The fix should happen at contribution size, not through hidden late-game slowdown.

## Chosen Approach

We will replace score compression with weighted fixed-point accumulation.

Rules:

- `recordCount` remains a whole-number registration count.
- `buildPoints` becomes the only value used for:
  - option unlock checks
  - tier calculation
  - scaled magnitude calculation
  - “next upgrade” progress
- every form type contributes the same fixed amount every time it is registered
- the contribution does not change at high totals

One-line rule:

`Collection history stays linear, and progression power also stays linear, but each item type starts with a modpack-safe fixed weight instead of pretending every item is worth the same amount.`

## Build Point Accumulation Model

### Storage Format

To avoid float drift in saves and runtime state, build points should be stored internally as integer hundredths.

Examples:

- `0.80 pt` -> `80`
- `0.10 pt` -> `10`
- `24.0 pt` -> `2400`

The UI may display one decimal place, but serialization and runtime logic should use integer centi-points.

### Approved Fixed Weights

These weights are the approved first pass.
They are intentionally conservative for `Utility`.

| Form type | Discipline | Build points |
| --- | --- | --- |
| `Weapon` | Attack | `0.80` |
| `Ammo` | Attack | `0.35` |
| `Armor` | Defense | `0.40` |
| `AlchemyItem` | Utility | `0.25` |
| `Ingredient` | Utility | `0.10` |
| `Book` | Utility | `0.08` |
| `Scroll` | Utility | `0.18` |
| `SoulGem` | Utility | `0.30` |
| `Misc` | Utility | `0.05` |

### Why These Numbers

- `Weapon` is kept high because Attack has the narrowest practical input pool.
- `Armor` is lower than Weapon because armor counts rise faster in large lists.
- `SoulGem` and `AlchemyItem` stay meaningful, but far below the old `1 per item` rule.
- `Book`, `Ingredient`, and especially `Misc` are intentionally tiny because they are the largest modpack inflation sources.

This is the core balancing idea:

`Utility does not need a slower curve if it starts with a smaller per-item step.`

## Tier And Unlock Model

### Tier Interval

The approved tier cadence is:

- `8.0 pt = 1 tier`

Formula:

```text
tier = floor(buildPoints / 8.0)
```

With centi-points:

```text
tier = buildPointsCenti / 800
```

This keeps tier progression linear forever.

### Unlock Threshold Ladder

The current unlock ordering should remain intact, but its thresholds should move from raw-count semantics to point semantics.

Approved ladder:

| Previous unlock | New unlock |
| --- | --- |
| `5` | `4.0 pt` |
| `10` | `8.0 pt` |
| `15` | `12.0 pt` |
| `20` | `16.0 pt` |
| `25` | `20.0 pt` |
| `30` | `24.0 pt` |
| `35` | `28.0 pt` |
| `40` | `32.0 pt` |

This preserves the catalog’s relative pacing while converting its absolute scale to the new weighted model.

## Expected Balance Envelope

The purpose of the fixed-point model is not to nerf every discipline equally.
It is to preserve meaningful growth while shrinking the large-modpack spread.

Target expectations:

- Attack should remain close to current feel in ordinary saves.
- Defense should land somewhat lower than the current compressed pass at high totals.
- Utility should land much lower than the current compressed pass when the save is dominated by books, ingredients, soul gems, and misc clutter.

Informal target ranges for the current observed save shape:

- `Attack 33 records` -> roughly `22-26 pt`
- `Defense 119 records` -> roughly `47-48 pt`
- `Utility 204 records` -> roughly `26-34 pt`, depending on actual form mix

That puts the current screenshot closer to:

- Attack tier `2-3`
- Defense tier `5`
- Utility tier `3-4`

which is materially safer than the live `Utility 204 -> tier 6` result.

## Runtime Rules

### Registration

- successful registration still increments the visible discipline record count by `1`
- successful registration also adds the form-type-specific build point gain
- undo must subtract both:
  - `1` record count
  - the matching fixed build point amount

### Unlocks

- option unlock checks should use `buildPoints`
- the old `unlockScore` naming should be retired or explicitly migrated to `unlockPoints`

### Scaling

- active option magnitude should use build-point-derived tier only
- next-tier magnitude should use the next linear point threshold, not a compressed helper
- the previous compression helpers should be removed after the migration is complete

## UI And Payload Rules

The UI should not hide the distinction between collection history and progression power.

Approved payload intent:

- keep `recordCount`
- add `buildPoints`
- keep `currentTier`
- replace next-upgrade progress with point-based fields

The Build UI should show both:

- discipline record count
- progression build points

Example presentation:

- `기록 204`
- `빌드 포인트 29.4`
- `다음 강화까지 2.6 pt`

This preserves motivation while making progression math honest.

## Persistence And Migration

### Source Of Truth

The system can no longer derive progression from record count alone, because the same record count can represent very different form mixes.

Therefore the runtime snapshot must persist build-point totals as their own fields.

Approved persistence rule:

- keep existing per-discipline record counts
- add per-discipline build-point totals in centi-points
- continue to keep enough registration history for undo and migration logic

### Migration

Existing saves should be migrated by replaying registered forms and re-deriving:

- record counts
- build-point totals

Migration should use current registered form types when available.
If a historical registration can no longer resolve to a concrete form, it should fall back to the old discipline bucket and use a conservative default weight for that discipline.

Approved conservative fallback weights:

- Attack fallback: `0.60 pt`
- Defense fallback: `0.35 pt`
- Utility fallback: `0.10 pt`

These fallbacks should be rare, but they prevent unresolved historical records from producing zero progression.

## Rejected Alternative

### Piecewise Effective-Score Compression

This design intentionally replaces the earlier `rawScore -> effectiveScore` compression model.

Reasons:

- it fixed balance by weakening later registrations
- it made late-game collection feel less rewarding
- it obscured the practical value of the next item
- it still required special casing `Utility` as a curve problem instead of a contribution problem

The user explicitly preferred a system where every new registration keeps a predictable fixed value.

## Testing Expectations

The implementation should prove all of the following:

- weighted point gain is assigned from form type correctly
- unlock thresholds are now point-based
- runtime magnitudes scale from build-point tier, not record count
- payloads expose both record count and build point progress
- UI copy no longer claims a hidden `10 score = 1 tier` rule
- migration reconstructs safe point totals from existing registrations

## Final Decision

The final approved design is:

- keep record counts for motivation and visibility
- remove late-game compression
- add per-discipline fixed build-point totals
- drive unlocks and scaling from build points only
- use conservative item-type weights, especially in `Utility`

This is the cleanest way to make large modpacks feel fair without making late progression psychologically flat.
