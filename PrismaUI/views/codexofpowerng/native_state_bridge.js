(function (global) {
  "use strict";

  function noop() {}

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function applyDocumentLanguage(documentObj, uiLang) {
    if (!documentObj || !documentObj.documentElement) return;
    documentObj.documentElement.lang = uiLang === "ko" ? "ko" : "en";
  }

  const DEFAULT_BUILD_SLOT_LAYOUT = Object.freeze([
    Object.freeze({ slotId: "attack_1", slotKind: "attack", optionId: null, occupied: false }),
    Object.freeze({ slotId: "attack_2", slotKind: "attack", optionId: null, occupied: false }),
    Object.freeze({ slotId: "defense_1", slotKind: "defense", optionId: null, occupied: false }),
    Object.freeze({ slotId: "utility_1", slotKind: "utility", optionId: null, occupied: false }),
    Object.freeze({ slotId: "utility_2", slotKind: "utility", optionId: null, occupied: false }),
    Object.freeze({ slotId: "wildcard_1", slotKind: "wildcard", optionId: null, occupied: false }),
  ]);

  function normalizeRewardsPayload(nextRewards) {
    return nextRewards && typeof nextRewards === "object" ? nextRewards : { totals: [] };
  }

  function normalizeBuildPayload(nextBuild) {
    const source = nextBuild && typeof nextBuild === "object" ? nextBuild : {};
    const disciplines = source.disciplines && typeof source.disciplines === "object" ? source.disciplines : {};
    const bySlotId = new Map();
    if (Array.isArray(source.activeSlots)) {
      for (const slot of source.activeSlots) {
        if (!slot || typeof slot !== "object" || typeof slot.slotId !== "string") continue;
        bySlotId.set(slot.slotId, slot);
      }
    }

    return {
      disciplines: {
        attack: {
          score: Number((disciplines.attack && disciplines.attack.score) || 0) >>> 0,
          unlockedBaselineCount: Number(
            (disciplines.attack && disciplines.attack.unlockedBaselineCount) || 0,
          ) >>> 0,
        },
        defense: {
          score: Number((disciplines.defense && disciplines.defense.score) || 0) >>> 0,
          unlockedBaselineCount: Number(
            (disciplines.defense && disciplines.defense.unlockedBaselineCount) || 0,
          ) >>> 0,
        },
        utility: {
          score: Number((disciplines.utility && disciplines.utility.score) || 0) >>> 0,
          unlockedBaselineCount: Number(
            (disciplines.utility && disciplines.utility.unlockedBaselineCount) || 0,
          ) >>> 0,
        },
      },
      options: Array.isArray(source.options) ? source.options : [],
      activeSlots: DEFAULT_BUILD_SLOT_LAYOUT.map((slot) => Object.assign({}, slot, bySlotId.get(slot.slotId) || {})),
      migrationNotice:
        source.migrationNotice && typeof source.migrationNotice === "object"
          ? {
              needsNotice: !!source.migrationNotice.needsNotice,
              legacyRewardsMigrated: !!source.migrationNotice.legacyRewardsMigrated,
              unresolvedHistoricalRegistrations:
                Number(source.migrationNotice.unresolvedHistoricalRegistrations || 0) >>> 0,
            }
          : {
              needsNotice: false,
              legacyRewardsMigrated: false,
              unresolvedHistoricalRegistrations: 0,
            },
    };
  }

  function normalizeUndoItems(nextUndoItems) {
    return Array.isArray(nextUndoItems) ? nextUndoItems : [];
  }

  function createNativeStateBridge(opts) {
    const options = opts || {};
    const documentObj = options.documentObj || (global && global.document) || null;
    const getUiLang = asFn(options.getUiLang, () => "en");
    const setUiLang = asFn(options.setUiLang, noop);
    const setStateValue = asFn(options.setStateValue, noop);
    const setInventoryPage = asFn(options.setInventoryPage, noop);
    const setRegisteredItems = asFn(options.setRegisteredItems, noop);
    const setBuildValue = asFn(options.setBuildValue, noop);
    const setRewardsValue = asFn(options.setRewardsValue, noop);
    const setUndoItems = asFn(options.setUndoItems, noop);
    const setSettingsValue = asFn(options.setSettingsValue, noop);
    const applyI18n = asFn(options.applyI18n, noop);
    const renderStatus = asFn(options.renderStatus, noop);
    const renderQuick = asFn(options.renderQuick, noop);
    const renderRegistered = asFn(options.renderRegistered, noop);
    const renderBuild = asFn(options.renderBuild, noop);
    const renderUndo = asFn(options.renderUndo, noop);
    const renderRewards = asFn(options.renderRewards, noop);
    const renderSettings = asFn(options.renderSettings, noop);
    const showToast = asFn(options.showToast, noop);
    const resetQuickVirtualWindow = asFn(options.resetQuickVirtualWindow, noop);
    const resetRegisteredVirtualWindow = asFn(options.resetRegisteredVirtualWindow, noop);
    const schedulePostRefreshVirtualResync = asFn(options.schedulePostRefreshVirtualResync, noop);
    const isTabActive = asFn(options.isTabActive, () => true);

    function syncLanguage(nextLang) {
      const resolved = nextLang === "ko" ? "ko" : "en";
      setUiLang(resolved);
      applyDocumentLanguage(documentObj, resolved);
      return resolved;
    }

    return Object.freeze({
      onState(nextState) {
        const resolvedState = nextState || {};
        setStateValue(resolvedState);
        syncLanguage(typeof resolvedState.language === "string" ? resolvedState.language : getUiLang());
        applyI18n();
        renderStatus();
      },

      onInventory(nextInventoryPage) {
        setInventoryPage(nextInventoryPage);
        resetQuickVirtualWindow();
        if (isTabActive("tabQuick")) {
          renderQuick();
          schedulePostRefreshVirtualResync();
        }
      },

      onRegistered(nextRegistered) {
        setRegisteredItems(nextRegistered);
        resetRegisteredVirtualWindow();
        if (isTabActive("tabRegistered")) {
          renderRegistered();
          schedulePostRefreshVirtualResync();
        }
      },

      onBuild(nextBuild) {
        setBuildValue(normalizeBuildPayload(nextBuild));
        if (isTabActive("tabBuild")) renderBuild();
      },

      onRewards(nextRewards) {
        setRewardsValue(normalizeRewardsPayload(nextRewards));
        if (isTabActive("tabRewards")) renderRewards();
      },

      onUndoList(nextUndoItems) {
        setUndoItems(normalizeUndoItems(nextUndoItems));
        if (isTabActive("tabUndo")) renderUndo();
      },

      onSettings(nextSettings) {
        setSettingsValue(nextSettings);
        renderSettings();

        const overrideValue =
          nextSettings && typeof nextSettings.languageOverride === "string" ? nextSettings.languageOverride : "";
        if (overrideValue === "en" || overrideValue === "ko") {
          syncLanguage(overrideValue);
          applyI18n();
        }
      },

      onToast(toastPayload) {
        const payload = toastPayload || {};
        showToast(payload.level || "info", payload.message || "");
      },
    });
  }

  const api = Object.freeze({
    applyDocumentLanguage,
    createNativeStateBridge,
  });

  if (typeof module !== "undefined" && module && module.exports) {
    module.exports = api;
  }

  global.COPNGNativeStateBridge = api;
})(typeof window !== "undefined" ? window : globalThis);
