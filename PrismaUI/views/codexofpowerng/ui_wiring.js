(function (global) {
  "use strict";

  function noop() {}

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function byId(doc, id) {
    if (!doc || typeof doc.getElementById !== "function") return null;
    return doc.getElementById(id);
  }

  function addListener(el, eventName, handler, options) {
    if (!el || typeof el.addEventListener !== "function" || typeof handler !== "function") return;
    el.addEventListener(eventName, handler, options);
  }

  function installUIWiring(opts) {
    const options = opts || {};
    const doc = options.documentObj || (global && global.document) || null;
    const win = options.windowObj || global;
    if (!doc) return noop;

    const safeCall = asFn(options.safeCall, noop);
    const setTab = asFn(options.setTab, noop);
    const syncRewardCharacterImageState = asFn(options.syncRewardCharacterImageState, noop);
    const requestInventoryPage = asFn(options.requestInventoryPage, noop);
    const getInventoryPage = asFn(options.getInventoryPage, function () {
      return { page: 0, pageSize: 200 };
    });
    const setInventoryPageSize = asFn(options.setInventoryPageSize, noop);
    const showConfirm = asFn(
      options.showConfirm,
      async function () {
        return false;
      }
    );
    const t = asFn(options.t, function (_key, fallback) {
      return fallback;
    });
    const scheduleRenderQuick = asFn(options.scheduleRenderQuick, noop);
    const scheduleRenderRegistered = asFn(options.scheduleRenderRegistered, noop);
    const renderUndo = asFn(options.renderUndo, noop);
    const scheduleVirtualRender = asFn(options.scheduleVirtualRender, noop);
    const renderRewards = asFn(options.renderRewards, noop);
    const onQuickBodyClick = asFn(options.onQuickBodyClick, noop);
    const onUndoBodyClick = asFn(options.onUndoBodyClick, noop);
    const onSaveSettings = asFn(options.onSaveSettings, noop);
    const updateToggleKeyResolved = asFn(options.updateToggleKeyResolved, noop);
    const setCaptureToggleKey = asFn(options.setCaptureToggleKey, noop);
    const showToast = asFn(options.showToast, noop);
    const closeLangMenu = asFn(options.closeLangMenu, noop);
    const openLangMenu = asFn(options.openLangMenu, noop);
    const syncLangDropdown = asFn(options.syncLangDropdown, noop);
    const isLangMenuOpen = asFn(options.isLangMenuOpen, function () {
      return false;
    });
    const loadUiScalePrefs = asFn(options.loadUiScalePrefs, noop);
    const syncUiScaleControls = asFn(options.syncUiScaleControls, noop);
    const applyUiScaleFromPrefs = asFn(options.applyUiScaleFromPrefs, noop);
    const syncInputScaleControls = asFn(options.syncInputScaleControls, noop);
    const scheduleAutoUiScale = asFn(options.scheduleAutoUiScale, noop);
    const getPerfMode = asFn(options.getPerfMode, function () {
      return "auto";
    });
    const onPerfModeChanged = asFn(options.onPerfModeChanged, noop);
    const onUiScaleModeChanged = asFn(options.onUiScaleModeChanged, noop);
    const onManualScaleChange = asFn(options.onManualScaleChange, noop);
    const onInputScaleChange = asFn(options.onInputScaleChange, noop);

    const rootScrollEl = options.rootScrollEl || byId(doc, "root");
    const quickBody = options.quickBody || byId(doc, "quickBody");
    const undoBody = options.undoBody || byId(doc, "undoBody");
    const rewardCharacterImgEl = options.rewardCharacterImgEl || byId(doc, "rewardCharacterImg");
    const invPageSizeEl = options.invPageSizeEl || byId(doc, "invPageSize");
    const btnInvPrev = options.btnInvPrev || byId(doc, "btnInvPrev");
    const btnInvNext = options.btnInvNext || byId(doc, "btnInvNext");
    const btnCaptureToggleKeyEl = options.btnCaptureToggleKeyEl || byId(doc, "btnCaptureToggleKey");
    const langBtnEl = options.langBtnEl || byId(doc, "langBtn");
    const langMenuEl = options.langMenuEl || byId(doc, "langMenu");
    const langSelectEl = options.langSelectEl || byId(doc, "setLang");
    const uiScaleModeEl = options.uiScaleModeEl || byId(doc, "setUiScaleMode");
    const uiScaleRangeEl = options.uiScaleRangeEl || byId(doc, "setUiScaleRange");
    const uiScaleNumberEl = options.uiScaleNumberEl || byId(doc, "setUiScaleNumber");
    const perfModeEl = options.perfModeEl || byId(doc, "setPerfMode");
    const inputScaleRangeEl = options.inputScaleRangeEl || byId(doc, "setInputScaleRange");
    const inputScaleNumberEl = options.inputScaleNumberEl || byId(doc, "setInputScaleNumber");
    const inputScalePresetEl = options.inputScalePresetEl || byId(doc, "setInputScalePreset");

    if (typeof doc.querySelectorAll === "function") {
      const tabButtons = doc.querySelectorAll(".tabs button");
      if (tabButtons && typeof tabButtons.forEach === "function") {
        tabButtons.forEach(function (btn) {
          addListener(btn, "click", function () {
            setTab(btn.dataset ? btn.dataset.tab : "");
          });
        });
      }
    }

    if (rewardCharacterImgEl) {
      addListener(rewardCharacterImgEl, "load", syncRewardCharacterImageState);
      addListener(rewardCharacterImgEl, "error", syncRewardCharacterImageState);
      syncRewardCharacterImageState();
    }

    addListener(byId(doc, "btnRefreshState"), "click", function () {
      safeCall("copng_requestState", {});
      safeCall("copng_getSettings", {});
    });

    addListener(byId(doc, "btnClose"), "click", function () {
      safeCall("copng_requestToggle", {});
    });

    addListener(byId(doc, "btnRefreshInv"), "click", function () {
      requestInventoryPage(0);
    });

    if (invPageSizeEl) {
      addListener(invPageSizeEl, "change", function () {
        const v = parseInt(invPageSizeEl.value, 10);
        const page = getInventoryPage() || {};
        const next = Number.isFinite(v) && v > 0 ? v : Number(page.pageSize || 200);
        setInventoryPageSize(next);
        requestInventoryPage(0);
      });
    }

    if (btnInvPrev) {
      addListener(btnInvPrev, "click", function () {
        const page = getInventoryPage() || {};
        requestInventoryPage(Math.max(0, Number(page.page || 0) - 1));
      });
    }
    if (btnInvNext) {
      addListener(btnInvNext, "click", function () {
        const page = getInventoryPage() || {};
        requestInventoryPage(Number(page.page || 0) + 1);
      });
    }

    addListener(byId(doc, "btnRefreshReg"), "click", function () {
      safeCall("copng_requestRegistered", {});
    });
    addListener(byId(doc, "btnRefreshUndo"), "click", function () {
      safeCall("copng_requestUndoList", {});
      renderUndo();
    });
    addListener(byId(doc, "btnRefreshRewards"), "click", function () {
      safeCall("copng_requestRewards", {});
    });

    addListener(byId(doc, "btnRefundRewards"), "click", async function () {
      const ok = await showConfirm(t("confirm.refund", "Refund rewards? This cannot be undone."));
      if (!ok) return;
      safeCall("copng_refundRewards", {});
    });

    addListener(byId(doc, "quickFilter"), "input", scheduleRenderQuick);
    addListener(byId(doc, "regFilter"), "input", scheduleRenderRegistered);

    if (rootScrollEl) {
      addListener(
        rootScrollEl,
        "scroll",
        function () {
          scheduleVirtualRender();
        },
        { passive: true }
      );
      addListener(rootScrollEl, "scroll", function () {
        if (isLangMenuOpen()) closeLangMenu();
      });
    }

    if (win && typeof win.addEventListener === "function") {
      addListener(win, "resize", function () {
        scheduleVirtualRender({ force: true });
        renderRewards();
      });
    }

    if (quickBody) {
      addListener(quickBody, "click", onQuickBodyClick);
    }
    if (undoBody) {
      addListener(undoBody, "click", onUndoBodyClick);
    }

    addListener(byId(doc, "btnReloadSettings"), "click", function () {
      safeCall("copng_getSettings", {});
    });
    addListener(byId(doc, "btnSaveSettings"), "click", onSaveSettings);
    addListener(byId(doc, "btnCloseSettings"), "click", function () {
      safeCall("copng_requestToggle", {});
    });

    const toggleKeyInputEl = byId(doc, "setToggleKey");
    if (toggleKeyInputEl) {
      addListener(toggleKeyInputEl, "input", updateToggleKeyResolved);
      addListener(toggleKeyInputEl, "blur", updateToggleKeyResolved);
    }

    if (btnCaptureToggleKeyEl) {
      addListener(btnCaptureToggleKeyEl, "click", function () {
        setCaptureToggleKey(true);
        showToast("info", t("toast.bindKey", "Press a key to bind (Esc cancels)."));
      });
    }

    if (langBtnEl && langMenuEl && langSelectEl) {
      syncLangDropdown();

      addListener(langBtnEl, "click", function (e) {
        if (e && e.preventDefault) e.preventDefault();
        if (isLangMenuOpen()) closeLangMenu();
        else openLangMenu();
      });

      addListener(
        win,
        "click",
        function (e) {
          if (!isLangMenuOpen()) return;
          const dd = byId(doc, "langDropdown");
          if (!dd) return closeLangMenu();
          const target = e && e.target ? e.target : null;
          if (target && dd.contains(target)) return;
          closeLangMenu();
        },
        true
      );
    }

    loadUiScalePrefs();
    syncUiScaleControls();
    applyUiScaleFromPrefs();
    syncInputScaleControls();
    addListener(win, "resize", scheduleAutoUiScale);

    if (perfModeEl) {
      perfModeEl.value = getPerfMode();
      addListener(perfModeEl, "change", function () {
        onPerfModeChanged(perfModeEl.value);
      });
    }

    if (uiScaleModeEl) {
      addListener(uiScaleModeEl, "change", function () {
        onUiScaleModeChanged(uiScaleModeEl.value);
      });
    }

    if (uiScaleRangeEl) {
      addListener(uiScaleRangeEl, "input", function () {
        onManualScaleChange(uiScaleRangeEl.value);
      });
    }
    if (uiScaleNumberEl) {
      addListener(uiScaleNumberEl, "input", function () {
        onManualScaleChange(uiScaleNumberEl.value);
      });
    }

    if (inputScaleRangeEl) {
      addListener(inputScaleRangeEl, "input", function () {
        onInputScaleChange(inputScaleRangeEl.value);
      });
    }
    if (inputScaleNumberEl) {
      addListener(inputScaleNumberEl, "input", function () {
        onInputScaleChange(inputScaleNumberEl.value);
      });
    }
    if (inputScalePresetEl) {
      addListener(inputScalePresetEl, "change", function () {
        onInputScaleChange(inputScalePresetEl.value, { toast: true });
      });
    }

    return noop;
  }

  const api = Object.freeze({
    installUIWiring: installUIWiring,
  });

  if (typeof module !== "undefined" && module && module.exports) {
    module.exports = api;
  }

  global.COPNGUIWiring = api;
})(typeof window !== "undefined" ? window : globalThis);
