const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_rendering.js");

const html = fs.readFileSync(viewPath, "utf8");
const moduleSource = fs.readFileSync(modulePath, "utf8");
const mod = require(modulePath);

test("view loads ui_rendering module before inline bootstrap", () => {
  assert.match(
    html,
    /<script src="ui_rendering\.js"><\/script>/,
    "index.html should load ui_rendering.js before the inline bootstrap",
  );
});

test("ui_rendering module exports rendering factory and helpers", () => {
  assert.equal(typeof mod.createUIRendering, "function");
  assert.equal(typeof mod.escapeHtml, "function");
  assert.equal(typeof mod.sanitizeI18nHtml, "function");
});

test("index delegates render layer to ui_rendering module", () => {
  assert.match(html, /const uiRendering = uiRenderingApi\.createUIRendering\(\{/);
  assert.match(html, /refs:\s*\{[\s\S]*quickBody,[\s\S]*quickVirtual:/);
  assert.match(html, /refs:\s*\{[\s\S]*quickBody,[\s\S]*regBody,[\s\S]*statusEl,/);
  assert.match(html, /<div class="card section active quickScreen" id="tabQuick">/);
  assert.match(html, /<div class="quickBatchSummary" id="quickBatchSummary">/);
  assert.match(html, /<div class="card section buildScreen" id="tabBuild">/);
  assert.match(html, /<div class="buildBoard shrineLayout" id="buildPanel">/);
  assert.match(html, /<div class="heroTop">[\s\S]*<div class="heroMain">[\s\S]*<div class="heroBottom">[\s\S]*id="status"[\s\S]*data-tab="tabBuild"[\s\S]*<div class="heroAside">[\s\S]*id="btnRefreshState"[\s\S]*id="btnClose"/);
  assert.match(html, /rewardCharacterImgEl/);
  assert.match(html, /rewardImageFallbackEl/);
  assert.match(html, /const \{\s*applyI18n,[\s\S]*renderSettings,[\s\S]*\} = uiRendering;/);
  assert.doesNotMatch(html, /function renderStatus\(/);
  assert.doesNotMatch(html, /function renderQuick\(/);
  assert.doesNotMatch(html, /function renderRegistered\(/);
  assert.doesNotMatch(html, /function renderUndo\(/);
  assert.doesNotMatch(html, /function renderBuild\(/);
  assert.doesNotMatch(html, /function renderSettings\(/);
  assert.doesNotMatch(html, /function applyI18n\(/);
  assert.doesNotMatch(html, /<\/div>\s*<\/div>\s*<div class="toolbar">\s*<div class="left">\s*<span class="pill" id="status">/);
});

test("index includes a more opaque performance-first visual contract for quick and build screens", () => {
  assert.match(html, /\.root\.isViewportLocked\s*\{[\s\S]*overflow:\s*hidden/);
  assert.match(html, /--panel:\s*rgba\(18,\s*20,\s*28,\s*0\.84\)/);
  assert.match(html, /--text:\s*rgba\(255,\s*255,\s*255,\s*0\.97\)/);
  assert.match(html, /--muted:\s*rgba\(224,\s*228,\s*240,\s*0\.78\)/);
  assert.match(html, /\.quickToolbar,\s*\.buildToolbar\s*\{[\s\S]*background:\s*rgba\(10,\s*12,\s*18,\s*0\.88\)/);
  assert.match(html, /\.quickScreen\.section\.active\s*\{[\s\S]*display:\s*grid[\s\S]*height:\s*var\(--sectionViewportPx,\s*auto\)/);
  assert.match(html, /\.buildScreen\.section\.active\s*\{[\s\S]*display:\s*grid[\s\S]*height:\s*var\(--sectionViewportPx,\s*auto\)/);
  assert.match(html, /\.quickTableShell,\s*#quickTableScroller\s*\{[\s\S]*rgba\(8,\s*10,\s*18,\s*0\.88\)/);
  assert.match(html, /#quickTableScroller\s*\{[\s\S]*height:\s*var\(--quickViewportPx,\s*auto\)/);
  assert.match(html, /#quickTableScroller\s*\{[\s\S]*max-height:\s*var\(--quickViewportPx,\s*min\(calc\(560px \* var\(--uiScale\)\),\s*calc\(52vh\)\)\)/);
  assert.match(html, /\.quickBatchSummary\s*\{[\s\S]*rgba\(10,\s*12,\s*18,\s*0\.9\)/);
  assert.doesNotMatch(html, /\.quickBatchSummary\s*\{[^}]*backdrop-filter:/);
  assert.match(html, /\.quickScreen\s*\{[\s\S]*display:\s*grid/);
  assert.match(html, /\.quickScreen\s*\{[\s\S]*grid-template-rows:\s*auto minmax\(0,\s*1fr\) auto auto/);
  assert.match(html, /\.quickTableShell,\s*#quickTableScroller\s*\{[\s\S]*overflow:\s*auto/);
  assert.match(html, /\.quickTableShell,\s*#quickTableScroller\s*\{[\s\S]*overscroll-behavior:\s*contain/);
  assert.match(html, /<div class="quickTableShell" id="quickTableScroller">/);
  assert.match(html, /\.itemName\s*\{[\s\S]*font-size:\s*calc\(17px \* var\(--uiScale\)\)/);
  assert.match(html, /\.stateTag\s*\{[\s\S]*font-size:\s*calc\(11\.5px \* var\(--uiScale\)\)/);
  assert.match(html, /#tabQuick \.sectionOrnament\s*\{[\s\S]*box-shadow:\s*none/);
  assert.match(html, /\.reasonText\s*\{[\s\S]*color:\s*rgba\(245,\s*247,\s*255,\s*0\.82\)/);
  assert.match(html, /\.buildScreen\.section\.active\s*\{[\s\S]*grid-template-rows:\s*auto minmax\(0,\s*1fr\)/);
  assert.match(html, /\.buildScreen\.section\.active\s+\.buildFixedBoard\s*\{[\s\S]*height:\s*100%/);
  assert.doesNotMatch(html, /class="small buildHelp"/);
});

test("ui_rendering module owns i18n, tab, and list/build/settings renderers", () => {
  assert.match(moduleSource, /function applyI18n\(/);
  assert.match(moduleSource, /function getActiveTabId\(/);
  assert.match(moduleSource, /function isTabActive\(/);
  assert.match(moduleSource, /function syncRootViewportMode\(/);
  assert.match(moduleSource, /function setTab\(/);
  assert.match(moduleSource, /classList\.toggle\("isViewportLocked", lockViewport\)/);
  assert.match(moduleSource, /function renderTab\(/);
  assert.match(moduleSource, /function renderActiveTab\(/);
  assert.match(moduleSource, /function renderStatus\(/);
  assert.match(moduleSource, /function renderQuick\(/);
  assert.match(moduleSource, /function renderRegistered\(/);
  assert.match(moduleSource, /function renderUndo\(/);
  assert.match(moduleSource, /function renderBuild\(/);
  assert.match(moduleSource, /function renderSettings\(/);
});

test("grouped quick renderer marks virtual mode via data attribute even when dataset is unavailable", () => {
  const quickBody = {
    innerHTML: "",
    _attrs: Object.create(null),
    setAttribute(name, value) {
      this._attrs[name] = String(value);
    },
    getAttribute(name) {
      return Object.prototype.hasOwnProperty.call(this._attrs, name) ? this._attrs[name] : null;
    },
    removeAttribute(name) {
      delete this._attrs[name];
    },
  };

  const refs = {
    quickBody,
    quickBatchMetaEl: { textContent: "" },
    quickBatchGainEl: { textContent: "" },
    btnRegisterBatchEl: { disabled: false },
    invMetaEl: { textContent: "" },
    invPageSizeEl: { value: "200" },
    btnInvPrev: { disabled: false },
    btnInvNext: { disabled: false },
    quickVirtual: { rows: [] },
  };

  const rendering = mod.createUIRendering({
    documentObj: {
      getElementById(id) {
        if (id === "quickFilter") return { value: "" };
        if (id === "quickActionableOnly") return { checked: false };
        return null;
      },
    },
    refs,
    t: (_key, fallback) => fallback,
    tFmt: (_key, fallback, vars) =>
      String(fallback).replace(/\{([a-zA-Z0-9_]+)\}/g, (_, name) => String(vars[name])),
    coalesce: (value, fallback) => (value == null ? fallback : value),
    clamp: (value, lo, hi) => Math.max(lo, Math.min(hi, value)),
    toHex32: (value) => `0x${(Number(value) >>> 0).toString(16).toUpperCase()}`,
    getInventoryPage: () => ({
      page: 0,
      pageSize: 200,
      total: 1,
      hasMore: false,
      items: [],
      sections: [{ discipline: "attack", rows: [{ formId: 1, regKey: 1, name: "Iron Sword", totalCount: 1, safeCount: 1, actionable: true }] }],
    }),
    getQuickSelectedId: () => 0,
    setQuickSelectedId: () => {},
    setQuickVisibleIds: () => {},
    getQuickBatchSelectedIds: () => [],
    setQuickBatchSelectedIds: () => {},
    getQuickActionableOnly: () => false,
    getRegistered: () => [],
    getUndoItems: () => [],
    getBuild: () => ({ disciplines: {}, options: [], activeSlots: [], migrationNotice: {} }),
    getRewards: () => ({ totals: [] }),
    getSettings: () => null,
    getInputScale: () => 1,
    getLangMenuOpen: () => false,
    setLangMenuOpen: () => {},
    getLotdGateBlockingToastShown: () => false,
    setLotdGateBlockingToastShown: () => {},
    registerBatchPanelApi: {
      buildRegisterBatchViewModel: (inventoryPage) => ({
        sections: inventoryPage.sections,
        rows: inventoryPage.sections[0].rows,
        summary: {
          selectedRows: 0,
          disciplineGain: { attack: 0, defense: 0, utility: 0 },
          formIds: [],
        },
      }),
      renderRegisterBatchTbody: () => '<tr class="dataRow"><td colspan="5">ok</td></tr>',
    },
  });

  rendering.renderQuick();

  assert.equal(quickBody.getAttribute("data-virtual-mode"), "grouped");
  assert.match(quickBody.innerHTML, /ok/);
  assert.match(refs.invMetaEl.textContent, /Inventory: showing 1\/1/);
});

test("grouped quick renderer falls back to document quickBody element when refs.quickBody is missing", () => {
  const quickBody = {
    innerHTML: "",
    _attrs: Object.create(null),
    setAttribute(name, value) {
      this._attrs[name] = String(value);
    },
    getAttribute(name) {
      return Object.prototype.hasOwnProperty.call(this._attrs, name) ? this._attrs[name] : null;
    },
    removeAttribute(name) {
      delete this._attrs[name];
    },
  };

  const rendering = mod.createUIRendering({
    documentObj: {
      getElementById(id) {
        if (id === "quickBody") return quickBody;
        if (id === "quickFilter") return { value: "" };
        if (id === "quickActionableOnly") return { checked: false };
        return null;
      },
    },
    refs: {
      quickVirtual: { rows: [] },
      quickBatchMetaEl: { textContent: "" },
      quickBatchGainEl: { textContent: "" },
      btnRegisterBatchEl: { disabled: false },
      invMetaEl: { textContent: "" },
      invPageSizeEl: { value: "200" },
      btnInvPrev: { disabled: false },
      btnInvNext: { disabled: false },
    },
    t: (_key, fallback) => fallback,
    tFmt: (_key, fallback, vars) =>
      String(fallback).replace(/\{([a-zA-Z0-9_]+)\}/g, (_, name) => String(vars[name])),
    coalesce: (value, fallback) => (value == null ? fallback : value),
    clamp: (value, lo, hi) => Math.max(lo, Math.min(hi, value)),
    toHex32: (value) => `0x${(Number(value) >>> 0).toString(16).toUpperCase()}`,
    getInventoryPage: () => ({
      page: 0,
      pageSize: 200,
      total: 1,
      hasMore: false,
      items: [],
      sections: [{ discipline: "attack", rows: [{ formId: 1, regKey: 1, name: "Iron Sword", totalCount: 1, safeCount: 1, actionable: true }] }],
    }),
    getQuickSelectedId: () => 0,
    setQuickSelectedId: () => {},
    setQuickVisibleIds: () => {},
    getQuickBatchSelectedIds: () => [],
    setQuickBatchSelectedIds: () => {},
    getQuickActionableOnly: () => false,
    getRegistered: () => [],
    getUndoItems: () => [],
    getBuild: () => ({ disciplines: {}, options: [], activeSlots: [], migrationNotice: {} }),
    getRewards: () => ({ totals: [] }),
    getSettings: () => null,
    getInputScale: () => 1,
    getLangMenuOpen: () => false,
    setLangMenuOpen: () => {},
    getLotdGateBlockingToastShown: () => false,
    setLotdGateBlockingToastShown: () => {},
    registerBatchPanelApi: {
      buildRegisterBatchViewModel: (inventoryPage) => ({
        sections: inventoryPage.sections,
        rows: inventoryPage.sections[0].rows,
        summary: {
          selectedRows: 0,
          disciplineGain: { attack: 0, defense: 0, utility: 0 },
          formIds: [],
        },
      }),
      renderRegisterBatchTbody: () => '<tr class="dataRow"><td colspan="5">ok</td></tr>',
    },
  });

  rendering.renderQuick();

  assert.equal(quickBody.getAttribute("data-virtual-mode"), "grouped");
  assert.match(quickBody.innerHTML, /ok/);
});

test("fallback quick renderer forces an immediate virtual table pass so tbody does not stay stale", () => {
  const quickBody = {
    innerHTML: '<tr><td colspan="4" class="small">(아이템이 없습니다)</td></tr>',
    setAttribute() {},
    removeAttribute() {},
  };

  let renderQuickVirtualCalls = 0;
  const rendering = mod.createUIRendering({
    documentObj: {
      getElementById(id) {
        if (id === "quickFilter") return { value: "" };
        return null;
      },
    },
    refs: {
      quickBody,
      quickVirtual: { rows: [] },
      invMetaEl: { textContent: "" },
      invPageSizeEl: { value: "200" },
      btnInvPrev: { disabled: false },
      btnInvNext: { disabled: false },
    },
    t: (_key, fallback) => fallback,
    tFmt: (_key, fallback, vars) =>
      String(fallback).replace(/\{([a-zA-Z0-9_]+)\}/g, (_, name) => String(vars[name])),
    coalesce: (value, fallback) => (value == null ? fallback : value),
    clamp: (value, lo, hi) => Math.max(lo, Math.min(hi, value)),
    toHex32: (value) => `0x${(Number(value) >>> 0).toString(16).toUpperCase()}`,
    getInventoryPage: () => ({
      page: 0,
      pageSize: 200,
      total: 2,
      hasMore: false,
      items: [
        { formId: 1, regKey: 10, group: 0, groupName: "Weapons", name: "Iron Sword", safeCount: 1, totalCount: 1 },
        { formId: 2, regKey: 20, group: 1, groupName: "Armor", name: "Iron Armor", safeCount: 1, totalCount: 1 },
      ],
    }),
    getQuickSelectedId: () => 0,
    setQuickSelectedId: () => {},
    setQuickVisibleIds: () => {},
    getQuickBatchSelectedIds: () => [],
    setQuickBatchSelectedIds: () => {},
    getQuickActionableOnly: () => false,
    getRegistered: () => [],
    getUndoItems: () => [],
    getBuild: () => ({ disciplines: {}, options: [], activeSlots: [], migrationNotice: {} }),
    getRewards: () => ({ totals: [] }),
    getSettings: () => null,
    getInputScale: () => 1,
    getLangMenuOpen: () => false,
    setLangMenuOpen: () => {},
    getLotdGateBlockingToastShown: () => false,
    setLotdGateBlockingToastShown: () => {},
    scheduleVirtualRender: () => {},
    getVirtualTableManager: () => ({
      renderQuickVirtual() {
        renderQuickVirtualCalls += 1;
        quickBody.innerHTML = '<tr data-row-id="1"><td>Iron Sword</td></tr>';
      },
      renderRegisteredVirtual() {},
    }),
  });

  rendering.renderQuick();

  assert.equal(renderQuickVirtualCalls, 1);
  assert.match(quickBody.innerHTML, /Iron Sword/);
});

test("renderBuild re-resolves the silhouette image and fallback after dynamic build markup render", () => {
  const styleCalls = [];
  const buildPanelEl = {
    innerHTML: "",
    style: {
      setProperty(name, value) {
        styleCalls.push([name, value]);
      },
    },
    getBoundingClientRect() {
      return { top: 220 };
    },
  };
  const imageEl = { complete: true, naturalWidth: 256, style: {} };
  const fallbackEl = {
    classList: {
      toggle() {},
    },
  };
  const syncCalls = [];

  const rendering = mod.createUIRendering({
    documentObj: {
      getElementById(id) {
        if (id === "rewardCharacterImg") return imageEl;
        if (id === "rewardImageFallback") return fallbackEl;
        return null;
      },
      querySelector(selector) {
        if (selector === ".root") {
          return {
            getBoundingClientRect() {
              return { top: 40, bottom: 940 };
            },
          };
        }
        return null;
      },
    },
    refs: {
      buildPanelEl,
      buildMetaEl: { textContent: "" },
      rewardCharacterImgEl: null,
      rewardImageFallbackEl: null,
    },
    buildPanelApi: {
      renderBuildPanelHtml() {
        return `
          <div class="buildCharacterStage">
            <img id="rewardCharacterImg" src="assets/character.png" alt="" />
            <div id="rewardImageFallback"></div>
          </div>`;
      },
    },
    rewardOrbitApi: {
      syncRewardCharacterImageState(args) {
        syncCalls.push(args);
      },
    },
    t: (_key, fallback) => fallback,
    tFmt: (_key, fallback, vars) =>
      String(fallback).replace(/\{([a-zA-Z0-9_]+)\}/g, (_, name) => String(vars[name])),
    coalesce: (value, fallback) => (value == null ? fallback : value),
    clamp: (value, lo, hi) => Math.max(lo, Math.min(hi, value)),
    toHex32: (value) => `0x${(Number(value) >>> 0).toString(16).toUpperCase()}`,
    getBuild: () => ({
      disciplines: {
        attack: { score: 5 },
        defense: { score: 3 },
        utility: { score: 1 },
      },
      options: [],
      activeSlots: [],
      migrationNotice: {},
    }),
    getInventoryPage: () => ({ page: 0, pageSize: 200, total: 0, hasMore: false, items: [] }),
    getQuickSelectedId: () => 0,
    setQuickSelectedId: () => {},
    setQuickVisibleIds: () => {},
    getQuickBatchSelectedIds: () => [],
    setQuickBatchSelectedIds: () => {},
    getQuickActionableOnly: () => false,
    getRegistered: () => [],
    getUndoItems: () => [],
    getRewards: () => ({ totals: [] }),
    getSettings: () => null,
    getInputScale: () => 1,
    getLangMenuOpen: () => false,
    setLangMenuOpen: () => {},
    getLotdGateBlockingToastShown: () => false,
    setLotdGateBlockingToastShown: () => {},
  });

  rendering.renderBuild();

  assert.equal(syncCalls.length, 1);
  assert.equal(syncCalls[0].characterImgEl, imageEl);
  assert.equal(syncCalls[0].fallbackEl, fallbackEl);
  assert.deepEqual(styleCalls[0], ["--buildViewportPx", "696px"]);
});

test("renderQuick syncs the quick table scroller viewport height for internal scrolling", () => {
  const styleCalls = [];
  const quickScrollEl = {
    style: {
      setProperty(name, value) {
        styleCalls.push([name, value]);
      },
    },
    getBoundingClientRect() {
      return { top: 220 };
    },
  };

  const rendering = mod.createUIRendering({
    documentObj: {
      getElementById(id) {
        if (id === "quickFilter") return { value: "" };
        if (id === "quickActionableOnly") return { checked: false };
        return null;
      },
      querySelector(selector) {
        if (selector === ".root") {
          return {
            getBoundingClientRect() {
              return { top: 40, bottom: 940 };
            },
          };
        }
        return null;
      },
    },
    refs: {
      quickScrollEl,
      quickBody: {
        innerHTML: "",
        setAttribute() {},
        removeAttribute() {},
      },
      quickBatchMetaEl: { textContent: "" },
      quickBatchGainEl: { textContent: "" },
      btnRegisterBatchEl: { disabled: false },
      invMetaEl: { textContent: "" },
      invPageSizeEl: { value: "200" },
      btnInvPrev: { disabled: false },
      btnInvNext: { disabled: false },
      quickVirtual: { rows: [] },
    },
    t: (_key, fallback) => fallback,
    tFmt: (_key, fallback, vars) =>
      String(fallback).replace(/\{([a-zA-Z0-9_]+)\}/g, (_, name) => String(vars[name])),
    coalesce: (value, fallback) => (value == null ? fallback : value),
    clamp: (value, lo, hi) => Math.max(lo, Math.min(hi, value)),
    toHex32: (value) => `0x${(Number(value) >>> 0).toString(16).toUpperCase()}`,
    getInventoryPage: () => ({
      page: 0,
      pageSize: 200,
      total: 1,
      hasMore: false,
      items: [],
      sections: [{ discipline: "attack", rows: [{ formId: 1, regKey: 1, name: "Iron Sword", totalCount: 1, safeCount: 1, actionable: true }] }],
    }),
    getQuickSelectedId: () => 0,
    setQuickSelectedId: () => {},
    setQuickVisibleIds: () => {},
    getQuickBatchSelectedIds: () => [],
    setQuickBatchSelectedIds: () => {},
    getQuickActionableOnly: () => false,
    getRegistered: () => [],
    getUndoItems: () => [],
    getBuild: () => ({ disciplines: {}, options: [], activeSlots: [], migrationNotice: {} }),
    getRewards: () => ({ totals: [] }),
    getSettings: () => null,
    getInputScale: () => 1,
    getLangMenuOpen: () => false,
    setLangMenuOpen: () => {},
    getLotdGateBlockingToastShown: () => false,
    setLotdGateBlockingToastShown: () => {},
    registerBatchPanelApi: {
      buildRegisterBatchViewModel: (inventoryPage) => ({
        sections: inventoryPage.sections,
        rows: inventoryPage.sections[0].rows,
        summary: {
          selectedRows: 0,
          disciplineGain: { attack: 0, defense: 0, utility: 0 },
          formIds: [],
        },
      }),
      renderRegisterBatchTbody: () => '<tr class="dataRow"><td colspan="5">ok</td></tr>',
    },
  });

  rendering.renderQuick();

  assert.deepEqual(styleCalls[0], ["--quickViewportPx", "696px"]);
});
