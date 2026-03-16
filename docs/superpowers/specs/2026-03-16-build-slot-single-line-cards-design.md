# Build Slot Single-Line Cards Design

## Goal

Build 탭 우측 하단 `활성 슬롯` 패널을 더 컴팩트하게 만들어, 선택 옵션 패널의 가독성을 유지하면서 슬롯 상태를 빠르게 훑는 요약 영역으로 정리한다.

## Problem

현재 활성 슬롯 카드는 다음처럼 세로 3단 적층이다.

- 슬롯 종류 chip
- 옵션 이름
- 액션 또는 빈 슬롯 안내

이 구조는 overflow 문제를 해결한 뒤에도 여전히 카드 높이를 크게 만들고, 우측 레일 하단 패널이 필요 이상으로 무거워 보이게 만든다.

## Approved Direction

승인된 방향은 다음과 같다.

- 패널 전체 구조나 `data-wheel-surface="build-slots"` 계약은 유지한다.
- 각 슬롯 카드는 `슬롯 마크 | 옵션명 | 액션`의 1행 레이아웃으로 압축한다.
- 빈 슬롯 문구는 현재보다 더 짧은 표현으로 줄인다.
- 옵션명은 한 줄 말줄임 처리해 좁은 폭에서도 레이아웃을 유지한다.
- `@media (max-width: 980px)` 단일열 fallback은 유지한다.

## Chosen Approach

추천안으로 확정된 접근은 슬롯 카드 내부 구조만 재배치하는 방식이다.

### Card Contract

- `buildSlotMatrixCard`는 세로 stack 대신 가로 3열 정렬을 사용한다.
- 좌측은 discipline chip, 중앙은 옵션명, 우측은 액션 또는 빈 슬롯 힌트다.
- 카드 높이는 내용 기반이지만, 현재 3단 카드보다 확실히 낮아야 한다.

### Density Contract

- 슬롯 카드 패딩과 내부 gap을 한 단계 더 줄인다.
- 버튼 높이와 폰트 크기도 함께 낮춘다.
- 슬롯 이름과 빈 슬롯 힌트는 모두 한 줄에서 끝나야 한다.

## Rejected Alternatives

### 1. 패널 전체를 더 낮게 고정

Rejected because:

- 이미 overflow fix 이후에도 내부 카드 높이가 문제라 근본 해결이 아니다.

### 2. 슬롯 6개 전체를 가로 한 줄로 배치

Rejected because:

- 현재 우측 레일 폭에서는 카드별 가독성이 급격히 나빠진다.
- 모바일/좁은 창 대응도 더 불안정해진다.

## Files And Responsibilities

- [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
  - 슬롯 카드 마크업을 1행 요약 구조에 맞게 조정한다.
- [PrismaUI/views/codexofpowerng/index.html](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/index.html)
  - 슬롯 카드의 1행 레이아웃, 말줄임, compact spacing 규칙을 정의한다.
- [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
  - 새 카드 구조와 compact CSS contract를 source-level assertion으로 고정한다.

## Verification Surface

- 렌더링 테스트에서 슬롯 카드가 `header/name/actions`의 세로 흐름 대신 1행 정렬 규칙을 갖는지 확인해야 한다.
- 옵션명 말줄임 규칙과 더 짧아진 빈 슬롯 문구가 source contract에 반영되어야 한다.
- 빈 슬롯 힌트도 한 줄 말줄임 규칙을 가져야 한다.
- `buildSlotMatrixScroller`와 모바일 breakpoint 계약은 유지되어야 한다.
- 고정 Build viewport에서 6개 슬롯 카드와 버튼이 잘리지 않고, 공간이 부족할 때 슬롯 그리드만 스크롤되는지 런타임으로 확인해야 한다.
