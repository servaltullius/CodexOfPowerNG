# Build Catalog Clarity Review

## Context

The build catalog has expanded to 32 slotted options across three disciplines and nine themes.
The catalog-first Build UI is in place and now emphasizes discoverability over the older minimal 3x3 presentation.

At this size, naming clarity and role separation matter more than before:

- theme names and option names now appear together in the same UI
- players scan option rows before opening detail panels
- duplicated effects create flat slot competition and make theme identity weaker

This review pass is intentionally limited to clarity and role separation.
It does not rebalance numbers yet.

## Scope

In scope:

- option naming clarity
- option description clarity
- role overlap within the current 32-option catalog
- prioritization of which catalog rows need redesign first

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

- attack theme `파괴` vs option `파괴`
- attack theme `정밀` vs option `정밀`
- defense theme `수호` vs option `수호`
- defense theme `보루` vs option `보루`

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

- prioritize player-facing meaning over raw internal magnitude
- keep `%`, max resource, regeneration, and cooldown language where intuitive
- avoid exposing raw fractional values when they are not self-explanatory

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

Examples:

- `슬롯 활성 시 대장 작업의 품질이 향상됩니다.`
- `슬롯 활성 시 자물쇠 해제가 쉬워집니다.`
- `슬롯 활성 시 소매치기 성공률이 높아집니다.`

### 4. Role Overlap Exists In The Current Catalog

Some rows are fine as intentional laddering, but two groups are strong redesign candidates.

#### Immediate Redesign Candidates

1. `수호 / 방벽`

- both currently represent armor rating
- the signpost row is stronger than the standard row
- they do not create different defensive choices

Decision:

- keep `수호` as the core armor/guard signpost
- redesign `방벽` into a distinct guard-side survivability effect rather than a weaker armor duplicate

2. `정조준 / 급소`

- both currently increase critical chance
- precision theme loses variation because two rows express the same decision with different strength

Decision:

- keep `정조준` as the base critical row
- redesign `급소` into a different precision payoff rather than a stronger crit duplicate

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

- redesign `수호 / 방벽`
- redesign `정조준 / 급소`

### Pass 4: Follow-up Review

- reevaluate slot competition
- then revisit unlock curve and balance

## Success Criteria

This review pass is successful when:

- no option shares its displayed name with its parent theme
- descriptions read as player-facing outcomes rather than runtime internals
- the two strongest overlap pairs are redesigned into distinct choices
- future balance feedback can talk about option strength without first needing to decode what the option means
