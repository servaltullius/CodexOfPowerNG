# Resistance Theme Redesign

## Context

The current `resistance` theme still loses slot competition after the utility, attack, guard, and exploration timing passes.

Current shipped rows:

- `build.defense.warding` -> `magic_resist`
- `build.defense.fireward` -> `fire_resist`
- `build.defense.frostward` -> `frost_resist`
- `build.defense.stormward` -> `shock_resist`
- `build.defense.antidote` -> `poison_resist`
- `build.defense.purity` -> `disease_resist`
- `build.defense.absorption` -> `absorb_chance`

Current defense slot economy:

- `Defense1`
- `Wildcard1`

The theme asks the player to spend one of those scarce slots on a narrow, situational mitigation line. That trade is consistently worse than broad `guard` or `bastion` rows.

## Problem Statement

The failure mode is structural, not timing-only.

- Unlocking the rows earlier does not fix uptake.
- Individual elemental or status rows are too narrow for the slot cost.
- `resistance` reads like a bag of niche counters instead of a coherent defense identity.
- The current catalog shape also scales poorly. More narrow rows would make the theme longer without making it stronger.

The redesign must therefore broaden each resistance row so a slotted choice protects against multiple threat types at once.

## Scope

In scope:

- redesigning the `resistance` theme into broader bundled rows
- updating runtime support so one slotted option can drive multiple actor values
- updating catalog contracts, copy, and tests

Out of scope:

- changing slot counts
- changing the other defense themes
- importing new legacy effect families beyond the existing resistance set

## Approaches Considered

### 1. Keep individual rows and only change scores

Rejected.

This has already failed in practice. The theme would remain narrow and slot-inefficient.

### 2. Convert some resistance rows into baseline milestones

Rejected.

This would hide the problem instead of solving it. It also weakens the distinction between `guard` and `resistance` by moving thematic mitigation out of slotted choice.

### 3. Replace narrow rows with bundled resistance packages

Recommended.

This preserves `resistance` as a real theme while making each slot matter. It matches the current build philosophy: a slotted option should produce a broad, legible change in how the build plays or survives.

## Proposed Theme Structure

The theme should shrink from seven narrow rows to four broad rows:

1. `build.defense.warding`
- keep as the signpost
- broad magical mitigation
- remains the general anti-magic identity row

2. `build.defense.elementalWard`
- new bundled elemental defense row
- covers `fire_resist`, `frost_resist`, and `shock_resist`

3. `build.defense.purification`
- new bundled status-defense row
- covers `poison_resist` and `disease_resist`

4. `build.defense.absorption`
- keep as the capstone/special row
- high-impact magical defense payoff

Rows removed as standalone options:

- `build.defense.fireward`
- `build.defense.frostward`
- `build.defense.stormward`
- `build.defense.antidote`
- `build.defense.purity`

## Naming And UX Intent

The theme should read as broad magical defense rather than itemized counters.

Recommended player-facing shape:

- `마법결계 / Magic Ward`
- `원소방벽 / Elemental Ward`
- `정화결계 / Purification Ward`
- `주문흡수 / Absorption`

Intent by row:

- `마법결계`: “I am broadly safer against magic.”
- `원소방벽`: “I am broadly safer against elemental damage.”
- `정화결계`: “I am broadly safer against poison and disease.”
- `주문흡수`: “I get a rare, high-impact magical defense payoff.”

## Runtime Design

The current catalog/runtime model assumes one option maps to one `effectKey`. That is not sufficient for bundled rows.

The redesign should introduce bundled runtime keys while keeping `BuildEffectType::ActorValue`.

Recommended new synthetic runtime keys:

- `magic_resist_bundle`
- `elemental_resist_bundle`
- `status_resist_bundle`

Runtime behavior:

- `magic_resist_bundle`
  - maps to `RE::ActorValue::kMagicResist`
- `elemental_resist_bundle`
  - maps to:
    - `RE::ActorValue::kFireResist`
    - `RE::ActorValue::kFrostResist`
    - `RE::ActorValue::kElectricResist`
- `status_resist_bundle`
  - maps to:
    - `RE::ActorValue::kPoisonResist`
    - `RE::ActorValue::kDiseaseResist`

This keeps the catalog shape stable:

- one row
- one option id
- one slot choice

But allows runtime application to fan out to multiple concrete actor values.

## Magnitude Rules

This redesign is primarily structural. Exact balance can still be tuned later, but the rows should be broad enough to justify a scarce defense slot.

Initial guidance:

- `magic_resist_bundle`
  - keep close to current `warding` intent, but express it as a broad magic-defense row
- `elemental_resist_bundle`
  - each elemental branch should receive the same magnitude
- `status_resist_bundle`
  - poison and disease should receive the same magnitude
- `absorption`
  - remains singular and special

The first implementation should favor clarity and slot relevance over perfect numeric tuning.

## Migration Rules

Existing save/build state may reference removed resistance option ids.

Migration rules:

- if an active slot contains `build.defense.fireward`, `build.defense.frostward`, or `build.defense.stormward`
  - migrate to `build.defense.elementalWard`
- if an active slot contains `build.defense.antidote` or `build.defense.purity`
  - migrate to `build.defense.purification`
- `build.defense.warding` remains stable
- `build.defense.absorption` remains stable

This mapping should also be reflected in the legacy expansion inventory notes.

## UI Impact

No structural build-UI redesign is required for this pass.

Expected improvements:

- fewer rows in the `resistance` theme
- less scrolling inside the defense catalog
- stronger signpost/standard/special progression
- clearer slot value when comparing `resistance` to `guard` or `bastion`

## Testing Strategy

Required coverage:

- build catalog contract:
  - removed rows no longer present
  - new bundled rows present with correct ids/theme/hierarchy/unlock order
- build runtime:
  - bundled rows fan out to all expected actor values
  - once-only behavior still prevents double-stacking
- build UI rendering:
  - new titles and descriptions appear
  - theme row count reflects the reduced catalog
- migration/serialization:
  - legacy option ids remap correctly if active in saved state

## Success Criteria

This redesign is successful when:

- `resistance` no longer depends on score shuffling to be attractive
- each resistance slot choice protects against multiple threats
- the theme is shorter and easier to scan
- runtime support remains deterministic and save-safe
- the build UI can show the redesigned theme without further structural changes
