# Build Catalog Competition And Tempo Review

## Context

The build catalog now ships with 32 slotted options across:

- 3 disciplines
- 9 themes
- 6 active slots (`Attack1`, `Attack2`, `Defense1`, `Utility1`, `Utility2`, `Wildcard1`)

The previous clarity pass improved naming and copy, and the role-separation pass removed the two strongest duplicate pairs.
The next review pass is about how the catalog behaves when players actually choose slots and unlock rows over time.

This pass focuses on:

- slot competition
- unlock timing
- baseline milestone pressure

It does not add new runtime surfaces or new legacy import batches.

## Scope

In scope:

- identifying themes that crowd each other inside the current slot layout
- identifying themes whose unlock order creates early autopicks
- identifying themes whose signpost/standard ordering is backwards
- identifying where baseline milestones flatten theme choice

Out of scope:

- adding new effects
- expanding the slot layout
- changing runtime implementation
- changing the build UI structure

## Findings

### 1. Attack Frontloads Generic Power Too Hard

Attack currently combines:

- `build.attack.ferocity` at 5 score (`+5% attack damage`)
- attack baseline at 10 score (`+2% attack damage`)
- attack baseline at 25 score (`+4% attack damage`)
- `build.attack.vitals` at 30 score (`+3% attack damage`)

That creates a very early and very stable generic damage package before `devastation` and `fury` get their more specialized rows.

Result:

- physical builds are pushed toward `damage + speed` first
- `fury` reads as secondary sustain rather than a competing attack identity
- the baseline damage milestones reduce the marginal identity of attack signpost rows

Decision:

- attack baseline damage needs review before more offensive rows are added
- `ferocity` and attack baseline should not together define the entire early attack curve

### 2. Utility/Livelihood Crowds Out Other Utility Themes

`livelihood` currently contains:

- `cache` at 5
- `barter` at 15
- `smithing` at 20
- `hauler` at 20
- `alchemy` at 25
- `meditation` at 30
- `magicka` at 30
- `enchanting` at 35

Utility baseline also grants:

- economy at 10
- carry weight at 25

Result:

- early and mid utility progression is strongly pulled into `livelihood`
- `exploration` and `trickery` have to compete against universal convenience rather than equally strong theme identities
- baseline milestones duplicate the same two livelihood outcomes that the catalog already prioritizes

Decision:

- utility baseline and livelihood early unlocks need review together
- livelihood currently behaves like a default utility package rather than one utility theme among several

### 3. Exploration Signpost Ordering Is Backwards

`exploration` currently unlocks:

- `wayfinder` at 20
- `mobility` at 30 (signpost)
- `echo` at 35

The theme signpost appears after a standard row that already expresses the same family (`speed_mult`).

Result:

- the theme is introduced late
- the standard row teaches the theme before the signpost does
- the player sees “more movement speed later” rather than a clear exploration identity first

Decision:

- signpost rows should appear first or at least no later than the first standard row in the same theme
- `exploration` needs unlock-order correction even if its effect set stays the same

### 4. Guard Is Too Dense In The 20-30 Score Band

`guard` currently unlocks:

- `guard` at 5
- `bulwark` at 20
- `recovery` at 25
- `endurance` at 30
- `restoration` at 35

Defense baseline also grants:

- armor at 10
- armor at 25

Result:

- guard gets the broadest progression support from both slotted rows and baseline
- `bastion` appears later and with narrower identity
- `resistance` opens earlier but remains low-pressure because its rows are too narrow for scarce defense slots

Decision:

- defense progression is biased toward guard
- later defense passes should rebalance guard density relative to bastion and resistance

### 5. Resistance Unlocks Early But Still Loses Slot Competition

`resistance` opens at:

- 10 (`warding`)
- 15 (`fireward`, `frostward`)
- 20 (`stormward`, `antidote`)
- 25 (`purity`)

Despite this, resistance still loses slot competition because its rows are narrow and defensive slots are scarce.

Result:

- early unlock timing alone does not create uptake
- resistance feels like “situational rows that ask for a rare slot”
- this is not only a timing issue; it is also a breadth/packaging issue

Decision:

- do not solve resistance only by moving scores
- treat resistance as a theme-structure problem first and a pacing problem second

### 6. Post-pass Resistance Reassessment

After the utility, attack, guard, and exploration timing passes:

- resistance rows still compete for the same `Defense1` / `Wildcard1` economy
- the theme still asks the player to spend a scarce defense slot on narrow elemental or status coverage
- early access alone does not offset the opportunity cost of skipping broad guard or bastion rows

Conclusion:

- score-only tuning is closed for resistance in this review pass
- the next resistance pass must broaden row breadth instead of shifting unlock scores again
- valid follow-up directions include bundled elemental coverage, broader magical mitigation packages, or other multi-threat defensive rows

## Design Rules For The Next Revision

1. Signpost rows should introduce a theme, not trail behind its standard rows.
2. Baseline milestones should reinforce identity, not duplicate the most obvious slotted picks.
3. A single theme should not dominate a discipline's early and midgame by stacking baseline plus universal convenience.
4. If a theme loses slot competition because each row is too narrow, score changes alone are not enough.
5. Early unlocks should create distinct directions, not just stack more of the same broad stat family.

## Priority Order

### Pass 1: Utility Baseline And Livelihood Review

- reevaluate utility baseline milestones
- reevaluate `cache`, `barter`, and `hauler` timing relative to `exploration` and `trickery`

### Pass 2: Attack Early Curve Review

- reevaluate attack baseline milestones
- reevaluate `ferocity` and `vitals` placement relative to `fury`

### Pass 3: Exploration Ordering Review

- move the theme signpost ahead of or alongside the first movement row

### Pass 4: Defense Theme Pressure Review

- reduce guard dominance
- review whether resistance needs broader rows rather than earlier rows

## Success Criteria

This review pass is successful when:

- the next balancing batch has a clear ordering target
- baseline and slotted competition are treated as one system rather than independent knobs
- utility no longer defaults to livelihood by timing alone
- attack no longer frontloads generic damage so early that other attack themes read as secondary
- exploration signpost ordering is corrected in the next implementation pass
