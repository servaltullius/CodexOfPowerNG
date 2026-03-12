(function (root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) {
    module.exports = api;
  }
  if (root) {
    root.COPNGUIBootstrap = api;
  }
})(typeof globalThis !== "undefined" ? globalThis : this, function () {
  "use strict";

  function noop() {}

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function installInputCorrectionFallback(opts) {
    const options = opts || {};
    const documentObj = options.documentObj;
    const windowObj = options.windowObj;
    const rootEl = options.rootEl;
    const clamp = asFn(options.clamp, (value, lo, hi) => Math.max(lo, Math.min(hi, value)));
    const getCurrentUiScale = asFn(options.getCurrentUiScale, () => 1);
    const getInputScale = asFn(options.getInputScale, () => 1);
    if (!documentObj || !windowObj || !rootEl) return noop;

    const INTERACTIVE_SEL = "button, input, select, textarea, a, [role='button'], [data-action], tr[data-row-id]";
    const isElementNode = (node) => !!(node && node.nodeType === 1);
    const pickInteractiveElement = (el) => {
      if (!isElementNode(el)) return null;
      if (el.matches && el.matches(INTERACTIVE_SEL)) return el;
      if (el.closest) return el.closest(INTERACTIVE_SEL);
      return null;
    };
    const makeMouseEventLike = (e, pt) =>
      new windowObj.MouseEvent("click", {
        bubbles: true,
        cancelable: true,
        composed: true,
        view: windowObj,
        detail: e.detail || 0,
        screenX: e.screenX || 0,
        screenY: e.screenY || 0,
        clientX: pt.x,
        clientY: pt.y,
        button: e.button || 0,
        buttons: e.buttons || 1,
        ctrlKey: !!e.ctrlKey,
        shiftKey: !!e.shiftKey,
        altKey: !!e.altKey,
        metaKey: !!e.metaKey,
      });

    let lastWheelTs = 0;
    const wheelNow = (e) => {
      const ts = typeof e.timeStamp === "number" && Number.isFinite(e.timeStamp) ? e.timeStamp : 0;
      if (ts > 0) return ts;
      if (typeof performance !== "undefined" && performance.now) return performance.now();
      return Date.now();
    };
    const normalizeWheelDelta = (e, container) => {
      let dy = Number(e.deltaY || 0);
      if (!Number.isFinite(dy) || dy === 0) return 0;
      const mode = Number(e.deltaMode || 0);
      if (mode === 1) dy *= 16;
      else if (mode === 2) dy *= Number(container.clientHeight || 800);

      const uiScale = clamp(getCurrentUiScale(), 1.0, 3.0);
      const abs = Math.abs(dy);
      const now = wheelNow(e);
      const dt = lastWheelTs > 0 ? now - lastWheelTs : 999;
      lastWheelTs = now;

      if (abs > 0 && abs < 12 && dt > 24) dy = Math.sign(dy) * 80 * uiScale;
      else if (abs < 40) dy = dy * 1.5 * uiScale;
      else dy = dy * uiScale;

      const maxStep = Number(container.clientHeight || 800) * 0.45;
      return clamp(dy, -maxStep, maxStep);
    };

    const onWheel = (e) => {
      if (!e || e.isTrusted === false) return;
      const maxScroll = Math.max(0, rootEl.scrollHeight - rootEl.clientHeight);
      if (maxScroll <= 0) return;
      const deltaPx = normalizeWheelDelta(e, rootEl);
      if (!Number.isFinite(deltaPx) || deltaPx === 0) return;
      if (e.preventDefault) e.preventDefault();
      if (e.stopPropagation) e.stopPropagation();
      rootEl.scrollTop = clamp(rootEl.scrollTop + deltaPx, 0, maxScroll);
    };
    rootEl.addEventListener("wheel", onWheel, { passive: false });

    let lastMouseupFixed = { t: 0, x: -9999, y: -9999 };
    const nowMs = () => {
      try {
        return Date.now();
      } catch {
        return 0;
      }
    };

    const onMouseUp = (e) => {
      if (!e || e.isTrusted === false) return;
      if (e.button !== undefined && e.button !== 0) return;
      if (isElementNode(e.target)) return;

      const w = Number(windowObj.innerWidth || 0);
      const h = Number(windowObj.innerHeight || 0);
      if (!Number.isFinite(w) || !Number.isFinite(h) || w <= 0 || h <= 0) return;

      const sx = clamp(getInputScale(), 0.5, 3.0);
      const needsScale = Number.isFinite(sx) && Math.abs(sx - 1.0) >= 0.01;
      const cx = Number(e.clientX || 0);
      const cy = Number(e.clientY || 0);
      const clampPt = (pt) => ({
        x: clamp(Math.round(pt.x), 0, Math.max(0, w - 1)),
        y: clamp(Math.round(pt.y), 0, Math.max(0, h - 1)),
      });

      const candidates = [{ pt: clampPt({ x: cx, y: cy }) }];
      if (needsScale) {
        candidates.push({ pt: clampPt({ x: cx * sx, y: cy * sx }) });
        candidates.push({ pt: clampPt({ x: cx / sx, y: cy / sx }) });
      }

      let best = null;
      for (const candidate of candidates) {
        const raw = documentObj.elementFromPoint(candidate.pt.x, candidate.pt.y);
        const picked = pickInteractiveElement(raw);
        if (!picked) continue;
        if (!rootEl.contains(picked)) continue;
        best = { target: picked, pt: candidate.pt };
        break;
      }
      if (!best) return;

      lastMouseupFixed = { t: nowMs(), x: Math.round(cx), y: Math.round(cy) };
      best.target.dispatchEvent(makeMouseEventLike(e, best.pt));
    };
    const onClick = (e) => {
      const fixedAge = nowMs() - (lastMouseupFixed.t || 0);
      if (fixedAge < 0 || fixedAge >= 250) return;
      const cx0 = Math.round(Number(e.clientX || 0));
      const cy0 = Math.round(Number(e.clientY || 0));
      if (Math.abs(cx0 - lastMouseupFixed.x) <= 1 && Math.abs(cy0 - lastMouseupFixed.y) <= 1) {
        if (e.preventDefault) e.preventDefault();
        if (e.stopImmediatePropagation) e.stopImmediatePropagation();
        else if (e.stopPropagation) e.stopPropagation();
      }
    };
    documentObj.addEventListener("mouseup", onMouseUp, true);
    documentObj.addEventListener("click", onClick, true);

    return function detach() {
      if (typeof rootEl.removeEventListener === "function") rootEl.removeEventListener("wheel", onWheel);
      if (typeof documentObj.removeEventListener === "function") {
        documentObj.removeEventListener("mouseup", onMouseUp, true);
        documentObj.removeEventListener("click", onClick, true);
      }
    };
  }

  function installEscapeCloseFallback(opts) {
    const options = opts || {};
    const windowObj = options.windowObj;
    const safeCall = asFn(options.safeCall, noop);
    if (!windowObj || typeof windowObj.addEventListener !== "function") return noop;

    const handler = (e) => {
      const key = String((e && e.key) || "");
      const code = String((e && e.code) || "");
      const keyCode = Number((e && (e.keyCode || e.which)) || 0) || 0;
      const isEscape = key === "Escape" || code === "Escape" || keyCode === 27;
      if (!isEscape) return;
      if (e && e.preventDefault) e.preventDefault();
      safeCall("copng_requestToggle", {});
    };
    windowObj.addEventListener("keydown", handler);
    return function detach() {
      if (typeof windowObj.removeEventListener === "function") {
        windowObj.removeEventListener("keydown", handler);
      }
    };
  }

  function initializeUI(opts) {
    const options = opts || {};
    const uiWiringApi = options.uiWiringApi || null;
    const inputCorrectionApi = options.inputCorrectionApi || null;
    const inputShortcutsApi = options.inputShortcutsApi || null;
    const stateApi = options.stateApi;
    const interactionsApi = options.interactionsApi;
    const renderingApi = options.renderingApi;
    const safeCall = asFn(options.safeCall, noop);
    const documentObj = options.documentObj;
    const windowObj = options.windowObj;
    const rootScrollEl = options.rootScrollEl || null;
    const t = asFn(options.t, (_key, fallback) => fallback);

    if (uiWiringApi && typeof uiWiringApi.installUIWiring === "function") {
      uiWiringApi.installUIWiring({
        documentObj,
        windowObj,
        rootScrollEl,
        quickBody: options.quickBody,
        undoBody: options.undoBody,
        buildPanelEl: options.buildPanelEl,
        invPageSizeEl: options.invPageSizeEl,
        btnInvPrev: options.btnInvPrev,
        btnInvNext: options.btnInvNext,
        btnCaptureToggleKeyEl: options.btnCaptureToggleKeyEl,
        langBtnEl: options.langBtnEl,
        langMenuEl: options.langMenuEl,
        langSelectEl: options.langSelectEl,
        uiScaleModeEl: options.uiScaleModeEl,
        uiScaleRangeEl: options.uiScaleRangeEl,
        uiScaleNumberEl: options.uiScaleNumberEl,
        perfModeEl: options.perfModeEl,
        inputScaleRangeEl: options.inputScaleRangeEl,
        inputScaleNumberEl: options.inputScaleNumberEl,
        inputScalePresetEl: options.inputScalePresetEl,
        safeCall,
        setTab: renderingApi.setTab,
        syncRewardCharacterImageState: renderingApi.syncRewardCharacterImageState,
        requestInventoryPage: interactionsApi.requestInventoryPage,
        getInventoryPage: stateApi.getInventoryPage,
        setInventoryPageSize: stateApi.setInventoryPageSize,
        showConfirm: stateApi.showConfirm,
        t,
        scheduleRenderQuick: interactionsApi.scheduleRenderQuick,
        scheduleRenderRegistered: interactionsApi.scheduleRenderRegistered,
        renderUndo: renderingApi.renderUndo,
        scheduleVirtualRender: options.scheduleVirtualRender,
        renderBuild: renderingApi.renderBuild,
        onQuickBodyClick: interactionsApi.onQuickBodyClick,
        onUndoBodyClick: interactionsApi.onUndoBodyClick,
        onBuildPanelClick: interactionsApi.onBuildPanelClick,
        onBatchRegisterClick: interactionsApi.onBatchRegisterClick,
        onClearBatchSelection: interactionsApi.clearQuickBatchSelection,
        onQuickActionableOnlyChanged: interactionsApi.onQuickActionableOnlyChanged,
        onSaveSettings: interactionsApi.saveSettingsFromUi,
        updateToggleKeyResolved: stateApi.updateToggleKeyResolved,
        setCaptureToggleKey: stateApi.setCaptureToggleKey,
        showToast: stateApi.showToast,
        closeLangMenu: renderingApi.closeLangMenu,
        openLangMenu: renderingApi.openLangMenu,
        syncLangDropdown: renderingApi.syncLangDropdown,
        isLangMenuOpen: stateApi.getLangMenuOpen,
        loadUiScalePrefs: stateApi.loadUiScalePrefs,
        syncUiScaleControls: stateApi.syncUiScaleControls,
        applyUiScaleFromPrefs: stateApi.applyUiScaleFromPrefs,
        syncInputScaleControls: stateApi.syncInputScaleControls,
        scheduleAutoUiScale: stateApi.scheduleAutoUiScale,
        getPerfMode: stateApi.getPerfMode,
        onPerfModeChanged: interactionsApi.onPerfModeChanged,
        onUiScaleModeChanged: interactionsApi.onUiScaleModeChanged,
        onManualScaleChange: interactionsApi.onManualScaleChange,
        onInputScaleChange: interactionsApi.onInputScaleChange,
      });
    }

    safeCall("copng_requestState", {});
    safeCall("copng_getSettings", {});
    safeCall("copng_requestInventory", { page: 0, pageSize: 200 });
    safeCall("copng_requestRegistered", {});
    safeCall("copng_requestUndoList", {});
    safeCall("copng_requestBuild", {});

    if (inputCorrectionApi && typeof inputCorrectionApi.installInputCorrection === "function") {
      inputCorrectionApi.installInputCorrection({
        documentObj,
        windowObj,
        rootEl: rootScrollEl,
        clamp: stateApi.clamp,
        getCurrentUiScale: stateApi.getCurrentUiScale,
        getInputScale: stateApi.getInputScale,
      });
    } else {
      safeCall("copng_log", {
        level: "warn",
        message: "input_correction.js unavailable; using fallback input correction",
      });
      installInputCorrectionFallback({
        documentObj,
        windowObj,
        rootEl: rootScrollEl,
        clamp: stateApi.clamp,
        getCurrentUiScale: stateApi.getCurrentUiScale,
        getInputScale: stateApi.getInputScale,
      });
    }

    if (inputShortcutsApi && typeof inputShortcutsApi.installKeydownHandler === "function") {
      inputShortcutsApi.installKeydownHandler({
        KC: options.KC,
        t,
        showToast: stateApi.showToast,
        safeCall,
        setTab: renderingApi.setTab,
        setInputScale: stateApi.setInputScale,
        getInputScale: stateApi.getInputScale,
        getToggleKeyDik: stateApi.getToggleKeyDik,
        setToggleKeyInputFromDik: stateApi.setToggleKeyInputFromDik,
        formatToggleKeyDisplay: stateApi.formatToggleKeyDisplay,
        getCaptureToggleKey: stateApi.getCaptureToggleKey,
        setCaptureToggleKey: stateApi.setCaptureToggleKey,
        isLangMenuOpen: stateApi.getLangMenuOpen,
        closeLangMenu: renderingApi.closeLangMenu,
        getQuickVisibleIds: stateApi.getQuickVisibleIds,
        getQuickSelectedId: stateApi.getQuickSelectedId,
        setQuickSelected: interactionsApi.setQuickSelected,
        scrollQuickIndexIntoView: interactionsApi.scrollQuickIndexIntoView,
        scheduleVirtualRender: options.scheduleVirtualRender,
        getKeyNavRaf: stateApi.getKeyNavRaf,
        setKeyNavRaf: stateApi.setKeyNavRaf,
        toHex32: stateApi.toHex32,
        documentObj,
        windowObj,
        requestAnimationFrameFn: options.requestAnimationFrameFn,
      });
    } else {
      installEscapeCloseFallback({
        windowObj,
        safeCall,
      });
    }

    safeCall("copng_log", { level: "info", message: "UI loaded" });
  }

  return {
    initializeUI,
    installInputCorrectionFallback,
    installEscapeCloseFallback,
  };
});
