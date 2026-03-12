# Codex of Power NG Register/Build UI Redesign Design

## Summary

The record-build rework has made the mod structurally stronger, but the new UI still reads too much like a generic table app.
The quick-register list now functions correctly, yet its current presentation does not communicate discipline identity strongly enough and does not feel sufficiently "in-world" for the mod's fantasy.

This redesign will keep the current information architecture intact while changing the visual hierarchy and layout language:

- Quick Register will become a denser `codex list` surface
- Build will become a `shrine / altar` surface
- Attack / Defense / Utility will gain distinct visual identity
- The overall shell will feel less like a spreadsheet and more like a deliberate game overlay

The goal is not to make the UI decorative at the expense of throughput.
The goal is to make the UI look and read like the progression system it now is.

## Goals

- Make Attack / Defense / Utility immediately distinguishable at a glance.
- Reduce the flat "all pills are the same" feeling in the register list.
- Make Quick Register feel like a record book rather than a raw admin table.
- Make Build feel like a build-assembly ritual surface rather than a neutral management panel.
- Improve per-row readability and on-screen density for large inventories.
- Preserve the current grouped-register flow, build slot model, and native payload contracts.

## Non-Goals

- Rework the underlying build progression rules again.
- Replace the register list with a card grid.
- Add new gameplay mechanics or new build effects as part of the visual redesign.
- Introduce heavy animation or ornate decoration that harms scanning speed.
- Redesign every tab equally; this effort focuses on Quick Register first and Build second.

## Current UX Problems

### 1. Discipline Identity Is Too Weak

Attack, Defense, and Utility are conceptually central to the rework, but the current rows and badges do not make them feel meaningfully different.
The list technically contains discipline grouping, yet visually it still reads as one long neutral table.

This weakens two important parts of the mod fantasy:

- the idea that the player is building a character through different record disciplines
- the idea that registration materials are distinct inputs into different build directions

### 2. The Quick Register Surface Feels Too Generic

The current layout is practical but too flat:

- toolbar controls compete with one another
- rows are tall but still visually bland
- the primary action button dominates every row equally
- state tags, IDs, and quantities do not form a clear reading hierarchy

The result is that the screen feels closer to an inventory spreadsheet than a codex.

### 3. The Build Screen Needs More Ceremony

The Build tab should feel like the place where the player assembles a current loadout from accumulated records.
Right now the functional building blocks exist, but the screen should carry more presence and clearer slot-centered hierarchy.

### 4. The Mod Needs More Visual Personality

Now that the reward system has shifted from random grants to deliberate build assembly, the UI needs to signal that stronger identity.
Without that, the player may understand the mechanics but not feel the intended progression fantasy.

## Approaches Considered

### 1. Minimal Correction

Keep the table almost unchanged and only recolor badges/buttons.

Pros:

- lowest implementation risk
- smallest CSS/markup delta

Cons:

- would not solve the "still feels like a table app" problem
- would undershoot the new build-system identity

### 2. Full Card/Gamepad Surface

Replace the quick list with card-like rows and make both tabs much more ornamental.

Pros:

- strongest immediate game-feel
- most visually dramatic

Cons:

- loses density for large inventories
- harms throughput in the screen that most needs fast scanning

### 3. Hybrid Recommended Approach

Use a `codex list` for Quick Register and a `shrine / altar` layout for Build.

Pros:

- fits each screen's actual job
- preserves throughput where needed
- adds enough identity to feel purpose-built
- lets the build tab carry more ceremony without burdening the register tab

Cons:

- requires more deliberate shell/layout work than a minimal recolor

This is the chosen direction.

## Chosen Direction

### Quick Register: Codex List

Quick Register remains a one-column grouped list because it is a working surface.
However, it should read as a record book:

- stronger discipline section headers
- more compact rows
- clearer item/status hierarchy
- subtler default action buttons
- discipline-colored accents and badges

In short:

- practical like a list
- atmospheric like a codex

### Build: Shrine / Altar

Build becomes the more ceremonial screen.
Its visual center should be the currently active six-slot build, with left/right supporting panels for browsing and detail.

In short:

- Quick Register is the workbench
- Build is the altar

## Visual System

### Base Surface

The existing dark overlay foundation can remain, but the redesign should introduce stronger layer separation:

- shell background: dark glass / lacquered panel
- card sections: slightly warmer, subtly textured surfaces
- thin engraved lines, bevels, or inset borders for hierarchy
- lower dependence on flat monochrome pills

The UI should feel crafted, not chrome-default.

### Discipline Palette

The three disciplines need stable visual tokens that repeat in:

- section headers
- badges
- slot borders
- selected row glow
- active card outlines
- mini dividers / accent bars

Recommended palette:

- Attack: crimson / copper / ember
- Defense: steel / blue-gray / cold silver
- Utility: brass / olive / antiqued gold

These colors should be muted enough for long reading sessions, but distinct enough that the player can identify discipline before reading text.

### Typography and Weight

The current typography should gain stronger hierarchy:

- item name: strongest text in each row
- state tags: small but legible
- form ID: quiet metadata
- count: aligned and easy to compare
- section label: strong, not oversized

The redesign should rely more on weight, spacing, and grouping than on saturated colors alone.

## Quick Register Screen Design

### Screen Role

Quick Register is a throughput-first screen.
It must support:

- scanning many items
- spotting protected rows instantly
- selecting multiple rows confidently
- understanding discipline gain before confirming

This means the redesign should add flavor without sacrificing list efficiency.

### Top Bar

The top bar should be simplified into clearer zones:

- left zone: search + actionable-only filter
- right zone: inventory meta, page size, paging, refresh

The inventory meta pill should stay, but visually quieter than the actual list content.
Refresh and page controls should read as utility controls rather than primary calls to action.

### Section Headers

Each discipline section header should become a strong separator with:

- a discipline accent line or bar
- discipline iconography or emblem-like shape
- section label
- optional section count if helpful

The section header should feel like opening a codex chapter rather than reading a plain table divider.

### Row Structure

Recommended row structure:

- selection checkbox
- discipline marker / badge
- item information block
- quantity block
- action block

Rendered reading order:

`[select] [discipline] [item name + status + FormID] [safe/total count] [register]`

### Row Information Hierarchy

Inside the item information block:

- line 1: item name
- line 2: form ID
- line 3: state tags and protection reason

Important behavior:

- protection reasons remain visible in disabled rows
- disabled rows should still look intentionally part of the system, not washed-out leftovers
- action state should be obvious before the player reaches the button

### Buttons and Selection

The current row action button is too visually dominant when every row uses the same treatment.
The redesign should change this:

- unselected, actionable rows: restrained secondary action styling
- hovered or selected rows: stronger emphasis
- disabled rows: clearly unavailable without becoming illegible
- batch selection state: visible through row tint and checkbox color

The primary "Register Selected" action remains the strongest action on the screen, but per-row "Register" should stop overpowering everything else.

### Density

The list should become denser than it is now.
The goal is not tiny rows, but less wasted height.

This requires:

- tighter vertical spacing
- smaller metadata block
- more disciplined button sizing
- cleaner state-tag wrapping

The player should see meaningfully more rows per screen without the screen feeling cramped.

### Batch Summary

The batch summary area should feel like a footer tool tray:

- selected count
- Attack / Defense / Utility gain summary
- Clear
- Register Selected

It should remain obvious, but the styling should feel integrated with the codex surface rather than bolted on below the table.

## Build Screen Design

### Screen Role

Build is the expression screen.
It is where the player sees current identity, not just raw inventory.

The build tab should therefore lead with the currently active build rather than with a neutral grid of cards.

### Summary Bar

The top of the Build screen should summarize:

- Attack score
- Defense score
- Utility score
- slot usage summary if useful

This bar should read as a build ledger, not as a decorative header.

### Shrine / Altar Center

The six active slots form the visual center of the screen.
This area should feel like a build altar:

- slots are spatially grouped
- slot shapes or frames vary slightly by slot kind
- discipline borders/tints are visible even before text is read
- occupied vs empty vs locked is obvious

The player should be able to understand the current build from the center cluster alone.

### Left Panel: Discipline Options

The left side should contain:

- Attack / Defense / Utility tabs or toggles
- option list/cards for the selected discipline

Each option card should clearly show:

- title
- locked / unlocked / active state
- threshold requirement
- whether it can be slotted in the current available destinations

### Right Panel: Option Detail

The right side should contain detail for the focused option:

- effect description
- unlock requirement
- slot compatibility
- currently active slot if any
- action buttons such as activate, swap, or deactivate

This keeps comparison on the left and decision-making on the right.

### State Language

Build cards and slots should use consistent state language:

- locked
- unlocked
- active
- available to slot
- incompatible / unavailable

The visual styling of these states should reuse the discipline palette without collapsing all status into color alone.

## Shared Interaction Rules

- Disabled rows remain visible with clear reason text.
- Hover and selection states should be more readable than today.
- The interface should keep keyboard and large-inventory friendliness.
- Visual emphasis should follow task importance, not merely button presence.
- Register remains work-first; Build remains expression-first.

## Implementation Boundaries

This redesign should stay mostly inside the current Prisma UI layer.

Expected primary files:

- `PrismaUI/views/codexofpowerng/index.html`
- `PrismaUI/views/codexofpowerng/ui_rendering.js`
- `PrismaUI/views/codexofpowerng/ui_register_batch_panel.js`
- `PrismaUI/views/codexofpowerng/ui_build_panel.js`
- `PrismaUI/views/codexofpowerng/ui_i18n.js`

Native payload contracts should remain unchanged unless the redesign reveals a concrete missing field.
The work should focus on layout, markup, CSS tokens, and small render-composition adjustments.

## Testing Strategy

The redesign should be protected with JS tests that cover:

- register shell structure and required DOM references
- discipline-specific row/section classes
- grouped register row markup and disabled-state rendering
- build panel structure and state labels
- i18n keys needed by the new presentation

Manual verification should confirm:

- Quick Register is denser and easier to scan
- Attack / Defense / Utility are visually distinct
- protected rows remain readable
- Build feels slot-centered and more ceremonial
- the overlay still works at high DPI and large inventories

## Success Criteria

The redesign is successful when:

- a player can tell discipline identity before reading the row text
- the register list feels like a codex, not a spreadsheet
- build slot layout feels like a current loadout, not a generic card dump
- batch registration remains fast
- the UI feels more like part of the mod's fantasy than a debug overlay
