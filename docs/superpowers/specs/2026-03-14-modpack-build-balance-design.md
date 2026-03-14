# Modpack Build Balance Design

## Summary

The current build model assumes that registration count and practical build power can grow on the same linear scale.
That works well enough for vanilla-sized item pools, but it breaks down in large modpacks such as Nolvus or LoreRim where the number of registerable items rises sharply, especially in `Utility`.

The goal of this design is to keep the new build system readable and rewarding while preventing modpack item volume from turning raw registration count directly into runaway build power.
The key change is to separate `rawScore` from `effectiveScore`.
Unlocks will continue to use raw registration totals, while active option strength will scale from a compressed effective score curve.

This preserves player-facing progression and unlock cadence, but it stops high-count load orders from pushing active option values far past the balance envelope that the current catalog assumes.

## Goals

- Preserve the existing registration loop and automatic unlock model.
- Keep early and midgame vanilla pacing close to the current live feel.
- Prevent large modpacks from converting item-count inflation into near-linear late-game stat inflation.
- Reduce the current `Utility` discipline bias without making registration rules opaque.
- Retune the most exposed option magnitudes so their capped practical range matches the compressed score curve.

## Non-Goals

- Do not redesign the three-discipline model.
- Do not add new slot types or change slot counts.
- Do not introduce spendable score currencies or respec points.
- Do not add per-item manual discipline assignment.
- Do not solve every future balance issue with one pass; this pass targets score inflation and the most obvious overtuned rows.

## Problem Statement

### 1. Raw item count is not stable across load orders

The current build system grants one score per successful registration and uses a shared `10 score = 1 tier` rule for option growth.
This assumes the available item pool is roughly within a vanilla-like range.
That assumption is false in large curated modpacks.

When the registerable pool doubles or triples, the system does not merely speed up unlocks.
It also increases the final values of active options because the same raw score feeds scaling.

### 2. Utility grows faster than the other disciplines

The current discipline mapping sends all of the following into `Utility`:

- alchemy items
- ingredients
- books
- scrolls
- soul gems
- misc items

That mapping is simple and readable, but in modpacks it produces a strong structural skew.
Utility receives the widest and noisiest item pool, so it reaches high tiers earlier and more often than Attack or Defense.

### 3. Linear scaling turns catalog values into modpack liabilities

Several options are reasonable on vanilla-adjacent score ranges but become too efficient when the score curve stretches.
This is most visible in:

- carry weight
- movement speed
- crafting modifiers
- stealth / utility modifiers
- broad defensive stats such as health and stamina

The root problem is not one bad row.
The root problem is that the catalog was tuned against a much smaller practical score environment.

## Chosen Approach

We will separate progression into two score concepts:

- `rawScore`: exact registration-derived score for a discipline
- `effectiveScore`: compressed score used only for active option scaling

Rules:

- `rawScore` determines unlock eligibility.
- `effectiveScore` determines current tier and scaled magnitude.
- `rawScore` is still shown in the UI as the real record count.
- the UI may optionally show the current effective tier, but not a second hidden currency

One-line rule:

`Registration count stays honest, but combat/crafting/traversal power is derived from a modpack-safe compressed curve.`

## Score Compression Model

### Attack / Defense

Attack and Defense keep most of their early linear feel, then taper off.

Proposed curve:

- `0-30 raw`: 100% effective
- `31-80 raw`: 50% effective on the overflow
- `81+ raw`: 25% effective on the overflow

Formula:

```text
if raw <= 30:
  effective = raw
else if raw <= 80:
  effective = 30 + floor((raw - 30) * 0.5)
else:
  effective = 55 + floor((raw - 80) * 0.25)
```

Examples:

- raw `35` -> effective `32`
- raw `60` -> effective `45`
- raw `120` -> effective `65`

### Utility

Utility needs stronger tapering because its input pool is structurally larger.

Proposed curve:

- `0-20 raw`: 100% effective
- `21-60 raw`: 40% effective on the overflow
- `61+ raw`: 20% effective on the overflow

Formula:

```text
if raw <= 20:
  effective = raw
else if raw <= 60:
  effective = 20 + floor((raw - 20) * 0.4)
else:
  effective = 36 + floor((raw - 60) * 0.2)
```

Examples:

- raw `35` -> effective `26`
- raw `60` -> effective `36`
- raw `120` -> effective `48`

## Why This Approach

### 1. Unlock pacing stays readable

The user still sees the honest registration count and still unlocks options from that same count.
We avoid the frustrating feeling that “the mod stopped counting my progress.”

### 2. Early vanilla feel remains close to current live behavior

Most normal score ranges remain near-linear.
The compression mainly affects the high-count ranges that modpacks hit far more often than vanilla.

### 3. It solves the real problem instead of sanding down every row forever

If we only nerf row magnitudes, a sufficiently inflated item pool will still eventually break the tuning again.
Compressing effective score addresses the structural source of the runaway growth.

### 4. Utility can be corrected without making registration rules obscure

This pass does not immediately remove broad item categories from Utility scoring.
Instead, it makes the downstream power curve safer while preserving the simple mental model of current registration mapping.

## Option Retune Pass

Score compression alone is not enough.
Several rows should also be brought down so their new peak values stay within a more conservative envelope.

The first pass should use these revised magnitudes:

### Attack

- `build.attack.ferocity`: `5 / 1` -> `4 / 0.75`
- `build.attack.precision`: `3 / 0.5` -> `2.5 / 0.35`

### Defense

- `build.defense.guard`: `10 / 2` -> `8 / 1.5`
- `build.defense.bulwark`: `8 / 4` -> `6 / 2.5`

### Utility

- `build.utility.cache`: `25 / 2` -> `20 / 1`
- `build.utility.barter`: `10 / 1` -> `8 / 1`
- `build.utility.smithing`: `0.05 / 0.01` -> `0.03 / 0.005`
- `build.utility.alchemy`: `0.05 / 0.01` -> `0.03 / 0.005`
- `build.utility.enchanting`: `0.05 / 0.01` -> `0.03 / 0.005`
- `build.utility.mobility`: `3 / 0.25` -> `2 / 0.15`
- `build.utility.sneak`: `0.05 / 0.01` -> `0.03 / 0.005`
- `build.utility.lockpicking`: `0.05 / 0.01` -> `0.03 / 0.005`
- `build.utility.pickpocket`: `0.05 / 0.01` -> `0.03 / 0.005`
- `build.utility.conjuration`: `0.05 / 0.01` -> `0.03 / 0.005`
- `build.utility.illusion`: `0.05 / 0.01` -> `0.03 / 0.005`
- `build.utility.echo`: `-0.01 / -0.003` -> `-0.007 / -0.002`

These values are intentionally conservative.
The goal of this pass is to restore safe headroom for large load orders, not to maximize top-end power.

## Runtime Rules

### Unlocks

- continue to use `rawScore`
- no option should relock differently than it does today
- rollback should still subtract raw registration score

### Scaling

- current option magnitude should use `effectiveScore`
- tier derivation should use compressed effective score, not raw score
- all active option recomputation paths should call the same effective score helper

### Serialization

- store raw discipline scores only
- do not serialize effective score as a separate source of truth
- recompute effective score on load

This keeps saves deterministic and avoids migration complexity.

## UI Rules

The UI should continue to surface the real registration total as the primary number because that is what the player earned.

Recommended display model:

- top summary: show `rawScore`
- option details: show current effect and next tier preview from effective scaling
- next-tier messaging: based on the next compressed effective tier threshold, not the next raw multiple of ten

This means the detail panel should not imply that every raw ten points always yields a tier forever.
If the compression curve changes the next tier threshold, the panel should show the actual remaining raw score needed.

## Rejected Alternatives

### 1. Nerf every row only

Rejected because it does not solve the inflated score source.
It would force repeated nerf passes whenever modpack item volume rises again.

### 2. Increase the global tier interval from 10 to 15 or 20

Rejected because it hurts vanilla pacing too much and still does not address Utility inflation specifically.

### 3. Immediately exclude categories like `Misc` from Utility

Rejected for this pass because it changes registration semantics too abruptly and makes player expectations less clear.
That remains a valid future follow-up if Utility still outpaces the others after effective-score compression.

## Risks

### 1. Unlocks may feel more exciting than actual strength gains

If compression is too aggressive, players may unlock new options frequently but feel that equipped options grow too slowly.
That is why the early curve remains near-linear.

### 2. Utility may still lead in very large load orders

This design reduces the slope but does not fully erase the mapping bias.
That is acceptable for this pass as long as Utility no longer dominates by default.

### 3. Some rows may become too weak under both compression and retuning

That is possible, especially for niche utility rows.
The intended mitigation is targeted post-pass adjustment, not abandoning the score compression model.

## Validation Targets

The implementation should prove these outcomes:

- vanilla-like score ranges remain close to current live values
- high raw scores no longer translate to near-linear high-end option growth
- Utility high-score outcomes stay meaningfully below current live values
- unlock behavior is unchanged
- save/load round-trips continue to depend on raw stored score only

## Follow-Up Criteria

If this pass lands and Utility still dominates modpack play, the next pass should consider one of:

- category-weighted raw score contributions
- excluding part of `Misc` from score contribution
- a second Utility-only taper segment

That work should only start after the effective-score model is validated in runtime and UI.
