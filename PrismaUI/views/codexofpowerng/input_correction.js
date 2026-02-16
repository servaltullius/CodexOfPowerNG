(function (global) {
  "use strict";

  const INTERACTIVE_SEL = "button, input, select, textarea, a, [role='button'], [data-action], tr[data-row-id]";

  function noop() {}

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function defaultClamp(v, min, max) {
    const n = Number(v);
    if (!Number.isFinite(n)) return Number(min);
    if (n < min) return min;
    if (n > max) return max;
    return n;
  }

  function isElementLike(node) {
    if (!node || typeof node !== "object") return false;
    if (typeof Element !== "undefined" && node instanceof Element) return true;
    return node.nodeType === 1;
  }

  function getRootElement(doc, options) {
    if (options && options.rootEl) return options.rootEl;
    if (!doc || typeof doc.querySelector !== "function") return null;
    return doc.querySelector((options && options.rootSelector) || ".root");
  }

  function wheelNow(e, perfObj) {
    const ts = typeof e.timeStamp === "number" && Number.isFinite(e.timeStamp) ? e.timeStamp : 0;
    if (ts > 0) return ts;
    if (perfObj && typeof perfObj.now === "function") return perfObj.now();
    return Date.now();
  }

  function installDirectWheelScroll(opts) {
    const options = opts || {};
    const doc = options.documentObj || (global && global.document) || null;
    if (!doc) return noop;

    const rootEl = getRootElement(doc, options);
    if (!rootEl || typeof rootEl.addEventListener !== "function") return noop;

    const clampFn = asFn(options.clamp, defaultClamp);
    const getCurrentUiScale = asFn(options.getCurrentUiScale, function () {
      return 1;
    });
    const perfObj = options.performanceObj || (global && global.performance) || null;

    let lastWheelTs = 0;

    function normalizeWheelDelta(e, container) {
      let dy = Number(e.deltaY || 0);
      if (!Number.isFinite(dy) || dy === 0) return 0;

      const mode = Number(e.deltaMode || 0);
      if (mode === 1) dy *= 16;
      else if (mode === 2) dy *= Number(container.clientHeight || 800);

      const uiScale = clampFn(getCurrentUiScale(), 1.0, 3.0);
      const abs = Math.abs(dy);

      const now = wheelNow(e, perfObj);
      const dt = lastWheelTs > 0 ? now - lastWheelTs : 999;
      lastWheelTs = now;

      // Tiny deltas from Ultralight notched wheel: use a moderate fixed step.
      if (abs > 0 && abs < 12 && dt > 24) {
        dy = Math.sign(dy) * 60 * uiScale;
      } else if (abs < 40) {
        dy = dy * 1.5 * uiScale;
      } else {
        dy = dy * uiScale;
      }

      const maxStep = Number(container.clientHeight || 800) * 0.45;
      return clampFn(dy, -maxStep, maxStep);
    }

    const onWheel = function (e) {
      if (!e || e.isTrusted === false) return;
      const maxScroll = Math.max(0, Number(rootEl.scrollHeight || 0) - Number(rootEl.clientHeight || 0));
      if (maxScroll <= 0) return;

      const deltaPx = normalizeWheelDelta(e, rootEl);
      if (!Number.isFinite(deltaPx) || deltaPx === 0) return;

      if (e.preventDefault) e.preventDefault();
      if (e.stopPropagation) e.stopPropagation();

      rootEl.scrollTop = clampFn(Number(rootEl.scrollTop || 0) + deltaPx, 0, maxScroll);
    };

    rootEl.addEventListener("wheel", onWheel, { passive: false });

    return function detachWheel() {
      if (typeof rootEl.removeEventListener === "function") {
        rootEl.removeEventListener("wheel", onWheel, false);
      }
    };
  }

  function getEventTargetElement(e) {
    if (!e) return null;
    const t = e.target;
    if (isElementLike(t)) return t;
    if (t && isElementLike(t.parentNode)) return t.parentNode;
    const path = typeof e.composedPath === "function" ? e.composedPath() : null;
    if (Array.isArray(path)) {
      for (const p of path) {
        if (isElementLike(p)) return p;
      }
    }
    return null;
  }

  function pickInteractiveElement(el) {
    if (!isElementLike(el)) return null;
    if (typeof el.matches === "function" && el.matches(INTERACTIVE_SEL)) return el;
    if (typeof el.closest === "function") return el.closest(INTERACTIVE_SEL);
    return null;
  }

  function shouldCorrectMouseEvent(e, inputScale) {
    if (!e || e.isTrusted === false) return false;
    if (e.button !== undefined && e.button !== 0) return false; // left click only
    const needsNormalizeTarget = !isElementLike(e.target);
    const needsScale = Number.isFinite(inputScale) && Math.abs(inputScale - 1.0) >= 0.01;
    return needsNormalizeTarget || needsScale;
  }

  function scoreTarget(el, rootEl) {
    if (!isElementLike(el)) return -1000000;
    let score = 0;
    const tag = String(el.tagName || "").toUpperCase();
    if (tag === "HTML" || tag === "BODY") score -= 5;

    const inRoot = !!(rootEl && typeof rootEl.contains === "function" && rootEl.contains(el));
    score += inRoot ? 100 : -100;

    if (typeof el.matches === "function" && el.matches(INTERACTIVE_SEL)) score += 60;
    if (typeof el.closest === "function" && el.closest(INTERACTIVE_SEL)) score += 25;
    if (typeof el.closest === "function" && el.closest("tr[data-row-id]")) score += 5;
    return score;
  }

  function makeMouseEventLike(win, e, type, pt) {
    const MouseEventCtor =
      (win && typeof win.MouseEvent === "function" && win.MouseEvent) ||
      (typeof MouseEvent !== "undefined" ? MouseEvent : null);

    const init = {
      bubbles: true,
      cancelable: true,
      composed: true,
      view: win || global,
      detail: e.detail || 0,
      screenX: e.screenX || 0,
      screenY: e.screenY || 0,
      clientX: pt.x,
      clientY: pt.y,
      button: e.button || 0,
      buttons: e.buttons || 1,
      ctrlKey: !!e.ctrlKey,
      shiftKey: !!e.shiftKey,
      altKey: !!e.altKey,
      metaKey: !!e.metaKey,
    };

    if (MouseEventCtor) return new MouseEventCtor(type, init);
    return { type: type, ...init };
  }

  function nowMs() {
    try {
      return Date.now();
    } catch {
      return 0;
    }
  }

  function buildCandidates(cx, cy, inputScale, needsScale, rootEl, clampPt) {
    const candidates = [];
    candidates.push({ label: "identity", pt: clampPt({ x: cx, y: cy }) });

    if (!needsScale) return candidates;

    candidates.push({ label: "origin-mul", pt: clampPt({ x: cx * inputScale, y: cy * inputScale }) });
    candidates.push({ label: "origin-div", pt: clampPt({ x: cx / inputScale, y: cy / inputScale }) });

    if (rootEl && typeof rootEl.getBoundingClientRect === "function") {
      const r = rootEl.getBoundingClientRect();
      const rx = Number(r.left || 0);
      const ry = Number(r.top || 0);
      candidates.push({
        label: "root-mul",
        pt: clampPt({ x: rx + (cx - rx) * inputScale, y: ry + (cy - ry) * inputScale }),
      });
      candidates.push({
        label: "root-div",
        pt: clampPt({ x: rx + (cx - rx) / inputScale, y: ry + (cy - ry) / inputScale }),
      });
    }

    return candidates;
  }

  function installMouseEventCorrection(opts) {
    const options = opts || {};
    const doc = options.documentObj || (global && global.document) || null;
    const win = options.windowObj || global;
    if (!doc || typeof doc.addEventListener !== "function") return noop;

    const clampFn = asFn(options.clamp, defaultClamp);
    const getInputScale = asFn(options.getInputScale, function () {
      return 1;
    });

    // Fallback: in some Ultralight builds, clicking directly on text can fail to dispatch a usable "click"
    // to the element listeners. Use mouseup->synthetic click, and suppress the subsequent native click.
    let lastMouseupFixed = { t: 0, x: -9999, y: -9999 };

    const onMouseup = function (e) {
      if (!e || e.isTrusted === false) return;
      if (e.button !== undefined && e.button !== 0) return;
      // Only apply this fallback when Ultralight reports a non-Element target (typically a Text node).
      if (isElementLike(e.target)) return;

      const w = Number((win && win.innerWidth) || 0);
      const h = Number((win && win.innerHeight) || 0);
      if (!Number.isFinite(w) || !Number.isFinite(h) || w <= 0 || h <= 0) return;

      const rootEl = getRootElement(doc, options);
      const inputScale = Number(getInputScale());
      const needsScale = Number.isFinite(inputScale) && Math.abs(inputScale - 1.0) >= 0.01;

      const cx = Number(e.clientX || 0);
      const cy = Number(e.clientY || 0);

      const clampPt = function (pt) {
        return {
          x: clampFn(Math.round(pt.x), 0, Math.max(0, w - 1)),
          y: clampFn(Math.round(pt.y), 0, Math.max(0, h - 1)),
        };
      };

      const candidates = buildCandidates(cx, cy, inputScale, needsScale, rootEl, clampPt);

      let bestTarget = null;
      let bestPt = null;
      let bestScore = -1000000;
      for (const c of candidates) {
        const t = typeof doc.elementFromPoint === "function" ? doc.elementFromPoint(c.pt.x, c.pt.y) : null;
        const picked = pickInteractiveElement(t);
        if (!picked) continue;
        const s = scoreTarget(picked, rootEl);
        if (s > bestScore) {
          bestScore = s;
          bestTarget = picked;
          bestPt = c.pt;
        }
      }

      if (!isElementLike(bestTarget) || !bestPt) return;
      if (rootEl && typeof rootEl.contains === "function" && !rootEl.contains(bestTarget)) return;

      lastMouseupFixed = { t: nowMs(), x: Math.round(cx), y: Math.round(cy) };
      if (typeof bestTarget.dispatchEvent === "function") {
        bestTarget.dispatchEvent(makeMouseEventLike(win, e, "click", bestPt));
      }
    };

    const onClick = function (e) {
      const inputScale = Number(getInputScale());
      if (!shouldCorrectMouseEvent(e, inputScale)) return;

      const fixedAge = nowMs() - (lastMouseupFixed.t || 0);
      if (fixedAge >= 0 && fixedAge < 250) {
        const cx0 = Math.round(Number(e.clientX || 0));
        const cy0 = Math.round(Number(e.clientY || 0));
        if (Math.abs(cx0 - lastMouseupFixed.x) <= 1 && Math.abs(cy0 - lastMouseupFixed.y) <= 1) {
          if (e.preventDefault) e.preventDefault();
          if (e.stopImmediatePropagation) e.stopImmediatePropagation();
          else if (e.stopPropagation) e.stopPropagation();
          return;
        }
      }

      const w = Number((win && win.innerWidth) || 0);
      const h = Number((win && win.innerHeight) || 0);
      if (!Number.isFinite(w) || !Number.isFinite(h) || w <= 0 || h <= 0) return;

      const rootEl = getRootElement(doc, options);
      const needsNormalizeTarget = !isElementLike(e.target);
      const needsScale = Number.isFinite(inputScale) && Math.abs(inputScale - 1.0) >= 0.01;

      const origRaw = getEventTargetElement(e);
      const origTarget = pickInteractiveElement(origRaw);
      const origScore = scoreTarget(origTarget, rootEl);

      const cx = Number(e.clientX || 0);
      const cy = Number(e.clientY || 0);

      const clampPt = function (pt) {
        return {
          x: clampFn(Math.round(pt.x), 0, Math.max(0, w - 1)),
          y: clampFn(Math.round(pt.y), 0, Math.max(0, h - 1)),
        };
      };

      const candidates = buildCandidates(cx, cy, inputScale, needsScale, rootEl, clampPt);

      let bestTarget = null;
      let bestPt = null;
      let bestScore = -1000000;
      for (const c of candidates) {
        const t = typeof doc.elementFromPoint === "function" ? doc.elementFromPoint(c.pt.x, c.pt.y) : null;
        const picked = pickInteractiveElement(t);
        if (!picked) continue;
        const s = scoreTarget(picked, rootEl);
        if (s > bestScore) {
          bestScore = s;
          bestTarget = picked;
          bestPt = c.pt;
        }
      }

      if (!isElementLike(bestTarget)) {
        if (needsNormalizeTarget && isElementLike(origTarget)) {
          bestTarget = origTarget;
          bestPt = clampPt({ x: cx, y: cy });
        } else {
          return;
        }
      }

      // Only touch events that are inside our UI.
      const bestInRoot = rootEl && typeof rootEl.contains === "function" ? rootEl.contains(bestTarget) : true;
      const origInRoot = rootEl && typeof rootEl.contains === "function" && isElementLike(origTarget) ? rootEl.contains(origTarget) : false;
      if (!bestInRoot && !origInRoot) return;

      if (!needsNormalizeTarget) {
        if (bestTarget === origTarget) return;
        if (bestScore <= origScore) return;
      }

      if (e.preventDefault) e.preventDefault();
      if (e.stopImmediatePropagation) e.stopImmediatePropagation();
      else if (e.stopPropagation) e.stopPropagation();

      if (typeof bestTarget.dispatchEvent === "function") {
        bestTarget.dispatchEvent(makeMouseEventLike(win, e, "click", bestPt));
      }
    };

    doc.addEventListener("mouseup", onMouseup, true);
    doc.addEventListener("click", onClick, true);

    return function detachMouseCorrection() {
      if (typeof doc.removeEventListener === "function") {
        doc.removeEventListener("mouseup", onMouseup, true);
        doc.removeEventListener("click", onClick, true);
      }
    };
  }

  function installInputCorrection(opts) {
    const detachWheel = installDirectWheelScroll(opts);
    const detachMouse = installMouseEventCorrection(opts);

    return function detachInputCorrection() {
      detachWheel();
      detachMouse();
    };
  }

  const api = Object.freeze({
    installDirectWheelScroll: installDirectWheelScroll,
    installMouseEventCorrection: installMouseEventCorrection,
    installInputCorrection: installInputCorrection,
  });

  if (typeof module !== "undefined" && module && module.exports) {
    module.exports = api;
  }

  global.COPNGInputCorrection = api;
})(typeof window !== "undefined" ? window : globalThis);
