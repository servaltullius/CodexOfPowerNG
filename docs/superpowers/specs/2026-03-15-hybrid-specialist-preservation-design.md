# Hybrid Specialist Preservation Design

## Summary

The first hybrid weak-option pass solved one real problem:

- `Reserve`
- `Magicka Well`
- `Meditation`
- `Long Haul`

were no longer obvious dead picks.

But the current numbers introduced a new risk:

- hybrid options now solve too much at once
- they begin to crowd out the specialist options they were supposed to sit beside
- slot competition can collapse back into a new meta, just with different winners

This pass defines the corrective rule:

`specialist options must remain best-in-slot inside their own stat lane; hybrid options may be broader, but must lose on pure efficiency.`

## Problem

The current hybrid tuning overreaches in three competition pairs:

### 1. `Reserve` vs `Secondwind`

Current hybrid state:

- `Reserve`: stamina pool plus meaningful stamina regeneration
- `Secondwind`: pure stamina regeneration specialist

Risk:

- `Reserve` now improves burst and partially covers sustain
- if its regeneration rider is too large, it reduces the reason to pick `Secondwind`

### 2. `Magicka Well` vs `Meditation`

Current hybrid state:

- `Magicka Well`: magicka pool plus regen
- `Meditation`: regen plus a smaller pool rider

Risk:

- both now affect pool and rhythm
- if `Magicka Well` carries too much regen, the specialist identity of `Meditation` weakens

### 3. `Long Haul` vs `Cache`

Current hybrid state:

- `Long Haul`: stamina plus carry weight
- `Cache`: pure carry weight specialist

Risk:

- `Long Haul` can start reading like “carry weight plus extra utility”
- if the carry rider is too high, `Cache` stops being the clear hauling specialist

## Design Goal

Preserve the value of the hybrid model without allowing it to erase specialist picks.

Success means:

- hybrid options are viable
- specialist options still own their stat lane
- the player picks between shapes of value, not between “strictly more stuff” and “less stuff”

## Core Rule

For every hybrid/specialist pair:

1. The specialist option must remain the strongest pure solution for its lane.
2. The hybrid option may address two nearby needs, but each one must be meaningfully weaker than the specialist's main output.
3. If a hybrid option starts competing on the specialist's core metric, the support rider is the first thing to reduce.
4. If that is not enough, reduce the hybrid's primary stat before buffing the specialist further.

This is a role-separation rule, not just a number-tuning rule.

## Reference Snapshots

All competition checks in this pass use the same three checkpoints.

### 1. Unlock Snapshot

Definition:

- evaluate the option at tier `0`

Purpose:

- prove the option is not dead on arrival

### 2. Representative Mid Snapshot

Definition:

- evaluate the option at tier `2`

Purpose:

- this is the current real-world comparison zone where the hybrid overreach was observed

### 3. Next-Tier Preview Snapshot

Definition:

- evaluate the tier `2` option's next-tier preview, which means tier `3`

Purpose:

- keep the Build panel current/next preview aligned with the same competition contract

These checkpoints are mandatory for runtime sanity review and Build panel verification.

## Chosen Approach

Use the existing hybrid structure, but reduce support riders so that specialists stay clearly ahead on their main axis.

Why this approach:

- it preserves the fantasy already introduced by the last pass
- it avoids another large runtime/UI structural change
- it is safer than reverting hybrids completely
- it directly targets the new meta risk without recreating dead options

## Rejected Alternatives

### 1. Revert hybrids entirely

Rejected because:

- it would likely bring back the original dead-pick problem
- the threshold-style options would again struggle to justify a slot

### 2. Buff specialists harder instead of trimming hybrids

Rejected as the first move because:

- it escalates overall power
- it risks creating two strong options instead of restoring healthy differentiation

### 3. Delay specialists later in unlock order

Rejected because:

- this is not primarily a pacing problem
- the issue is lane ownership, not unlock timing

## Competition Contracts

### Attack / Fury

#### `Reserve`

Role:

- burst threshold option

Allowed strengths:

- helps the player fit one more aggressive action into a short window

Must not do:

- rival `Secondwind` in long-fight stamina sustain

#### `Secondwind`

Role:

- stamina sustain specialist

Must remain:

- the best pure stamina regeneration choice in the fury theme

Quantitative contract:

- at tier `0`, `Reserve` stamina regen must stay below `Secondwind`
- at tier `2`, `Reserve` stamina regen must stay below `Secondwind`
- at tier `3` preview, `Reserve` stamina regen must stay below `Secondwind`
- `Reserve` must still provide at least:
  - tier `0`: `+8 stamina`
  - tier `2`: `+12 stamina`
  - tier `3`: `+14 stamina`

### Utility / Livelihood

#### `Magicka Well`

Role:

- spell pool anchor

Allowed strengths:

- increases available magicka enough to support larger casts or one more cast in a sequence

Must not do:

- rival `Meditation` in magicka rhythm and sustain

#### `Meditation`

Role:

- magicka sustain specialist

Must remain:

- the best pure magicka regeneration choice in livelihood

Quantitative contract:

- at tier `0`, `Magicka Well` magicka regen must stay below `Meditation`
- at tier `2`, `Magicka Well` magicka regen must stay below `Meditation`
- at tier `3` preview, `Magicka Well` magicka regen must stay below `Meditation`
- at tier `0`, `Magicka Well` magicka pool must exceed `Meditation`
- at tier `2`, `Magicka Well` magicka pool must exceed `Meditation`
- at tier `3` preview, `Magicka Well` magicka pool must exceed `Meditation`

#### `Long Haul`

Role:

- expedition utility

Allowed strengths:

- mixes some travel stamina with some extra carrying room

Must not do:

- rival `Cache` in pure hauling capacity

#### `Cache`

Role:

- carrying specialist

Must remain:

- the best pure carry-weight option in livelihood

Quantitative contract:

- at tier `0`, `Long Haul` carry weight must stay below `Cache`
- at tier `2`, `Long Haul` carry weight must stay below `Cache`
- at tier `3` preview, `Long Haul` carry weight must stay below `Cache`
- `Long Haul` must still provide at least:
  - tier `0`: `+6 stamina`, `+4 carry weight`
  - tier `2`: `+10 stamina`, `+6 carry weight`
  - tier `3`: `+12 stamina`, `+7 carry weight`

## Approved Numerical Direction

These are the approved targets for the next corrective pass.

### `build.attack.reserve`

Keep the burst pool identity, trim sustain:

- primary: `+8 Stamina`
- primary per tier: `+2 Stamina`
- support: `+4% Stamina Rate`
- support per tier: `+2% Stamina Rate`

Reference specialist:

- `build.attack.secondwind` stays at `+10% Stamina Rate`, `+5% per tier`

Design result:

- `Reserve` helps burst windows
- `Secondwind` clearly wins sustained regeneration

### `build.utility.magicka`

Keep pool-first identity, trim regen:

- primary: `+12 Magicka`
- primary per tier: `+4 Magicka`
- support: `+2% Magicka Rate`
- support per tier: `+1% Magicka Rate`

### `build.utility.meditation`

Keep regen-first identity, reduce pool rider:

- primary: `+8% Magicka Rate`
- primary per tier: `+4% Magicka Rate`
- support: `+2 Magicka`
- support per tier: `+1 Magicka`

Design result:

- `Magicka Well` wins on pool depth
- `Meditation` wins on rhythm and sustain

### `build.utility.hauler`

Reduce both axes to preserve `Cache`:

- primary: `+6 Stamina`
- primary per tier: `+2 Stamina`
- support: `+4 Carry Weight`
- support per tier: `+1 Carry Weight`

Reference specialist:

- `build.utility.cache` stays at `+20 Carry Weight`, `+1 per tier`

Design result:

- `Long Haul` becomes a travel/generalist option
- `Cache` remains the clear hauling specialist

## Implementation Boundary

The corrective pass stays inside the current hybrid bundle structure, but the ownership split must be explicit.

### Primary Values

Owned by:

- catalog definitions in [BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)

Fields:

- `magnitude`
- `magnitudePerTier`

This controls:

- `Reserve` primary stamina
- `Magicka Well` primary magicka
- `Meditation` primary magicka regen
- `Long Haul` primary stamina

### Support Riders

Owned by:

- `GetResolvedBuildEffectBundle()` in [BuildOptionCatalog.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildOptionCatalog.cpp)

This controls:

- `Reserve` support stamina regen
- `Magicka Well` support magicka regen
- `Meditation` support magicka floor
- `Long Haul` support carry weight

### Player-Facing Copy

Owned by:

- [ui_i18n.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_i18n.js)

Required description keys:

- `build.attack.reserve.description`
- `build.utility.magicka.description`
- `build.utility.meditation.description`
- `build.utility.hauler.description`

### Test Surfaces

Required:

- catalog contract
- runtime totals
- Build panel rendering
- i18n description contract

## UI Rule

The UI must continue to present both parts of a hybrid option, but the copy should not imply that the option is a specialist replacement.

Implications:

- descriptions should emphasize the primary fantasy first
- row/detail text can still list both parts
- no wording should suggest “best” or “stronger version” of the specialist option

Required verification rule:

- the four description keys above must use primary-first wording
- the copy must not imply better sustain than `Secondwind`, better regen than `Meditation`, or better hauling than `Cache`

## Implementation Scope

In scope:

- adjust hybrid bundle magnitudes
- update descriptions where needed
- update runtime and UI tests for the new competition contract

Out of scope:

- changing slot compatibility again
- adding new bundle types
- changing unlock points
- retuning unrelated themes

## Validation

Validation should answer one question:

`Does each specialist still clearly own its stat lane after the hybrid pass?`

Required checks:

1. Catalog contract
- verify updated hybrid primary and support values

2. Runtime totals
- verify the concrete actor values match the new reduced riders

3. UI rendering
- verify current and next-tier text still show both parts cleanly

4. Copy contract
- verify the four localized description keys match the specialist-preserving wording

5. Pairwise sanity review
- `Reserve < Secondwind` in stamina regen at tier `0`, `2`, `3`
- `Reserve` still clears the minimum stamina floor at tier `0`, `2`, `3`
- `Magicka Well < Meditation` in magicka regen at tier `0`, `2`, `3`
- `Magicka Well > Meditation` in magicka pool at tier `0`, `2`, `3`
- `Long Haul < Cache` in carry weight at tier `0`, `2`, `3`
- `Long Haul` still clears the expedition floor at tier `0`, `2`, `3`

6. Hybrid viability review
- confirm the corrective pass did not recreate dead picks:
  - `Reserve` still has a burst-threshold use-case
  - `Magicka Well` still has a pool-depth use-case
  - `Long Haul` still has visible expedition value

## Expected Outcome

If this pass works:

- hybrids remain selectable
- specialists stop being overshadowed
- slot choice becomes “narrow but strongest” versus “broader but weaker,” which is the intended competition shape
