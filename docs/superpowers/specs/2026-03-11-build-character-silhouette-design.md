# Codex of Power NG Build Character Silhouette Design

## Summary

The Build screen successfully shifted the mod away from a legacy reward panel and toward a shrine-like build assembly surface.
However, one important visual symbol was lost during that transition: the character image that previously gave the progression screen a sense of focus and identity.

This change restores that visual anchor without bringing back the old reward-orbit presentation.
The character image will return as a low-contrast altar silhouette behind the active slot cluster on the Build screen.

The intent is to recover ritual atmosphere and screen identity while preserving the newer readability improvements.

## Goals

- Restore a clear visual centerpiece to the Build screen.
- Reinforce the existing shrine / altar metaphor.
- Keep slots, score cards, and focus-panel text as the primary readable content.
- Reuse the existing `assets/character.png` pipeline instead of inventing a new asset path.
- Avoid regressing the recent readability and contrast improvements.

## Non-Goals

- Bring back the legacy reward orbit as a live Build-screen element.
- Turn the character image into an interactive UI control.
- Re-center the entire layout around a decorative illustration.
- Add new gameplay information to the character visual itself.
- Rework Quick Register as part of this change.

## Current Problem

The Build screen layout is stronger than before, but it currently lacks a symbolic center.
The score cards, slot cluster, and focus panel are functional, yet the middle of the screen reads more like a neutral management grid than a ritual assembly space.

The older character image is still part of the UI language and asset set, but the current Build surface no longer renders it.
As a result:

- the screen has less personality than intended
- the shrine metaphor is weaker than the design direction suggests
- the transition from the old reward surface to the new build surface feels more abrupt than it needs to

## Approaches Considered

### 1. Omit the Character Image Entirely

Pros:

- lowest visual risk
- no chance of hurting readability

Cons:

- leaves the Build screen visually under-anchored
- wastes an existing symbolic asset
- weakens the altar fantasy

### 2. Restore the Character as a Central Low-Contrast Silhouette

Pros:

- best match for the shrine / altar direction
- recovers screen identity without competing with text
- lets the slot cluster feel intentionally staged rather than floating

Cons:

- requires careful opacity, masking, and layer ordering
- can regress readability if contrast is handled poorly

### 3. Reintroduce the Character as a Side Illustration

Pros:

- easiest way to avoid covering slot content
- straightforward layout containment

Cons:

- feels secondary rather than symbolic
- does not support the "altar center" metaphor strongly enough

This change uses approach 2.

## Chosen Direction

The character image returns only on the Build screen and only as a visual altar center.

- place it behind the active slot cluster
- keep it non-interactive
- keep it lower contrast than the slot cards and text
- do not restore orbit nodes around it

This means the screen center once again has a recognizable in-world visual anchor, but the current build system remains the actual subject of the screen.

## Visual Treatment

### Placement

The image should live inside the central build-slot region rather than in a separate panel.
It should sit behind the slot cards so the current build still reads as the foreground layer.

### Contrast

The image should be intentionally subdued:

- low-to-medium opacity
- darker center value than the surrounding shell
- softer edges than the foreground slot cards
- readable silhouette, not full-detail illustration

### Masking

The image should be partially controlled with overlay treatment so it does not fight the slot labels:

- central darkening or vignette
- subtle gradient fade toward edges
- optional blur or light haze if needed

### Layering

Foreground order should remain:

1. build slot cards
2. slot labels and actions
3. focus/detail content
4. character silhouette
5. background panel

The silhouette supports the scene but never outranks the interactive layer.

## Orbit Decision

The old reward orbit should not return as part of this change.
The new Build screen already has a stronger primary structure through the active slot system, and restoring orbit nodes would likely overcomplicate the visual center.

The image comes back.
The orbit does not.

## Asset and Fallback Behavior

The existing `assets/character.png` should remain the source asset.
If the image is missing or fails to load:

- the Build screen should still render cleanly
- the slot cluster should remain centered and readable
- the fallback message can remain available for diagnostics, but the screen should not visually collapse around it

## Testing Focus

The key checks for implementation are:

- Build screen markup includes a character silhouette layer again.
- Existing slot and focus-panel structure remains intact.
- Missing-image fallback still behaves safely.
- Readability-focused CSS tokens continue to win over decorative styling.

## Result

This is a small, targeted visual reintroduction.
It does not reopen the old reward-screen design.
It simply gives the new Build shrine a missing center of gravity.
