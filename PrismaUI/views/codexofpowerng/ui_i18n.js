(function (root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) {
    module.exports = api;
  }
  if (root) {
    root.COPNGUII18n = api;
  }
})(typeof globalThis !== "undefined" ? globalThis : this, function () {
  "use strict";

  const MESSAGES = Object.freeze({
    en: Object.freeze({
      "app.title": "Codex of Power NG",
      "app.desc": "Quick Register consumes 1 item and permanently records it. Use Settings to configure safety and rewards.",
      "btn.refresh": "Refresh",
      "btn.refreshList": "Refresh List",
      "btn.close": "Close",
      "btn.prev": "Prev",
      "btn.next": "Next",
      "btn.register": "Register",
      "btn.undo": "Undo",
      "btn.refund": "Refund",
      "btn.reload": "Reload",
      "btn.save": "Save",
      "btn.bindKey": "Bind…",
      "tab.quick": "Quick Register",
      "tab.registered": "Registered",
      "tab.undo": "Undo",
      "tab.rewards": "Rewards",
      "tab.settings": "Settings",
      "placeholder.search": "Search...",
      "th.group": "Group",
      "th.item": "Item",
      "th.count": "Count",
      "th.reward": "Reward",
      "th.total": "Total",
      "rewards.rolls": "Rolls",
      "rewards.every": "Every",
      "rewards.mult": "Mult",
      "rewards.none": "(No rewards yet)",
      "rewards.imageMissing": "Character image missing: assets/character.png",
      "rewards.orbitHelp": "Top reward effects are shown around the character.",
      "rewards.more": "+{n} more",
      "undo.meta": "Recent registrations (latest first)",
      "undo.help": "Up to 10 recent registrations are shown. To keep rewards consistent, only the latest entry can be undone.",
      "undo.none": "(No recent registrations)",
      "undo.onlyLatest": "Latest only",
      "undo.rewarded": "rewarded",
      "undo.noReward": "no reward",
      "quick.help":
        "Only shows items safe to consume (not worn, active quest items excluded, and optionally not favorited). List is paginated to avoid stutter. Click a row to select, then press Enter to register.",
      "rewards.help": "Refund removes only recorded bonus stats. It does not restore consumed items and does not clear registrations.",
      "settings.toggleKeyHelp":
        "Toggle key. Click <span class=\"mono\">Bind…</span> then press a key, or type: <span class=\"mono\">F4</span> / <span class=\"mono\">E</span> / <span class=\"mono\">1</span> / <span class=\"mono\">0x3E</span>",
      "settings.resolved": "Resolved",
      "settings.invalid": "invalid",
      "settings.languageOverride": "Language override",
      "lang.auto": "auto (game)",
      "lang.en": "en (English)",
      "lang.ko": "ko (Korean)",
      "settings.uiTitle": "UI",
      "settings.pauseGame": "Pause game while menu is open",
      "settings.disableFocusMenu": "Disable FocusMenu overlay (advanced)",
      "settings.disableFocusMenuHelp":
        "If the menu shows but you cannot interact (no cursor), keep this OFF. Turning it ON may prevent cursor/input capture.",
      "settings.destroyOnClose": "Destroy view on close (performance)",
      "settings.destroyOnCloseHelp":
        "If FPS drops persist after closing the UI, keep this ON. Turning it OFF keeps UI state but may cost performance.",
      "settings.uiSizeTitle": "UI Size",
      "settings.uiSizeMode": "Mode",
      "settings.uiSizeAuto": "Auto (Recommended)",
      "settings.uiSizeManual": "Manual",
      "settings.uiSizeManualScale": "Manual scale",
      "settings.uiSizeTip": "Tip: 4K users often prefer <span class=\"mono\">2.0~2.6</span>. This setting affects only this UI.",
      "settings.perfTitle": "Performance",
      "settings.perfMode": "Performance mode",
      "settings.perfAuto": "Auto (Recommended)",
      "settings.perfOff": "Off",
      "settings.perfOn": "On (Smooth cursor)",
      "settings.perfHelp": "Reduces shadows and hover effects to improve cursor smoothness.",
      "settings.inputTitle": "Input",
      "settings.inputScaleHelp":
        "If clicks don't match the cursor (high DPI / Windows scaling), set Input scale to your scaling factor (e.g. 1.75 or 2.00). Press 0 to reset.",
      "settings.inputScale": "Input scale",
      "settings.inputScalePreset": "Preset",
      "settings.inputScaleTip":
        "Tip: Use <span class=\"mono\">[</span>/<span class=\"mono\">]</span> to adjust when mouse clicking is difficult. Press <span class=\"mono\">0</span> to reset.",
      "settings.safetyTitle": "Safety",
      "settings.protectFavorites": "Protect favorited/hotkey items",
      "settings.normalizeRegistration": "Normalize registration (template/variant map)",
      "settings.requireTccDisplayed": "LOTD items: require Displayed (Curator's Companion)",
      "settings.requireTccDisplayedHelp": "Applies to LOTD/TCC-tracked items. If TCC lists are unavailable, registration is blocked for safety.",
      "settings.lootNotify": "Loot notification",
      "settings.rewardsTitle": "Rewards",
      "settings.enableRewards": "Enable rewards",
      "settings.rewardEvery": "Every",
      "settings.rewardMultiplier": "Multiplier",
      "settings.allowSkillRewards": "Allow skill rewards",
      "inv.pagesize.100": "100 / page",
      "inv.pagesize.200": "200 / page",
      "inv.pagesize.300": "300 / page",
      "inv.pagesize.400": "400 / page",
      "toast.info": "info",
      "toast.error": "error",
      "toast.warn": "warning",
      "toast.bindKey": "Press a key to bind (Esc cancels).",
      "toast.bindKeyUnknown": "Unsupported key. Type a DIK code (e.g. 0x3E).",
      "toast.bindKeySet": "Hotkey set",
      "toast.lotdGateBlocked": "LOTD Display gate is enabled, but TCC lists are unavailable. Registration is blocked.",
      "confirm.refund": "Refund rewards? This cannot be undone.",
      "btn.cancel": "Cancel",
      "btn.ok": "OK",
      "status.ui": "UI",
      "status.loading": "loading",
      "status.ready": "ready",
      "status.hidden": "hidden",
      "status.shown": "shown",
      "status.focus": "focus",
      "status.noFocus": "no-focus",
      "status.registered": "Registered",
      "status.lang": "Lang",
      "status.hotkey": "Hotkey",
      "status.lotdGate": "LOTD Gate",
      "status.lotdOk": "TCC ready",
      "status.lotdBlocked": "TCC missing",
      "settings.lotdGateWarnTitle": "LOTD gate warning",
      "settings.lotdGateWarnBody":
        "LOTD Display gate is enabled, but TCC lists are unavailable. Registration is currently blocked for safety.",
      "inv.inventory": "Inventory",
      "inv.page": "page",
      "inv.showing": "showing",
      "inv.total": "total",
      "inv.none": "(No items)",
      "reg.none": "(No registered items)",
      "inv.unknown": "unknown",
    }),
    ko: Object.freeze({
      "app.title": "코덱스 오브 파워 NG",
      "app.desc": "빠른 등록은 아이템 1개를 소비하고 영구적으로 기록합니다. 설정에서 안전장치/보상을 조정하세요.",
      "btn.refresh": "새로고침",
      "btn.refreshList": "목록 새로고침",
      "btn.close": "닫기",
      "btn.prev": "이전",
      "btn.next": "다음",
      "btn.register": "등록",
      "btn.undo": "되돌리기",
      "btn.refund": "환불",
      "btn.reload": "다시 불러오기",
      "btn.save": "저장",
      "btn.bindKey": "키 지정…",
      "tab.quick": "빠른 등록",
      "tab.registered": "등록됨",
      "tab.undo": "되돌리기",
      "tab.rewards": "보상",
      "tab.settings": "설정",
      "placeholder.search": "검색...",
      "th.group": "분류",
      "th.item": "아이템",
      "th.count": "개수",
      "th.reward": "보상",
      "th.total": "합계",
      "rewards.rolls": "횟수",
      "rewards.every": "N개마다",
      "rewards.mult": "배수",
      "rewards.none": "(아직 받은 보상이 없습니다)",
      "rewards.imageMissing": "캐릭터 이미지 없음: assets/character.png",
      "rewards.orbitHelp": "상위 보상 효과를 캐릭터 주변에 표시합니다.",
      "rewards.more": "+{n}개 더",
      "undo.meta": "최근 등록 내역 (최신순)",
      "undo.help": "최근 등록 10개까지 표시됩니다. 보상 일관성을 위해 최신 항목만 되돌릴 수 있습니다.",
      "undo.none": "(최근 등록 내역이 없습니다)",
      "undo.onlyLatest": "최신만 가능",
      "undo.rewarded": "보상 있음",
      "undo.noReward": "보상 없음",
      "quick.help":
        "소비해도 안전한 아이템만 표시합니다(착용 중 제외, 진행 중인 퀘스트 아이템 제외, 즐겨찾기 보호는 설정). 렉 방지를 위해 페이지네이션됩니다. 행을 클릭해 선택한 뒤 Enter로 등록할 수 있습니다.",
      "rewards.help": "환불은 기록된 보너스 스탯만 제거합니다. 소비한 아이템은 복구되지 않으며, 등록 기록도 지워지지 않습니다.",
      "settings.toggleKeyHelp":
        "토글 키. <span class=\"mono\">키 지정…</span>을 누르고 원하는 키를 누르거나, 직접 입력: <span class=\"mono\">F4</span> / <span class=\"mono\">E</span> / <span class=\"mono\">1</span> / <span class=\"mono\">0x3E</span>",
      "settings.resolved": "해석",
      "settings.invalid": "유효하지 않음",
      "settings.languageOverride": "언어 강제",
      "lang.auto": "자동(게임)",
      "lang.en": "영어(en)",
      "lang.ko": "한국어(ko)",
      "settings.uiTitle": "UI",
      "settings.pauseGame": "메뉴가 열려있는 동안 게임 일시정지",
      "settings.disableFocusMenu": "FocusMenu 오버레이 비활성화(고급)",
      "settings.disableFocusMenuHelp":
        "메뉴는 보이는데 조작이 안 된다면(커서 없음) 이 옵션을 끄세요. 켜면 커서/입력 캡처가 안 될 수 있습니다.",
      "settings.destroyOnClose": "닫을 때 뷰 제거(성능)",
      "settings.destroyOnCloseHelp":
        "UI를 닫아도 프레임드랍이 남으면 이 옵션을 켜세요. 끄면 UI 상태를 유지하지만 성능에 영향이 있을 수 있습니다.",
      "settings.uiSizeTitle": "UI 크기",
      "settings.uiSizeMode": "모드",
      "settings.uiSizeAuto": "자동(권장)",
      "settings.uiSizeManual": "수동",
      "settings.uiSizeManualScale": "수동 배율",
      "settings.uiSizeTip": "팁: 4K 사용자는 보통 <span class=\"mono\">2.0~2.6</span>를 선호합니다. 이 설정은 이 UI에만 적용됩니다.",
      "settings.perfTitle": "성능",
      "settings.perfMode": "성능 모드",
      "settings.perfAuto": "자동(권장)",
      "settings.perfOff": "끄기",
      "settings.perfOn": "켜기(부드러운 커서)",
      "settings.perfHelp": "그림자/호버 효과를 줄여 커서 움직임을 부드럽게 합니다.",
      "settings.inputTitle": "입력",
      "settings.inputScaleHelp":
        "커서와 클릭 위치가 어긋나면(고DPI/윈도우 배율) 입력 배율을 윈도우 배율(예: 1.75 또는 2.00)로 설정하세요. 0을 누르면 초기화됩니다.",
      "settings.inputScale": "입력 배율",
      "settings.inputScalePreset": "프리셋",
      "settings.inputScaleTip":
        "팁: 마우스 클릭이 어렵다면 <span class=\"mono\">[</span>/<span class=\"mono\">]</span>로 배율을 조정할 수 있습니다. <span class=\"mono\">0</span>을 누르면 초기화됩니다.",
      "settings.safetyTitle": "안전",
      "settings.protectFavorites": "즐겨찾기/단축키 아이템 보호",
      "settings.normalizeRegistration": "등록 정규화(템플릿/변형 맵)",
      "settings.requireTccDisplayed": "LOTD 물품은 Displayed(큐레이터 컴패니언)일 때만 등록",
      "settings.requireTccDisplayedHelp": "LOTD/TCC 추적 대상에 적용되며, TCC 리스트를 찾지 못하면 안전을 위해 등록이 차단됩니다.",
      "settings.lootNotify": "루팅 알림",
      "settings.rewardsTitle": "보상",
      "settings.enableRewards": "보상 활성화",
      "settings.rewardEvery": "N개마다",
      "settings.rewardMultiplier": "배수",
      "settings.allowSkillRewards": "스킬 보상 허용",
      "inv.pagesize.100": "100 / 페이지",
      "inv.pagesize.200": "200 / 페이지",
      "inv.pagesize.300": "300 / 페이지",
      "inv.pagesize.400": "400 / 페이지",
      "toast.info": "알림",
      "toast.error": "오류",
      "toast.warn": "경고",
      "toast.bindKey": "키를 눌러 지정하세요 (Esc 취소).",
      "toast.bindKeyUnknown": "지원하지 않는 키입니다. DIK 코드를 입력하세요(예: 0x3E).",
      "toast.bindKeySet": "단축키 설정",
      "toast.lotdGateBlocked": "LOTD 전시 게이트가 켜져 있지만 TCC 리스트를 찾지 못했습니다. 등록이 차단됩니다.",
      "confirm.refund": "보상을 환불할까요? 되돌릴 수 없습니다.",
      "btn.cancel": "취소",
      "btn.ok": "확인",
      "status.ui": "UI",
      "status.loading": "로딩",
      "status.ready": "준비됨",
      "status.hidden": "숨김",
      "status.shown": "표시",
      "status.focus": "포커스",
      "status.noFocus": "포커스 없음",
      "status.registered": "등록됨",
      "status.lang": "언어",
      "status.hotkey": "단축키",
      "status.lotdGate": "LOTD 게이트",
      "status.lotdOk": "TCC 준비됨",
      "status.lotdBlocked": "TCC 없음",
      "settings.lotdGateWarnTitle": "LOTD 게이트 경고",
      "settings.lotdGateWarnBody":
        "LOTD 전시 게이트가 켜져 있지만 TCC 리스트를 찾지 못했습니다. 안전을 위해 현재 등록이 차단됩니다.",
      "inv.inventory": "인벤토리",
      "inv.page": "페이지",
      "inv.showing": "표시",
      "inv.total": "전체",
      "inv.none": "(아이템이 없습니다)",
      "reg.none": "(등록된 아이템이 없습니다)",
      "inv.unknown": "알 수 없음",
    }),
  });

  function normalizeLanguage(language) {
    return language === "ko" ? "ko" : "en";
  }

  function createTranslator(opts) {
    const options = opts || {};
    const getLanguage = typeof options.getLanguage === "function" ? options.getLanguage : () => "en";

    function t(key, fallback) {
      const language = normalizeLanguage(getLanguage());
      const table = MESSAGES[language] || MESSAGES.en;
      if (table && Object.prototype.hasOwnProperty.call(table, key)) {
        return table[key];
      }
      return fallback !== undefined ? fallback : key;
    }

    function tFmt(key, fallback, vars) {
      const base = String(t(key, fallback));
      if (!vars || typeof vars !== "object") return base;
      return base.replace(/\{([a-zA-Z0-9_]+)\}/g, (match, name) =>
        Object.prototype.hasOwnProperty.call(vars, name) ? String(vars[name]) : match,
      );
    }

    return Object.freeze({
      t,
      tFmt,
    });
  }

  return Object.freeze({
    MESSAGES,
    normalizeLanguage,
    createTranslator,
  });
});
