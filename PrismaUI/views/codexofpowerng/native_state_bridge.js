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

  function normalizeRewardsPayload(nextRewards) {
    return nextRewards && typeof nextRewards === "object" ? nextRewards : { totals: [] };
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
    const setRewardsValue = asFn(options.setRewardsValue, noop);
    const setUndoItems = asFn(options.setUndoItems, noop);
    const setSettingsValue = asFn(options.setSettingsValue, noop);
    const applyI18n = asFn(options.applyI18n, noop);
    const renderStatus = asFn(options.renderStatus, noop);
    const renderQuick = asFn(options.renderQuick, noop);
    const renderRegistered = asFn(options.renderRegistered, noop);
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
