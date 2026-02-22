# Codex of Power NG (CodexOfPowerNG)

> New mod line. Not compatible with Codex of Power / SVCollection.
>  
> 신규 모드 라인입니다. Codex of Power / SVCollection과 호환되지 않습니다.

## Quick Links
- Latest release: `https://github.com/servaltullius/CodexOfPowerNG/releases/latest`
- Changelog: `CHANGELOG.md`
- Release package path in this repo: `releases/Codex of Power NG.zip`
- Release note rules (KR): `docs/releases/PATCH_NOTE_RULES.ko.md`

## 한국어 안내 (KR)

### 1) 이 모드가 실제로 하는 일 (상세)
Codex of Power NG는 아이템 수집을 "일회성 파밍"에서 "누적 기록형 진행"으로 바꾸는 SKSE 기반 시스템 모드입니다.

핵심 게임 루프:
- 인벤토리에서 아이템을 선택해 `Register`합니다.
- 등록 시 해당 아이템 1개를 소비하고, 등록 기록이 저장됩니다.
- 등록 기록을 기반으로 장기적인 수집/정리 플레이를 유도합니다.
- 옵션으로 보상(Rewards) 시스템을 켜면 일정 등록 수마다 추가 보너스를 받을 수 있습니다.

플레이 체감 포인트:
- 아이템을 "쌓아두기만" 하지 않고 실제로 소모해 진행 목표를 만듭니다.
- 등록/보상 규칙을 설정에서 조정해 취향에 맞는 난이도와 템포를 만들 수 있습니다.
- 고DPI 환경, 대량 인벤토리 환경에서도 조작 가능하도록 UI 성능/입력 보정을 제공합니다.

무엇을 하지 않는 모드인가:
- 퀘스트/지역/월드스페이스를 추가하지 않습니다.
- 새로운 ESP/ESM 폼을 요구하지 않습니다.
- 구버전(Codex/SVCollection) 세이브 상태를 마이그레이션하지 않습니다.

### 2) 모드 소개
Codex of Power NG는 기존 SVCollection 계열을 이어가는 패치가 아니라, 런타임/구조/UI를 새로 설계한 별도 모드입니다.
- ESP/ESM 없이 동작 (`SKSE/Plugins/CodexOfPowerNG.dll`)
- SkyUI/MCM/UIExtensions 대신 Prisma UI(Web UI) 사용
- 기본 핫키 `F4`로 메뉴 토글

### 3) 필수 요구사항
- Skyrim SE/AE + [SKSE](https://skse.silverlock.org/)
- [Prisma UI (`PrismaUI.dll`)](https://www.nexusmods.com/skyrimspecialedition/mods/148718)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
- [Media Keys Fix SKSE](https://www.nexusmods.com/skyrimspecialedition/mods/92948)

### 4) 설치 방법 (MO2 기준)
1. `releases/Codex of Power NG.zip`를 MO2에서 모드로 설치합니다.
2. 아래 파일이 설치되었는지 확인합니다.
   - `SKSE/Plugins/CodexOfPowerNG.dll`
   - `PrismaUI/views/codexofpowerng/index.html`
3. 선택 항목으로 아래 설정 파일을 조정할 수 있습니다.
   - 기본값 템플릿: `SKSE/Plugins/CodexOfPowerNG/settings.json`
   - 사용자 오버라이드(권장): `SKSE/Plugins/CodexOfPowerNG/settings.user.json`
   - 인게임 Settings 저장 시 `settings.user.json`에 기록되어 업데이트/재설치 시 덮어쓰기 영향을 줄입니다.
   - 안전을 위해 저장 시 이전 파일을 `settings.user.json.bak`로 보관합니다(복구용).
   - 크래시 등으로 `settings.user.json.tmp`가 남아있으면 다음 실행 시 정리/복구를 시도합니다.

### 5) 구버전 사용자 필수 정리 절차
NG는 구버전 런타임 상태와 호환되지 않습니다. 아래 잔재를 먼저 정리하세요.
- `MCM/Settings/SVCollection.ini` 삭제
- `MCM/Settings/keybinds.json`에서 `modName: "SVCollection"` 항목 제거
- MO2 사용 시 보통 `overwrite` 경로를 우선 점검

NG는 잔재가 남아 있으면 런타임에서 경고 로그/알림을 표시합니다.

### 6) 기본 사용법
- `F4`: 메뉴 열기/닫기
- Quick Register:
  - 목록에서 아이템 선택 후 `Register` 클릭 시 해당 아이템 1개 소비 + 등록
  - 행 선택 후 `Enter`로 등록 가능
- Settings:
  - 언어: `auto / en / ko`
  - UI 크기: Auto/Manual 스케일
  - 고DPI 클릭 보정: `Input scale` (예: 175% -> 1.75, 200% -> 2.0)
  - 커서/호버 끊김 완화: `Performance mode` -> `On (Smooth cursor)`

### 7) 자주 묻는 문제
- 클릭 위치가 어긋남: `Input scale`을 Windows 배율과 맞춥니다.
- UI 열림 상태에서 커서/호버가 무거움: `Performance mode`를 `On`으로 설정합니다.
- UI를 닫았는데도 프레임 드랍이 남음: `Destroy view on close`를 `ON` 권장(기본값).
- UI 포커스로 게임 입력이 묶인 느낌: `ESC` 또는 `F4`로 닫아 복구합니다.
- Undo 성공 메시지에 `[경고: 보상 롤백이 적용되지 않음]`이 붙음: 아이템 복구는 성공했지만 해당 액션의 보상 델타가 현재 actor 값에 적용되지 않아(이미 정규화/동기화된 경우) 추가 롤백할 값이 없다는 의미입니다.

### 8) 로그 위치
- Prisma UI: `Documents/My Games/Skyrim Special Edition/SKSE/PrismaUI.log`
- Codex NG: `Documents/My Games/Skyrim Special Edition/SKSE/CodexOfPowerNG.log`

## English Guide (EN)

### 1) What this mod actually does (detailed)
Codex of Power NG turns item collection into a long-term progression loop instead of one-time hoarding.

Core gameplay loop:
- Pick an inventory item and `Register` it.
- Registration consumes 1 copy of that item and stores a persistent record.
- Your records become the progression layer for collection-focused play.
- Optional Rewards can grant bonuses every N registrations (configurable).

Why this feels different:
- It creates a meaningful item sink and collection objective.
- You can tune pacing/safety with settings instead of hardcoded rules.
- It is designed to stay usable in high-DPI and large-inventory scenarios.

What this mod does not do:
- It does not add quests/worldspace content.
- It does not require new ESP/ESM forms.
- It does not migrate old Codex/SVCollection runtime state.

### 2) What this mod is
Codex of Power NG is not a continuation patch for SVCollection. It is a separately rebuilt mod line with a new runtime/UI architecture.
- No ESP/ESM required (`SKSE/Plugins/CodexOfPowerNG.dll`)
- Prisma UI (Web UI) replaces SkyUI/MCM/UIExtensions
- Default hotkey is `F4`

### 3) Requirements
- Skyrim SE/AE + [SKSE](https://skse.silverlock.org/)
- [Prisma UI (`PrismaUI.dll`)](https://www.nexusmods.com/skyrimspecialedition/mods/148718)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
- [Media Keys Fix SKSE](https://www.nexusmods.com/skyrimspecialedition/mods/92948)

### 4) Installation (MO2)
1. Install `releases/Codex of Power NG.zip` as a mod in MO2.
2. Verify the following files exist:
   - `SKSE/Plugins/CodexOfPowerNG.dll`
   - `PrismaUI/views/codexofpowerng/index.html`
3. Optional runtime settings file:
   - Default template: `SKSE/Plugins/CodexOfPowerNG/settings.json`
   - User override (recommended): `SKSE/Plugins/CodexOfPowerNG/settings.user.json`
   - In-game Settings save writes to `settings.user.json` so updates/reinstalls are less likely to wipe your preferences.
   - For safety, saves keep a previous copy as `settings.user.json.bak` (for recovery).
   - If `settings.user.json.tmp` is left behind (for example after a crash), the next launch will attempt cleanup/recovery.

### 5) Mandatory cleanup for legacy users
NG is not compatible with old Codex/SVCollection runtime residue.
- Delete `MCM/Settings/SVCollection.ini`
- Remove entries with `modName: "SVCollection"` from `MCM/Settings/keybinds.json`
- If you use MO2, check the `overwrite` path first

If residue is detected, NG logs and shows an in-game warning.

### 6) Basic usage
- `F4`: Toggle menu
- Quick Register:
  - Select an item and click `Register` to consume 1 item and register it
  - Row selection + `Enter` is also supported
- Settings:
  - Language: `auto / en / ko`
  - UI size: Auto/Manual scale
  - High-DPI hitbox correction: `Input scale` (ex: 175% -> 1.75, 200% -> 2.0)
  - Cursor smoothness: `Performance mode` -> `On (Smooth cursor)`

### 7) Common troubleshooting
- Click mismatch on high DPI: set `Input scale` to your Windows scaling factor.
- Cursor/hover feels choppy while UI is open: set `Performance mode` to `On`.
- FPS drop remains after closing UI: keep `Destroy view on close` enabled.
- Input feels stuck in UI focus mode: close with `ESC` or `F4`.
- Undo success message includes `[warning: reward rollback not applied]`: item restore succeeded, but there was no remaining actor-value delta to roll back for that action (for example, the value was already normalized/resynced).

### 8) Log locations
- Prisma UI: `Documents/My Games/Skyrim Special Edition/SKSE/PrismaUI.log`
- Codex NG: `Documents/My Games/Skyrim Special Edition/SKSE/CodexOfPowerNG.log`

## Build from Source (WSL -> Windows cross)
See `AGENTS.md` for full build notes.

```bash
export VCPKG_ROOT="/mnt/c/Users/kdw73/vcpkg"
cmake --preset wsl-release
cmake --build --preset wsl-release
cmake --install build/wsl-release
cd dist/CodexOfPowerNG
zip -r -FS "../../releases/Codex of Power NG.zip" .
```
