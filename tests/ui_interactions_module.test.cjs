const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_interactions.js");

const html = fs.readFileSync(viewPath, "utf8");
const moduleSource = fs.readFileSync(modulePath, "utf8");
const mod = require(modulePath);

test("view loads ui_interactions module before inline bootstrap", () => {
  assert.match(html, /<script src="ui_interactions\.js"><\/script>/);
});

test("ui_interactions module exports interaction factory", () => {
  assert.equal(typeof mod.createUIInteractions, "function");
});

test("index delegates interaction layer to ui_interactions module", () => {
  assert.match(html, /const uiInteractions = uiInteractionsApi\.createUIInteractions\(\{/);
  assert.doesNotMatch(html, /function requestInventoryPage\(/);
  assert.doesNotMatch(html, /function scheduleRenderQuick\(/);
  assert.doesNotMatch(html, /function scheduleRenderRegistered\(/);
  assert.doesNotMatch(html, /function onQuickBodyClick\(/);
  assert.doesNotMatch(html, /function onUndoBodyClick\(/);
  assert.doesNotMatch(html, /function saveSettingsFromUi\(/);
});

test("ui_interactions module owns quick/undo/settings handlers", () => {
  assert.match(moduleSource, /function requestInventoryPage\(/);
  assert.match(moduleSource, /function onQuickBodyClick\(/);
  assert.match(moduleSource, /function onUndoBodyClick\(/);
  assert.match(moduleSource, /function onBatchRegisterClick\(/);
  assert.match(moduleSource, /function toggleQuickBatchSelection\(/);
  assert.match(moduleSource, /build-select-discipline/);
  assert.match(moduleSource, /build-select-theme/);
  assert.match(moduleSource, /build-select-option/);
  assert.match(moduleSource, /function saveSettingsFromUi\(/);
  assert.match(moduleSource, /safeCall\("copng_saveSettings", payload\);/);
});

test("build interactions update build selection for discipline, theme, and option rows", () => {
  const calls = [];
  const renders = [];
  let selection = { discipline: "attack", theme: "devastation", optionId: "" };
  const buildPanelEl = { nodeType: 1 };

  function makeNode(attrs, parentNode) {
    return {
      nodeType: 1,
      parentNode,
      getAttribute(name) {
        return Object.prototype.hasOwnProperty.call(attrs, name) ? attrs[name] : "";
      },
    };
  }

  const api = mod.createUIInteractions({
    buildPanelEl,
    renderBuild: () => renders.push({ ...selection }),
    stateApi: {
      getInventoryPage: () => ({ pageSize: 200 }),
      coalesce: (value, fallback) => (value == null ? fallback : value),
      getQuickSelectedId: () => 0,
      setQuickSelectedId: () => {},
      getQuickVirtual: () => ({ tbodyTopPx: 0, rowHeightPx: 0 }),
      getCurrentUiScale: () => 1,
      toHex32: (value) => String(value >>> 0),
      getToggleKeyDik: () => 0,
      setToggleKeyInputFromDik: () => {},
      showToast: () => {},
      clamp: (value, lo, hi) => Math.max(lo, Math.min(hi, value)),
      getInputScale: () => 1,
      setUiScaleMode: () => {},
      setUiScaleManual: () => {},
      getUiScaleManual: () => 1,
      applyManualUiScale: () => {},
      saveUiScalePrefs: () => {},
      syncUiScaleControls: () => {},
      setInputScale: () => {},
      setPerfMode: () => {},
      savePerfModePref: () => {},
      applyPerfModeFromPrefs: () => {},
      applyUiScaleFromPrefs: () => {},
      getUiScaleMode: () => "auto",
      setBuildSelection: (patch) => {
        selection = Object.assign({}, selection, patch);
      },
    },
    safeCall: (name, payload) => calls.push({ name, payload }),
  });

  api.onBuildPanelClick({ target: makeNode({ "data-action": "build-select-discipline", "data-discipline": "utility" }, buildPanelEl) });
  api.onBuildPanelClick({ target: makeNode({ "data-action": "build-select-theme", "data-theme-id": "exploration" }, buildPanelEl) });
  api.onBuildPanelClick({ target: makeNode({ "data-action": "build-select-option", "data-option-id": "build.utility.mobility" }, buildPanelEl) });

  assert.deepEqual(selection, {
    discipline: "utility",
    theme: "exploration",
    optionId: "build.utility.mobility",
  });
  assert.equal(renders.length, 3);
  assert.deepEqual(calls, []);
});

test("ui interactions supports select-then-confirm batch registration", () => {
  const calls = [];
  let selected = [];

  const interactions = mod.createUIInteractions({
    documentObj: null,
    rootScrollEl: null,
    quickBody: null,
    undoBody: null,
    buildPanelEl: null,
    stateApi: {
      getInventoryPage: () => ({ sections: [] }),
      getQuickSelectedId: () => 0,
      setQuickSelectedId: () => {},
      getQuickVirtual: () => ({ tbodyTopPx: NaN, rowHeightPx: 0 }),
      getCurrentUiScale: () => 1,
      coalesce: (value, fallback) => (value == null ? fallback : value),
      clamp: (value, lo, hi) => Math.max(lo, Math.min(hi, value)),
      toHex32: (value) => String(value >>> 0),
      getQuickBatchSelectedIds: () => selected,
      setQuickBatchSelectedIds: (next) => {
        selected = Array.isArray(next) ? next.slice() : [];
      },
    },
    safeCall: (name, payload) => {
      calls.push({ name, payload });
    },
    renderQuick: () => {},
    renderRegistered: () => {},
    registerBatchPanelApi: {
      buildRegisterBatchViewModel: (_inventoryPage, selectedIds) => ({
        summary: {
          selectedRows: selectedIds.length,
          disciplineGain: { attack: 2, defense: 0, utility: 1 },
          formIds: selectedIds.slice(),
        },
      }),
    },
  });

  interactions.toggleQuickBatchSelection(46775);
  interactions.toggleQuickBatchSelection(61234);
  interactions.toggleQuickBatchSelection(71234);

  assert.deepEqual(selected, [46775, 61234, 71234]);

  interactions.onBatchRegisterClick();
  assert.deepEqual(calls[0], {
    name: "copng_requestRegisterBatch",
    payload: { formIds: [46775, 61234, 71234] },
  });
});

test("build swap requests use distinct source and destination slots", () => {
  const calls = [];
  const button = {
    nodeType: 1,
    getAttribute(name) {
      return {
        "data-action": "build-swap",
        "data-option-id": "build.attack.ferocity",
        "data-from-slot-id": "attack_1",
        "data-to-slot-id": "wildcard_1",
      }[name] || "";
    },
    parentNode: null,
  };
  const buildPanelEl = { nodeType: 1 };
  button.parentNode = buildPanelEl;

  const api = mod.createUIInteractions({
    buildPanelEl,
    stateApi: {
      getInventoryPage: () => ({ pageSize: 200 }),
      coalesce: (value, fallback) => (value == null ? fallback : value),
      getQuickSelectedId: () => 0,
      setQuickSelectedId: () => {},
      getQuickVirtual: () => ({ tbodyTopPx: 0, rowHeightPx: 0 }),
      getCurrentUiScale: () => 1,
      toHex32: (value) => String(value >>> 0),
      getToggleKeyDik: () => 0,
      setToggleKeyInputFromDik: () => {},
      showToast: () => {},
      clamp: (value, lo, hi) => Math.max(lo, Math.min(hi, value)),
      getInputScale: () => 1,
      setUiScaleMode: () => {},
      setUiScaleManual: () => {},
      getUiScaleManual: () => 1,
      applyManualUiScale: () => {},
      saveUiScalePrefs: () => {},
      syncUiScaleControls: () => {},
      setInputScale: () => {},
      setPerfMode: () => {},
      savePerfModePref: () => {},
      applyPerfModeFromPrefs: () => {},
      applyUiScaleFromPrefs: () => {},
      getUiScaleMode: () => "auto",
    },
    safeCall: (name, payload) => calls.push({ name, payload }),
  });

  api.onBuildPanelClick({ target: button });

  assert.deepEqual(calls, [
    {
      name: "copng_swapBuildOption",
      payload: {
        optionId: "build.attack.ferocity",
        fromSlotId: "attack_1",
        toSlotId: "wildcard_1",
      },
    },
  ]);
});
