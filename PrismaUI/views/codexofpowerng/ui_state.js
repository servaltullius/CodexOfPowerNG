(function (root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) {
    module.exports = api;
  }
  if (root) {
    root.COPNGUIState = api;
  }
})(typeof globalThis !== "undefined" ? globalThis : this, function () {
  "use strict";

  function noop() {}

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function defaultClamp(value, lo, hi) {
    return Math.max(lo, Math.min(hi, value));
  }

  function defaultCoalesce(value, fallback) {
    return value == null ? fallback : value;
  }

  function defaultToHex32(value) {
    const n = Number(value >>> 0);
    let hex = n.toString(16).toUpperCase();
    while (hex.length < 8) hex = "0" + hex;
    return "0x" + hex;
  }

  function createUIState(opts) {
    const options = opts || {};
    const documentObj = options.documentObj || (typeof document !== "undefined" ? document : null);
    const windowObj = options.windowObj || (typeof window !== "undefined" ? window : null);
    const localStorageObj =
      options.localStorageObj ||
      ((typeof localStorage !== "undefined" && localStorage) || (windowObj && windowObj.localStorage) || null);
    const refs = options.refs || {};
    const t = asFn(options.t, (_key, fallback) => fallback);
    const KC = options.KC || null;
    const safeCall = asFn(options.safeCall, noop);
    const clamp = asFn(options.clamp, defaultClamp);
    const coalesce = asFn(options.coalesce, defaultCoalesce);
    const toHex32 = asFn(options.toHex32, defaultToHex32);
    const scheduleVirtualRender = asFn(options.scheduleVirtualRender, noop);

    let captureToggleKey = false;
    let langMenuOpen = false;
    let toastTimer = null;
    let lotdGateBlockingToastShown = false;
    let uiLang = options.initialUiLang || "en";
    let state = {};
    let inventoryPage = { page: 0, pageSize: 200, total: 0, hasMore: false, items: [] };
    let quickSelectedId = 0;
    let quickVisibleIds = [];
    const quickVirtual = { rows: [], lastStart: -1, lastEnd: -1, tbodyTopPx: NaN, rowHeightPx: 0 };
    let registered = [];
    let undoItems = [];
    const regVirtual = { rows: [], lastStart: -1, lastEnd: -1, tbodyTopPx: NaN, rowHeightPx: 0 };
    let rewards = { totals: [] };
    let settings = null;
    let keyNavRaf = 0;

    const UI_SCALE_MODE_KEY = "copng.ui.scaleMode";
    const UI_SCALE_MANUAL_KEY = "copng.ui.manualScale";
    let uiScaleMode = "auto";
    let uiScaleManual = 1.0;

    const PERF_MODE_KEY = "copng.ui.perfMode";
    let perfMode = "auto";

    const INPUT_SCALE_KEY = "copng.ui.inputScale";
    let inputScale = 1.0;
    let inputScaleHasLocal = false;

    function loadInputScalePref() {
      try {
        const storedRaw = String((localStorageObj && localStorageObj.getItem && localStorageObj.getItem(INPUT_SCALE_KEY)) || "");
        const stored = parseFloat(storedRaw);
        if (Number.isFinite(stored) && stored > 0) {
          inputScaleHasLocal = true;
        }
      } catch {}
    }

    function saveInputScalePref() {
      try {
        if (localStorageObj && localStorageObj.setItem) {
          localStorageObj.setItem(INPUT_SCALE_KEY, String(inputScale));
        }
      } catch {}
    }

    function syncInputScaleControls() {
      if (refs.inputScaleRangeEl) refs.inputScaleRangeEl.value = String(clamp(inputScale, 0.5, 3.0));
      if (refs.inputScaleNumberEl) {
        refs.inputScaleNumberEl.value = String(Math.round(clamp(inputScale, 0.5, 3.0) * 100) / 100);
      }
      if (refs.inputScalePresetEl) {
        const presets = [1.0, 1.25, 1.5, 1.75, 2.0];
        let nearest = presets[0];
        let best = Math.abs(inputScale - nearest);
        for (const preset of presets) {
          const diff = Math.abs(inputScale - preset);
          if (diff < best) {
            best = diff;
            nearest = preset;
          }
        }
        refs.inputScalePresetEl.value = String(nearest);
      }
      if (refs.inputScalePillEl) refs.inputScalePillEl.textContent = `Input: ${inputScale.toFixed(2)}`;
    }

    function getCurrentUiScale() {
      const getComputedStyleFn =
        (windowObj && typeof windowObj.getComputedStyle === "function" && windowObj.getComputedStyle.bind(windowObj)) ||
        (typeof getComputedStyle === "function" ? getComputedStyle : null);
      if (!documentObj || !documentObj.documentElement || !getComputedStyleFn) return 1;
      const raw = String(getComputedStyleFn(documentObj.documentElement).getPropertyValue("--uiScale") || "").trim();
      const value = parseFloat(raw);
      return Number.isFinite(value) && value > 0 ? value : 1;
    }

    function getEffectiveDpr() {
      const w = Number((windowObj && windowObj.innerWidth) || 0);
      const h = Number((windowObj && windowObj.innerHeight) || 0);
      const dpr = Number((windowObj && windowObj.devicePixelRatio) || 1);
      const sw = Number((windowObj && windowObj.screen && windowObj.screen.width) || 0);
      const sh = Number((windowObj && windowObj.screen && windowObj.screen.height) || 0);

      const dprRaw = Number.isFinite(dpr) && dpr > 0 ? dpr : 1;
      const dprHint = Number.isFinite(inputScale) && inputScale > 0 ? inputScale : 1;
      const screenScaleW = sw > 0 && w > 0 ? sw / w : 1;
      const screenScaleH = sh > 0 && h > 0 ? sh / h : 1;
      const screenScale = Math.max(screenScaleW, screenScaleH);

      let effectiveDpr = dprRaw;
      if (effectiveDpr < 1.01) {
        if (Number.isFinite(screenScale) && screenScale > 1.01) effectiveDpr = Math.max(effectiveDpr, screenScale);
        if (dprHint > 1.01) effectiveDpr = Math.max(effectiveDpr, dprHint);
      }
      return clamp(effectiveDpr, 1.0, 3.0);
    }

    function shouldEnableLowFx() {
      if (perfMode === "on") return true;
      if (perfMode === "off") return false;
      return getCurrentUiScale() >= 2.0 || getEffectiveDpr() >= 1.6;
    }

    function applyPerfModeFromPrefs() {
      if (!documentObj || !documentObj.documentElement || !documentObj.documentElement.classList) return;
      documentObj.documentElement.classList.toggle("lowfx", shouldEnableLowFx());
    }

    function setInputScale(next, { persist = true, toast = false } = {}) {
      const value = parseFloat(String(next));
      if (!Number.isFinite(value) || value <= 0) return;
      inputScale = clamp(value, 0.5, 3.0);
      inputScaleHasLocal = !!persist;
      if (persist) saveInputScalePref();
      syncInputScaleControls();
      if (uiScaleMode === "auto") scheduleAutoUiScale();
      applyPerfModeFromPrefs();
      if (toast) showToast("info", `Input scale: ${inputScale.toFixed(2)}`);
    }

    function loadPerfModePref() {
      try {
        const stored = String((localStorageObj && localStorageObj.getItem && localStorageObj.getItem(PERF_MODE_KEY)) || "auto");
        perfMode = stored === "on" || stored === "off" ? stored : "auto";
      } catch {
        perfMode = "auto";
      }
    }

    function savePerfModePref() {
      try {
        if (localStorageObj && localStorageObj.setItem) {
          localStorageObj.setItem(PERF_MODE_KEY, perfMode);
        }
      } catch {}
    }

    function computeAutoUiScale() {
      const w = Number((windowObj && windowObj.innerWidth) || 0);
      const h = Number((windowObj && windowObj.innerHeight) || 0);
      const sw = Number((windowObj && windowObj.screen && windowObj.screen.width) || 0);
      const sh = Number((windowObj && windowObj.screen && windowObj.screen.height) || 0);
      const effectiveDpr = getEffectiveDpr();
      const viewportPx = Math.min(w, h) * effectiveDpr;
      const screenPx = Math.min(sw, sh);
      const minDim = Math.max(viewportPx, screenPx);
      if (!Number.isFinite(minDim) || minDim <= 0) return 1;
      return clamp(Math.max(0.0, minDim / 1080), 1.0, 3.0);
    }

    function loadUiScalePrefs() {
      try {
        const mode = String((localStorageObj && localStorageObj.getItem && localStorageObj.getItem(UI_SCALE_MODE_KEY)) || "auto");
        uiScaleMode = mode === "manual" ? "manual" : "auto";
        const stored = parseFloat(String((localStorageObj && localStorageObj.getItem && localStorageObj.getItem(UI_SCALE_MANUAL_KEY)) || ""));
        uiScaleManual = Number.isFinite(stored) && stored > 0 ? clamp(stored, 1.0, 3.0) : 1.9;
      } catch {
        uiScaleMode = "auto";
        uiScaleManual = 1.9;
      }
    }

    function saveUiScalePrefs() {
      try {
        if (localStorageObj && localStorageObj.setItem) {
          localStorageObj.setItem(UI_SCALE_MODE_KEY, uiScaleMode);
          localStorageObj.setItem(UI_SCALE_MANUAL_KEY, String(uiScaleManual));
        }
      } catch {}
    }

    function syncUiScaleControls() {
      if (!refs.uiScaleModeEl || !refs.uiScaleRangeEl || !refs.uiScaleNumberEl) return;
      refs.uiScaleModeEl.value = uiScaleMode;
      const manualEnabled = uiScaleMode === "manual";
      refs.uiScaleRangeEl.disabled = !manualEnabled;
      refs.uiScaleNumberEl.disabled = !manualEnabled;
      const shown = manualEnabled ? uiScaleManual : getCurrentUiScale();
      refs.uiScaleRangeEl.value = String(clamp(shown, 1.0, 3.0));
      refs.uiScaleNumberEl.value = String(Math.round(clamp(shown, 1.0, 3.0) * 100) / 100);
    }

    function applyManualUiScale() {
      if (documentObj && documentObj.documentElement && documentObj.documentElement.style) {
        documentObj.documentElement.style.setProperty("--uiScale", String(clamp(uiScaleManual, 1.0, 3.0)));
      }
      scheduleVirtualRender({ force: true });
    }

    function applyAutoUiScale() {
      if (uiScaleMode !== "auto") return;
      const w = Number((windowObj && windowObj.innerWidth) || 0);
      const h = Number((windowObj && windowObj.innerHeight) || 0);
      const dpr = Number((windowObj && windowObj.devicePixelRatio) || 1);
      const sw = Number((windowObj && windowObj.screen && windowObj.screen.width) || 0);
      const sh = Number((windowObj && windowObj.screen && windowObj.screen.height) || 0);
      const dprRaw = Number.isFinite(dpr) && dpr > 0 ? dpr : 1;
      const dprHint = Number.isFinite(inputScale) && inputScale > 0 ? inputScale : 1;
      const screenScaleW = sw > 0 && w > 0 ? sw / w : 1;
      const screenScaleH = sh > 0 && h > 0 ? sh / h : 1;
      const screenScale = Math.max(screenScaleW, screenScaleH);
      const effectiveDpr = getEffectiveDpr();
      const scale = computeAutoUiScale();

      if (documentObj && documentObj.documentElement && documentObj.documentElement.style) {
        documentObj.documentElement.style.setProperty("--uiScale", String(scale));
      }
      safeCall("copng_log", {
        level: "info",
        message: `UI scale -> ${scale.toFixed(3)} (viewport ${w}x${h}, screen ${sw}x${sh}, dpr ${dprRaw} (effective ${effectiveDpr}, screenScale ${screenScale.toFixed(
          2,
        )}, inputScale ${dprHint}))`,
      });
      applyPerfModeFromPrefs();
      scheduleVirtualRender({ force: true });
    }

    let uiScaleTimer = null;
    function scheduleAutoUiScale() {
      if (uiScaleMode !== "auto") return;
      if (uiScaleTimer) clearTimeout(uiScaleTimer);
      uiScaleTimer = setTimeout(applyAutoUiScale, 150);
    }

    function applyUiScaleFromPrefs() {
      if (uiScaleMode === "manual") {
        applyManualUiScale();
        syncUiScaleControls();
        applyPerfModeFromPrefs();
        safeCall("copng_log", { level: "info", message: `UI scale -> ${getCurrentUiScale().toFixed(3)} (manual)` });
        return;
      }

      applyAutoUiScale();
      syncUiScaleControls();
      applyPerfModeFromPrefs();
    }

    function showToast(level, message) {
      if (!refs.toastEl || !refs.toastMetaEl || !refs.toastMsgEl) return;
      if (toastTimer) clearTimeout(toastTimer);
      const lvl = String(level || "info").toLowerCase();
      refs.toastMetaEl.textContent = t("toast." + lvl, lvl);
      refs.toastMsgEl.textContent = message || "";
      refs.toastEl.classList.add("show");
      toastTimer = setTimeout(() => refs.toastEl.classList.remove("show"), 2500);
    }

    function showConfirm(message) {
      return new Promise((resolve) => {
        if (!documentObj || typeof documentObj.getElementById !== "function") {
          resolve(false);
          return;
        }
        const overlay = documentObj.getElementById("confirmOverlay");
        const msgEl = documentObj.getElementById("confirmMsg");
        const okBtn = documentObj.getElementById("confirmOk");
        const cancelBtn = documentObj.getElementById("confirmCancel");
        if (!overlay || !msgEl || !okBtn || !cancelBtn) {
          resolve(false);
          return;
        }
        msgEl.textContent = message || "";
        overlay.style.display = "";
        function cleanup(result) {
          overlay.style.display = "none";
          okBtn.removeEventListener("click", onOk);
          cancelBtn.removeEventListener("click", onCancel);
          resolve(result);
        }
        function onOk() {
          cleanup(true);
        }
        function onCancel() {
          cleanup(false);
        }
        okBtn.addEventListener("click", onOk);
        cancelBtn.addEventListener("click", onCancel);
      });
    }

    function getToggleKeyDik() {
      if (settings && Number.isFinite(settings.toggleKeyCode)) return (settings.toggleKeyCode >>> 0) & 0xff;
      if (state && Number.isFinite(state.toggleKeyCode)) return (state.toggleKeyCode >>> 0) & 0xff;
      return 0x3e;
    }

    function formatToggleKeyLabel(dik) {
      const name = KC && KC.dikToKeyName ? KC.dikToKeyName(dik) : null;
      if (name) return name;
      const hex = KC && KC.dikToHex ? KC.dikToHex(dik) : null;
      return hex || toHex32(dik >>> 0);
    }

    function formatToggleKeyDisplay(dik) {
      const formatted = KC && KC.formatDikDisplay ? KC.formatDikDisplay(dik) : null;
      if (formatted) return formatted;
      const hex = KC && KC.dikToHex ? KC.dikToHex(dik) : null;
      return hex || toHex32(dik >>> 0);
    }

    function updateToggleKeyResolved() {
      if (!refs.toggleKeyResolvedEl || !documentObj || typeof documentObj.getElementById !== "function") return;
      const raw = String((documentObj.getElementById("setToggleKey") && documentObj.getElementById("setToggleKey").value) || "").trim();
      const parsed = KC && KC.parseKeybindInput ? KC.parseKeybindInput(raw) : null;
      if (parsed == null) {
        refs.toggleKeyResolvedEl.textContent = `${t("settings.resolved", "Resolved")}: ${t("settings.invalid", "invalid")}`;
        return;
      }
      refs.toggleKeyResolvedEl.textContent = `${t("settings.resolved", "Resolved")}: ${formatToggleKeyDisplay(parsed)}`;
    }

    function setToggleKeyInputFromDik(dik) {
      if (!documentObj || typeof documentObj.getElementById !== "function") return;
      const input = documentObj.getElementById("setToggleKey");
      if (!input) return;
      input.value = formatToggleKeyLabel(dik);
      updateToggleKeyResolved();
    }

    loadInputScalePref();
    loadPerfModePref();

    return Object.freeze({
      getUiLang: () => uiLang,
      setUiLang: (next) => {
        uiLang = next;
      },
      getState: () => state,
      setState: (next) => {
        state = next || {};
      },
      getInventoryPage: () => inventoryPage,
      setInventoryPage: (next) => {
        inventoryPage = next;
      },
      setInventoryPageSize: (next) => {
        inventoryPage.pageSize = next;
      },
      getQuickSelectedId: () => quickSelectedId,
      setQuickSelectedId: (next) => {
        quickSelectedId = Number(next) >>> 0;
      },
      getQuickVisibleIds: () => quickVisibleIds,
      setQuickVisibleIds: (next) => {
        quickVisibleIds = Array.isArray(next) ? next : [];
      },
      getQuickVirtual: () => quickVirtual,
      getRegistered: () => registered,
      setRegistered: (next) => {
        registered = next;
      },
      getUndoItems: () => undoItems,
      setUndoItems: (next) => {
        undoItems = next;
      },
      getRegVirtual: () => regVirtual,
      getRewards: () => rewards,
      setRewards: (next) => {
        rewards = next;
      },
      getSettings: () => settings,
      setSettings: (next) => {
        settings = next;
      },
      getCaptureToggleKey: () => captureToggleKey,
      setCaptureToggleKey: (next) => {
        captureToggleKey = !!next;
      },
      getLangMenuOpen: () => langMenuOpen,
      setLangMenuOpen: (next) => {
        langMenuOpen = !!next;
      },
      getLotdGateBlockingToastShown: () => lotdGateBlockingToastShown,
      setLotdGateBlockingToastShown: (next) => {
        lotdGateBlockingToastShown = !!next;
      },
      getKeyNavRaf: () => keyNavRaf,
      setKeyNavRaf: (next) => {
        keyNavRaf = next;
      },
      getInputScale: () => inputScale,
      getInputScaleHasLocal: () => inputScaleHasLocal,
      loadInputScalePref,
      saveInputScalePref,
      syncInputScaleControls,
      setInputScale,
      getPerfMode: () => perfMode,
      setPerfMode: (next) => {
        perfMode = next;
      },
      loadPerfModePref,
      savePerfModePref,
      getEffectiveDpr,
      applyPerfModeFromPrefs,
      getUiScaleMode: () => uiScaleMode,
      setUiScaleMode: (next) => {
        uiScaleMode = next;
      },
      getUiScaleManual: () => uiScaleManual,
      setUiScaleManual: (next) => {
        uiScaleManual = next;
      },
      getCurrentUiScale,
      computeAutoUiScale,
      loadUiScalePrefs,
      saveUiScalePrefs,
      syncUiScaleControls,
      applyManualUiScale,
      applyUiScaleFromPrefs,
      applyAutoUiScale,
      scheduleAutoUiScale,
      showToast,
      showConfirm,
      getToggleKeyDik,
      formatToggleKeyLabel,
      formatToggleKeyDisplay,
      updateToggleKeyResolved,
      setToggleKeyInputFromDik,
      clamp,
      coalesce,
      toHex32,
    });
  }

  return {
    createUIState,
  };
});
