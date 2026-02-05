# Codex of Power NG UI Input/Hitbox + Performance Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to implement this plan task-by-task.

**Goal:** Reduce UI “프리징/끊김” 체감(특히 Quick Register 인벤토리 로딩)과 고해상도/Windows 디스플레이 스케일(175–200%) 환경에서 클릭/조작성을 개선한다.

**Architecture:** Quick Register 인벤토리 요청은 “전체 스캔+정확한 total” 대신 “요청 페이지 분량만 빠르게 구성 + hasMore 플래그”로 전환해 main thread 스톨을 줄인다. UI는 클릭 타겟을 키우고(행 클릭/선택/키보드) 렌더링을 덜 자주/덜 크게 갱신하도록 한다.

**Tech Stack:** C++ (CommonLibSSE-NG), SKSE TaskInterface(AddTask/AddUITask), Prisma UI (Ultralight), nlohmann/json, HTML/CSS/JS.

---

### Task 1: Inventory page 응답을 “빠르게” 만들기 (total 제거/hasMore 추가)

**Files:**
- Modify: `include/CodexOfPowerNG/Registration.h`
- Modify: `src/Registration.cpp`
- Modify: `src/PrismaUIManager.cpp`
- Modify: `PrismaUI/views/codexofpowerng/index.html`

**Step 1: 설계 고정**
- `QuickRegisterList.total`은 **0이면 “unknown”** 으로 해석한다.
- `QuickRegisterList.hasMore`를 추가한다.
- `BuildQuickRegisterList(offset, limit)`는:
  - offset 이후 “eligible” 아이템을 최대 `limit`개 채우면 **즉시 종료**
  - `hasMore`는 “page를 채운 뒤, 다음 eligible 1개가 더 있는지”만 확인해서 세팅
  - `total`은 기본 0(unknown)으로 반환

**Step 2: 구현**
- `QuickRegisterList` 구조체에 `bool hasMore` 추가
- `BuildQuickRegisterList`에서 `totalEligible++` 전체 스캔 제거
- `PrismaUIManager::QueueSendInventory`에서 payload에 `hasMore` 포함, `total`은 0이면 생략(또는 0 유지)
- UI 쪽 page meta 계산을:
  - `total > 0`이면 기존처럼 `page/totalPages`
  - `total == 0`이면 `hasMore` + `items.length` 기반으로 Prev/Next enable/disable

**Step 3: 검증 (빌드)**
- Run: `cmake --build --preset wsl-release`
- Expected: build success

---

### Task 2: 클릭 타겟/조작성 개선(행 클릭/키보드/필터 디바운스)

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`

**Step 1: Quick Register 행 클릭**
- Register 버튼 외에도, 행 클릭으로 “선택” 가능하게 한다(즉시 소비/등록은 하지 않음).
- 선택된 행은 시각적으로 강조하고, `Enter` 키로 Register 실행.
- 행 더블클릭으로 바로 Register(옵션)도 고려하되, 기본은 안전하게 “선택+Enter/버튼”으로 둔다.

**Step 2: 필터 디바운스**
- `quickFilter` / `regFilter` 입력은 `requestAnimationFrame` 또는 `setTimeout(50~100ms)` 디바운스로 렌더링 호출 빈도를 낮춘다.

**Step 3: 검증 (정적)**
- 브라우저에서 열어도 문법 오류가 없는 수준으로 JS를 유지(문법/오타).

---

### Task 3: 기본 UI 동작(커서/카메라) 개선을 위한 기본 설정 조정

**Files:**
- Modify: `include/CodexOfPowerNG/Config.h`
- Modify: `SKSE/Plugins/CodexOfPowerNG/settings.json`

**Step 1: 기본 pauseGame**
- 사용자 전제(“메뉴 열면 메뉴만 봄”)에 맞춰 `ui.pauseGame` 기본값을 `true`로 변경한다.
- 기존 유저가 settings.json을 이미 가지고 있으면 그 값이 우선되므로, “신규 설치/기본값”을 위한 변경임을 문서화한다.

**Step 2: 검증 (빌드)**
- Run: `cmake --build --preset wsl-release`
- Expected: build success

---

### Task 4: 문서(AGENTS.md) 갱신 + 산출물 zip 업데이트

**Files:**
- Modify: `AGENTS.md`

**Step 1: 문서 반영**
- 인벤토리 페이지 응답이 `hasMore` 기반으로 동작하고 `total`은 optional/unknown일 수 있음을 반영
- 클릭/선택/Enter 조작법 추가
- 기본 `ui.pauseGame` 변경 반영

**Step 2: 설치/배포 산출**
- Run: `cmake --install build/wsl-release`
- Run: `cd dist/CodexOfPowerNG && zip -r -FS \"../../releases/Codex of Power NG.zip\" .`
- Expected: `releases/Codex of Power NG.zip` 갱신

