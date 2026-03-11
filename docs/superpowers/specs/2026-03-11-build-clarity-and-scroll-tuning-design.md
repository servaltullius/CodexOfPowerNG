# Codex of Power NG Scroll Profiles and Build Performance Tuning Design

## Summary

In-game validation showed that the current scroll tuning problem is not one issue shared equally by every screen.

- `Quick Register` wants a larger, more confident wheel step so long inventory sweeps feel fast.
- `Build` wants a calmer reading step, but the heavier problem there is render cost from layered effects.

The chosen direction is to stop treating wheel handling as one global feel and instead split the policy by active screen while also trimming the heaviest visual effects on the Build screen.

## Goals

- Let `Quick Register` scroll in larger immediate steps without feeling like hard notched jumps.
- Let `Build` scroll in smaller, more readable steps.
- Improve `Build` smoothness by reducing expensive blur / glow / layered background work while keeping the shrine mood intact.
- Preserve the current information architecture, grouped register flow, and lower-altar character silhouette placement.

## Non-Goals

- Rework the register or build layouts again.
- Introduce nested scroll regions inside the Build screen.
- Add long inertial scroll animation or requestAnimationFrame easing.
- Add a new user-facing setting just for this pass.

## Chosen Direction

### 1. Separate Scroll Profiles by Active Screen

Wheel input should stay immediate, not animated.

The next pass will use different normalization profiles depending on the active section:

- `Quick Register`
  - larger immediate step
  - tuned for scanning long grouped lists
  - still clamps very small deltas so one notch is useful
- `Build`
  - smaller immediate step
  - tuned for reading cards and slot details
  - no extra easing layer

This keeps the interaction honest. The UI responds right away, but each screen gets a wheel feel that matches its job instead of forcing one compromise across both.

### 2. Conservative Build-Screen Performance Reduction

The Build screen should keep its shrine / altar identity, but some of its most expensive visual treatments are no longer worth the scroll cost.

This pass will reduce the weight of:

- `backdrop-filter` blur on build summary and major build panels
- large slot-card and silhouette shadow intensity
- overly layered altar gradients that do not materially improve readability

This is intentionally conservative. The goal is not to flatten the screen into a generic menu. The goal is to keep the current composition while removing the most obvious sources of in-game drag.

### 3. Keep the Silhouette and Structure

The lower altar silhouette stays in place.

- no move back behind slots
- no orbit restoration
- no layout restructuring

That visual direction is already approved. This tuning pass only protects its readability and performance context.

## Testing Focus

- input correction tests should prove that wheel handling stays immediate and that `Quick Register` and `Build` now use different scroll profiles
- rendering tests should lock the lighter Build-screen treatment without changing the current layout contract
- existing UI regression tests must remain green
