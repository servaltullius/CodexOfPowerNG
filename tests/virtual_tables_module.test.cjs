const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "virtual_tables.js");

const html = fs.readFileSync(viewPath, "utf8");
const mod = require(modulePath);

function fakeRect(top) {
  return {
    top: Number(top || 0),
  };
}

test("view loads virtual tables module", () => {
  assert.match(
    html,
    /<script src="virtual_tables\.js"><\/script>/,
    "index.html should load virtual_tables.js before inline script",
  );
});

test("virtual tables module exports expected API", () => {
  assert.equal(typeof mod.createVirtualTableManager, "function");
  assert.equal(typeof mod.getOffsetTopInRoot, "function");
});

test("quick renderer bypasses virtualization for small lists", () => {
  const rootScrollEl = {
    scrollTop: 0,
    clientHeight: 900,
    getBoundingClientRect: () => fakeRect(0),
  };
  const quickBody = {
    innerHTML: "",
    getBoundingClientRect: () => fakeRect(10),
  };
  const regBody = {
    innerHTML: "",
    getBoundingClientRect: () => fakeRect(10),
  };
  const quickVirtual = {
    rows: [{ formId: 0x100, regKey: 0x200, group: "Weapons", name: "Iron Sword", safeCount: 1, totalCount: 3 }],
    lastStart: -1,
    lastEnd: -1,
    tbodyTopPx: NaN,
    rowHeightPx: 0,
  };
  const regVirtual = { rows: [], lastStart: -1, lastEnd: -1, tbodyTopPx: NaN, rowHeightPx: 0 };

  const mgr = mod.createVirtualTableManager({
    rootScrollEl,
    quickBody,
    regBody,
    quickVirtual,
    regVirtual,
    minRows: 24,
    getActiveSectionId: () => "tabQuick",
    getQuickSelectedId: () => 0x100,
    t: (_k, fallback) => fallback,
    coalesce: (v, f) => (v == null ? f : v),
    toHex32: (v) => `0x${(Number(v) >>> 0).toString(16).toUpperCase()}`,
    escapeHtml: (s) => String(s),
    getCurrentUiScale: () => 1,
  });

  mgr.renderQuickVirtual({ force: true });
  assert.match(quickBody.innerHTML, /data-row-id="256"/);
  assert.match(quickBody.innerHTML, /class="dataRow rowOdd selected"/);
  assert.doesNotMatch(quickBody.innerHTML, /spacerRow/);
});

test("quick renderer uses quickScrollEl as its scroll surface when provided", () => {
  const rootScrollEl = {
    scrollTop: 0,
    clientHeight: 900,
    getBoundingClientRect: () => fakeRect(0),
  };
  const quickScrollEl = {
    scrollTop: 140,
    clientHeight: 120,
    getBoundingClientRect: () => fakeRect(100),
  };
  const quickBody = {
    innerHTML: "",
    getBoundingClientRect: () => fakeRect(110 - Number(quickScrollEl.scrollTop || 0)),
  };
  const regBody = {
    innerHTML: "",
    getBoundingClientRect: () => fakeRect(10),
  };
  const quickVirtual = {
    rows: [
      { formId: 1, regKey: 11, group: "A", name: "A1", safeCount: 1, totalCount: 1 },
      { formId: 2, regKey: 12, group: "A", name: "A2", safeCount: 1, totalCount: 1 },
      { formId: 3, regKey: 13, group: "A", name: "A3", safeCount: 1, totalCount: 1 },
      { formId: 4, regKey: 14, group: "A", name: "A4", safeCount: 1, totalCount: 1 },
      { formId: 5, regKey: 15, group: "A", name: "A5", safeCount: 1, totalCount: 1 },
      { formId: 6, regKey: 16, group: "A", name: "A6", safeCount: 1, totalCount: 1 },
    ],
    lastStart: -1,
    lastEnd: -1,
    tbodyTopPx: NaN,
    rowHeightPx: 0,
  };
  const regVirtual = { rows: [], lastStart: -1, lastEnd: -1, tbodyTopPx: NaN, rowHeightPx: 0 };

  const mgr = mod.createVirtualTableManager({
    rootScrollEl,
    quickScrollEl,
    quickBody,
    regBody,
    quickVirtual,
    regVirtual,
    minRows: 1,
    overscan: 0,
    quickRowBasePx: 60,
    getActiveSectionId: () => "tabQuick",
    getQuickSelectedId: () => 0,
    t: (_k, fallback) => fallback,
    coalesce: (v, f) => (v == null ? f : v),
    toHex32: (v) => `0x${(Number(v) >>> 0).toString(16).toUpperCase()}`,
    escapeHtml: (s) => String(s),
    getCurrentUiScale: () => 1,
  });

  mgr.renderQuickVirtual({ force: true });
  assert.match(quickBody.innerHTML, /A3|A4|A5/, "Quick virtualization should use the inner quick scroller viewport instead of the root viewport");
  assert.doesNotMatch(quickBody.innerHTML, /A1/, "Rows above the quick scroller viewport should be virtualized out");
});

test("registered renderer uses spacer rows when virtualization is active", () => {
  const rootScrollEl = {
    scrollTop: 0,
    clientHeight: 108,
    getBoundingClientRect: () => fakeRect(0),
  };
  const quickBody = {
    innerHTML: "",
    getBoundingClientRect: () => fakeRect(10),
  };
  const regBody = {
    innerHTML: "",
    getBoundingClientRect: () => fakeRect(10),
  };
  const quickVirtual = { rows: [], lastStart: -1, lastEnd: -1, tbodyTopPx: NaN, rowHeightPx: 0 };
  const regVirtual = {
    rows: [
      { formId: 1, group: "A", groupName: "A", name: "A1" },
      { formId: 2, group: "A", groupName: "A", name: "A2" },
      { formId: 3, group: "A", groupName: "A", name: "A3" },
      { formId: 4, group: "A", groupName: "A", name: "A4" },
      { formId: 5, group: "A", groupName: "A", name: "A5" },
      { formId: 6, group: "A", groupName: "A", name: "A6" },
    ],
    lastStart: -1,
    lastEnd: -1,
    tbodyTopPx: NaN,
    rowHeightPx: 0,
  };

  const mgr = mod.createVirtualTableManager({
    rootScrollEl,
    quickBody,
    regBody,
    quickVirtual,
    regVirtual,
    minRows: 1,
    overscan: 0,
    regRowBasePx: 54,
    getActiveSectionId: () => "tabRegistered",
    t: (_k, fallback) => fallback,
    coalesce: (v, f) => (v == null ? f : v),
    toHex32: (v) => `0x${(Number(v) >>> 0).toString(16).toUpperCase()}`,
    escapeHtml: (s) => String(s),
    getCurrentUiScale: () => 1,
  });

  mgr.renderRegisteredVirtual({ force: true });
  assert.match(regBody.innerHTML, /spacerRow/);
  assert.match(regBody.innerHTML, /A1|A2|A3|A4|A5|A6/);
});

test("scheduleVirtualRender queues with requestAnimationFrame and renders on flush", () => {
  const queue = [];
  const rootScrollEl = {
    scrollTop: 0,
    clientHeight: 900,
    getBoundingClientRect: () => fakeRect(0),
  };
  const quickBody = {
    innerHTML: "",
    getBoundingClientRect: () => fakeRect(10),
  };
  const regBody = {
    innerHTML: "",
    getBoundingClientRect: () => fakeRect(10),
  };
  const quickVirtual = {
    rows: [{ formId: 1, regKey: 2, group: "A", name: "A", safeCount: 1, totalCount: 1 }],
    lastStart: -1,
    lastEnd: -1,
    tbodyTopPx: NaN,
    rowHeightPx: 0,
  };
  const regVirtual = { rows: [], lastStart: -1, lastEnd: -1, tbodyTopPx: NaN, rowHeightPx: 0 };
  let now = 0;

  const mgr = mod.createVirtualTableManager({
    rootScrollEl,
    quickBody,
    regBody,
    quickVirtual,
    regVirtual,
    getActiveSectionId: () => "tabQuick",
    getQuickSelectedId: () => 0,
    t: (_k, fallback) => fallback,
    coalesce: (v, f) => (v == null ? f : v),
    toHex32: (v) => `0x${(Number(v) >>> 0).toString(16).toUpperCase()}`,
    escapeHtml: (s) => String(s),
    getCurrentUiScale: () => 1,
    nowFn: () => {
      now += 33;
      return now;
    },
    requestAnimationFrameFn: (cb) => {
      queue.push(cb);
      return queue.length;
    },
  });

  mgr.scheduleVirtualRender({ force: true });
  assert.equal(queue.length, 1);
  assert.equal(quickBody.innerHTML, "");
  queue.shift()();
  assert.match(quickBody.innerHTML, /data-row-id="1"/);
});

test("quick virtual renderer respects grouped mode marker even without dataset support", () => {
  const rootScrollEl = {
    scrollTop: 0,
    clientHeight: 900,
    getBoundingClientRect: () => fakeRect(0),
  };
  const quickBody = {
    innerHTML: '<tr><td colspan="5">grouped rows stay</td></tr>',
    _attrs: { "data-virtual-mode": "grouped" },
    getBoundingClientRect: () => fakeRect(10),
    getAttribute(name) {
      return Object.prototype.hasOwnProperty.call(this._attrs, name) ? this._attrs[name] : null;
    },
  };
  const regBody = {
    innerHTML: "",
    getBoundingClientRect: () => fakeRect(10),
  };
  const quickVirtual = { rows: [], lastStart: -1, lastEnd: -1, tbodyTopPx: NaN, rowHeightPx: 0 };
  const regVirtual = { rows: [], lastStart: -1, lastEnd: -1, tbodyTopPx: NaN, rowHeightPx: 0 };

  const mgr = mod.createVirtualTableManager({
    rootScrollEl,
    quickBody,
    regBody,
    quickVirtual,
    regVirtual,
    getActiveSectionId: () => "tabQuick",
    getQuickSelectedId: () => 0,
    t: (_k, fallback) => fallback,
    coalesce: (v, f) => (v == null ? f : v),
    toHex32: (v) => `0x${(Number(v) >>> 0).toString(16).toUpperCase()}`,
    escapeHtml: (s) => String(s),
    getCurrentUiScale: () => 1,
  });

  mgr.renderQuickVirtual({ force: true });
  assert.equal(quickBody.innerHTML, '<tr><td colspan="5">grouped rows stay</td></tr>');
});
