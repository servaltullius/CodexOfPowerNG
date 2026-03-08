## Goal
- PrismaUI에 남아 있는 `I18N`/정적 데이터 덩어리를 `index.html` 밖으로 분리한다.
- 새 상태 저장소 경계의 핵심 동작을 호스트 친화 C++ 테스트로 직접 검증한다.
- CI에 패키징 스모크 레일을 추가하고, README/릴리즈 절차 문서를 현재 구조에 맞춘다.

## Non-goals
- UI 마크업/스타일 전면 재디자인
- Skyrim 런타임 동작 변경
- 실제 GitHub Releases 업로드 자동화

## Affected files
- `PrismaUI/views/codexofpowerng/index.html`
- `PrismaUI/views/codexofpowerng/ui_i18n.js`
- `include/CodexOfPowerNG/*StateStoreOps.h`
- `src/*StateStore.cpp`
- `tests/*.test.cpp`
- `.github/workflows/validate.yml`
- `scripts/package_release.sh`
- `scripts/check_release_zip.sh`
- `README.md`
- 필요 시 관련 JS 구조 테스트

## Constraints
- 동작 변경 없이 구조와 운영성만 개선한다.
- `index.html`은 조립과 DOM 참조 중심을 유지한다.
- 새 C++ 테스트는 `scripts/test.sh`에서 바로 도는 host-friendly 형태를 우선한다.
- 패키징 스모크는 “아카이브 구조 검증”임을 문서에 명확히 적는다.

## Milestones
1. `ui_i18n.js` 추가 및 `index.html` 인라인 사전 제거
2. 상태 저장소 pure helper/ops 추출
3. host C++ 테스트 추가
4. 패키징/아카이브 검증 스크립트 및 CI 연결
5. README/릴리즈 절차 문서 동기화

## Validation
- 대상 Node 테스트
- 대상 host C++ 테스트
- `bash scripts/test.sh`
- `env VCPKG_ROOT=/mnt/c/Users/<user>/vcpkg cmake --build --preset wsl-release`
- `bash scripts/package_release.sh`
- `bash scripts/check_release_zip.sh`

## Risks / Rollback
- UI 문자열을 옮기면서 테스트가 `index.html` 문자열에 직접 의존하면 깨질 수 있다.
- 패키징 스모크는 실제 Windows 빌드를 대체하지 않으므로 README에 범위를 분명히 적는다.
- 롤백은 `ui_i18n.js` 제거 + helper 헤더 제거 + 기존 스크립트/워크플로 복원으로 가능하다.
