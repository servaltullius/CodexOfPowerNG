# Build Option Scaling Design

## Summary

현재 빌드 성장 모델은 `계통 점수 -> 별도 baseline milestone 보너스`와 `해금/장착된 정적 옵션`의 조합이다. 이 모델은 구현은 단순하지만, 플레이어가 기대하는 감각과 어긋난다. 기록을 더 많이 쌓았을 때 기대하는 것은 `계통 자체의 별도 패시브`가 아니라, `내가 장착한 옵션이 더 강해지는 것`이다.

이번 설계는 baseline milestone 모델을 제거하고, `계통 점수 기반 옵션 스케일링`으로 성장 모델을 교체한다. 해금은 기존처럼 계통 점수로 처리하되, 장착한 옵션의 최종 수치는 같은 계통 점수에 따라 계속 강화된다.

## Goals

- `기록을 쌓을수록 장착한 옵션이 직접 강해진다`는 감각을 만든다.
- 해금과 강화가 같은 `discipline score` 축을 사용하도록 단순화한다.
- baseline milestone과 slotted option의 이중 성장 구조를 제거한다.
- 플레이어가 현재 옵션 수치, 현재 단계, 다음 단계까지 남은 점수를 쉽게 이해할 수 있게 한다.
- 모드 추가로 아이템 풀이 늘어나더라도 규칙이 흔들리지 않게 `절대 점수` 기준을 유지한다.

## Non-Goals

- discipline 수를 늘리지 않는다.
- slot 구조를 바꾸지 않는다.
- 기록 점수를 진행률 기반으로 바꾸지 않는다.
- 이번 설계에서 새로운 effect family를 추가하지 않는다.
- `skill rewards`를 다시 편입하지 않는다.

## Problem With The Baseline Model

### 1. 플레이어가 성장의 원인을 잘못 읽게 된다

현재 baseline은 discipline score가 오르면 별도 passive가 붙는 구조다. 이 경우:

- 어떤 수치가 option 자체 효과인지
- 어떤 수치가 baseline 누적인지

가 분리된다. 플레이어는 `기동이 강해졌다`보다 `유틸 점수에 따라 뭔가 따로 붙는다`로 읽게 된다.

### 2. 옵션 비교를 흐린다

baseline을 옵션 설명에 합산하면 비교가 어려워진다. baseline을 별도로 보여주면 설명은 깔끔하지만, 이번에는 성장의 중심이 옵션이 아니라 baseline처럼 느껴진다.

### 3. 빌드 컨셉과 어긋난다

현재 빌드 시스템은 `기록 기반 빌드 조립`을 표방한다. 이 철학에 더 맞는 성장 방식은 `내가 장착한 선택이 강해지는 것`이지, 계통별 숨은 패시브가 같이 자라는 방식이 아니다.

## Chosen Approach

성장 모델을 다음처럼 재정의한다.

- `discipline score`는 해금과 강화에 모두 사용한다.
- 옵션은 해금 후 장착할 수 있다.
- 장착된 옵션의 최종 효과는 `discipline score 10점마다 1단계` 강화된다.
- 모든 옵션은 같은 단계 규칙을 공유한다.
- 옵션마다 다른 점은 `기본값`과 `단계당 증가량`이다.
- 기존 baseline milestone은 제거한다.

한 줄로 표현하면:

`계통 점수는 별도 패시브를 주지 않고, 장착한 옵션의 수치를 키우는 자원이다.`

## Scaling Model

### Shared Rule

- 스케일 기준: 해당 옵션의 discipline score
- 단계 규칙: `discipline score 10점마다 +1 tier`
- tier 계산 예시:
  - 0-9점: tier 0
  - 10-19점: tier 1
  - 20-29점: tier 2
  - 30-39점: tier 3
  - ...

### Option Formula

각 옵션은 다음 공식을 가진다.

- `baseMagnitude`
- `perTierMagnitude`
- `displayRule`

최종 수치:

- `finalMagnitude = baseMagnitude + (tier * perTierMagnitude)`

예시:

- `기동`
  - base: `+2% 이동 속도`
  - perTier: `+1%`
  - utility score 20점 -> tier 2 -> `+4%`
- `비축`
  - base: `+10 carry weight`
  - perTier: `+5`
  - utility score 30점 -> tier 3 -> `+25`
- `맹공`
  - base: `+3% 공격 피해`
  - perTier: `+2%`
  - attack score 40점 -> tier 4 -> `+11%`

## Why Shared 10-Point Tiers

### 1. 설명이 쉽다

플레이어는 다음 규칙만 기억하면 된다.

- `같은 계통 기록 10점마다 장착한 옵션이 더 강해진다`

### 2. 해금과 강화 축이 일치한다

현재도 해금은 discipline score를 사용한다. 강화도 같은 축을 쓰면 시스템이 단순해진다.

### 3. 아이템 추가 모드에 덜 흔들린다

진행률 기반이 아니라 절대 점수 기반이므로, 아이템 풀의 총량이 달라도 규칙 자체는 바뀌지 않는다.

## Data Model Changes

### Remove

- discipline baseline milestone definitions
- baseline unlocked count as a player-facing concept
- baseline effect summary as a primary UI object

### Add

각 `BuildOptionDef`는 아래 필드를 가져야 한다.

- `baseMagnitude`
- `perTierMagnitude`
- `scalesWithDisciplineScore: true`
- optional `displayPrecision` or equivalent formatting hint if current magnitude rendering needs it

런타임 계산은 catalog row에서 `base + tier scaling`을 적용한 결과를 쓰도록 바뀐다.

## Runtime Behavior

옵션 활성 시 적용되는 값은 catalog 정의의 정적 magnitude가 아니라, 현재 discipline score를 반영한 계산 결과여야 한다.

필수 조건:

- score 변화 시 활성 옵션 재계산
- 장착/비장착 변경 시 재계산
- 세이브 로드 후 현재 score 기준으로 재계산
- 기존 baseline 누적 효과는 더 이상 적용하지 않음

즉 런타임에는 `baseline delta`를 따로 누적하지 않고, `현재 활성 옵션의 스케일된 최종값 집합`만 동기화한다.

## UI Model

### Top Summary Cards

상단 카드는 baseline 요약 대신 다음만 보여준다.

- discipline name
- discipline score
- 현재 tier 또는 다음 tier까지 남은 점수

예:

- `공격 33`
- `다음 강화까지 7점`

### Catalog Rows

중앙 카탈로그 행은 `현재 효과`를 보여준다.

예:

- `기동 — 이동 속도 +4%`
- `비축 — 소지 한도 +25`

즉 row copy는 더 이상 정적 설명이 아니라, 현재 score를 반영한 수치가 포함된다.

### Detail Panel

오른쪽 상세는 다음 정보를 보여준다.

- 현재 효과
- 현재 tier
- 다음 tier에서의 효과
- 다음 tier까지 남은 점수

예:

- `현재 효과: 이동 속도 +4%`
- `현재 단계: 2`
- `다음 단계 효과: 이동 속도 +5%`
- `다음 강화까지 7점`

이 방식이면 baseline 보조 문구는 필요 없어지고, 플레이어가 성장 구조를 즉시 이해할 수 있다.

## Copy Rules

- 플레이어가 익숙한 단위면 수치를 직접 표시한다.
  - `%`
  - `+10`
  - `+25`
- 내부 단위가 직관적이지 않으면 결과 중심 문구를 유지하되, 가능하면 tier별 현재 값과 다음 값을 비교 가능한 형태로 표현한다.
- 같은 effect family는 같은 표현 규칙을 공유한다.

## Migration Rules

### Save/Data Migration

- 기존 baseline milestone state는 더 이상 player-facing 의미를 갖지 않는다.
- load 시 baseline 누적 효과를 재적용하지 않는다.
- 기존 save에서 활성 옵션은 그대로 유지하되, 다음 기준으로 최종 수치를 다시 계산한다:
  - 현재 discipline score
  - 현재 장착 슬롯

### UI Migration

- baseline summary strings 제거
- baseline current summary 제거
- tier/delta strings 추가

## Balance Guidance

이번 설계는 밸런스를 확정하지 않는다. 다만 아래 원칙을 둔다.

- 모든 옵션이 같은 `10점당 1 tier` 규칙을 공유한다.
- 옵션 차별화는 `baseMagnitude`와 `perTierMagnitude`로 만든다.
- 너무 강한 옵션은 unlock score를 늦추거나 per-tier magnitude를 낮춘다.
- 너무 약한 옵션은 unlock score를 늦추는 대신 per-tier magnitude를 높일 수 있다.

## Open Questions For Implementation

- 현재 32개 옵션 각각의 `baseMagnitude / perTierMagnitude`를 어떻게 정할지
- raw runtime 단위를 쓰는 옵션들의 display formatter를 어디까지 공통화할지
- summary card에 `현재 tier`와 `다음 강화까지 남은 점수`를 둘 다 보여줄지, 하나만 보여줄지
- 기존 baseline-related serialization 필드를 바로 제거할지, 한 버전 유예를 둘지

## Follow-up

구현 단계는 다음 순서로 나누는 것이 맞다.

1. baseline milestone 제거와 scaling metadata 도입
2. runtime recalculation을 scaled option model로 전환
3. build UI를 current/next tier 중심으로 개편
4. option별 base/per-tier 밸런스 조정
