(function (global) {
  "use strict";

  function noop() {}

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function defaultCoalesce(value, fallback) {
    return value === undefined || value === null ? fallback : value;
  }

  function normalizeNumber(value, fallback) {
    const numeric = Number(value);
    return Number.isFinite(numeric) ? numeric : fallback;
  }

  const DEFAULT_BUILD_SLOT_LAYOUT = Object.freeze([
    Object.freeze({ slotId: "attack_1", slotKind: "attack", optionId: null, occupied: false }),
    Object.freeze({ slotId: "attack_2", slotKind: "attack", optionId: null, occupied: false }),
    Object.freeze({ slotId: "defense_1", slotKind: "defense", optionId: null, occupied: false }),
    Object.freeze({ slotId: "utility_1", slotKind: "utility", optionId: null, occupied: false }),
    Object.freeze({ slotId: "utility_2", slotKind: "utility", optionId: null, occupied: false }),
    Object.freeze({ slotId: "wildcard_1", slotKind: "wildcard", optionId: null, occupied: false }),
  ]);

  function parseJsonPayload(rawPayload, fallback) {
    try {
      return typeof rawPayload === "string" ? JSON.parse(rawPayload) : rawPayload;
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
      options: Array.isArray(source.options)
        ? source.options.map((option) =>
            option && typeof option === "object"
              ? Object.assign({}, option, {
                  currentTier: Number(option.currentTier || 0) >>> 0,
                  unlockPoints: normalizeNumber(option.unlockPoints != null ? option.unlockPoints : option.unlockScore, 0),
                  unlockScore: normalizeNumber(option.unlockScore != null ? option.unlockScore : option.unlockPoints, 0),
                  nextTierPoints: normalizeNumber(option.nextTierPoints != null ? option.nextTierPoints : option.nextTierScore, 0),
                  pointsToNextTier: normalizeNumber(option.pointsToNextTier != null ? option.pointsToNextTier : option.scoreToNextTier, 0),
                  nextTierScore: normalizeNumber(option.nextTierScore != null ? option.nextTierScore : option.nextTierPoints, 0),
                  scoreToNextTier: normalizeNumber(option.scoreToNextTier != null ? option.scoreToNextTier : option.pointsToNextTier, 0),
                })
              : option,
          )
        : [],
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

  function installFallbackNativeCallbacks(opts) {
    const options = opts || {};
    const win = options.windowObj || global;
    if (!win) return noop;

    const coalesce = asFn(options.coalesce, defaultCoalesce);
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

    win.copng_setState = function (jsonStr) {
      onState(parseJsonPayload(jsonStr, {}) || {});
    };

    win.copng_setInventory = function (jsonStr) {
      onInventory(normalizeInventoryPayload(parseJsonPayload(jsonStr, null), coalesce));
    };

    win.copng_setRegistered = function (jsonStr) {
      const payload = parseJsonPayload(jsonStr, []);
      onRegistered(Array.isArray(payload) ? payload : []);
    };

    win.copng_setBuild = function (jsonStr) {
      onBuild(normalizeBuildPayload(parseJsonPayload(jsonStr, null)));
    };

    win.copng_setRewards = function (jsonStr) {
      onRewards(parseJsonPayload(jsonStr, { totals: [] }));
    };

    win.copng_setUndoList = function (jsonStr) {
      const payload = parseJsonPayload(jsonStr, []);
      onUndoList(Array.isArray(payload) ? payload : []);
    };

    win.copng_setSettings = function (jsonStr) {
      onSettings(parseJsonPayload(jsonStr, null));
    };

    win.copng_toast = function (jsonStr) {
      onToast(normalizeToastPayload(jsonStr));
    };

    return function detachFallbackNativeCallbacks() {
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

  function installNativeBridge(opts) {
    const options = opts || {};
    const bridgeApi = options.interopBridgeApi || (global && global.COPNGInteropBridge) || null;
    if (bridgeApi && typeof bridgeApi.installNativeCallbacks === "function") {
      return bridgeApi.installNativeCallbacks(options);
    }
    return installFallbackNativeCallbacks(options);
  }

  const api = Object.freeze({
    installFallbackNativeCallbacks: installFallbackNativeCallbacks,
    installNativeBridge: installNativeBridge,
  });

  if (typeof module !== "undefined" && module && module.exports) {
    module.exports = api;
  }

  global.COPNGNativeBridgeBootstrap = api;
})(typeof window !== "undefined" ? window : globalThis);
