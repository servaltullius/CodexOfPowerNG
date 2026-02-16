(function (global) {
  "use strict";

  function noop() {}

  function asFn(maybeFn, fallback) {
    return typeof maybeFn === "function" ? maybeFn : fallback;
  }

  function defaultClamp(v, lo, hi) {
    return Math.max(lo, Math.min(hi, v));
  }

  function defaultCoalesce(value, fallback) {
    return value === undefined || value === null ? fallback : value;
  }

  function defaultEscapeHtml(value) {
    return String(value == null ? "" : value);
  }

  function defaultT(_key, fallback) {
    return fallback;
  }

  function defaultToHex32(v) {
    const n = Number((v >>> 0) >>> 0);
    let hex = n.toString(16).toUpperCase();
    while (hex.length < 8) hex = "0" + hex;
    return "0x" + hex;
  }

  function getOffsetTopInRoot(rootEl, el) {
    if (!rootEl || !el || !rootEl.getBoundingClientRect || !el.getBoundingClientRect) return 0;
    const rr = rootEl.getBoundingClientRect();
    const re = el.getBoundingClientRect();
    return Number(re.top || 0) - Number(rr.top || 0) + Number(rootEl.scrollTop || 0);
  }

  function createVirtualTableManager(opts) {
    const options = opts || {};

    const rootScrollEl = options.rootScrollEl || null;
    const quickBody = options.quickBody || null;
    const regBody = options.regBody || null;
    const quickVirtual = options.quickVirtual || { rows: [], lastStart: -1, lastEnd: -1, tbodyTopPx: NaN, rowHeightPx: 0 };
    const regVirtual = options.regVirtual || { rows: [], lastStart: -1, lastEnd: -1, tbodyTopPx: NaN, rowHeightPx: 0 };

    const getCurrentUiScale = asFn(options.getCurrentUiScale, function () {
      return 1;
    });
    const getQuickSelectedId = asFn(options.getQuickSelectedId, function () {
      return 0;
    });

    const clamp = asFn(options.clamp, defaultClamp);
    const coalesce = asFn(options.coalesce, defaultCoalesce);
    const escapeHtml = asFn(options.escapeHtml, defaultEscapeHtml);
    const t = asFn(options.t, defaultT);
    const toHex32 = asFn(options.toHex32, defaultToHex32);

    const quickRowBasePx = Number.isFinite(Number(options.quickRowBasePx)) ? Number(options.quickRowBasePx) : 78;
    const regRowBasePx = Number.isFinite(Number(options.regRowBasePx)) ? Number(options.regRowBasePx) : 54;
    const overscan = Number.isFinite(Number(options.overscan)) ? Number(options.overscan) : 15;
    const minRows = Number.isFinite(Number(options.minRows)) ? Number(options.minRows) : 24;
    const renderThrottleMs = Number.isFinite(Number(options.renderThrottleMs)) ? Number(options.renderThrottleMs) : 32;

    const documentObj = options.documentObj || (global && global.document) || null;
    const requestAnimationFrameFn =
      asFn(
        options.requestAnimationFrameFn,
        global && typeof global.requestAnimationFrame === "function" ? global.requestAnimationFrame.bind(global) : null
      ) ||
      function (cb) {
        return setTimeout(cb, 0);
      };
    const nowFn =
      asFn(options.nowFn, function () {
        if (typeof performance !== "undefined" && performance.now) return performance.now();
        return Date.now();
      }) ||
      function () {
        return Date.now();
      };

    const getActiveSectionId = asFn(options.getActiveSectionId, function () {
      if (!documentObj || !documentObj.querySelector) return "";
      const active = documentObj.querySelector(".section.active");
      return active && active.id ? active.id : "";
    });

    let virtualRaf = 0;
    let virtualForceNext = false;
    let lastVirtualRenderTs = 0;

    function buildQuickRowHtml(item, idx) {
      const it = item || {};
      const id = Number(it.formId) >>> 0;
      const selectedId = Number(getQuickSelectedId()) >>> 0;
      const classes = ["dataRow", idx % 2 === 0 ? "rowOdd" : "", id && id === selectedId ? "selected" : ""].filter(Boolean).join(" ");

      return `
          <tr data-row-id="${id}" class="${classes}">
            <td><span class="pill">${escapeHtml(it.groupName || String(it.group))}</span></td>
            <td>
              <div class="itemName">${escapeHtml(it.name || "(unnamed)")}</div>
              <div class="small mono">${toHex32(id)} → ${toHex32(it.regKey)}</div>
            </td>
            <td class="mono colCount"><span class="good">${coalesce(it.safeCount, 0)}</span>/${coalesce(it.totalCount, 0)}</td>
            <td class="colAction"><button class="primary" data-action="reg" data-id="${id}"><span class="btnLabel">${t(
              "btn.register",
              "Register"
            )}</span></button></td>
          </tr>`;
    }

    function buildRegisteredRowHtml(item, idx) {
      const it = item || {};
      const classes = ["dataRow", idx % 2 === 0 ? "rowOdd" : ""].filter(Boolean).join(" ");
      return `
          <tr class="${classes}">
            <td class="colGroup"><span class="pill">${escapeHtml(it.groupName || String(it.group))}</span></td>
            <td><span class="itemName">${escapeHtml(it.name || "(unnamed)")}</span></td>
            <td class="colFormId mono">${toHex32(it.formId)}</td>
          </tr>`;
    }

    function renderQuickVirtual(params) {
      const force = !!(params && params.force);
      if (!rootScrollEl || !quickBody) return;
      if (getActiveSectionId() !== "tabQuick") return;

      const rows = quickVirtual.rows || [];
      const total = rows.length;
      if (total === 0) {
        quickBody.innerHTML = `<tr><td colspan="4" class="small">${escapeHtml(t("inv.none", "(No items)"))}</td></tr>`;
        quickVirtual.lastStart = 0;
        quickVirtual.lastEnd = 0;
        return;
      }

      const shouldVirtualize = total > minRows;
      if (!shouldVirtualize) {
        if (!force && quickVirtual.lastStart === 0 && quickVirtual.lastEnd === total) return;
        quickVirtual.lastStart = 0;
        quickVirtual.lastEnd = total;
        quickVirtual.tbodyTopPx = NaN;

        let html = "";
        for (let i = 0; i < total; i++) {
          html += buildQuickRowHtml(rows[i], i);
        }
        quickBody.innerHTML = html;
        return;
      }

      if (force || !Number.isFinite(quickVirtual.tbodyTopPx) || !Number.isFinite(quickVirtual.rowHeightPx) || quickVirtual.rowHeightPx <= 0) {
        const uiScale = getCurrentUiScale();
        quickVirtual.rowHeightPx = quickRowBasePx * uiScale;
        quickVirtual.tbodyTopPx = getOffsetTopInRoot(rootScrollEl, quickBody);
      }

      const rowHeight = quickVirtual.rowHeightPx;
      const tbodyTop = quickVirtual.tbodyTopPx;
      const viewTop = Number(rootScrollEl.scrollTop || 0);
      const startRaw = Math.floor((viewTop - tbodyTop) / rowHeight);
      const approxVisible = Math.ceil(Number(rootScrollEl.clientHeight || 900) / rowHeight) + 2;
      const start = clamp(startRaw - overscan, 0, Math.max(0, total - 1));
      const end = clamp(start + approxVisible + overscan * 2, 0, total);

      if (!force && start === quickVirtual.lastStart && end === quickVirtual.lastEnd) return;

      quickVirtual.lastStart = start;
      quickVirtual.lastEnd = end;

      const topPad = start * rowHeight;
      const bottomPad = (total - end) * rowHeight;

      let html = "";
      if (topPad > 0) html += `<tr class="spacerRow"><td colspan="4" style="height:${Math.round(topPad)}px"></td></tr>`;

      for (let i = start; i < end; i++) {
        html += buildQuickRowHtml(rows[i], i);
      }

      if (bottomPad > 0) html += `<tr class="spacerRow"><td colspan="4" style="height:${Math.round(bottomPad)}px"></td></tr>`;
      quickBody.innerHTML = html;
    }

    function renderRegisteredVirtual(params) {
      const force = !!(params && params.force);
      if (!rootScrollEl || !regBody) return;
      if (getActiveSectionId() !== "tabRegistered") return;

      const rows = regVirtual.rows || [];
      const total = rows.length;
      if (total === 0) {
        regBody.innerHTML = `<tr><td colspan="3" class="small">${escapeHtml(t("reg.none", "(No registered items)"))}</td></tr>`;
        regVirtual.lastStart = 0;
        regVirtual.lastEnd = 0;
        return;
      }

      const shouldVirtualize = total > minRows;
      if (!shouldVirtualize) {
        if (!force && regVirtual.lastStart === 0 && regVirtual.lastEnd === total) return;
        regVirtual.lastStart = 0;
        regVirtual.lastEnd = total;
        regVirtual.tbodyTopPx = NaN;

        let html = "";
        for (let i = 0; i < total; i++) {
          html += buildRegisteredRowHtml(rows[i], i);
        }
        regBody.innerHTML = html;
        return;
      }

      if (force || !Number.isFinite(regVirtual.tbodyTopPx) || !Number.isFinite(regVirtual.rowHeightPx) || regVirtual.rowHeightPx <= 0) {
        const uiScale = getCurrentUiScale();
        regVirtual.rowHeightPx = regRowBasePx * uiScale;
        regVirtual.tbodyTopPx = getOffsetTopInRoot(rootScrollEl, regBody);
      }

      const rowHeight = regVirtual.rowHeightPx;
      const tbodyTop = regVirtual.tbodyTopPx;
      const viewTop = Number(rootScrollEl.scrollTop || 0);
      const startRaw = Math.floor((viewTop - tbodyTop) / rowHeight);
      const approxVisible = Math.ceil(Number(rootScrollEl.clientHeight || 900) / rowHeight) + 2;
      const start = clamp(startRaw - overscan, 0, Math.max(0, total - 1));
      const end = clamp(start + approxVisible + overscan * 2, 0, total);

      if (!force && start === regVirtual.lastStart && end === regVirtual.lastEnd) return;

      regVirtual.lastStart = start;
      regVirtual.lastEnd = end;

      const topPad = start * rowHeight;
      const bottomPad = (total - end) * rowHeight;

      let html = "";
      if (topPad > 0) html += `<tr class="spacerRow"><td colspan="3" style="height:${Math.round(topPad)}px"></td></tr>`;

      for (let i = start; i < end; i++) {
        html += buildRegisteredRowHtml(rows[i], i);
      }

      if (bottomPad > 0) html += `<tr class="spacerRow"><td colspan="3" style="height:${Math.round(bottomPad)}px"></td></tr>`;
      regBody.innerHTML = html;
    }

    function scheduleVirtualRender(params) {
      const force = !!(params && params.force);
      if (force) virtualForceNext = true;
      if (virtualRaf) return;

      virtualRaf = requestAnimationFrameFn(function () {
        virtualRaf = 0;
        const now = nowFn();
        if (!virtualForceNext && now - lastVirtualRenderTs < renderThrottleMs) {
          scheduleVirtualRender();
          return;
        }
        lastVirtualRenderTs = now;
        const f = virtualForceNext;
        virtualForceNext = false;
        renderQuickVirtual({ force: f });
        renderRegisteredVirtual({ force: f });
      });
    }

    function resetVirtualWindow(v) {
      if (!v) return;
      v.lastStart = -1;
      v.lastEnd = -1;
      v.tbodyTopPx = NaN;
    }

    function schedulePostRefreshVirtualResync() {
      scheduleVirtualRender({ force: true });
      requestAnimationFrameFn(function () {
        scheduleVirtualRender({ force: true });
      });
    }

    return Object.freeze({
      scheduleVirtualRender: scheduleVirtualRender,
      resetVirtualWindow: resetVirtualWindow,
      schedulePostRefreshVirtualResync: schedulePostRefreshVirtualResync,
      renderQuickVirtual: renderQuickVirtual,
      renderRegisteredVirtual: renderRegisteredVirtual,
    });
  }

  const api = Object.freeze({
    createVirtualTableManager: createVirtualTableManager,
    getOffsetTopInRoot: getOffsetTopInRoot,
  });

  if (typeof module !== "undefined" && module && module.exports) {
    module.exports = api;
  }

  global.COPNGVirtualTables = api;
})(typeof window !== "undefined" ? window : globalThis);
