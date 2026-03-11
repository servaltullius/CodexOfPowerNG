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
    const buildPanelEl = options.buildPanelEl || null;
    const stateApi = options.stateApi;
    const safeCall = asFn(options.safeCall, noop);
    const t = asFn(options.t, (_key, fallback) => fallback);
    const KC = options.KC || null;
    const renderQuick = asFn(options.renderQuick, noop);
    const renderRegistered = asFn(options.renderRegistered, noop);
    const registerBatchPanelApi = options.registerBatchPanelApi || null;
    const clamp = asFn(options.clamp, (value, lo, hi) => Math.max(lo, Math.min(hi, value)));
    const getOffsetTopInRoot = asFn(options.getOffsetTopInRoot, () => 0);
    const QUICK_ROW_BASE_PX = Number(options.quickRowBasePx || 78);
    const getQuickBatchSelectedIds = asFn(stateApi && stateApi.getQuickBatchSelectedIds, () => []);
    const setQuickBatchSelectedIds = asFn(stateApi && stateApi.setQuickBatchSelectedIds, noop);
    const getQuickActionableOnly = asFn(stateApi && stateApi.getQuickActionableOnly, () => false);
    const setQuickActionableOnly = asFn(stateApi && stateApi.setQuickActionableOnly, noop);

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
          if (el.getAttribute("data-action") === "batch-toggle") {
            const id = Number(el.getAttribute("data-id"));
            if (Number.isFinite(id) && id > 0) {
              toggleQuickBatchSelection(id);
            }
            return;
          }
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

    function buildQuickBatchSummary() {
      if (registerBatchPanelApi && typeof registerBatchPanelApi.buildRegisterBatchViewModel === "function") {
        const viewModel = registerBatchPanelApi.buildRegisterBatchViewModel(
          stateApi.getInventoryPage ? stateApi.getInventoryPage() : {},
          getQuickBatchSelectedIds(),
          {
            actionableOnly: getQuickActionableOnly(),
            query:
              documentObj && typeof documentObj.getElementById === "function"
                ? String(((documentObj.getElementById("quickFilter") || {}).value) || "")
                : "",
          },
        );
        return viewModel && viewModel.summary
          ? viewModel.summary
          : { selectedRows: 0, disciplineGain: { attack: 0, defense: 0, utility: 0 }, formIds: [] };
      }

      const ids = getQuickBatchSelectedIds();
      return {
        selectedRows: ids.length,
        disciplineGain: { attack: 0, defense: 0, utility: 0 },
        formIds: ids.slice(),
      };
    }

    function toggleQuickBatchSelection(nextId) {
      const id = Number(nextId) >>> 0;
      if (!id) return;
      const prev = getQuickBatchSelectedIds();
      const has = prev.indexOf(id) !== -1;
      setQuickBatchSelectedIds(has ? prev.filter((value) => value !== id) : prev.concat(id));
      renderQuick();
    }

    function clearQuickBatchSelection() {
      setQuickBatchSelectedIds([]);
      renderQuick();
    }

    function onBatchRegisterClick() {
      const summary = buildQuickBatchSummary();
      if (!summary || !Array.isArray(summary.formIds) || summary.formIds.length === 0) return;
      safeCall("copng_requestRegisterBatch", { formIds: summary.formIds.slice() });
    }

    function onQuickActionableOnlyChanged(next) {
      setQuickActionableOnly(next);
      renderQuick();
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

    function onBuildPanelClick(e) {
      let node = e.target;
      while (node && node !== buildPanelEl) {
        if (node.nodeType === 1) {
          const el = node;
          const action = el.getAttribute("data-action");
          if (action === "build-activate") {
            const optionId = String(el.getAttribute("data-option-id") || "");
            const slotId = String(el.getAttribute("data-slot-id") || "");
            if (optionId && slotId) {
              safeCall("copng_activateBuildOption", { optionId, slotId });
            }
            return;
          }
          if (action === "build-deactivate") {
            const slotId = String(el.getAttribute("data-slot-id") || "");
            if (slotId) {
              safeCall("copng_deactivateBuildOption", { slotId });
            }
            return;
          }
          if (action === "build-swap") {
            const optionId = String(el.getAttribute("data-option-id") || "");
            const fromSlotId = String(el.getAttribute("data-from-slot-id") || "");
            const toSlotId = String(el.getAttribute("data-to-slot-id") || "");
            if (optionId && fromSlotId && toSlotId) {
              safeCall("copng_swapBuildOption", { optionId, fromSlotId, toSlotId });
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
      buildQuickBatchSummary,
      toggleQuickBatchSelection,
      clearQuickBatchSelection,
      onQuickBodyClick,
      onUndoBodyClick,
      onBuildPanelClick,
      onBatchRegisterClick,
      onQuickActionableOnlyChanged,
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
