# Codex of Power NG (CodexOfPowerNG)

> New mod line. Not compatible with Codex of Power / SVCollection.
>  
> 신규 모드 라인입니다. Codex of Power / SVCollection과 호환되지 않습니다.

## Quick Links
- Latest release: `https://github.com/servaltullius/CodexOfPowerNG/releases/latest`
- Changelog: `CHANGELOG.md`
- Release package path in this repo: `releases/Codex of Power NG.zip`

## 한국어 안내 (KR)

### 1) 모드 소개
Codex of Power NG는 기존 SVCollection 계열을 이어가는 패치가 아니라, 런타임/구조/UI를 새로 설계한 별도 모드입니다.
- ESP/ESM 없이 동작 (`SKSE/Plugins/CodexOfPowerNG.dll`)
- SkyUI/MCM/UIExtensions 대신 Prisma UI(Web UI) 사용
- 기본 핫키 `F4`로 메뉴 토글

### 2) 필수 요구사항
- Skyrim SE/AE + SKSE
- Prisma UI (`PrismaUI.dll`)
- Address Library for SKSE Plugins
- Media Keys Fix SKSE

### 3) 설치 방법 (MO2 기준)
1. `releases/Codex of Power NG.zip`를 MO2에서 모드로 설치합니다.
2. 아래 파일이 설치되었는지 확인합니다.
   - `SKSE/Plugins/CodexOfPowerNG.dll`
   - `PrismaUI/views/codexofpowerng/index.html`
3. 선택 항목으로 아래 설정 파일을 조정할 수 있습니다.
   - `SKSE/Plugins/CodexOfPowerNG/settings.json`

### 4) 구버전 사용자 필수 정리 절차
NG는 구버전 런타임 상태와 호환되지 않습니다. 아래 잔재를 먼저 정리하세요.
- `MCM/Settings/SVCollection.ini` 삭제
- `MCM/Settings/keybinds.json`에서 `modName: "SVCollection"` 항목 제거
- MO2 사용 시 보통 `overwrite` 경로를 우선 점검

NG는 잔재가 남아 있으면 런타임에서 경고 로그/알림을 표시합니다.

### 5) 기본 사용법
- `F4`: 메뉴 열기/닫기
- Quick Register:
  - 목록에서 아이템 선택 후 `Register` 클릭 시 해당 아이템 1개 소비 + 등록
  - 행 선택 후 `Enter`로 등록 가능
- Settings:
  - 언어: `auto / en / ko`
  - UI 크기: Auto/Manual 스케일
  - 고DPI 클릭 보정: `Input scale` (예: 175% -> 1.75, 200% -> 2.0)
  - 커서/호버 끊김 완화: `Performance mode` -> `On (Smooth cursor)`

### 6) 자주 묻는 문제
- 클릭 위치가 어긋남: `Input scale`을 Windows 배율과 맞춥니다.
- UI 열림 상태에서 커서/호버가 무거움: `Performance mode`를 `On`으로 설정합니다.
- UI를 닫았는데도 프레임 드랍이 남음: `Destroy view on close`를 `ON` 권장(기본값).
- UI 포커스로 게임 입력이 묶인 느낌: `ESC` 또는 `F4`로 닫아 복구합니다.

### 7) 로그 위치
- Prisma UI: `Documents/My Games/Skyrim Special Edition/SKSE/PrismaUI.log`
- Codex NG: `Documents/My Games/Skyrim Special Edition/SKSE/CodexOfPowerNG.log`

## English Guide (EN)

### 1) What this mod is
Codex of Power NG is not a continuation patch for SVCollection. It is a separately rebuilt mod line with a new runtime/UI architecture.
- No ESP/ESM required (`SKSE/Plugins/CodexOfPowerNG.dll`)
- Prisma UI (Web UI) replaces SkyUI/MCM/UIExtensions
- Default hotkey is `F4`

### 2) Requirements
- Skyrim SE/AE + SKSE
- Prisma UI (`PrismaUI.dll`)
- Address Library for SKSE Plugins
- Media Keys Fix SKSE

### 3) Installation (MO2)
1. Install `releases/Codex of Power NG.zip` as a mod in MO2.
2. Verify the following files exist:
   - `SKSE/Plugins/CodexOfPowerNG.dll`
   - `PrismaUI/views/codexofpowerng/index.html`
3. Optional runtime settings file:
   - `SKSE/Plugins/CodexOfPowerNG/settings.json`

### 4) Mandatory cleanup for legacy users
NG is not compatible with old Codex/SVCollection runtime residue.
- Delete `MCM/Settings/SVCollection.ini`
- Remove entries with `modName: "SVCollection"` from `MCM/Settings/keybinds.json`
- If you use MO2, check the `overwrite` path first

If residue is detected, NG logs and shows an in-game warning.

### 5) Basic usage
- `F4`: Toggle menu
- Quick Register:
  - Select an item and click `Register` to consume 1 item and register it
  - Row selection + `Enter` is also supported
- Settings:
  - Language: `auto / en / ko`
  - UI size: Auto/Manual scale
  - High-DPI hitbox correction: `Input scale` (ex: 175% -> 1.75, 200% -> 2.0)
  - Cursor smoothness: `Performance mode` -> `On (Smooth cursor)`

### 6) Common troubleshooting
- Click mismatch on high DPI: set `Input scale` to your Windows scaling factor.
- Cursor/hover feels choppy while UI is open: set `Performance mode` to `On`.
- FPS drop remains after closing UI: keep `Destroy view on close` enabled.
- Input feels stuck in UI focus mode: close with `ESC` or `F4`.

### 7) Log locations
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
