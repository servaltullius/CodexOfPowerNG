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

  function normalizeBuildPayload(payload) {
    const source = payload && typeof payload === "object" ? payload : {};
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
