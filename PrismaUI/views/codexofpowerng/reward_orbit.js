(function (root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) {
    module.exports = api;
  }
  if (root) {
    root.COPNGRewardOrbit = api;
  }
})(typeof globalThis !== "undefined" ? globalThis : this, function () {
  "use strict";

  const REWARD_ORBIT_POSITIONS_REGULAR = [
    { x: 50, y: 10 },
    { x: 75, y: 18 },
    { x: 88, y: 40 },
    { x: 75, y: 62 },
    { x: 50, y: 76 },
    { x: 25, y: 62 },
    { x: 12, y: 40 },
    { x: 25, y: 18 },
  ];

  const REWARD_ORBIT_POSITIONS_COMPACT = [
    { x: 50, y: 12 },
    { x: 78, y: 28 },
    { x: 78, y: 60 },
    { x: 50, y: 76 },
    { x: 22, y: 60 },
    { x: 22, y: 28 },
  ];

  function toNumber(value, fallback) {
    const num = Number(value);
    return Number.isFinite(num) ? num : fallback;
  }

  function defaultEscapeHtml(value) {
    return String(value == null ? "" : value);
  }

  function defaultT(_key, fallback) {
    return fallback;
  }

  function defaultTFmt(_key, fallback, vars) {
    return String(fallback).replace(/\{([a-zA-Z0-9_]+)\}/g, (m, k) => (vars && Object.prototype.hasOwnProperty.call(vars, k) ? String(vars[k]) : m));
  }

  function defaultCoalesce(value, fallback) {
    return value == null ? fallback : value;
  }

  function rewardMagnitude(item) {
    const n = Number(item && item.total);
    return Number.isFinite(n) ? Math.abs(n) : 0;
  }

  function getRewardOrbitLayout(opts) {
    const scale = toNumber(opts && opts.scale, 1);
    const viewportW = toNumber(opts && opts.viewportW, 0);
    const compact = scale >= 2.0 || viewportW < 1280;
    if (compact) {
      return {
        positions: REWARD_ORBIT_POSITIONS_COMPACT,
        nodeWidthPx: 162,
      };
    }
    return {
      positions: REWARD_ORBIT_POSITIONS_REGULAR,
      nodeWidthPx: 188,
    };
  }

  function syncRewardCharacterImageState(opts) {
    const characterImgEl = opts && opts.characterImgEl;
    const fallbackEl = opts && opts.fallbackEl;
    if (!characterImgEl || !fallbackEl) return;
    const loaded = !!(characterImgEl.complete && characterImgEl.naturalWidth > 0);
    characterImgEl.style.display = loaded ? "" : "none";
    if (fallbackEl.classList && fallbackEl.classList.toggle) {
      fallbackEl.classList.toggle("show", !loaded);
    }
  }

  function renderRewardOrbit(opts) {
    if (!opts || !opts.orbitEl) return;
    const orbitEl = opts.orbitEl;
    const srcRows = Array.isArray(opts.rows) ? opts.rows : [];
    const escapeHtml = typeof opts.escapeHtml === "function" ? opts.escapeHtml : defaultEscapeHtml;
    const t = typeof opts.t === "function" ? opts.t : defaultT;
    const tFmt = typeof opts.tFmt === "function" ? opts.tFmt : defaultTFmt;
    const coalesce = typeof opts.coalesce === "function" ? opts.coalesce : defaultCoalesce;
    const onAfterRender = typeof opts.onAfterRender === "function" ? opts.onAfterRender : null;

    const layout = getRewardOrbitLayout({
      scale: opts.scale,
      viewportW: opts.viewportW,
    });
    orbitEl.style.setProperty("--rewardNodeWidth", `${layout.nodeWidthPx}px`);

    if (srcRows.length === 0) {
      orbitEl.innerHTML = `<div class="rewardOrbitEmpty small">${escapeHtml(t("rewards.none", "(No rewards yet)"))}</div>`;
      if (onAfterRender) onAfterRender();
      return;
    }

    const orbitRows = srcRows
      .slice()
      .sort((a, b) => rewardMagnitude(b) - rewardMagnitude(a) || String(a.label || "").localeCompare(String(b.label || "")));

    const maxNodes = layout.positions.length;
    const visible = orbitRows.slice(0, maxNodes);
    let html = "";
    for (let i = 0; i < visible.length; i++) {
      const item = visible[i] || {};
      const pos = layout.positions[i % layout.positions.length];
      html += `
            <div class="rewardEffectNode" style="left:${pos.x}%;top:${pos.y}%">
              <div class="rewardEffectBadge">
                <span class="name">${escapeHtml(item.label || "Unknown")}</span>
                <span class="value mono">${escapeHtml(item.display || String(coalesce(item.total, 0)))}</span>
              </div>
            </div>`;
    }

    const remain = Math.max(0, orbitRows.length - visible.length);
    if (remain > 0) {
      html += `
            <div class="rewardEffectNode" style="left:50%;top:90%">
              <div class="rewardEffectBadge rewardMore">${escapeHtml(tFmt("rewards.more", "+{n} more", { n: remain }))}</div>
            </div>`;
    }

    orbitEl.innerHTML = html;
    if (onAfterRender) onAfterRender();
  }

  return {
    REWARD_ORBIT_POSITIONS_REGULAR,
    REWARD_ORBIT_POSITIONS_COMPACT,
    rewardMagnitude,
    getRewardOrbitLayout,
    syncRewardCharacterImageState,
    renderRewardOrbit,
  };
});
