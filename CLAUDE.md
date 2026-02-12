# Project Instructions

> 상세 아키텍처/런타임 문서: `AGENTS.md` 참조

## Quick Overview

Skyrim SE SKSE plugin — C++ 백엔드 + PrismaUI(Ultralight) 웹 오버레이 UI.
인게임에서 아이템 수집/등록을 관리하는 도감 모드.

## Key Files

| 역할 | 경로 |
|------|------|
| UI (단일 페이지) | `PrismaUI/views/codexofpowerng/index.html` |
| UI 매니저 (C++) | `src/PrismaUIManager.cpp` |
| 아이템 등록 로직 | `src/Registration.cpp` |
| 등록 규칙/제외 | `src/RegistrationRules.cpp` |
| 인벤토리 유틸 | `src/Inventory.cpp` |
| 이벤트 핸들러 | `src/Events.cpp` |
| 직렬화 (co-save) | `src/Serialization.cpp` |
| 설정 | `src/Config.cpp`, `include/CodexOfPowerNG/Config.h` |
| 번역 (EN/KO) | `SKSE/Plugins/CodexOfPowerNG/lang/{en,ko}.json` |
| JS 모듈 | `PrismaUI/views/codexofpowerng/{keycodes,lang_ui}.js` |
| 타입 정의 | `include/CodexOfPowerNG/Registration.h` |

## Build (WSL → Windows)

```bash
export VCPKG_ROOT="/mnt/c/Users/kdw73/vcpkg"
cmake --preset wsl-release
cmake --build --preset wsl-release
cmake --install build/wsl-release
# zip
cd dist/CodexOfPowerNG && zip -r -FS "../../releases/Codex of Power NG.zip" .
```

로컬에 MSVC 컴파일 환경 없음. `clang-cl` + `lld-link` 크로스 빌드.

## Testing

```bash
node tests/lang_ui.test.cjs
node tests/keycodes.test.cjs
node tests/quick_register_rules.test.cjs
node tests/settings_sticky_actions.test.cjs
```

C++ 단위 테스트 없음 — 빌드 성공 + JS 테스트 통과로 검증.

## Code Conventions

### C++
- **들여쓰기**: 탭 (스페이스 아님)
- **익명 namespace 내부**: 탭 2개
- **명명된 namespace public 함수**: 탭 1개
- 편집 시 반드시 Read로 원본 탭/스페이스 확인 후 수정할 것 (Edit 도구가 탭↔스페이스 불일치로 실패함)

### HTML/JS
- HTML: 2-space
- JS (`<script>` 내부): 6-space 기본 (4 HTML + 2 JS)

### 릴리즈
- GitHub 릴리즈 노트는 **한국어**로 작성
- CHANGELOG.md는 영어

## Type Names (혼동 주의)

- `Registration::QuickRegisterList` (NOT QuickRegisterPage)
- `Registration::ListItem` (NOT RegisteredItem)
- `L10n::T(string_view, string_view)` — 로컬라이제이션 함수

## Thread Model

- **Main thread** (`AddTask`): 게임 오브젝트 접근 (인벤토리, RemoveItem, ActorValue)
- **UI thread** (`AddUITask`): PrismaUI API (Show/Focus/Hide/CallJS)
- C++ ↔ JS: `CallJS` (C++→JS), `RegisterJSListener` (JS→C++)

## Architecture Decisions

- `IsQuestObject()` 사전 검사 제거됨 (v1.0.5) — 대형 퀘스트 모드 호환성. `RemoveItem()` + 수량 비교가 실제 안전장치.
- 페이지네이션: 전체 eligible 수집 → 정렬 → 슬라이스 (v1.0.4). `total` 필드는 항상 정확한 값.
- 스레드 관리: detached thread 대신 저장 + shutdown 시 join (v1.0.4).
- co-save 직렬화: 레코드별 람다로 격리 — 한 레코드 실패가 나머지에 영향 없음 (v1.0.4).

## Repo Hygiene

- `MCM/`, `Scripts/`, `SKSE/Plugins/SVCollection/` 레거시 자산은 제거됨. 다시 추가하지 말 것.
- repo-wide 포매팅/무관한 리팩터링 금지.
