# Build Detail Rail Layout Design

> Partially superseded on 2026-03-15 by [2026-03-15-build-slot-overflow-layout-design.md](/home/kdw73/Codex%20of%20Power%20NG/docs/superpowers/specs/2026-03-15-build-slot-overflow-layout-design.md) for the active-slot panel overflow contract.

## Goal

Build 탭 우측 레일에서 `선택 옵션` 패널이 의사결정 중심 영역이 되도록 확장하고, `활성 슬롯` 패널은 참조용 요약 영역으로 축소한다.

## Problem

현재 우측 레일은 [index.html](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/index.html)에서 `buildDetailRail`이 `minmax(0, 1fr) auto` 구조라 하단 `활성 슬롯` 패널이 카드/버튼 밀도로 높이를 많이 차지한다. 실제 플레이어는 우측 레일에서 다음 정보를 더 오래 읽는다.

- 선택 옵션 제목과 상태
- 현재 효과와 다음 단계 효과
- 해금 조건과 호환 슬롯

반면 활성 슬롯은 현재 장착된 옵션을 빠르게 훑는 용도라, 시각 비중이 과하다.

## Approved Direction

추천안으로 확정된 방향은 다음과 같다.

- 우측 레일 비율을 `선택 옵션 68% / 활성 슬롯 32%` 수준으로 재배치한다.
- 활성 슬롯 패널은 유지하되, 카드 밀도와 액션 버튼 존재감을 낮춰 참조 정보처럼 보이게 만든다.
- `980px` 이하에서는 기존 단일 열 흐름을 유지해 모바일/좁은 창 회귀를 피한다.

## Layout Contract

### Desktop

- `buildDetailRail`은 고정 비율 그리드로 바꾼다.
- 선택 옵션 패널은 상단에서 더 많은 높이를 확보해야 한다.
- 활성 슬롯 패널은 `auto`가 아니라 명시적 `minmax(..., 0.8fr)` 수준으로 제한한다.

### Slot Panel Density

- 슬롯 카드 패딩과 gap을 줄인다.
- 슬롯 이름 폰트와 버튼 높이를 한 단계 낮춘다.
- 카드의 장식성은 유지하되, 선택 옵션 패널보다 덜 눈에 띄어야 한다.

## Files And Responsibilities

- [PrismaUI/views/codexofpowerng/index.html](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/index.html)
  - 우측 레일 비율, 슬롯 패널 높이, 슬롯 카드 밀도, 반응형 유지
- [PrismaUI/views/codexofpowerng/ui_build_panel.js](/home/kdw73/Codex%20of%20Power%20NG/PrismaUI/views/codexofpowerng/ui_build_panel.js)
  - 구조는 유지하고, 테스트가 확인할 수 있는 클래스/마크업 계약을 계속 제공
- [tests/build_ui_rendering_module.test.cjs](/home/kdw73/Codex%20of%20Power%20NG/tests/build_ui_rendering_module.test.cjs)
  - 레이아웃 contract와 우측 레일 패널 존재 순서를 검증

## Verification Surface

- CSS source에 `buildDetailRail` ratio와 슬롯 카드 축소 규칙이 존재해야 한다.
- Build panel 렌더링 테스트는 선택 옵션 패널과 활성 슬롯 패널이 동일 우측 레일에 유지되는 구조를 확인해야 한다.
- 모바일 breakpoint(`max-width: 980px`)는 유지되어야 한다.
