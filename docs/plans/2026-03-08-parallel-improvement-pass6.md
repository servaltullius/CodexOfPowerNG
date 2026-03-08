## Goal
- `index.html`에 남아 있는 PrismaUI 상태, 상호작용, 초기 배선 계층을 모듈로 완전히 분리한다.
- 렌더링 전용 모듈 분리(pass5) 다음 단계로, `state`, `input`, `wiring/bootstrap` 책임을 `ui_state.js`, `ui_interactions.js`, `ui_bootstrap.js`로 이동한다.

## Non-goals
- PrismaUI 마크업/스타일 구조 재설계
- 네이티브 브리지 프로토콜 변경
- 새 기능 추가나 설정 payload 변경

## Affected files
- `PrismaUI/views/codexofpowerng/index.html`
- `PrismaUI/views/codexofpowerng/ui_state.js`
- `PrismaUI/views/codexofpowerng/ui_interactions.js`
- `PrismaUI/views/codexofpowerng/ui_bootstrap.js`
- `tests/input_correction_module.test.cjs`
- `tests/undo_registration_flow.test.cjs`
- `tests/settings_payload_validation.test.cjs`
- `tests/virtual_refresh_resync.test.cjs`
- 신규 구조 고정용 모듈 테스트들

## Constraints
- 동작 변화 없이 책임만 이동한다.
- `index.html`은 부트스트랩 조립과 DOM 참조만 담당한다.
- 가상 테이블 리싱크, 입력 보정 fallback, ESC fallback 같은 기존 안전 레일은 유지한다.
- 기존 테스트가 보던 계약 문자열은 모듈 이동에 맞춰 새 파일로 이전한다.

## Milestones
1. `index.html`에 새 모듈 스크립트 로드
2. 상태 저장/입력 스케일/토스트/키 표시 책임을 `ui_state.js`로 연결
3. quick/undo/settings 저장과 입력 변경 핸들러를 `ui_interactions.js`로 연결
4. wiring, 초기 native pull, input correction/hotkey bootstrap을 `ui_bootstrap.js`로 연결
5. 구조 회귀 테스트 추가 및 기존 테스트 기대치 갱신

## Validation
- 대상 Node 테스트 실행
- `bash scripts/test.sh`
- `env VCPKG_ROOT=/mnt/c/Users/<user>/vcpkg cmake --build --preset wsl-release`

## Risks / Rollback
- 위험: `index.html`에서 제거한 전역/인라인 함수명을 다른 테스트나 모듈이 여전히 참조할 수 있다.
- 대응: 모듈 로드 순서와 wiring API 연결을 테스트로 고정한다.
- 롤백: pass5 기준의 인라인 상태/입력/배선 블록으로 되돌리면 된다.
