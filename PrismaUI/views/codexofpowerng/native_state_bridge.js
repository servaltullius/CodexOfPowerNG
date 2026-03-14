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

  const DEFAULT_BUILD_THEME_MAP = Object.freeze({
    attack: Object.freeze([
      Object.freeze({ id: "devastation", titleKey: "build.theme.attack.devastation", optionCount: 0 }),
      Object.freeze({ id: "precision", titleKey: "build.theme.attack.precision", optionCount: 0 }),
      Object.freeze({ id: "fury", titleKey: "build.theme.attack.fury", optionCount: 0 }),
    ]),
    defense: Object.freeze([
      Object.freeze({ id: "guard", titleKey: "build.theme.defense.guard", optionCount: 0 }),
      Object.freeze({ id: "bastion", titleKey: "build.theme.defense.bastion", optionCount: 0 }),
      Object.freeze({ id: "resistance", titleKey: "build.theme.defense.resistance", optionCount: 0 }),
    ]),
    utility: Object.freeze([
      Object.freeze({ id: "livelihood", titleKey: "build.theme.utility.livelihood", optionCount: 0 }),
      Object.freeze({ id: "exploration", titleKey: "build.theme.utility.exploration", optionCount: 0 }),
      Object.freeze({ id: "trickery", titleKey: "build.theme.utility.trickery", optionCount: 0 }),
    ]),
  });

  function normalizeRewardsPayload(nextRewards) {
    return nextRewards && typeof nextRewards === "object" ? nextRewards : { totals: [] };
  }

  function normalizeNumber(value, fallback) {
    const numeric = Number(value);
    return Number.isFinite(numeric) ? numeric : fallback;
  }

  function normalizeBuildOption(option) {
    const source = option && typeof option === "object" ? option : null;
    if (!source) return null;
    return Object.assign({}, source, {
      id: typeof source.id === "string" ? source.id : "",
      discipline: typeof source.discipline === "string" ? source.discipline : "",
      themeId: typeof source.themeId === "string" ? source.themeId : "",
      themeTitleKey: typeof source.themeTitleKey === "string" ? source.themeTitleKey : "",
      hierarchy: typeof source.hierarchy === "string" ? source.hierarchy : "standard",
      layer: typeof source.layer === "string" ? source.layer : "",
      unlockPoints: normalizeNumber(source.unlockPoints != null ? source.unlockPoints : source.unlockScore, 0),
      unlockScore: normalizeNumber(source.unlockScore != null ? source.unlockScore : source.unlockPoints, 0),
      unlocked: !!source.unlocked,
      state: typeof source.state === "string" ? source.state : "",
      slotCompatibility: typeof source.slotCompatibility === "string" ? source.slotCompatibility : "",
      compatibleSlots: Array.isArray(source.compatibleSlots)
        ? source.compatibleSlots.map((slotId) => String(slotId || "")).filter(Boolean)
        : [],
      activeSlotId:
        typeof source.activeSlotId === "string"
          ? source.activeSlotId
          : source.activeSlotId == null
            ? null
            : "",
      effectType: typeof source.effectType === "string" ? source.effectType : "",
      effectKey: typeof source.effectKey === "string" ? source.effectKey : "",
      magnitude: source.magnitude && typeof source.magnitude === "object" ? Object.assign({}, source.magnitude) : source.magnitude,
      baseMagnitude:
        source.baseMagnitude && typeof source.baseMagnitude === "object"
          ? Object.assign({}, source.baseMagnitude)
          : source.baseMagnitude,
      magnitudePerTier:
        source.magnitudePerTier && typeof source.magnitudePerTier === "object"
          ? Object.assign({}, source.magnitudePerTier)
          : source.magnitudePerTier,
      currentMagnitude:
        source.currentMagnitude && typeof source.currentMagnitude === "object"
          ? Object.assign({}, source.currentMagnitude)
          : source.currentMagnitude,
      nextMagnitude:
        source.nextMagnitude && typeof source.nextMagnitude === "object"
          ? Object.assign({}, source.nextMagnitude)
          : source.nextMagnitude,
      currentTier: Number(source.currentTier || 0) >>> 0,
      nextTierPoints: normalizeNumber(source.nextTierPoints != null ? source.nextTierPoints : source.nextTierScore, 0),
      pointsToNextTier: normalizeNumber(source.pointsToNextTier != null ? source.pointsToNextTier : source.scoreToNextTier, 0),
      nextTierScore: normalizeNumber(source.nextTierScore != null ? source.nextTierScore : source.nextTierPoints, 0),
      scoreToNextTier: normalizeNumber(source.scoreToNextTier != null ? source.scoreToNextTier : source.pointsToNextTier, 0),
      exclusivityGroup: typeof source.exclusivityGroup === "string" ? source.exclusivityGroup : "",
      titleKey: typeof source.titleKey === "string" ? source.titleKey : "",
      descriptionKey: typeof source.descriptionKey === "string" ? source.descriptionKey : "",
    });
  }

  function normalizeBuildTheme(theme) {
    const source = theme && typeof theme === "object" ? theme : {};
    return {
      id: typeof source.id === "string" ? source.id : "",
      titleKey: typeof source.titleKey === "string" ? source.titleKey : "",
      optionCount: Number(source.optionCount || 0) >>> 0,
      rows: Array.isArray(source.rows) ? source.rows.map(normalizeBuildOption).filter(Boolean) : [],
    };
  }

  function deriveGroupedCatalog(themeMap, options) {
    const result = {};
    for (const discipline of ["attack", "defense", "utility"]) {
      const themes = Array.isArray(themeMap[discipline]) ? themeMap[discipline] : [];
      result[discipline] = {
        discipline,
        themes: themes.map((theme) => {
          const themeId = String((theme && theme.id) || "");
          const rows = options.filter(
            (option) =>
              String((option && option.discipline) || "").toLowerCase() === discipline &&
              String((option && option.themeId) || "").toLowerCase() === themeId.toLowerCase(),
          );
          return {
            id: themeId,
            titleKey: String((theme && theme.titleKey) || ""),
            optionCount: Number((theme && theme.optionCount) || rows.length || 0) >>> 0,
            rows,
          };
        }),
      };
    }
    return result;
  }

  function normalizeBuildGroupedCatalog(groupedCatalog, themeMap, options) {
    const source = groupedCatalog && typeof groupedCatalog === "object" ? groupedCatalog : null;
    if (!source) return deriveGroupedCatalog(themeMap, options);

    const result = {};
    for (const discipline of ["attack", "defense", "utility"]) {
      const disciplineSource = source[discipline] && typeof source[discipline] === "object" ? source[discipline] : {};
      const themes = Array.isArray(disciplineSource.themes) ? disciplineSource.themes.map(normalizeBuildTheme) : null;
      result[discipline] = {
        discipline,
        themes: themes || deriveGroupedCatalog(themeMap, options)[discipline].themes,
      };
    }
    return result;
  }

  function normalizeBuildPayload(nextBuild) {
    const source = nextBuild && typeof nextBuild === "object" ? nextBuild : {};
    const disciplines = source.disciplines && typeof source.disciplines === "object" ? source.disciplines : {};
    const themeMap = source.themeMap && typeof source.themeMap === "object" ? source.themeMap : {};
    const bySlotId = new Map();
    if (Array.isArray(source.activeSlots)) {
      for (const slot of source.activeSlots) {
        if (!slot || typeof slot !== "object" || typeof slot.slotId !== "string") continue;
        bySlotId.set(slot.slotId, slot);
      }
    }

    const normalizedThemeMap = {
      attack: Array.isArray(themeMap.attack) ? themeMap.attack : DEFAULT_BUILD_THEME_MAP.attack.slice(),
      defense: Array.isArray(themeMap.defense) ? themeMap.defense : DEFAULT_BUILD_THEME_MAP.defense.slice(),
      utility: Array.isArray(themeMap.utility) ? themeMap.utility : DEFAULT_BUILD_THEME_MAP.utility.slice(),
    };

    const normalizedOptions = Array.isArray(source.options)
      ? source.options.map(normalizeBuildOption).filter(Boolean)
      : [];
    const normalizedGroupedCatalog = normalizeBuildGroupedCatalog(
      source.groupedCatalog,
      normalizedThemeMap,
      normalizedOptions,
    );
    const payloadSelectedDiscipline =
      typeof source.selectedDiscipline === "string" ? source.selectedDiscipline : "";
    const payloadSelectedTheme = typeof source.selectedTheme === "string" ? source.selectedTheme : "";
    const derivedSelectedRows =
      normalizedGroupedCatalog[String(payloadSelectedDiscipline || "").toLowerCase()] &&
      Array.isArray(normalizedGroupedCatalog[String(payloadSelectedDiscipline || "").toLowerCase()].themes)
        ? normalizedGroupedCatalog[String(payloadSelectedDiscipline || "").toLowerCase()].themes.find(
            (theme) => String((theme && theme.id) || "").toLowerCase() === String(payloadSelectedTheme || "").toLowerCase(),
          )
        : null;

    return {
      disciplines: {
        attack: {
          score: Number((disciplines.attack && disciplines.attack.score) || 0) >>> 0,
          recordCount: Number((disciplines.attack && (disciplines.attack.recordCount != null ? disciplines.attack.recordCount : disciplines.attack.score)) || 0) >>> 0,
          buildPoints: normalizeNumber(disciplines.attack && (disciplines.attack.buildPoints != null ? disciplines.attack.buildPoints : disciplines.attack.score), 0),
          currentTier: Number((disciplines.attack && disciplines.attack.currentTier) || 0) >>> 0,
          nextTierPoints: normalizeNumber(disciplines.attack && (disciplines.attack.nextTierPoints != null ? disciplines.attack.nextTierPoints : disciplines.attack.nextTierScore), 0),
          pointsToNextTier: normalizeNumber(disciplines.attack && (disciplines.attack.pointsToNextTier != null ? disciplines.attack.pointsToNextTier : disciplines.attack.scoreToNextTier), 0),
          nextTierScore: normalizeNumber(disciplines.attack && (disciplines.attack.nextTierScore != null ? disciplines.attack.nextTierScore : disciplines.attack.nextTierPoints), 0),
          scoreToNextTier: normalizeNumber(disciplines.attack && (disciplines.attack.scoreToNextTier != null ? disciplines.attack.scoreToNextTier : disciplines.attack.pointsToNextTier), 0),
        },
        defense: {
          score: Number((disciplines.defense && disciplines.defense.score) || 0) >>> 0,
          recordCount: Number((disciplines.defense && (disciplines.defense.recordCount != null ? disciplines.defense.recordCount : disciplines.defense.score)) || 0) >>> 0,
          buildPoints: normalizeNumber(disciplines.defense && (disciplines.defense.buildPoints != null ? disciplines.defense.buildPoints : disciplines.defense.score), 0),
          currentTier: Number((disciplines.defense && disciplines.defense.currentTier) || 0) >>> 0,
          nextTierPoints: normalizeNumber(disciplines.defense && (disciplines.defense.nextTierPoints != null ? disciplines.defense.nextTierPoints : disciplines.defense.nextTierScore), 0),
          pointsToNextTier: normalizeNumber(disciplines.defense && (disciplines.defense.pointsToNextTier != null ? disciplines.defense.pointsToNextTier : disciplines.defense.scoreToNextTier), 0),
          nextTierScore: normalizeNumber(disciplines.defense && (disciplines.defense.nextTierScore != null ? disciplines.defense.nextTierScore : disciplines.defense.nextTierPoints), 0),
          scoreToNextTier: normalizeNumber(disciplines.defense && (disciplines.defense.scoreToNextTier != null ? disciplines.defense.scoreToNextTier : disciplines.defense.pointsToNextTier), 0),
        },
        utility: {
          score: Number((disciplines.utility && disciplines.utility.score) || 0) >>> 0,
          recordCount: Number((disciplines.utility && (disciplines.utility.recordCount != null ? disciplines.utility.recordCount : disciplines.utility.score)) || 0) >>> 0,
          buildPoints: normalizeNumber(disciplines.utility && (disciplines.utility.buildPoints != null ? disciplines.utility.buildPoints : disciplines.utility.score), 0),
          currentTier: Number((disciplines.utility && disciplines.utility.currentTier) || 0) >>> 0,
          nextTierPoints: normalizeNumber(disciplines.utility && (disciplines.utility.nextTierPoints != null ? disciplines.utility.nextTierPoints : disciplines.utility.nextTierScore), 0),
          pointsToNextTier: normalizeNumber(disciplines.utility && (disciplines.utility.pointsToNextTier != null ? disciplines.utility.pointsToNextTier : disciplines.utility.scoreToNextTier), 0),
          nextTierScore: normalizeNumber(disciplines.utility && (disciplines.utility.nextTierScore != null ? disciplines.utility.nextTierScore : disciplines.utility.nextTierPoints), 0),
          scoreToNextTier: normalizeNumber(disciplines.utility && (disciplines.utility.scoreToNextTier != null ? disciplines.utility.scoreToNextTier : disciplines.utility.pointsToNextTier), 0),
        },
      },
      themeMap: normalizedThemeMap,
      groupedCatalog: normalizedGroupedCatalog,
      selectedDiscipline: payloadSelectedDiscipline,
      selectedTheme: payloadSelectedTheme,
      selectedOptionId: typeof source.selectedOptionId === "string" ? source.selectedOptionId : "",
      selectedThemeRows: Array.isArray(source.selectedThemeRows)
        ? source.selectedThemeRows.map(normalizeBuildOption).filter(Boolean)
        : derivedSelectedRows && Array.isArray(derivedSelectedRows.rows)
          ? derivedSelectedRows.rows.slice()
          : [],
      selectedOptionDetail: normalizeBuildOption(source.selectedOptionDetail),
      options: normalizedOptions,
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
