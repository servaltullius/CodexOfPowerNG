# Codex of Power NG Record-Build Rework Design

## Summary

Codex of Power NG will shift from a random permanent reward model to a record-driven build model.
The core loop remains `collect -> register -> persist progress`, but rewards will no longer be granted as immediate random actor-value deltas.
Instead, registration progress will accumulate discipline scores, unlock build options, and let the player actively choose which larger effects are currently applied.

This keeps the mod's long-term growth fantasy intact while solving the current pain points:

- unwanted permanent rolls such as overly high armor growth
- demand for per-option rollback or exclusion
- unsafe feeling around batch registration when large random rewards can spike instantly

## Goals

- Preserve the mod's identity as a progression system built on persistent item registration.
- Make progression meaningfully affect player builds.
- Replace low-control random reward spikes with player-directed build assembly.
- Support safe batch registration once quest-item and favorite protections are in place.
- Keep the UI understandable in high-DPI and large-inventory scenarios.

## Non-Goals

- Rebuild the mod into a full perk tree RPG with dozens of branching dependencies.
- Add per-item manual discipline assignment during registration.
- Support build swapping in combat.
- Keep legacy random reward behavior as an optional parallel progression path.

## Current Problems

The current rewards model grants immediate permanent bonuses at cadence thresholds.
This creates strong growth, but it also creates three related issues:

1. High-impact stats can overshoot, especially defensive values.
2. Players want to remove or avoid specific outcomes without resetting everything.
3. Batch registration is hard to trust when many registrations can immediately push permanent stats in an unwanted direction.

The issue is not that rewards are too strong in principle.
The issue is that the current model combines:

- permanent progression
- random selection
- immediate application

in a way that gives players too little control once a build starts to drift.

## Design Principles

- Registration remains the source of long-term progression.
- The player should feel ownership over build direction.
- Growth should be persistent, but active build effects should be adjustable.
- Registration UX should optimize for throughput and safety.
- Build UX should optimize for comparison, selection, and slot management.
- The simplest structure that solves the feedback is preferred over a more elaborate talent-tree system.

## Core Progression Model

### 1. Three Disciplines

The build system will use three disciplines:

- Attack
- Defense
- Utility

These are the only top-level progression buckets.
All registration-derived build scores, unlocks, and active-slot decisions will be organized around these three disciplines.

### 2. Fixed Registration Mapping

Registration contributes to disciplines through fixed mapping rather than player choice at registration time.

- Weapons and ammo -> Attack
- Armor and accessories -> Defense
- Books, ingredients, consumables, and misc -> Utility

This keeps registration fast, keeps batch registration viable, and preserves a readable link between what the player records and how their build grows.

### 2.5. Discipline Resource Semantics

The first implementation will use one permanent score per discipline:

- `AttackScore`
- `DefenseScore`
- `UtilityScore`

These scores are not spendable currency.
They are durable progression totals derived from successful registrations that remain after normal play.

The scores are used for both:

- passive baseline progression thresholds
- slotted-option unlock thresholds

This means:

- registration increases score
- unlocking does not spend score
- the player never has to trade away existing progression in order to open an option
- the UI can always show current score, next threshold, and currently available options without computing a separate currency balance

This non-consumable model is the recommended first release because it keeps the system deterministic for saves, undo, migration, and UI.

### 3. Unlock vs Active Effect

The new system separates ownership from current application.

- `Unlocked`: the player's current discipline score meets the option threshold
- `Active`: the option is currently occupying a slot and affecting the build

This is the key structural change.
Progress still accumulates permanently, but the current build can be adjusted without requiring a full reset.

### 4. Two Reward Layers

The system will have two layers of progression:

- Small baseline progression that can remain cumulative and always-on
- Larger build-defining options that require activation slots

The purpose of this split is to preserve the feeling that registration always matters while preventing high-impact stats from stacking uncontrollably.

Examples:

- Small baseline progression: modest per-discipline advancement that always persists
- Slotted progression: larger bonuses such as attack specialization, defense specialization, carry utility, economy utility, or other strong modifiers

The exact numeric values are balancing work and are not part of this design document.
What matters here is the structural rule: strong effects should mostly live in the slotted layer.

### 5. Initial Option Contract

The first release needs a fixed native option catalog with stable IDs.
The catalog should not be user-editable in external files during the first implementation.

Each slotted option must define at least:

- `id`: stable string identifier
- `discipline`: Attack | Defense | Utility
- `layer`: Slotted
- `unlockScore`: required discipline score
- `slotCompatibility`: same-discipline slot, wildcard slot, or both
- `effectType`: explicit effect category such as actor value, carry weight, economy, loot, or similar
- `magnitude`: numeric or structured payload used by the effect implementation
- `exclusivityGroup`: optional key for mutually exclusive options
- `stackRule`: first release rule is `once only`
- `titleKey`
- `descriptionKey`

Baseline progression milestones should use a smaller contract:

- `discipline`
- `threshold`
- `effectType`
- `magnitude`

Baseline milestones are not separate cards and do not occupy slots.
They are derived passive effects tied to discipline score thresholds.

### 6. Unlock and Loss Rules

Unlock availability is derived from current discipline score, not from a one-time purchase.
This keeps the system consistent with registration undo.

That means:

- normal play makes unlocks feel permanent because registrations persist
- explicit registration rollback can reduce a discipline score
- if a score drops below an option threshold, that option becomes locked again
- if a now-locked option was active, it is automatically deactivated

This rule prevents exploit loops where a player could unlock an option and then keep it after undoing the progress that enabled it.

## Slot Model

### Recommended Slot Layout

Initial slot layout:

- Attack: 2
- Defense: 1
- Utility: 2
- Wildcard: 1

This is intentionally asymmetrical.
Defense is the most likely to create survivability spikes, so it gets the tightest dedicated slot budget.
Attack and Utility have more room for expression.
One wildcard slot gives the player flexibility without dissolving the discipline identity.

### Slot Rules

- A slotted option only applies while it occupies an active slot.
- Players can swap active options in the UI when out of combat.
- Players cannot swap build options during combat.
- A discipline slot can only host options from that discipline.
- The wildcard slot can host an option from any discipline.

## Rollback and Respec Model

### Primary Recovery Path

The first-class recovery path is not "refund everything."
It is "deactivate or replace the option causing the problem."

Examples:

- If Defense feels too strong, the player removes or swaps the relevant Defense option.
- If Utility is more valuable for the current character, the player moves the wildcard slot to a Utility option.

This makes per-option control a natural part of the build system instead of a patch feature.

### Secondary Recovery Path

A deeper respec system can exist later, but it is not required for the first implementation.

Future-compatible possibilities:

- discipline-level reset
- discipline-point reallocation
- explicit "refund unlocked option" flow

These should remain optional follow-ups.
The first release of the rework should prove that slot-based control already solves most user pain.

### Blacklisting

Global per-option blacklisting becomes much less important once strong options are unlocked-but-optional rather than forced on grant.
For the first release, blacklisting should not be treated as required.

## Registration UX

### Register Screen Role

The register screen becomes the conversion screen:

- review eligible items
- understand which discipline they feed
- safely select many items
- confirm batch registration with a visible summary

It should not behave like a decorative gallery.
It is a high-throughput inventory work surface.

### Register Screen Layout

Default layout:

- one vertical list
- grouped by discipline sections
- section order: Attack, Defense, Utility
- each row shows name, discipline badge, relevant state tags, quantity, safe quantity, and register action
- default list scope is the full register-relevant owned inventory, not only immediately actionable rows

Recommended supporting UI:

- search
- sort
- actionable-only filter
- section headers
- checkbox multi-select
- batch register button
- sticky or persistent selection summary panel

### Row Information

Each row should expose enough information to make batch selection trustworthy:

- item name
- discipline badge
- protected / quest / blocked / already registered state when relevant
- total owned count
- safe removable count
- single-item register action
- multi-select eligibility

Rows that are visible but not actionable should remain disabled with a clear reason tag.
The player should understand why an item cannot be registered instead of assuming it is missing from the list.

### Batch Registration Flow

Batch registration should use an explicit select-then-confirm flow.

- The player checks multiple rows.
- A summary panel shows how many items are selected.
- The summary shows expected discipline gains from the pending registration.
- The player confirms batch registration intentionally.

Each selected row represents exactly one registration action for that row, matching the existing single-register behavior.
`Safe removable count` is explanatory and determines actionability, but batch confirmation does not automatically register the full safe stack count of a row.

The system should avoid one-click "register all eligible items" as the default primary action.

## Build UX

### Build Screen Role

The old rewards screen becomes a build screen.
Its job is to help the player:

- understand current discipline scores
- browse available and unavailable options
- activate or swap current options
- understand what is currently shaping the build

### Build Screen Layout

Recommended layout:

- top summary for current discipline scores and slot usage
- discipline tabs: Attack / Defense / Utility
- central option card list for the selected discipline
- side panel showing active slots and current build summary

### Card States

Each option card should clearly communicate one of these states:

- Locked
- Unlocked
- Active

Each state should support its own action:

- Locked -> show requirements
- Unlocked -> activate action
- Active -> swap out or deactivate action

There is no separate manual unlock step in the first implementation.
Once a discipline score reaches an option threshold, that option becomes unlocked automatically.

### Why Cards Instead of a Tree

The system intentionally favors cards over a perk-tree graph.
The player task is comparison and slot management, not pathfinding through a complex dependency network.
Cards are also more maintainable and clearer in an overlay UI.

## State and Save Model

The rework introduces new persistent concepts:

- discipline score totals
- active slot selections
- migration version/state

The first implementation should persist:

- `attackScore`
- `defenseScore`
- `utilityScore`
- active slot entries
- migration version/state

Unlock availability does not need to be the primary saved source of truth.
It can be recomputed from discipline scores and the fixed native option catalog.

Baseline progression also does not need its own separate save table.
It should be derived from current discipline scores at runtime.

The legacy random reward state should no longer be the authoritative long-term progression model.

## Migration Strategy

### Existing Saves

This rework is not structurally compatible with the current random permanent reward model.
The recommended migration behavior is:

1. Preserve registered-item history.
2. Derive discipline scores from registered history using a deterministic conversion rule.
3. Start the new build system with no slotted options active by default.
4. Apply one legacy-reward cleanup path based on the stored legacy reward totals.
5. Mark migration complete only after score derivation and legacy cleanup both succeed.
6. Inform the player that their legacy reward state was migrated into discipline scores.

### Registered History Conversion Rules

Migration from registered history to discipline score should follow one deterministic rule set:

- if a stored registered entry already has a valid legacy group, map it directly
- if the legacy group is missing or unknown, resolve the form and apply the new fixed discipline mapping
- if the form cannot be resolved, skip that entry and count it as migration loss

Legacy discovery-group to discipline mapping:

- Weapons -> Attack
- Armors -> Defense
- Consumables -> Utility
- Ingredients -> Utility
- Books -> Utility
- Misc -> Utility

If migration skips unresolved entries, the player must receive a one-time notice that some historical registrations could not be converted because their source forms were unavailable.

### Legacy Reward Cleanup Rule

Legacy reward cleanup must use one path only.
The first implementation should treat stored legacy reward totals as the single cleanup source of truth and perform one one-time actor-value cleanup pass from that state.

Migration must not alternate between "clear" and "refund" behaviors based on circumstance.
If player or actor-value access is unavailable, cleanup is deferred, not replaced with another rule.

### Migration State Machine

Migration should use explicit runtime states:

- `not_started`
- `pending_cleanup`
- `complete`

`not_started`:

- legacy reward data exists
- new build state has not committed yet

`pending_cleanup`:

- discipline scores have been derived from current registered history
- active slots are empty
- legacy reward cleanup still needs to finish on a safe runtime boundary

`complete`:

- legacy reward cleanup has finished
- build state is authoritative
- future loads must not rerun migration

### Idempotency and Retry Rules

Migration must be safe to retry.

- discipline scores are derived from current registered history, so recomputation is deterministic
- active slots remain empty until migration reaches `complete`
- the completion marker is only written after legacy cleanup succeeds
- if player or actor-value access is unavailable, migration remains `pending_cleanup` and retries later
- if the game exits before completion, the next load resumes from deterministic registered history plus the stored legacy reward state

### Legacy Undo and Reward State Disposal

Once migration reaches `complete`:

- legacy reward totals are no longer authoritative
- legacy reward undo deltas are discarded
- the old rewards screen no longer drives build progression

This is the cleanest migration path because:

- the player's collection progress remains valuable
- the player is not trapped in a mismatched legacy random build
- the first post-update session becomes an intentional rebuild moment

### Backward Compatibility Goal

The goal is not to preserve exact old stat totals.
The goal is to preserve progression value and avoid silent punishment.

## Failure Handling and Guardrails

- Batch registration must continue honoring quest-item and favorite protection rules.
- Protected items must remain clearly non-actionable in the register list.
- Slot changes must be blocked during combat.
- UI must clearly distinguish between "not unlocked" and "unlocked but inactive."
- Migration must notify the player when legacy rewards are converted.
- The system must not apply strong new effects implicitly during migration.
- Migration completion must not be marked before legacy cleanup succeeds.

## Existing Integration Rules

### Undo Registration

`Undo registration` remains in scope.
Its behavior changes to fit the new score-based model:

1. restore the consumed item
2. remove the corresponding discipline score contribution
3. recompute unlock availability for the affected discipline
4. automatically deactivate any active option that is no longer eligible

Undo remains the inventory correction tool, not a build-management substitute.

### Global Reward Refund

The current global `refund rewards` action does not fit the new build model and should be removed from the main user flow.

The new first-line recovery path is:

- deactivate an option
- replace it with another unlocked option

If a deeper respec action is added later, it should be introduced as a build-system action rather than carried forward as the legacy reward refund button.

## Scope for the First Implementation Plan

In scope:

- discipline score state
- active slot state
- legacy reward migration behavior
- register-screen UX overhaul
- build-screen UX overhaul
- batch registration flow
- undo behavior rewrite for score-based progression

Out of scope for the first pass:

- deep dependency trees between options
- per-option blacklist system
- full arbitrary respec economy
- alternate discipline mapping modes
- legacy global reward refund compatibility mode

## Recommended Implementation Boundary

Treat the rework as one feature with three tightly related slices:

1. Progression model rewrite
2. Save/migration rewrite
3. UI contract and screen rewrite

These are coupled enough that they should be implemented under one shared design, but the implementation plan should still break them into small testable tasks.

## Risks

- The migration experience can feel punitive if legacy stat totals disappear without clear compensation.
- Too many always-on passive gains can recreate the same balance problem under a different name.
- Too few slots can make the build system feel restrictive.
- Too much slot flexibility can erase discipline identity.
- Register-screen density must remain readable even after more status indicators are added.

## Risk Mitigations

- Make migration explicit and message the player clearly.
- Keep most high-impact bonuses in the slotted layer.
- Start with the asymmetric slot budget and tune later based on playtesting.
- Use discipline-grouped registration to reinforce the new model.
- Keep build cards concise and use a dedicated side panel for slot state.

## Recommendation

Proceed with a record-driven build system based on:

- three fixed disciplines
- unlock vs active separation
- asymmetric discipline slots plus one wildcard slot
- grouped single-list registration UX
- card-based build management UI

This direction preserves the mod's identity while addressing the most important user feedback with a structural fix rather than a stack of exception features.
