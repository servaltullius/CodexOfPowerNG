# Build Catalog Clarity Review

## Context

The build catalog has expanded to 32 slotted options across three disciplines and nine themes.
The catalog-first Build UI is in place and now emphasizes discoverability over the older minimal 3x3 presentation.

At this size, naming clarity and role separation matter more than before:

- theme names and option names now appear together in the same UI
- players scan option rows before opening detail panels
- duplicated effects create flat slot competition and make theme identity weaker

This review pass is intentionally limited to clarity and role separation review.
It does not rebalance numbers yet.

## Scope

In scope:

- option naming clarity
- option description clarity
- role overlap within the current 32-option catalog
- prioritization of which catalog rows need redesign first
- deciding which overlap fixes can stay inside the current runtime surface and which need a follow-up runtime/design pass

Out of scope:

- adding more legacy effects
- actor value/runtime expansion
- unlock score rebalance
- slot layout changes

## Findings

### 1. Theme/Option Name Collisions

Several options currently share the same visible name as their parent theme.
This is acceptable in a tiny catalog but becomes confusing in a catalog-first UI.

Problem pairs:

- attack theme `파괴` vs option `build.attack.destruction`
- attack theme `정밀` vs option `build.attack.precision`
- defense theme `수호` vs option `build.defense.guard`
- defense theme `보루` vs option `build.defense.bastion`

Decision:

- keep theme names as the abstract category labels
- rename options to more concrete, action/effect-oriented names

Approved rename set:

- option `정밀` -> `속공`
- option `수호` -> `철갑`
- option `보루` -> `철벽`
- option `파괴` -> `파괴숙련`

### 2. Raw Runtime Values Leak Into Player Text

Several descriptions currently expose engine-facing values such as:

- `0.5 감소`
- `0.1 증가`
- `0.02 증가`
- `0.3 증가`
- `0.4% 증가`

These numbers are precise but not readable in-game.
Players need effect intent first, not runtime storage precision.

Decision:

- keep the `슬롯 활성 시 ...` structure from the current catalog copy rules
- keep exact user-facing numbers when the unit is immediately understandable
  - examples: `%`, max resource, armor rating, carry weight
- avoid exposing raw fractional values when the engine unit is not self-explanatory
- when the internal value is not a meaningful player-facing unit, describe the outcome first and omit the raw number
- this review explicitly refines the earlier `무엇이 얼마나 변하는지` rule:
  - keep exact magnitudes where the unit is player-legible
  - allow result-first wording without a raw number where the internal runtime unit is not player-legible

Examples:

- `슬롯 활성 시 파괴 주문 비용이 감소합니다.`
- `슬롯 활성 시 매지카가 더 빠르게 회복됩니다.`
- `슬롯 활성 시 일부 피해를 되돌려줍니다.`

### 3. “Efficiency” Language Is Too Vague

Crafting and trickery rows currently overuse broad wording like `효율 증가`.
This hides the actual player outcome.

Affected groups:

- `대장`, `연금`, `부여`
- `은신`, `자물쇠`, `소매치기`

Decision:

- rewrite these descriptions in player-result language
- describe what gets easier, stronger, or more favorable
- do not fall back to raw engine fractional values just to preserve precision if the unit is not player-legible
- this is an explicit exception/update to the earlier copy rule, not an accidental drift

Examples:

- `슬롯 활성 시 대장 작업의 품질이 향상됩니다.`
- `슬롯 활성 시 자물쇠 해제가 쉬워집니다.`
- `슬롯 활성 시 소매치기 성공률이 높아집니다.`

### 4. Role Overlap Exists In The Current Catalog

Some rows are fine as intentional laddering, but two groups are strong redesign candidates.

#### Immediate Redesign Candidates

1. `build.defense.guard` (`철갑`) / `build.defense.bulwark` (`방벽`)

- both currently represent armor rating
- the signpost row is stronger than the standard row
- they do not create different defensive choices

Decision:

- keep `build.defense.guard` as the core armor/guard signpost
- redesign `build.defense.bulwark` into a distinct guard-side survivability effect rather than a weaker armor duplicate

2. `build.attack.pinpoint` (`정조준`) / `build.attack.vitals` (`급소`)

- both currently increase critical chance
- precision theme loses variation because two rows express the same decision with different strength

Decision:

- keep `build.attack.pinpoint` as the base critical row
- redesign `build.attack.vitals` into a different precision payoff rather than a stronger crit duplicate

#### Acceptable For Now, But Likely To Need Differentiation Later

- `비축 / 짐꾼`
- `기동 / 길잡이`

These can still function as signpost/standard pairs today, but they should be watched in future expansion passes because they compete on nearly identical player outcomes.

## Design Rules For The Next Revision

1. Theme names stay abstract. Option names become concrete.
2. Descriptions explain player-visible outcomes before implementation detail.
3. Avoid raw fractional values in player text unless the unit is self-evident.
4. Each theme should expose at least two clearly distinct choice axes.
5. Signpost rows should not be followed by weaker duplicates of the same effect family.

## Priority Order

### Pass 1: Naming

- apply the four approved rename changes

### Pass 2: Description Rewrite

- magic-school cost reduction rows
- crafting/trickery rows
- resource/regen rows
- special rows with vague wording (`반격`, `메아리`, `주문흡수`)

### Pass 3: Role Separation

- redesign `build.defense.guard` / `build.defense.bulwark`
- redesign `build.attack.pinpoint` / `build.attack.vitals`

### Pass 4: Follow-up Review

- reevaluate slot competition
- then revisit unlock curve and balance

## Success Criteria

This review pass is successful when:

- no option shares its displayed name with its parent theme
- descriptions keep the current `슬롯 활성 시 ...` shape while reading as player-facing outcomes rather than runtime internals
- the two strongest overlap pairs are either redesigned into distinct choices within the current runtime surface or split into an explicit follow-up redesign task when new runtime support is required
- future balance feedback can talk about option strength without first needing to decode what the option means
