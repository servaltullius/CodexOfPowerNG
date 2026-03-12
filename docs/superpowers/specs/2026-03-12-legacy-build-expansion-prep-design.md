# Legacy Build Expansion Prep Design

## Summary

현재 Build UI 탐색 구조는 `레거시 보상 대부분`을 다시 수용할 수 있는 방향으로 바뀌었지만, 실제 효과 확장을 바로 시작할 단계는 아니다. 우선 필요한 것은 `어떤 레거시 효과를 넣을지`, `실제로 런타임에 적용 가능한지`, `기존 최소 카탈로그와 어떻게 이어질지`를 한 번에 정리하는 준비작업이다.

이번 설계의 목표는 효과 확장 구현 전에 필요한 준비 산출물을 고정하는 것이다. 핵심 원칙은 `분류와 적용 가능성 분리 금지`, `skill rewards 제외`, `테마 수 유지`, `마이그레이션 규칙 선확정`이다.

## Goals

- 레거시 보상 후보를 새 Build 구조에 편입할 수 있도록 정리한다.
- 각 효과를 `계통 / 테마 / 위계 / 런타임 적용 가능성`까지 한 줄에서 판단 가능하게 만든다.
- 현재 최소 카탈로그(`3 x 3`)와 확장 카탈로그 사이의 마이그레이션 규칙을 미리 고정한다.
- 이후 구현 배치를 `지원 가능 효과부터 순차 편입` 방식으로 안전하게 진행할 수 있게 한다.

## Non-Goals

- 이번 단계에서 레거시 효과를 실제 카탈로그에 대량 추가하지 않는다.
- 런타임 훅이나 ActorValue 적용 경로를 확장 구현하지 않는다.
- UI를 다시 수정하지 않는다.
- `skill rewards`를 새 Build 카탈로그에 편입하지 않는다.

## Scope Boundary

이번 준비작업은 `actor value / modifier / utility flag` 계열만 대상으로 한다.

명시적으로 제외하는 것:

- One-Handed / Two-Handed / Archery
- Heavy Armor / Light Armor / Block
- Alchemy / Lockpicking / Pickpocket
- 기타 스킬 수치 자체를 올리는 레거시 보상

이유:

- 스킬 성장과 빌드 성장 시스템을 섞지 않기 위해서
- 설명과 체감이 더 직접적인 AV/modifier 계열부터 확장하기 위해서
- 런타임 적용 경로와 밸런스를 단순하게 유지하기 위해서

## Why Preparation Work Comes First

### 1. 현재 런타임이 바로 적용할 수 있는 효과는 제한적이다

현재 BuildEffectRuntime은 다음 계열만 직접 매핑하고 있다.

- `attack_damage_mult`
- `weapon_speed_mult`
- `critical_chance`
- `damage_resist`
- `block_power_modifier`
- `health`
- `speed_mult`
- `carry_weight`
- `speechcraft_modifier`

즉 레거시 보상을 전부 UI에 먼저 넣으면, 설명은 있지만 실제 적용 경로가 없는 옵션이 섞일 수 있다.

### 2. 레거시 보상은 현재 카탈로그보다 훨씬 넓다

레거시 reward table에는 다음과 같은 범주가 섞여 있다.

- 체력 / 마력 / 지구력
- regen 계열
- 원소/마법/질병/독 저항
- 이동 속도 / 소지 한도
- 거래 / 제작 / 은신 / 자물쇠 / 소매치기 modifier
- shout recovery / absorb chance 등 특수값

이걸 바로 편입하면 `UI 설계`보다 `정의되지 않은 효과`가 먼저 문제가 된다.

### 3. 기존 3x3 최소 카탈로그와 연결 규칙이 필요하다

이미 저장된 build 상태와 현재 UI는 최소 카탈로그를 전제로 한다. 확장 카탈로그로 넘어갈 때, 기존 옵션이 어느 테마의 어떤 위계로 이어지는지 먼저 정해두지 않으면 마이그레이션과 설명 정합성이 깨진다.

## Chosen Approach

준비작업은 `단일 인벤토리 매트릭스`로 진행한다.

즉 효과마다 한 줄씩 아래 정보를 같이 적는 방식이다.

- 레거시 원천
- 새 Build 분류
- 새 Build 위계
- 런타임 적용 상태
- 기존 카탈로그와의 연결 관계
- 구현 비고

이 접근을 택한 이유:

- 카탈로그 분류와 런타임 가능성을 따로 관리하면 금방 어긋난다.
- 구현 시 `supported`만 먼저 편입하고 `needs-runtime`은 별도 배치로 넘기기 쉽다.
- UI용 분류표와 엔진용 매핑표를 한 번에 유지할 수 있다.

## Inventory Record Schema

각 레거시 효과 후보는 최소한 다음 필드를 가진다.

- `legacyKey`
- `sourceGroup`
- `label`
- `discipline`
- `theme`
- `hierarchy`
- `effectType`
- `runtimeKey`
- `magnitude`
- `runtimeStatus`
- `migrationFrom`
- `notes`

### Field Meanings

#### `legacyKey`

레거시 reward table 또는 관련 시스템에서 이 효과를 식별하는 키.

#### `sourceGroup`

효과가 주로 나오던 레거시 그룹.

예:

- `weapon`
- `armor`
- `consumable`
- `ingredient`
- `book`
- `misc`

#### `discipline`

- `attack`
- `defense`
- `utility`

#### `theme`

현재 합의된 테마만 사용한다.

- Attack: `devastation / precision / fury`
- Defense: `guard / bastion / resistance`
- Utility: `livelihood / exploration / finesse`

#### `hierarchy`

- `signpost`
- `standard`
- `special`

테마 안에서 옵션의 역할을 구분한다.

#### `runtimeStatus`

- `supported`
- `needs-runtime`
- `deferred`

이 필드가 준비작업의 핵심 게이트다.

## Runtime Status Rules

### `supported`

현재 또는 아주 작은 연결 수정만으로 런타임 적용 가능한 효과.

예:

- `damage_resist`
- `health`
- `carry_weight`
- `speechcraft_modifier`
- `speed_mult`
- `critical_chance`

### `needs-runtime`

테마상 편입 가치는 높지만, 새 런타임 매핑이나 적용 경로가 필요하다.

예상 후보:

- 각종 저항
- regen 계열
- stealth / lockpicking / pickpocket modifier
- crafting / alchemy / enchanting modifier
- shout recovery / absorb chance

### `deferred`

현재 확장 범위에서 제외하거나, 설계가 더 필요해 바로 편입하지 않을 항목.

주로 다음 경우다.

- skill reward
- 의미가 애매하거나 중복이 큰 효과
- 새 Build 정체성에 넣을 이유가 약한 효과

## Theme Rules

- 테마는 늘리지 않는다.
- 애매한 효과는 `주 테마 1개`를 강제로 정한다.
- 필요하면 `notes`에 보조 태그 성격을 적되, 1차 분류는 하나만 둔다.
- 테마 수를 늘리기보다 기존 테마 안에 효과를 더 많이 담는다.

## Migration Rules

준비작업 문서에는 현재 최소 카탈로그에서 확장 카탈로그로의 연결 규칙도 포함한다.

최소한 아래는 고정한다.

- `build.attack.ferocity` -> `attack / devastation`
- `build.attack.precision` -> `attack / precision`
- `build.attack.vitals` -> `attack / precision`
- `build.defense.guard` -> `defense / guard`
- `build.defense.bastion` -> `defense / bastion`
- `build.defense.endurance` -> `defense / guard`
- `build.utility.cache` -> `utility / livelihood`
- `build.utility.barter` -> `utility / livelihood`
- `build.utility.mobility` -> `utility / exploration`

목적은 두 가지다.

- 기존 저장/설명/UI 정합성 유지
- 확장 후에도 기존 유저가 어디로 이동했는지 이해 가능하게 만들기

## Deliverables

이번 준비작업에서 최종적으로 필요한 산출물은 세 가지다.

1. `레거시 효과 인벤토리 매트릭스`
2. `런타임 적용 가능성 분류`
3. `기존 최소 카탈로그 -> 확장 카탈로그 매핑 규칙`

실제 구현은 이 문서를 바탕으로 다음 순서로 진행한다.

- `supported` 먼저 편입
- `needs-runtime`은 별도 배치 구현
- `deferred`는 명시적으로 제외

## Success Criteria

준비작업이 끝났다고 말할 수 있는 조건은 아래다.

- 레거시 AV/modifier 계열이 누락 없이 매트릭스에 들어 있다.
- 각 항목이 정확히 하나의 `discipline/theme`를 가진다.
- 각 항목이 `supported / needs-runtime / deferred` 중 하나로 판정된다.
- 기존 3x3 카탈로그의 연결 규칙이 명시돼 있다.
- 다음 구현 배치를 `공격 -> 방어 -> 유틸` 또는 `supported -> needs-runtime` 순으로 안전하게 나눌 수 있다.

## Follow-up

다음 단계 구현 계획은 이 준비작업 문서를 바탕으로 아래를 진행한다.

- 레거시 효과 인벤토리 파일 작성
- 지원 가능 효과 우선 편입
- 런타임 확장 배치 분리
- 마이그레이션 및 설명 문구 정리
