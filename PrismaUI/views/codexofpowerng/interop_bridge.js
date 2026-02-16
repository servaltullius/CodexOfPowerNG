(function (global) {
  "use strict";

  function noop() {}

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function defaultCoalesce(value, fallback) {
    return value === undefined || value === null ? fallback : value;
  }

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

  function normalizeArrayPayload(payload) {
    return Array.isArray(payload) ? payload : [];
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
    const onRewards = asFn(options.onRewards, noop);
    const onSettings = asFn(options.onSettings, noop);
    const onToast = asFn(options.onToast, noop);

    const prev = {
      setState: win.copng_setState,
      setInventory: win.copng_setInventory,
      setRegistered: win.copng_setRegistered,
      setRewards: win.copng_setRewards,
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

    win.copng_setRewards = (jsonStr) => {
      onRewards(parseJsonPayload(jsonStr, { totals: [] }));
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
      win.copng_setRewards = prev.setRewards;
      win.copng_setSettings = prev.setSettings;
      win.copng_toast = prev.toast;
    };
  }

  const api = Object.freeze({
    parseJsonPayload,
    normalizeInventoryPayload,
    normalizeToastPayload,
    installNativeCallbacks,
  });

  if (typeof module !== "undefined" && module && module.exports) {
    module.exports = api;
  }

  global.COPNGInteropBridge = api;
})(typeof window !== "undefined" ? window : globalThis);
