# Build Slot Overflow Layout Design

> Supersedes the fixed-ratio detail-rail contract in [2026-03-15-build-detail-rail-layout-design.md](/home/kdw73/Codex%20of%20Power%20NG/docs/superpowers/specs/2026-03-15-build-detail-rail-layout-design.md) for the active-slot panel portion of the right rail.

## Goal

Build 탭 우측 하단 `활성 슬롯` 패널이 더 이상 카드/버튼을 잘라 먹지 않도록 하면서, `선택 옵션` 패널이 우측 레일의 주 정보 영역이라는 우선순위는 유지한다.

## Problem

현재 [index.html](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/index.html) 에서 `buildDetailRail`은 아래 행을 `minmax(calc(170px * var(--uiScale)), 0.8fr)`로 고정하고, `buildSlotSummaryPanel`은 `overflow: hidden`으로 막고 있다.

이 조합 때문에:

- 우측 하단 패널 높이가 내용보다 먼저 고정된다.
- 슬롯 카드 6개가 2열로 배치될 때 세로 여유가 모자라면 카드 하단과 버튼이 잘린다.
- 패널을 줄인 목적은 맞지만, 실제 사용성은 악화된다.

## Approved Direction

승인된 방향은 다음과 같다.

- `선택 옵션` 패널이 더 큰 비중을 차지하는 현재 정보 우선순위는 유지한다.
- `활성 슬롯` 패널은 고정 비율 높이 대신 내용 기반 높이로 바꾼다.
- 슬롯 패널 전체가 아니라 슬롯 그리드 내부만 필요 시 짧게 스크롤되도록 만든다.
- 슬롯 카드 자체도 한 단계 더 압축해, 평소에는 스크롤 없이 2열 6카드가 자연스럽게 보이도록 한다.

## Chosen Approach

추천안으로 확정된 접근은 `고정 행 비율`을 완화하고 `패널 내부 구조`를 나누는 방식이다.

### Layout Contract

- `buildDetailRail`의 하단 행은 `0.8fr` 고정 비중이 아니라 `auto` 성격으로 바꾼다.
- `buildSelectedOptionPanel`은 계속 우측 레일의 주 스크롤 표면으로 남긴다.
- `buildSlotSummaryPanel`은 `header + slot scroller` 구조로 바꾼다.

### Slot Panel Contract

- 패널 헤더는 고정 노출된다.
- 슬롯 카드 그리드만 별도 래퍼 안에서 스크롤 가능하다.
- 이 래퍼는 `buildSlotMatrixScroller` 클래스를 사용하고 `data-wheel-surface="build-slots"` 계약을 가져야 한다.
- 실제 카드가 패널 바깥으로 잘려 보이면 안 된다.
- 스크롤은 공간이 부족할 때만 발동해야 한다.

### Density Contract

- 슬롯 카드 패딩, gap, 버튼 높이, 슬롯 이름 line-height를 한 단계 더 낮춘다.
- 슬롯 패널은 참조용 요약 영역처럼 보여야 하며, 선택 옵션 패널보다 시각 존재감이 커지면 안 된다.

## Rejected Alternatives

### 1. 슬롯 패널 전체를 스크롤

Rejected because:

- 패널 전체가 답답해진다.
- 헤더와 슬롯 목록을 함께 스크롤해야 해서 참조성이 떨어진다.

### 2. 우측 하단 패널 높이를 다시 크게 복원

Rejected because:

- 잘림은 없어지지만, 방금 확보한 `선택 옵션` 가독성이 다시 줄어든다.
- 문제의 본체는 `작다`가 아니라 `고정 높이라 잘린다`는 점이다.

## Files And Responsibilities

- [PrismaUI/views/codexofpowerng/index.html](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/index.html)
  - `buildDetailRail`, `buildSlotSummaryPanel`, 슬롯 스크롤 래퍼, 카드 밀도, responsive contract
- [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
  - 슬롯 패널 마크업을 `header + matrix scroller`로 분리하는 계약 제공
- [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
  - CSS source contract와 슬롯 패널 마크업 계약 검증

## Verification Surface

- 소스 테스트에서 `buildDetailRail` 하단 행이 더 이상 `0.8fr` 고정 계약이 아니어야 한다.
- 슬롯 패널은 별도 스크롤 래퍼 클래스를 가져야 한다.
- 렌더링 테스트는 `buildSlotMatrixScroller` 가 `data-wheel-surface="build-slots"` 를 노출하는지 확인해야 한다.
- 슬롯 카드 압축 규칙이 CSS에 존재해야 한다.
- 런타임 검증은 고정 Build viewport 안에서 6개 슬롯 카드와 버튼이 잘리지 않고, 패널 전체가 아니라 슬롯 그리드만 스크롤되는지 확인해야 한다.
- `@media (max-width: 980px)` 단일열 fallback은 유지되어야 한다.
