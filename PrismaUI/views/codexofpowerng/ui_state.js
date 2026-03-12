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

  function createDefaultBuildState() {
    return {
      disciplines: {
        attack: { score: 0, unlockedBaselineCount: 0 },
        defense: { score: 0, unlockedBaselineCount: 0 },
        utility: { score: 0, unlockedBaselineCount: 0 },
      },
      themeMap: {
        attack: [],
        defense: [],
        utility: [],
      },
      groupedCatalog: {
        attack: { discipline: "attack", themes: [] },
        defense: { discipline: "defense", themes: [] },
        utility: { discipline: "utility", themes: [] },
      },
      selectedDiscipline: "",
      selectedTheme: "",
      selectedOptionId: "",
      selectedThemeRows: [],
      selectedOptionDetail: null,
      options: [],
      activeSlots: [],
      migrationNotice: {
        needsNotice: false,
        legacyRewardsMigrated: false,
        unresolvedHistoricalRegistrations: 0,
      },
    };
  }

  function normalizeBuildDisciplineId(value) {
    const normalized = String(value || "").toLowerCase();
    if (normalized === "attack" || normalized === "defense" || normalized === "utility") return normalized;
    return "";
  }

  function inferBuildThemeId(option) {
    const explicit = String((option && option.themeId) || "").toLowerCase();
    if (explicit) return explicit;

    const id = String((option && option.id) || "").toLowerCase();
    if (id.indexOf(".attack.") !== -1) {
      if (id.indexOf("precision") !== -1 || id.indexOf("vitals") !== -1) return "precision";
      if (id.indexOf("ferocity") !== -1) return "devastation";
      return "fury";
    }
    if (id.indexOf(".defense.") !== -1) {
      if (id.indexOf("guard") !== -1 || id.indexOf("endurance") !== -1) return "guard";
      if (id.indexOf("bastion") !== -1) return "bastion";
      return "resistance";
    }
    if (id.indexOf(".utility.") !== -1) {
      if (id.indexOf("cache") !== -1 || id.indexOf("barter") !== -1) return "livelihood";
      if (id.indexOf("mobility") !== -1) return "exploration";
      return "trickery";
    }
    return "";
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
    let quickBatchSelectedIds = [];
    let quickActionableOnly = false;
    const quickVirtual = { rows: [], lastStart: -1, lastEnd: -1, tbodyTopPx: NaN, rowHeightPx: 0 };
    let registered = [];
    let undoItems = [];
    const regVirtual = { rows: [], lastStart: -1, lastEnd: -1, tbodyTopPx: NaN, rowHeightPx: 0 };
    let rewards = { totals: [] };
    let build = createDefaultBuildState();
    let buildSelection = { discipline: "", theme: "", optionId: "" };
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

    function getBuildThemeMap(nextBuild) {
      const source = nextBuild && typeof nextBuild === "object" ? nextBuild : build;
      const explicitMap = source && source.themeMap && typeof source.themeMap === "object" ? source.themeMap : null;
      if (explicitMap) return explicitMap;

      const derived = {
        attack: [],
        defense: [],
        utility: [],
      };
      const options = Array.isArray(source && source.options) ? source.options : [];
      for (const option of options) {
        const discipline = normalizeBuildDisciplineId(option && option.discipline);
        const themeId = inferBuildThemeId(option);
        if (!discipline || !themeId) continue;
        const existing = derived[discipline].find((theme) => theme && theme.id === themeId);
        if (existing) {
          existing.optionCount = Number(existing.optionCount || 0) + 1;
          continue;
        }
        derived[discipline].push({
          id: themeId,
          titleKey: String((option && option.themeTitleKey) || ""),
          optionCount: 1,
        });
      }
      return derived;
    }

    function getBuildGroupedCatalog(nextBuild) {
      const source = nextBuild && typeof nextBuild === "object" ? nextBuild : build;
      const explicitCatalog =
        source && source.groupedCatalog && typeof source.groupedCatalog === "object" ? source.groupedCatalog : null;
      if (explicitCatalog) return explicitCatalog;

      const themeMap = getBuildThemeMap(source);
      const options = Array.isArray(source && source.options) ? source.options : [];
      const derived = {
        attack: { discipline: "attack", themes: [] },
        defense: { discipline: "defense", themes: [] },
        utility: { discipline: "utility", themes: [] },
      };

      for (const discipline of ["attack", "defense", "utility"]) {
        const themes = Array.isArray(themeMap[discipline]) ? themeMap[discipline] : [];
        derived[discipline].themes = themes.map((theme) => {
          const themeId = String((theme && theme.id) || "").toLowerCase();
          const rows = options.filter((option) => {
            if (normalizeBuildDisciplineId(option && option.discipline) !== discipline) return false;
            return !themeId || inferBuildThemeId(option) === themeId;
          });
          return {
            id: themeId,
            titleKey: String((theme && theme.titleKey) || ""),
            optionCount: Number((theme && theme.optionCount) || rows.length || 0) >>> 0,
            rows,
          };
        });
      }

      return derived;
    }

    function getBuildThemesForDiscipline(nextDiscipline, nextBuild) {
      const discipline = normalizeBuildDisciplineId(nextDiscipline);
      if (!discipline) return [];
      const groupedCatalog = getBuildGroupedCatalog(nextBuild);
      const groupedRows =
        groupedCatalog &&
        groupedCatalog[discipline] &&
        Array.isArray(groupedCatalog[discipline].themes)
          ? groupedCatalog[discipline].themes
          : null;
      const rows = groupedRows || (Array.isArray(getBuildThemeMap(nextBuild)[discipline]) ? getBuildThemeMap(nextBuild)[discipline] : []);
      return rows
        .map((row) => ({
          id: String((row && row.id) || "").toLowerCase(),
          titleKey: String((row && row.titleKey) || ""),
          optionCount:
            Number((row && row.optionCount) || (Array.isArray(row && row.rows) ? row.rows.length : 0) || 0) >>> 0,
        }))
        .filter((row) => row.id);
    }

    function getBuildRowsForSelection(selection, nextBuild) {
      const source = nextBuild && typeof nextBuild === "object" ? nextBuild : build;
      const selected = selection && typeof selection === "object" ? selection : buildSelection;
      const discipline = normalizeBuildDisciplineId(selected && selected.discipline);
      const themeId = String((selected && selected.theme) || "").toLowerCase();
      const payloadDiscipline = normalizeBuildDisciplineId(source && source.selectedDiscipline);
      const payloadThemeId = String((source && source.selectedTheme) || "").toLowerCase();
      if (
        discipline &&
        discipline === payloadDiscipline &&
        themeId &&
        themeId === payloadThemeId &&
        Array.isArray(source && source.selectedThemeRows) &&
        source.selectedThemeRows.length
      ) {
        return source.selectedThemeRows.slice();
      }

      const groupedCatalog = getBuildGroupedCatalog(source);
      const groupedThemes =
        groupedCatalog &&
        groupedCatalog[discipline] &&
        Array.isArray(groupedCatalog[discipline].themes)
          ? groupedCatalog[discipline].themes
          : [];
      if (groupedThemes.length) {
        if (!themeId) {
          return groupedThemes.flatMap((theme) => (Array.isArray(theme && theme.rows) ? theme.rows : []));
        }
        const selectedTheme = groupedThemes.find((theme) => String((theme && theme.id) || "").toLowerCase() === themeId);
        if (selectedTheme && Array.isArray(selectedTheme.rows)) {
          return selectedTheme.rows.slice();
        }
      }

      const rows = Array.isArray(source && source.options) ? source.options : [];
      return rows.filter((option) => {
        const optionDiscipline = normalizeBuildDisciplineId(option && option.discipline);
        if (!discipline || optionDiscipline !== discipline) return false;
        if (!themeId) return true;
        return inferBuildThemeId(option) === themeId;
      });
    }

    function ensureBuildSelection() {
      const disciplines = ["attack", "defense", "utility"];
      const themeMap = getBuildThemeMap(build);
      const options = Array.isArray(build && build.options) ? build.options : [];
      let discipline = normalizeBuildDisciplineId(buildSelection.discipline || build.selectedDiscipline);
      if (!discipline) {
        discipline =
          disciplines.find((candidate) => getBuildThemesForDiscipline(candidate, build).length > 0) ||
          disciplines.find((candidate) =>
            options.some((option) => normalizeBuildDisciplineId(option && option.discipline) === candidate),
          ) ||
          "attack";
      }

      const themes = getBuildThemesForDiscipline(discipline, build);
      let theme = String(buildSelection.theme || build.selectedTheme || "").toLowerCase();
      if (!themes.some((entry) => entry.id === theme)) {
        const nonEmptyTheme = themes.find((entry) => Number(entry.optionCount || 0) > 0);
        theme = String((nonEmptyTheme || themes[0] || {}).id || "");
      }

      const filteredRows = getBuildRowsForSelection({ discipline, theme }, build);
      let optionId = String(buildSelection.optionId || build.selectedOptionId || "");
      if (!filteredRows.some((row) => String((row && row.id) || "") === optionId)) {
        const activeRow = filteredRows.find((row) =>
          Array.isArray(build.activeSlots) &&
          build.activeSlots.some((slot) => String((slot && slot.optionId) || "") === String((row && row.id) || "")),
        );
        const unlockedRow = filteredRows.find((row) => !!(row && row.unlocked));
        optionId = String(((activeRow || unlockedRow || filteredRows[0] || {}).id) || "");
      }

      buildSelection = { discipline, theme, optionId };
      return buildSelection;
    }

    function setBuildSelection(next) {
      const patch = next && typeof next === "object" ? next : {};
      buildSelection = {
        discipline:
          patch.discipline === ""
            ? ""
            : normalizeBuildDisciplineId(Object.prototype.hasOwnProperty.call(patch, "discipline") ? patch.discipline : buildSelection.discipline) ||
              buildSelection.discipline,
        theme:
          patch.theme === undefined
            ? buildSelection.theme
            : String(patch.theme || "").toLowerCase(),
        optionId:
          patch.optionId === undefined
            ? buildSelection.optionId
            : String(patch.optionId || ""),
      };
      return ensureBuildSelection();
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
      getQuickBatchSelectedIds: () => quickBatchSelectedIds,
      setQuickBatchSelectedIds: (next) => {
        quickBatchSelectedIds = Array.isArray(next) ? next.map((id) => Number(id) >>> 0).filter((id) => id > 0) : [];
      },
      getQuickActionableOnly: () => quickActionableOnly,
      setQuickActionableOnly: (next) => {
        quickActionableOnly = !!next;
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
      getBuild: () => build,
      setBuild: (next) => {
        build = next || createDefaultBuildState();
        ensureBuildSelection();
      },
      getBuildSelection: () => ensureBuildSelection(),
      setBuildSelection,
      getBuildSelectedDiscipline: () => ensureBuildSelection().discipline,
      getBuildSelectedTheme: () => ensureBuildSelection().theme,
      getBuildSelectedOptionId: () => ensureBuildSelection().optionId,
      getBuildThemeMap: () => getBuildThemeMap(build),
      getBuildGroupedCatalog: () => getBuildGroupedCatalog(build),
      getBuildThemesForDiscipline: (discipline) => getBuildThemesForDiscipline(discipline, build),
      getBuildRowsForSelection: (selection) => getBuildRowsForSelection(selection, build),
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
