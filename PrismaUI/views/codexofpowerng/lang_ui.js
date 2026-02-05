(function (global) {
  "use strict";

  const VALUES = Object.freeze(["auto", "en", "ko"]);

  function normalizeLanguageValue(raw) {
    const s = String(raw || "")
      .trim()
      .toLowerCase();
    return VALUES.includes(s) ? s : "auto";
  }

  function getLanguageItems(t) {
    const tt = typeof t === "function" ? t : (key, fallback) => (fallback !== undefined ? fallback : key);
    return VALUES.map((value) => ({ value, label: tt("lang." + value, value) }));
  }

  const api = Object.freeze({
    normalizeLanguageValue,
    getLanguageItems,
  });

  // Node/CommonJS
  if (typeof module !== "undefined" && module && module.exports) {
    module.exports = api;
  }

  // Browser/Ultralight
  global.COPNG_LANGUI = api;
})(typeof window !== "undefined" ? window : globalThis);

