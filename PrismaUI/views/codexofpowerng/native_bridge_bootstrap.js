(function (global) {
  "use strict";

  function noop() {}

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function defaultCoalesce(value, fallback) {
    return value === undefined || value === null ? fallback : value;
  }

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
      return {
        page: pick(payload.page, 0) >>> 0,
        pageSize: pick(payload.pageSize, 200) >>> 0,
        total: pick(payload.total, 0) >>> 0,
        hasMore: !!payload.hasMore,
        items: Array.isArray(payload.items) ? payload.items : [],
      };
    }

    return { page: 0, pageSize: 200, total: 0, hasMore: false, items: [] };
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
    const onRewards = asFn(options.onRewards, noop);
    const onUndoList = asFn(options.onUndoList, noop);
    const onSettings = asFn(options.onSettings, noop);
    const onToast = asFn(options.onToast, noop);

    const prev = {
      setState: win.copng_setState,
      setInventory: win.copng_setInventory,
      setRegistered: win.copng_setRegistered,
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
