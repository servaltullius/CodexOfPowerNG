(function (global) {
  "use strict";

  function noop() {}

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function isAlnumToggleName(name) {
    return /^[A-Z0-9]$/.test(String(name || ""));
  }

  function isTypingTarget(target) {
    const tag = (target && target.tagName) || "";
    return tag === "INPUT" || tag === "TEXTAREA" || tag === "SELECT" || !!(target && target.isContentEditable);
  }

  function resolveEventDik(KC, code, key, keyCode) {
    let eventDik = null;

    if (KC && KC.jsCodeToDik) {
      eventDik = KC.jsCodeToDik(code);
      if (eventDik == null) eventDik = KC.jsCodeToDik(key);
    }
    if (eventDik == null && KC && KC.jsKeyCodeToDik) {
      eventDik = KC.jsKeyCodeToDik(keyCode);
    }
    if (eventDik == null && KC && KC.parseKeybindInput) {
      eventDik = KC.parseKeybindInput(key);
    }

    return eventDik;
  }

  function createKeydownHandler(opts) {
    const options = opts || {};
    const KC = options.KC || null;
    const getToggleKeyDik = asFn(options.getToggleKeyDik, () => 0x3e);
    const t = asFn(options.t, (_key, fallback) => fallback);
    const showToast = asFn(options.showToast, noop);
    const setToggleKeyInputFromDik = asFn(options.setToggleKeyInputFromDik, noop);
    const formatToggleKeyDisplay = asFn(options.formatToggleKeyDisplay, (dik) => String(dik));
    const isLangMenuOpen = asFn(options.isLangMenuOpen, () => false);
    const closeLangMenu = asFn(options.closeLangMenu, noop);
    const safeCall = asFn(options.safeCall, noop);
    const setTab = asFn(options.setTab, noop);
    const setInputScale = asFn(options.setInputScale, noop);
    const getInputScale = asFn(options.getInputScale, () => 1.0);
    const getCaptureToggleKey = asFn(options.getCaptureToggleKey, () => false);
    const setCaptureToggleKey = asFn(options.setCaptureToggleKey, noop);
    const getQuickVisibleIds = asFn(options.getQuickVisibleIds, () => []);
    const getQuickSelectedId = asFn(options.getQuickSelectedId, () => 0);
    const setQuickSelected = asFn(options.setQuickSelected, noop);
    const scrollQuickIndexIntoView = asFn(options.scrollQuickIndexIntoView, noop);
    const scheduleVirtualRender = asFn(options.scheduleVirtualRender, noop);
    const getKeyNavRaf = asFn(options.getKeyNavRaf, () => 0);
    const setKeyNavRaf = asFn(options.setKeyNavRaf, noop);
    const toHex32 = asFn(options.toHex32, (v) => String(v >>> 0));

    const doc = options.documentObj || (global && global.document) || null;
    const raf =
      asFn(
        options.requestAnimationFrameFn,
        global && typeof global.requestAnimationFrame === "function" ? global.requestAnimationFrame.bind(global) : null
      ) ||
      function (cb) {
        return setTimeout(cb, 0);
      };

    return function onKeydown(e) {
      const key = String((e && e.key) || "");
      const code = String((e && e.code) || "");
      const keyCode = Number((e && (e.keyCode || e.which)) || 0) || 0;
      const target = e && e.target ? e.target : null;

      const isTyping = isTypingTarget(target);

      const isEscape = key === "Escape" || code === "Escape" || keyCode === 27;
      const isEnter = key === "Enter" || code === "Enter" || keyCode === 13;
      const isUp = key === "ArrowUp" || code === "ArrowUp" || keyCode === 38;
      const isDown = key === "ArrowDown" || code === "ArrowDown" || keyCode === 40;
      const isBracketLeft = key === "[" || code === "BracketLeft";
      const isBracketRight = key === "]" || code === "BracketRight";
      const isZero = key === "0" || code === "Digit0";
      const isCtrlS = !!(e && (e.ctrlKey || e.metaKey)) && (key.toLowerCase() === "s" || code === "KeyS" || keyCode === 83);

      const eventDik = resolveEventDik(KC, code, key, keyCode);
      const toggleDik = getToggleKeyDik();
      const toggleName = KC && KC.dikToKeyName ? KC.dikToKeyName(toggleDik) : "";
      const toggleIsAlnum = isAlnumToggleName(toggleName);
      const isToggle = eventDik != null && ((eventDik >>> 0) & 0xff) === ((toggleDik >>> 0) & 0xff);

      if (getCaptureToggleKey()) {
        if (e && e.preventDefault) e.preventDefault();
        if (isEscape) {
          setCaptureToggleKey(false);
          showToast("info", t("toast.bindKey", "Press a key to bind (Esc cancels)."));
          return;
        }
        if (eventDik != null) {
          setToggleKeyInputFromDik(eventDik);
          setCaptureToggleKey(false);
          showToast("info", `${t("toast.bindKeySet", "Hotkey set")}: ${formatToggleKeyDisplay(eventDik)}`);
          return;
        }
        showToast("warn", t("toast.bindKeyUnknown", "Unsupported key."));
        return;
      }

      if (isLangMenuOpen() && isEscape) {
        if (e && e.preventDefault) e.preventDefault();
        closeLangMenu();
        return;
      }

      if (isEscape || (isToggle && !(isTyping && toggleIsAlnum))) {
        if (e && e.preventDefault) e.preventDefault();
        safeCall("copng_requestToggle", {});
        return;
      }

      const activeSection = doc && doc.querySelector ? doc.querySelector(".section.active") : null;
      const isQuickTab = !!(activeSection && activeSection.id === "tabQuick");

      if (!isTyping && isCtrlS) {
        if (e && e.preventDefault) e.preventDefault();
        const btn = doc && doc.getElementById ? doc.getElementById("btnSaveSettings") : null;
        if (btn && btn.click) btn.click();
        return;
      }

      if (!isTyping && (isBracketLeft || isBracketRight)) {
        if (e && e.preventDefault) e.preventDefault();
        const delta = isBracketRight ? 0.05 : -0.05;
        setInputScale(getInputScale() + delta, { persist: true, toast: true });
        return;
      }

      if (!isTyping && isZero) {
        if (e && e.preventDefault) e.preventDefault();
        setInputScale(1.0, { persist: true, toast: true });
        return;
      }

      if (isTyping) return;

      if (key === "1" || code === "Digit1") return setTab("tabQuick");
      if (key === "2" || code === "Digit2") return setTab("tabRegistered");
      if (key === "3" || code === "Digit3") return setTab("tabRewards");
      if (key === "4" || code === "Digit4") return setTab("tabSettings");

      if (!isQuickTab) return;

      const quickVisibleIds = getQuickVisibleIds() || [];

      if ((isUp || isDown) && quickVisibleIds.length) {
        if (e && e.preventDefault) e.preventDefault();
        if (getKeyNavRaf()) return;

        const direction = isUp ? -1 : 1;
        const rafId = raf(() => {
          setKeyNavRaf(0);
          const ids = getQuickVisibleIds() || [];
          if (!ids.length) return;

          const selectedId = getQuickSelectedId() >>> 0;
          const idx = ids.indexOf(selectedId);
          const base = idx >= 0 ? idx : 0;
          const nextIdx = direction < 0 ? (base <= 0 ? ids.length - 1 : base - 1) : (base + 1 >= ids.length ? 0 : base + 1);

          setQuickSelected(ids[nextIdx]);
          scrollQuickIndexIntoView(nextIdx);
          scheduleVirtualRender();
        });
        setKeyNavRaf(rafId || 1);
        return;
      }

      const quickSelectedId = getQuickSelectedId() >>> 0;
      if (isEnter && quickSelectedId) {
        if (e && e.preventDefault) e.preventDefault();
        safeCall("copng_log", { level: "info", message: `UI register enter: ${toHex32(quickSelectedId >>> 0)}` });
        safeCall("copng_registerItem", { formId: quickSelectedId >>> 0 });
      }
    };
  }

  function installKeydownHandler(opts) {
    const options = opts || {};
    const win = options.windowObj || global;
    if (!win || typeof win.addEventListener !== "function") return noop;

    const handler = createKeydownHandler(options);
    win.addEventListener("keydown", handler);

    return function detach() {
      if (typeof win.removeEventListener === "function") {
        win.removeEventListener("keydown", handler);
      }
    };
  }

  const api = Object.freeze({
    createKeydownHandler,
    installKeydownHandler,
  });

  if (typeof module !== "undefined" && module && module.exports) {
    module.exports = api;
  }

  global.COPNGInputShortcuts = api;
})(typeof window !== "undefined" ? window : globalThis);
