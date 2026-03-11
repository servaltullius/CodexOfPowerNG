# Codex of Power NG Performance-First UI Redesign Design

## Summary

Recent in-game validation shows that the current Prisma UI layout is no longer bottlenecked primarily by wheel-step tuning.

The larger issue is that the current presentation style asks an embedded game overlay renderer to draw a web-style layered interface with:

- multiple translucent panels
- blur
- shadow glow
- layered gradients
- long scroll surfaces

The new direction is to keep Prisma UI, but redesign the presentation around overlay performance instead of browser-style flourish.

## Goals

- Keep Prisma UI as the active frontend technology.
- Preserve the current gameplay structure:
  - Quick Register
  - Registered
  - Undo
  - Build
  - Settings
- Make `Quick Register` feel like a focused work list.
- Make `Build` feel like a mostly fixed three-panel build screen instead of a long scrolling composition.
- Preserve some shrine / codex mood without relying on expensive blur and glow.

## Non-Goals

- Replace Prisma UI with Scaleform, ImGui, or a native rewrite.
- Rework the gameplay loop again.
- Remove the current Attack / Defense / Utility build model.
- Add new user-facing performance settings in this pass.

## Chosen Direction

### 1. Keep Prisma UI, Change the Rendering Philosophy

The current problem is not best framed as "Prisma UI is unusable."

It is better framed as:

`Prisma UI overlay + current visual treatment is too expensive for the in-game rendering context.`

So the next redesign keeps Prisma UI and changes the presentation model from:

- soft translucent web overlay
- heavy compositing
- page-like scrolling surfaces

to:

- more opaque game-style panels
- stronger visual hierarchy from color / borders / spacing
- fewer layered effects

### 2. Quick Register Becomes a Pure Work Surface

Quick Register should optimize for:

- fast scanning
- large list movement
- protection / state readability
- batch action confidence

That means:

- keep the current grouped codex-list direction
- preserve the one-column work-list structure
- reduce decorative effects further
- keep larger immediate wheel movement than the Build screen

Quick Register is allowed to feel utilitarian.

### 3. Build Becomes a Mostly Fixed Management Surface

Build should no longer behave like a long page that happens to contain a build.

Instead:

- center altar panel stays mostly fixed
- right detail panel stays mostly fixed
- left option column becomes the primary scrollable reading area

The result should feel like a stable management screen:

- left: option browsing
- center: active slots / altar
- right: focused detail

This keeps the shrine identity while reducing how much of the screen must be repainted during wheel use.

### 4. Preserve Mood Through Cheap Elements

The Build screen should stay atmospheric, but the mood should come from cheaper tools:

- discipline colors
- panel framing
- typography hierarchy
- sparse ornament lines
- the lower altar silhouette

The Build screen should no longer depend on:

- large backdrop blur
- broad glow
- multiple overlapping translucent backgrounds
- oversized drop shadows

### 5. Balance Rule

This is a `balanced` redesign, not a total flattening.

So:

- mood stays
- structure stays
- expensive effects go

The target is not "plain."
The target is "game-overlay friendly."

## Screen Rules

### Quick Register

- remains scroll-heavy
- keeps grouped sections
- keeps larger immediate wheel step
- removes unnecessary softness and decorative layering where possible

### Build

- becomes mostly fixed-layout
- keeps smaller immediate wheel step
- shifts the visual language toward framed opaque panels
- keeps the altar silhouette and discipline identity

## Testing Focus

- rendering tests should lock the lighter, more opaque visual contract
- interaction tests should confirm screen-specific wheel behavior remains distinct
- existing grouped register and build rendering contracts must stay green
