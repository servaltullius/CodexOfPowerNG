# Codex of Power NG (CodexOfPowerNG)

기존 **Codex of Power / SVCollection**과는 **완전히 분리된 새 모드**입니다.  
**세이브 마이그레이션/호환은 지원하지 않습니다.**

## 개요
- **ESP/ESM 없음**: `CodexOfPowerNG.dll` + Prisma UI view만으로 동작합니다.
- **UI**: SkyUI/MCM/UIExtensions 대신 **Prisma UI(Web UI)** 기반 메뉴 사용
- **핫키**: 기본 `F4`로 메뉴 토글

## 요구사항(필수)
- Skyrim SE/AE + SKSE
- Prisma UI (`PrismaUI.dll`)
- Address Library for SKSE Plugins
- Media Keys Fix SKSE (Prisma UI 입력 요구사항)

## 설치(MO2)
1. MO2에서 `releases/Codex of Power NG.zip`를 **모드로 설치**
2. 아래 구조가 설치되는지 확인:
   - `SKSE/Plugins/CodexOfPowerNG.dll`
   - `PrismaUI/views/codexofpowerng/index.html`
3. (선택) 설정 파일:
   - `SKSE/Plugins/CodexOfPowerNG/settings.json`

## 사용법
- `F4`: 메뉴 열기/닫기
- Quick Register:
  - 목록에서 아이템을 선택하고 **Register**를 누르면 **아이템 1개가 소비**되고, 등록 기록이 저장됩니다.
  - 행 클릭으로 선택 → `Enter`로 등록할 수 있습니다.
- Settings:
  - 언어: `auto/en/ko`
  - UI 크기: Auto/Manual 스케일(4K는 보통 2.0~2.6 선호)
  - 고DPI 클릭 보정: Input scale (Windows 배율 175% → 1.75, 200% → 2.0)
  - 커서/호버 끊김 체감 시: Performance mode를 `On (Smooth cursor)`로

## 문제 해결(자주)
- **커서가 있는데 클릭이 안 맞음/일부만 클릭됨(고DPI)**  
  Settings → Input scale을 Windows 배율로 맞춰보세요.
- **UI가 열린 상태에서 커서/호버가 끊김**  
  Settings → Performance mode를 `On`으로(그림자/호버 비용 감소).
- **UI를 닫았는데도 프레임드랍이 남음**  
  Settings → Destroy view on close를 `ON` 권장(기본값).
- **UI가 포커스를 먹어서 게임 조작이 이상함**  
  `ESC` 또는 `F4`로 닫을 수 있도록 UI 측에 안전장치가 있습니다.

## 개발/빌드(WSL → Windows cross)
> 자세한 내용은 `AGENTS.md` 참고.

```bash
export VCPKG_ROOT="/mnt/c/Users/kdw73/vcpkg"
cmake --preset wsl-release
cmake --build --preset wsl-release
cmake --install build/wsl-release
cd dist/CodexOfPowerNG
zip -r -FS "../../releases/Codex of Power NG.zip" .
```

