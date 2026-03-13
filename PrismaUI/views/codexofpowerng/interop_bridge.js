(function (global) {
  "use strict";

  function noop() {}

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function defaultCoalesce(value, fallback) {
    return value === undefined || value === null ? fallback : value;
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

  function parseJsonPayload(jsonStr, fallback) {
    try {
      return typeof jsonStr === "string" ? JSON.parse(jsonStr) : jsonStr;
    } catch {
      return fallback;
    }
  }

  function normalizeInventoryPayload(payload, coalesce) {
    const pick = asFn(coalesce, defaultCoalesce);
    if (Array.isArray(payload)) {
      return {
        page: 0,
        pageSize: payload.length,
        total: payload.length,
        hasMore: false,
        items: payload,
      };
    }

    if (payload && typeof payload === "object") {
      const normalized = {
        page: pick(payload.page, 0) >>> 0,
        pageSize: pick(payload.pageSize, 200) >>> 0,
        total: pick(payload.total, 0) >>> 0,
        hasMore: !!payload.hasMore,
        items: Array.isArray(payload.items) ? payload.items : [],
      };
      if (Array.isArray(payload.sections)) {
        normalized.sections = payload.sections;
      }
      return normalized;
    }

    return { page: 0, pageSize: 200, total: 0, hasMore: false, items: [] };
  }

  function normalizeArrayPayload(payload) {
    return Array.isArray(payload) ? payload : [];
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
      unlockScore: Number(source.unlockScore || 0) >>> 0,
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
      nextTierScore: Number(source.nextTierScore || 0) >>> 0,
      scoreToNextTier: Number(source.scoreToNextTier || 0) >>> 0,
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

  function normalizeBuildPayload(payload) {
    const source = payload && typeof payload === "object" ? payload : {};
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
          currentTier: Number((disciplines.attack && disciplines.attack.currentTier) || 0) >>> 0,
          nextTierScore: Number((disciplines.attack && disciplines.attack.nextTierScore) || 0) >>> 0,
          scoreToNextTier: Number((disciplines.attack && disciplines.attack.scoreToNextTier) || 0) >>> 0,
        },
        defense: {
          score: Number((disciplines.defense && disciplines.defense.score) || 0) >>> 0,
          currentTier: Number((disciplines.defense && disciplines.defense.currentTier) || 0) >>> 0,
          nextTierScore: Number((disciplines.defense && disciplines.defense.nextTierScore) || 0) >>> 0,
          scoreToNextTier: Number((disciplines.defense && disciplines.defense.scoreToNextTier) || 0) >>> 0,
        },
        utility: {
          score: Number((disciplines.utility && disciplines.utility.score) || 0) >>> 0,
          currentTier: Number((disciplines.utility && disciplines.utility.currentTier) || 0) >>> 0,
          nextTierScore: Number((disciplines.utility && disciplines.utility.nextTierScore) || 0) >>> 0,
          scoreToNextTier: Number((disciplines.utility && disciplines.utility.scoreToNextTier) || 0) >>> 0,
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

  function normalizeToastPayload(rawPayload) {
    try {
      const obj = typeof rawPayload === "string" ? JSON.parse(rawPayload) : rawPayload;
      if (obj && typeof obj === "object") {
        return {
          level: obj.level || "info",
          message: obj.message || "",
        };
      }
    } catch {}

    return {
      level: "info",
      message: String(rawPayload || ""),
    };
  }

  function installNativeCallbacks(opts) {
    const options = opts || {};
    const win = options.windowObj || global;
    if (!win) return noop;

    const pick = asFn(options.coalesce, defaultCoalesce);
    const onState = asFn(options.onState, noop);
    const onInventory = asFn(options.onInventory, noop);
    const onRegistered = asFn(options.onRegistered, noop);
    const onBuild = asFn(options.onBuild, noop);
    const onRewards = asFn(options.onRewards, noop);
    const onUndoList = asFn(options.onUndoList, noop);
    const onSettings = asFn(options.onSettings, noop);
    const onToast = asFn(options.onToast, noop);

    const prev = {
      setState: win.copng_setState,
      setInventory: win.copng_setInventory,
      setRegistered: win.copng_setRegistered,
      setBuild: win.copng_setBuild,
      setRewards: win.copng_setRewards,
      setUndoList: win.copng_setUndoList,
      setSettings: win.copng_setSettings,
      toast: win.copng_toast,
    };

    win.copng_setState = (jsonStr) => {
      onState(parseJsonPayload(jsonStr, {}));
    };

    win.copng_setInventory = (jsonStr) => {
      const payload = parseJsonPayload(jsonStr, []);
      onInventory(normalizeInventoryPayload(payload, pick));
    };

    win.copng_setRegistered = (jsonStr) => {
      const payload = parseJsonPayload(jsonStr, []);
      onRegistered(normalizeArrayPayload(payload));
    };

    win.copng_setBuild = (jsonStr) => {
      onBuild(normalizeBuildPayload(parseJsonPayload(jsonStr, null)));
    };

    win.copng_setRewards = (jsonStr) => {
      onRewards(parseJsonPayload(jsonStr, { totals: [] }));
    };

    win.copng_setUndoList = (jsonStr) => {
      const payload = parseJsonPayload(jsonStr, []);
      onUndoList(normalizeArrayPayload(payload));
    };

    win.copng_setSettings = (jsonStr) => {
      onSettings(parseJsonPayload(jsonStr, null));
    };

    win.copng_toast = (jsonStr) => {
      onToast(normalizeToastPayload(jsonStr));
    };

    return function detachNativeCallbacks() {
      win.copng_setState = prev.setState;
      win.copng_setInventory = prev.setInventory;
      win.copng_setRegistered = prev.setRegistered;
      win.copng_setBuild = prev.setBuild;
      win.copng_setRewards = prev.setRewards;
      win.copng_setUndoList = prev.setUndoList;
      win.copng_setSettings = prev.setSettings;
      win.copng_toast = prev.toast;
    };
  }

  const api = Object.freeze({
    parseJsonPayload,
    normalizeInventoryPayload,
    normalizeBuildPayload,
    normalizeToastPayload,
    installNativeCallbacks,
  });

  if (typeof module !== "undefined" && module && module.exports) {
    module.exports = api;
  }

  global.COPNGInteropBridge = api;
})(typeof window !== "undefined" ? window : globalThis);
