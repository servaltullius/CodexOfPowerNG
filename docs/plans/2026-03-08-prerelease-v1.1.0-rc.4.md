## Goal
- `v1.1.0-rc.4` 프리릴리즈를 생성한다.
- 이번 세션의 구조 분리, 테스트 보강, release smoke 검증 결과를 한국어 릴리즈 노트에 반영한다.
- 원격 `main`, Git 태그, GitHub prerelease를 서로 일치시키고 릴리즈 에셋까지 업로드한다.

## Non-goals
- stable 릴리즈 생성
- Nexus 업로드 자동화
- 추가 기능 개발

## Affected files
- `docs/releases/v1.1.0-rc.4.md`
- `CHANGELOG.md`
- 필요 시 release 관련 계획 문서

## Constraints
- 릴리즈 노트는 한국어로 작성한다.
- `CHANGELOG.md`는 기존 정책에 맞춰 영어 형식을 유지한다.
- 실제 업로드 전에 install/package/check 검증을 다시 수행한다.

## Milestones
1. 릴리즈 노트/CHANGELOG 작성
2. install + package + zip smoke 재검증
3. 커밋/푸시
4. 태그/프리릴리즈 생성
5. 원격 상태 확인

## Validation
- `bash scripts/test.sh`
- `env VCPKG_ROOT=/mnt/c/Users/kdw73/vcpkg cmake --build --preset wsl-release`
- `env VCPKG_ROOT=/mnt/c/Users/kdw73/vcpkg cmake --install build/wsl-release`
- `bash scripts/package_release.sh`
- `bash scripts/check_release_zip.sh`

## Risks / Rollback
- release asset 이름과 문서 링크가 어긋나면 GitHub prerelease와 문서가 불일치할 수 있다.
- 원격 푸시/태그는 외부 부작용이 있으므로, 릴리즈 문서와 검증 결과를 먼저 고정한 뒤 실행한다.
- 문제 발생 시 GitHub prerelease 삭제와 태그 삭제가 필요할 수 있다.
