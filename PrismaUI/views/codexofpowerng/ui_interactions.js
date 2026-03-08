(function (root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) {
    module.exports = api;
  }
  if (root) {
    root.COPNGUIInteractions = api;
  }
})(typeof globalThis !== "undefined" ? globalThis : this, function () {
  "use strict";

  function noop() {}

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function createUIInteractions(opts) {
    const options = opts || {};
    const documentObj = options.documentObj || (typeof document !== "undefined" ? document : null);
    const rootScrollEl = options.rootScrollEl || null;
    const quickBody = options.quickBody || null;
    const undoBody = options.undoBody || null;
    const stateApi = options.stateApi;
    const safeCall = asFn(options.safeCall, noop);
    const t = asFn(options.t, (_key, fallback) => fallback);
    const KC = options.KC || null;
    const renderQuick = asFn(options.renderQuick, noop);
    const renderRegistered = asFn(options.renderRegistered, noop);
    const clamp = asFn(options.clamp, (value, lo, hi) => Math.max(lo, Math.min(hi, value)));
    const getOffsetTopInRoot = asFn(options.getOffsetTopInRoot, () => 0);
    const QUICK_ROW_BASE_PX = Number(options.quickRowBasePx || 78);

    let quickRenderTimer = null;
    let regRenderTimer = null;

    function requestInventoryPage(page) {
      const inventoryPage = stateApi.getInventoryPage() || {};
      const pageSize = stateApi.coalesce(inventoryPage.pageSize, 200) >>> 0;
      safeCall("copng_requestInventory", { page: page >>> 0, pageSize: pageSize > 0 ? pageSize : 200 });
    }

    function scheduleRenderQuick() {
      if (quickRenderTimer) clearTimeout(quickRenderTimer);
      quickRenderTimer = setTimeout(() => {
        quickRenderTimer = null;
        renderQuick();
      }, 70);
    }

    function scheduleRenderRegistered() {
      if (regRenderTimer) clearTimeout(regRenderTimer);
      regRenderTimer = setTimeout(() => {
        regRenderTimer = null;
        renderRegistered();
      }, 70);
    }

    function setQuickSelected(nextId) {
      const id = Number(nextId) >>> 0;
      if (!id) return;
      if (stateApi.getQuickSelectedId() === id) return;
      const prev = stateApi.getQuickSelectedId();
      stateApi.setQuickSelectedId(id);

      if (quickBody && prev) {
        const prevRow = quickBody.querySelector(`tr[data-row-id="${prev}"]`);
        if (prevRow) prevRow.classList.remove("selected");
      }
      if (quickBody) {
        const row = quickBody.querySelector(`tr[data-row-id="${id}"]`);
        if (row) row.classList.add("selected");
      }
    }

    function scrollQuickIndexIntoView(idx) {
      if (!rootScrollEl || !quickBody) return;
      if (Number.isFinite(idx)) idx = Number(idx);
      const quickVirtual = stateApi.getQuickVirtual();
      if (!Number.isFinite(quickVirtual.tbodyTopPx) || !Number.isFinite(quickVirtual.rowHeightPx) || quickVirtual.rowHeightPx <= 0) {
        const scale = stateApi.getCurrentUiScale();
        quickVirtual.rowHeightPx = QUICK_ROW_BASE_PX * scale;
        quickVirtual.tbodyTopPx = getOffsetTopInRoot(rootScrollEl, quickBody);
      }

      const uiScale = stateApi.getCurrentUiScale();
      const rowHeight = quickVirtual.rowHeightPx;
      const tbodyTop = quickVirtual.tbodyTopPx;
      const rowTop = tbodyTop + idx * rowHeight;
      const rowBottom = rowTop + rowHeight;
      const viewTop = Number(rootScrollEl.scrollTop || 0);
      const viewBottom = viewTop + Number(rootScrollEl.clientHeight || 900);

      if (rowTop < viewTop) {
        rootScrollEl.scrollTop = Math.max(0, rowTop - 4 * uiScale);
      } else if (rowBottom > viewBottom) {
        rootScrollEl.scrollTop = Math.max(0, rowBottom - Number(rootScrollEl.clientHeight || 900) + 4 * uiScale);
      }
    }

    function onQuickBodyClick(e) {
      let node = e.target;
      while (node && node !== quickBody) {
        if (node.nodeType === 1) {
          const el = node;
          if (el.tagName === "BUTTON" && el.getAttribute("data-action") === "reg") {
            const id = Number(el.getAttribute("data-id"));
            if (Number.isFinite(id) && id > 0) {
              setQuickSelected(id);
              safeCall("copng_log", { level: "info", message: `UI register click: ${stateApi.toHex32(id >>> 0)}` });
              safeCall("copng_registerItem", { formId: id >>> 0 });
            }
            return;
          }
          if (el.tagName === "TR" && el.getAttribute("data-row-id")) {
            const id = Number(el.getAttribute("data-row-id"));
            if (Number.isFinite(id) && id > 0) {
              setQuickSelected(id);
            }
            return;
          }
        }
        node = node.parentNode;
      }
    }

    function onUndoBodyClick(e) {
      let node = e.target;
      while (node && node !== undoBody) {
        if (node.nodeType === 1) {
          const el = node;
          if (el.tagName === "BUTTON" && el.getAttribute("data-action") === "undo") {
            if (el.disabled) return;
            const actionId = Number(el.getAttribute("data-id"));
            if (Number.isFinite(actionId) && actionId > 0) {
              safeCall("copng_undoRegisterItem", { actionId: Math.trunc(actionId) });
            }
            return;
          }
        }
        node = node.parentNode;
      }
    }

    function saveSettingsFromUi() {
      if (!documentObj || typeof documentObj.getElementById !== "function") return;
      const keyRaw = String((documentObj.getElementById("setToggleKey") || {}).value || "").trim();
      let key = KC && KC.parseKeybindInput ? KC.parseKeybindInput(keyRaw) : null;
      if (key == null) {
        key = stateApi.getToggleKeyDik();
        stateApi.setToggleKeyInputFromDik(key);
        stateApi.showToast("warn", `${t("toast.bindKeyUnknown", "Unsupported key.")}`);
      }
      const rewardEveryInput = parseInt(documentObj.getElementById("setRewardEvery").value, 10);
      const rewardMultInput = parseFloat(documentObj.getElementById("setRewardMult").value);
      const payload = {
        toggleKeyCode: (key >>> 0) & 0xff,
        languageOverride: documentObj.getElementById("setLang").value,
        uiPauseGame: documentObj.getElementById("setPauseGame").checked,
        uiDisableFocusMenu: documentObj.getElementById("setDisableFocusMenu").checked,
        uiDestroyOnClose: documentObj.getElementById("setDestroyOnClose").checked,
        uiInputScale: stateApi.clamp(stateApi.getInputScale(), 0.5, 3.0),
        normalizeRegistration: documentObj.getElementById("setNormalize").checked,
        requireTccDisplayed: documentObj.getElementById("setRequireTccDisplayed").checked,
        protectFavorites: documentObj.getElementById("setProtectFav").checked,
        enableLootNotify: documentObj.getElementById("setLootNotify").checked,
        enableRewards: documentObj.getElementById("setRewardsEnabled").checked,
        rewardEvery: Number.isFinite(rewardEveryInput) && rewardEveryInput > 0 ? rewardEveryInput : 5,
        rewardMultiplier: Number.isFinite(rewardMultInput) ? rewardMultInput : 1.0,
        allowSkillRewards: documentObj.getElementById("setSkillRewards").checked,
      };
      safeCall("copng_saveSettings", payload);
    }

    function onManualScaleChange(raw) {
      stateApi.setUiScaleMode("manual");
      const value = parseFloat(String(raw || ""));
      stateApi.setUiScaleManual(Number.isFinite(value) && value > 0 ? stateApi.clamp(value, 1.0, 3.0) : stateApi.getUiScaleManual());
      stateApi.applyManualUiScale();
      stateApi.saveUiScalePrefs();
      stateApi.syncUiScaleControls();
    }

    function onInputScaleChange(raw, { toast = false } = {}) {
      stateApi.setInputScale(raw, { persist: true, toast });
    }

    function onPerfModeChanged(raw) {
      const value = String(raw || "");
      stateApi.setPerfMode(value === "on" || value === "off" ? value : "auto");
      stateApi.savePerfModePref();
      stateApi.applyPerfModeFromPrefs();
    }

    function onUiScaleModeChanged(raw) {
      stateApi.setUiScaleMode(raw === "manual" ? "manual" : "auto");
      if (stateApi.getUiScaleMode() === "manual") {
        stateApi.setUiScaleManual(stateApi.clamp(stateApi.getCurrentUiScale(), 1.0, 3.0));
      }
      stateApi.saveUiScalePrefs();
      stateApi.applyUiScaleFromPrefs();
    }

    return Object.freeze({
      requestInventoryPage,
      scheduleRenderQuick,
      scheduleRenderRegistered,
      setQuickSelected,
      scrollQuickIndexIntoView,
      onQuickBodyClick,
      onUndoBodyClick,
      saveSettingsFromUi,
      onManualScaleChange,
      onInputScaleChange,
      onPerfModeChanged,
      onUiScaleModeChanged,
    });
  }

  return {
    createUIInteractions,
  };
});
