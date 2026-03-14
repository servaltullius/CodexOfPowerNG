# Hybrid Weak Option Competition Design

## Summary

The current weighted build-point model fixed large-modpack overgrowth and reduced wildcard meta abuse.
That solved the "everything scales too hard" problem, but it exposed a second balance issue:

- some options are now fair in absolute power
- but too weak inside their own theme
- so they lose slot competition even when the overall discipline is healthy

The clearest example is `build.attack.reserve`.
At the current `Attack 33 / 21.1 pt / tier 2` snapshot, `Reserve` only grants `+4 Stamina`.
That does not reliably cross a useful action threshold, so it loses to `Secondwind` even after the slot-compatibility pass.

The approved direction is to introduce a limited hybrid model for weak threshold-based options:

- keep one primary fantasy
- add one small supporting effect that improves practical feel
- avoid creating broad "always best" meta picks

This is not a general rewrite of the build catalog.
It is a targeted second-pass rule for options whose current flat bonus is too small to compete inside a constrained slot system.

## Goals

- Make weak threshold-style options compete inside their own theme instead of becoming obvious dead picks.
- Preserve the weighted build-point model and the recent slot meta-dispersion rules.
- Keep options readable: one core stat plus one small support effect at most.
- Avoid turning weak options into new universal best-in-slot choices.
- Make the player decision more about role selection than about finding the largest raw number.

## Non-Goals

- Do not retune every build option in one pass.
- Do not revert the recent `SameDisciplineOnly` / `SameOrWildcard` split.
- Do not add more than two gameplay effects to one option.
- Do not introduce proc logic, conditionals, or combat-state-dependent scripting.
- Do not replace the build-point model with a different progression system.

## Problem Statement

### 1. Threshold-based stats behave worse than linear efficiency stats

Some stats produce value immediately even in small amounts:

- damage
- resistances
- cooldown reduction
- cost reduction

Other stats often need to cross a threshold before the player really feels them:

- max stamina
- max magicka
- some health bonuses

In Skyrim, these threshold stats can be mathematically positive but experientially weak if the granted amount is too small.

### 2. Intra-theme competition matters more than absolute balance

After the recent balance passes, the main question is no longer:

`Is this discipline globally overpowered?`

The better question is:

`When a player has only one or two slots in this role, does each option have a believable reason to be chosen?`

If the answer is no, the option is effectively dead even if its number is technically fair.

### 3. Weak options should not be fixed by raw inflation alone

Pure number buffs are easy to ship, but they create two common failure modes:

- the option is still too weak if the number misses the threshold
- the option suddenly becomes the dominant pick once the threshold is exceeded

That is exactly the kind of brittle tuning that recreates static meta.

## Design Principles

### Role Separation Over Raw Magnitude

Inside one theme, options should answer different player questions.

Examples:

- `Reserve`: "How do I fit one more aggressive action into a short burst window?"
- `Secondwind`: "How do I sustain repeated actions across a longer fight?"

That is healthier than making both options compete only as alternate ways to increase stamina economy.

### One Support Effect Maximum

Hybrid options must remain simple.

Rule:

- one primary effect
- one support effect

No third rider, no hidden conditionals, no broad utility package.

### Support Effect Must Reinforce Theme, Not Replace It

Bad pattern:

- max stamina + damage

Good pattern:

- max stamina + small stamina regeneration

The support effect should strengthen the same role, not broaden the option into a universal winner.

### Specialist Options Should Stay Specialist

If a theme already has a strong "flow" option, the hybrid threshold option should not erase it.

Example:

- `Reserve` should become a better burst threshold tool
- `Secondwind` should remain the sustain specialist

## Chosen Approach

We will use a targeted hybrid pass for a small first-wave set of weak options.

Phase 1 target set:

- `build.attack.reserve`
- `build.utility.magicka`
- `build.utility.hauler`
- `build.utility.meditation`

These four options are enough to validate the pattern without disturbing already-healthy competition in other themes.

## Role Map

### Attack / Fury

#### `build.attack.reserve`

Current role:

- threshold stamina pool

Current weakness:

- the granted stamina is too low to reliably change short-fight decision making

New role:

- burst threshold option

Approved direction:

- larger flat stamina gain
- smaller stamina regeneration rider

Interpretation:

- `Reserve` helps you cross the "one more action" threshold
- `Secondwind` remains the better sustained recovery pick

#### `build.attack.secondwind`

Role remains:

- sustained stamina tempo

No hybrid change in this pass.
Its current identity is already clear enough once `Reserve` becomes more practical.

### Utility / Livelihood

#### `build.utility.magicka`

Current role:

- threshold magicka pool

Current weakness:

- the flat amount is too low to justify a slot over more immediately useful utility picks

New role:

- casting pool anchor

Approved direction:

- larger flat magicka gain
- smaller magicka regeneration rider

Interpretation:

- `Magicka` is the "I want a deeper spell pool" option
- `Meditation` remains the regen-first option

#### `build.utility.meditation`

Current role:

- magicka regeneration

Current weakness:

- if `Magicka` is buffed and `Meditation` is left untouched, the two options can collapse into one-sided competition

New role:

- regen specialist

Approved direction:

- retain regen-first identity
- optionally add a small flat magicka floor if needed, but keep regen clearly dominant

Interpretation:

- `Magicka` helps with pool depth
- `Meditation` helps with casting rhythm

#### `build.utility.hauler`

Current role:

- flat stamina bonus in the livelihood theme

Current weakness:

- its fantasy overlaps poorly with `Cache`
- it does not currently read as a clear expedition alternative

New role:

- expedition utility

Approved direction:

- flat stamina
- small carry-weight rider

Interpretation:

- `Cache` is pure hauling capacity
- `Hauler` is "travel farther while carrying a bit more"

This keeps the two options adjacent but not redundant.

## First-Pass Numerical Targets

These are starting targets for implementation, not permanent truth.

### `build.attack.reserve`

- base: `+8 Stamina`
- support: `+8% Stamina Rate`
- per tier: `+2 Stamina`, `+4% Stamina Rate`

### `build.utility.magicka`

- base: `+12 Magicka`
- support: `+4% Magicka Rate`
- per tier: `+4 Magicka`, `+2% Magicka Rate`

### `build.utility.meditation`

- base: `+8% Magicka Rate`
- support: `+4 Magicka`
- per tier: `+4% Magicka Rate`, `+2 Magicka`

### `build.utility.hauler`

- base: `+8 Stamina`
- support: `+8 Carry Weight`
- per tier: `+2 Stamina`, `+2 Carry Weight`

These numbers are intentionally modest.
The purpose is to create believable choice, not to create a new dominant package.

## Runtime Implementation Model

The preferred implementation path is to keep the current catalog/runtime architecture and add a small number of composite effect keys.

Recommended new effect keys:

- `reserve_bundle`
- `magicka_well_bundle`
- `meditation_bundle`
- `hauler_bundle`

Each key should resolve to multiple actor-value deltas inside [BuildEffectRuntime.cpp](/home/kdw73/Codex%20of%20Power%20NG/src/BuildEffectRuntime.cpp).

This is preferable to introducing a new effect system because:

- the existing payload and magnitude logic already supports catalog-driven scaling
- bundle keys keep the change localized
- the option catalog remains readable

## UI And Payload Rules

The Build panel must not show a hybrid option as a vague generic number.

Requirements:

- selected option detail must describe both parts of the bundle
- catalog row preview should show the combined player-facing effect text
- next-tier preview should also show both upgraded values

This means payload formatting needs explicit handling for the new bundle keys, not fallback formatting.

## Guardrails

To prevent new meta lock-in:

- no hybrid option may include direct damage, resistance, or global cost reduction unless that was already its primary identity
- no option in this pass gets more than two stat outputs
- specialist wildcard-compatible options must remain meaningfully narrower than broad signpost options
- if the hybrid version becomes the obvious best pick in all normal loadouts, the correct fix is usually to reduce the support rider first, not the primary fantasy

## Expected Outcomes

If this pass works:

- `Reserve` should stop feeling like a dead pick next to `Secondwind`
- `Magicka` and `Meditation` should separate into pool-vs-rhythm choices
- `Hauler` should stop feeling like a failed version of `Cache`
- players should have more reasons to pick different options inside the same theme without reintroducing runaway power

## Validation Strategy

We should validate at three layers:

1. Catalog contract
- confirm the targeted options now use the new bundle effect keys and intended magnitudes

2. Runtime totals
- prove each bundle resolves to the expected concrete actor values

3. Build UI
- prove the Build panel explains both current and next-tier bundle values cleanly in Korean and English

## Open Question Deferred

`build.defense.endurance` likely belongs in the same family of threshold-sensitive options.
However, this pass should not include it yet.

Reason:

- Attack and Utility already provide enough signal to validate the hybrid pattern
- Defense just went through a strong compression and anti-meta pass
- adding too many simultaneous hybrid changes will make it harder to evaluate which role split actually worked

If the first-wave pass succeeds, `Endurance` can be reviewed as the next candidate.
