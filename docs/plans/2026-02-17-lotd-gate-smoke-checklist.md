# LOTD 게이트 스모크 테스트 체크리스트 (v1.0.6-rc 계열)

## 목적
- `registration.requireTccDisplayed` 동작이 의도대로 동작하는지 빠르게 검증한다.
- TCC(FormList: `dbmMaster`, `dbmDisp`) 유무에 따라 UI 경고/등록 차단이 올바른지 확인한다.

## 테스트 전 준비
1. 최신 빌드 산출물을 설치한다.
2. 세이브를 로드하고 플레이어 인벤토리에 등록 가능한 아이템을 2~3개 준비한다.
3. F4로 Codex UI를 열고 `Settings > Safety`에서 `LOTD 물품은 Displayed(...)일 때만 등록` 체크를 조정할 수 있게 둔다.

## 시나리오 A: 게이트 비활성화 (`requireTccDisplayed=false`)
1. `Settings`에서 LOTD 게이트 옵션을 끈다.
2. `Quick Register`에서 아이템 등록을 시도한다.
3. 기대 결과:
   - 등록이 정상 진행된다.
   - 상단 상태 바의 LOTD 게이트 pill은 숨김 상태다.
   - Settings 상단 경고 배너가 보이지 않는다.

## 시나리오 B: 게이트 활성화 + TCC 리스트 사용 가능
1. `requireTccDisplayed=true`로 켠다.
2. `Refresh` 후 상태를 확인한다.
3. 기대 결과:
   - 상단 LOTD 게이트 pill이 표시되고 `TCC ready`(또는 한글 번역) 상태다.
   - Settings 경고 배너는 숨김 상태다.
4. 추가 확인:
   - LOTD 추적 + 미전시 아이템 등록 시 등록이 차단되고 미전시 안내 메시지가 노출된다.
   - LOTD 추적 + 전시 완료 아이템 등록은 정상 진행된다.

## 시나리오 C: 게이트 활성화 + TCC 리스트 사용 불가(미설치/로드순서 문제)
1. `requireTccDisplayed=true` 상태에서 TCC 리스트를 찾지 못하는 환경으로 실행한다.
2. `Refresh` 후 상태를 확인한다.
3. 기대 결과:
   - 상단 LOTD 게이트 pill이 경고 상태(`TCC missing`)로 표시된다.
   - 경고 토스트가 1회 표시된다.
   - `Settings` 상단 경고 배너가 표시된다.
   - 등록 시 `LOTD gate unavailable` 계열 메시지로 차단된다.
   - SKSE 로그에 경고가 1회 기록된다.

## 회귀 체크
1. 언어 `en/ko` 전환 시 상태 pill/배너/토스트 문구가 모두 번역된다.
2. UI를 닫았다 다시 열어도 경고 상태 표시가 현재 게이트 상태와 일치한다.
3. `Quick Register` 목록 갱신 후에도 차단 상태에서 등록 가능한 항목이 노출되지 않는다.
