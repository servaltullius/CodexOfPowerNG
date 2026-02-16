<INSTRUCTIONS>

# Codex of Power NG (CodexOfPowerNG)

## Agent First Action
- Context entrypoint: `.vibe/context/LATEST_CONTEXT.md` (or `.vibe/AGENT_CHECKLIST.md`)
- First command: `python3 scripts/vibe.py doctor --full`

## Project Intent (TL;DR)
- This repo is for a **new mod**: **Codex of Power NG** (not a continuation of Codex of Power / SVCollection).
- **No save migration / no backwards compatibility**. Ship with a clear “not compatible” notice.
- **UI**: SkyUI/MCM/UIExtensions 제거 → **Prisma UI(Web UI)** 기반으로 교체.
- **Runtime logic**: Papyrus/JContainers 의존 제거 → **SKSE DLL(C++)**로 이관.
- **Targets**: Skyrim SE 1.5.97 + Skyrim AE 1.6+ **single DLL** (CommonLibSSE-NG `flatrim` 권장).

## Identifiers / Paths
- Internal ID: `CodexOfPowerNG` (공백 없이 DLL/뷰/설정 키 통일)
- Prisma UI view root: `PrismaUI/views/codexofpowerng/index.html`
- SKSE plugin output: `SKSE/Plugins/CodexOfPowerNG.dll`
- MO2-ready release zip: `releases/Codex of Power NG.zip`

## Runtime Requirements (End-user)
- Skyrim Special Edition / Anniversary Edition + SKSE
- Prisma UI (PrismaUI.dll)
- Address Library for SKSE Plugins (PrismaUI 설치 요구사항)
- Media Keys Fix SKSE (PrismaUI에서 키보드 입력을 위해 요구됨)
- Logs:
  - PrismaUI log: `Documents/My Games/Skyrim Special Edition/SKSE/PrismaUI.log`
  - Our log: `Documents/My Games/Skyrim Special Edition/SKSE/CodexOfPowerNG.log`

## Build (Windows)
- Dependency manager: **vcpkg manifest mode**
- Requires `VCPKG_ROOT` to be set (or pass `CMAKE_TOOLCHAIN_FILE` manually).
- Recommended triplet: `x64-windows-static-md`

## Build (WSL → Windows cross, verified)
- Uses `clang-cl` + `lld-link` + Windows SDK/MSVC headers from `/mnt/c/...`
- Toolchain: `cmake/toolchains/wsl-clangcl-vcpkg.cmake` (expects VS2022 Community + Windows 10 SDK installed in default locations)
- vcpkg overlay port: `cmake/vcpkg-ports/commonlibsse-ng-flatrim` (WSL cross build fixes; do not remove unless upstreamed)

Commands (repo root):
- `export VCPKG_ROOT=/path/to/vcpkg` (example in this workspace: `export VCPKG_ROOT="/mnt/c/Users/kdw73/vcpkg"`)
- `cmake --preset wsl-release`
- `cmake --build --preset wsl-release`
- `cmake --install build/wsl-release`
- If you see vcpkg root mismatch errors (e.g. references to a missing `/tmp/vcpkg`), remove `build/wsl-release/` and re-run the preset with the correct `VCPKG_ROOT`.

Verified (this workspace):
- `cmake --preset wsl-release` ✅ (requires `VCPKG_ROOT` set)
- `cmake --build --preset wsl-release` ✅
- `cmake --install build/wsl-release` ✅

Expected outputs:
- `dist/CodexOfPowerNG/SKSE/Plugins/CodexOfPowerNG.dll`
- `dist/CodexOfPowerNG/SKSE/Plugins/CodexOfPowerNG/settings.json`
- `dist/CodexOfPowerNG/SKSE/Plugins/CodexOfPowerNG/lang/en.json`
- `dist/CodexOfPowerNG/SKSE/Plugins/CodexOfPowerNG/lang/ko.json`
- `dist/CodexOfPowerNG/PrismaUI/views/codexofpowerng/index.html`

## Release zip (WSL, verified)
- Create/update the MO2-ready archive at `releases/Codex of Power NG.zip`:
  - `cd dist/CodexOfPowerNG`
  - `zip -r -FS \"../../releases/Codex of Power NG.zip\" .`
  - Verified: zip updated ✅

## Release Patch Note Policy
- Release/patch note markdown must follow: `docs/releases/PATCH_NOTE_RULES.ko.md`
- Default language for release notes is Korean.
- Release note filenames must match version tags:
  - `docs/releases/vX.Y.Z.md`
  - `docs/releases/vX.Y.Z-rc.N.md`
- When publishing/updating a GitHub release, sync notes from the matching markdown file in `docs/releases/`.

## Repo Notes
- This workspace is git-initialized. Plan/checkpoints are kept in `docs/plans/`.
- vcpkg manifest files:
  - `vcpkg.json`
  - `vcpkg-configuration.json`

## Runtime Notes / Troubleshooting
- **No ESP/ESM**: NG는 **SKSE DLL + Prisma UI view**만으로 동작합니다(새 Form 추가 없음). 따라서 플러그인(`.esp`)이 없어도 정상입니다.
  - **Main menu → Load에서 프리징/무한 로딩**:
  - `TESContainerChangedEvent`는 **로드 중 인벤토리 복구 이벤트 폭주**가 발생할 수 있어, NG는 다음 가드를 둡니다:
    - `settings.enableLootNotify == true`일 때만 처리(기본 `true`)
    - `g_gameReady == true` + `RE::Main::gameActive == true`일 때만 처리
    - 로드/뉴게임 직후 **5초 그레이스 기간**(`kDebounceMs=5000`)으로 초기 폭주 구간 무시 (`Events::OnGameLoaded()`)
    - 알림 스팸 방지: `kNotifyThrottleMs=750`로 디버그 알림 throttle + `state.notifiedItems` 중복 방지
  - Prisma UI view는 **핫키(기본 F4)로 처음 열 때 지연 생성(lazy create)** 합니다(로드 직후 자동 생성하지 않음).
  - **핫키는 메인메뉴/로딩 화면에서는 동작하지 않도록 차단**합니다(PrismaUI focus/input 충돌로 프리징이 날 수 있음).
  - Prisma UI Focus 관련 프리징이 의심되면 `settings.json`의 `ui.disableFocusMenu`를 사용합니다:
    - 기본값: `false` (FocusMenu 오버레이 활성화, 커서/입력 캡처 정상)
    - 필요 시: `true` (FocusMenu 오버레이 비활성화, “오버레이만 띄우기” 용도)
    - `ui.pauseGame`는 UI focus 중 게임 일시정지 여부(기본 `true`)
    - **클릭 영역/커서가 어긋나는 고DPI(Windows 배율 175~200%) 문제**가 있으면 `ui.inputScale`로 입력 좌표를 보정합니다(기본 `1.0`).
      - 예: Windows 175% → `1.75`, 200% → `2.0`
      - 보정은 단순 곱/나눗셈뿐 아니라, UI 패널(`.root`) 기준으로도 스케일 후보를 만들어 **가장 “그럴듯한” 클릭 타겟**으로 리다이렉트합니다(잘못된 스케일을 넣어도 “아무것도 클릭 안 됨”을 최대한 피함).
      - UI에서 `[`/`]` 키로 0.05씩 조정 가능(마우스 클릭이 어려운 상황에서 유용)
      - UI에서 `0` 키로 `1.0`으로 리셋 가능(잘못 설정해서 클릭이 더 안 맞을 때 빠른 복구)
      - `Ctrl+S`로 설정 저장(또는 Settings 탭의 Save)
      - **버튼 텍스트를 눌렀을 때만 클릭이 안 먹는 경우**: Ultralight/PrismaUI에서 이벤트 `target`이 Element가 아닌(Text node) 경우가 있어, NG는 테이블 내 버튼 처리(event delegation)를 `parentNode` 기반으로 탐색하도록 방어합니다.
    - **UI를 닫은 뒤에도 프레임드랍/끊김이 남는 경우**: `ui.destroyOnClose`를 `true`로 두면(기본값) UI를 닫을 때 PrismaView를 **Destroy** 해서 백그라운드 자원/스크립트가 남지 않게 합니다.
      - UI 상태(페이지/필터/포커스 등)를 유지하고 싶으면 `false`로 두고 Hide/Show 방식으로 사용합니다.
  - UI가 보이는데 커서가 없으면: PrismaUI의 `Show()` 적용이 비동기라 `Focus()` 타이밍이 빨라질 수 있어, NG는 **약 50ms 지연 후 Focus**를 시도합니다.
    - `Focus() failed (attempt N)`가 반복되면 `Data/PrismaUI/views/codexofpowerng/index.html` 경로/설치 상태와 PrismaUI 설치를 우선 점검합니다.
  - Focus 직후 “프리징처럼 멈춰보임”이 발생하면: NG는 Focus 직후에 바로 상태/JS interop 호출을 하지 않고 **한 틱 지연**시켜 충돌 가능성을 줄입니다.
  - 인게임 UI 조작 관련 스톨/끊김이 보이면:
    - 게임 오브젝트 접근(인벤토리 조회/RemoveItem/ActorValue 수정)은 **main thread(AddTask)** 에서
    - PrismaUI API/JS interop(Show/Focus/Hide/InteropCall)은 **UI thread(AddUITask)** 에서 처리하도록 분리합니다.
    - 인벤토리 목록은 **페이지네이션(기본 200개/페이지)** 으로 전송/렌더링합니다(대량 테이블 DOM 생성으로 인한 “10초 멈춤” 방지).
      - UI: Quick Register 탭의 페이지 크기(`100/200/300/400`) 선택 + `Prev/Next/Refresh` 버튼으로 페이지 이동
        - 행 클릭으로 선택 → `Enter`로 등록(또는 Register 버튼 클릭)하여 “클릭 타겟이 작다”는 체감을 완화합니다.
      - C++: `player->GetInventory()`(비싼 맵 복사) 대신 `InventoryChanges->entryList` 순회로 목록을 구성합니다(루프에서 `GetItemCount()` 대신 `entry->countDelta` 사용).
        - 전체 스캔으로 정확한 `total`을 계산하지 않고, **요청 페이지(offset 이후 pageSize)** 만 구성 + `hasMore` 플래그로 다음 페이지 존재 여부를 판단합니다(메인 스레드 스톨 완화). 따라서 `total`은 unknown(0)일 수 있습니다.
      - 요청 파라미터 `pageSize`는 1..500 범위로 클램프합니다.
    - UI가 열려있는 동안 커서/호버가 끊기는 느낌이 있으면:
      - Quick Register / Registered 테이블은 **가상 스크롤(virtualization)** 로 렌더링합니다(보이는 행만 DOM에 유지).
      - 구현: `<tbody>`에 top/bottom spacer row(`tr.spacerRow`) + visible slice row(`tr.dataRow`)만 넣습니다.
      - 전제: 행 높이를 고정합니다(Quick: `78px * --uiScale`, Registered: `54px * --uiScale`) → 가상 스크롤 계산에 사용.
  - UI가 열린 뒤 게임 입력이 “먹통처럼” 느껴질 수 있습니다(커서/입력 포커스가 UI로 이동한 상태).
    - `ESC` 또는 `F4`를 누르면 UI가 닫히도록(뷰 포커스 상태에서도) JS 쪽에서 안전장치를 넣어둡니다.
  - UI 언어:
    - 기본 `auto`: 게임 INI의 `sLanguage:General`을 감지해 `en/ko` 중 선택합니다.
    - Settings의 `Language override`에서 `en/ko`로 강제할 수 있으며, Prisma UI 뷰도 `state.language` 기반으로 한/영 UI 텍스트가 바뀝니다.
    - Settings 저장(`settings.json` write) 및 로컬라이제이션 reload는 **백그라운드 작업으로 큐잉**하여 프레임 스톨을 줄입니다(즉시 UI에 반영 → 저장 완료 후 토스트/상태 갱신).
  - 4K/고해상도에서 UI가 너무 작게 보이면:
    - `PrismaUI/views/codexofpowerng/index.html`에서 **해상도 기반 자동 스케일(`--uiScale`)** 을 적용합니다(1080p 기준, 4K는 대략 ~2.0x).
      - Ultralight에서 `devicePixelRatio`가 1로 고정되는 경우가 있어, 아래 힌트로 자동 스케일을 보정합니다:
        - `screen.width/height` ÷ `innerWidth/innerHeight`로 **Windows 배율(screenScale)** 을 추정(가능한 경우)
        - `ui.inputScale`(Windows 배율)을 **대체 DPR 힌트**로 사용(고DPI 클릭 보정 설정과 공유)
    - UI 패널은 `width: min(98vw, 3600px)`로 크게 사용합니다(“메뉴만 보고 싶다”는 전제의 앱형 레이아웃).
    - Settings 탭의 `UI Size`에서 `Auto/Manual` 모드 + 슬라이더(1.0~3.0)를 제공하며, UI 전용으로 **localStorage**에 저장됩니다.
      - Keys: `copng.ui.scaleMode`, `copng.ui.manualScale`
    - 커서 이동/호버가 끊겨 보이면: Settings의 **Performance mode**를 사용합니다(로컬 저장).
      - Key: `copng.ui.perfMode` (`auto|off|on`)
      - `on`은 그림자/호버 효과를 줄여 **커서 부드러움**을 우선합니다.

## Repo Hygiene
- `MCM/`, `Scripts/`, `SKSE/Plugins/SVCollection/` 레거시 자산은 NG 레포/런타임에서 제거되었습니다. 다시 추가하지 마세요.

<!-- skills-scout:start -->
## Skills (Auto-Pinned by skills-scout)

This section is generated. Re-run pinning to update.

### Available skills
- (none matched this repo)
<!-- skills-scout:end -->

</INSTRUCTIONS>
