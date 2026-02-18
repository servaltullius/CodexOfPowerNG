const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_wiring.js");

const html = fs.readFileSync(viewPath, "utf8");
const mod = require(modulePath);

function makeElement(id) {
  const listeners = Object.create(null);
  return {
    id,
    value: "",
    checked: false,
    dataset: Object.create(null),
    classList: { add() {}, remove() {}, toggle() {} },
    addEventListener(type, fn) {
      if (!listeners[type]) listeners[type] = [];
      listeners[type].push(fn);
    },
    fire(type, event = {}) {
      const handlers = listeners[type] || [];
      const e = {
        target: this,
        preventDefault() {},
        stopPropagation() {},
        ...event,
      };
      for (const fn of handlers) fn(e);
    },
    contains(node) {
      return node === this;
    },
  };
}

test("view loads ui wiring module", () => {
  assert.match(
    html,
    /<script src="ui_wiring\.js"><\/script>/,
    "index.html should load ui_wiring.js before inline script",
  );
});

test("ui wiring module exports installUIWiring", () => {
  assert.equal(typeof mod.installUIWiring, "function");
});

test("installUIWiring binds core controls and forwards callbacks", async () => {
  const ids = [
    "btnRefreshState",
    "btnClose",
    "btnRefreshInv",
    "btnRefreshReg",
    "btnRefreshRewards",
    "btnRecoverCarryWeight",
    "btnRefundRewards",
    "btnReloadSettings",
    "btnSaveSettings",
    "btnCloseSettings",
    "quickFilter",
    "regFilter",
    "setToggleKey",
    "btnCaptureToggleKey",
    "langBtn",
    "langMenu",
    "langDropdown",
    "setLang",
    "setUiScaleMode",
    "setUiScaleRange",
    "setUiScaleNumber",
    "setPerfMode",
    "setInputScaleRange",
    "setInputScaleNumber",
    "setInputScalePreset",
  ];

  const map = new Map(ids.map((id) => [id, makeElement(id)]));
  const tabBtn = makeElement("tabBtn");
  tabBtn.dataset.tab = "tabQuick";
  const quickBody = makeElement("quickBody");
  const rewardCharacterImgEl = makeElement("rewardCharacterImg");
  const invPageSizeEl = makeElement("invPageSize");
  const btnInvPrev = makeElement("btnInvPrev");
  const btnInvNext = makeElement("btnInvNext");
  const rootScrollEl = makeElement("rootScroll");

  map.set("quickBody", quickBody);
  map.set("rewardCharacterImg", rewardCharacterImgEl);
  map.set("invPageSize", invPageSizeEl);

  const doc = {
    getElementById(id) {
      return map.get(id) || null;
    },
    querySelectorAll(sel) {
      if (sel === ".tabs button") return [tabBtn];
      return [];
    },
  };

  const winListeners = Object.create(null);
  const win = {
    addEventListener(type, fn) {
      if (!winListeners[type]) winListeners[type] = [];
      winListeners[type].push(fn);
    },
  };

  const safeCalls = [];
  const pageRequests = [];
  const inventory = { page: 2, pageSize: 200 };
  const setPageSize = [];
  let renderQuickInputs = 0;
  let renderRegInputs = 0;
  let saveSettingsCount = 0;
  let captureToggle = false;
  let toastInfoCount = 0;
  let perfModeRaw = "";
  let uiScaleModeRaw = "";
  const manualScaleRaw = [];
  const inputScaleRaw = [];
  let syncRewardImageCount = 0;
  let syncLangDropdownCount = 0;
  let loadUiScalePrefsCount = 0;
  let syncUiScaleControlsCount = 0;
  let applyUiScaleFromPrefsCount = 0;
  let syncInputScaleControlsCount = 0;
  let closeLangMenuCount = 0;
  let openLangMenuCount = 0;
  let quickBodyClickCount = 0;
  let setTabValue = "";

  mod.installUIWiring({
    documentObj: doc,
    windowObj: win,
    rootScrollEl,
    quickBody,
    rewardCharacterImgEl,
    invPageSizeEl,
    btnInvPrev,
    btnInvNext,
    safeCall: (name, payload) => safeCalls.push({ name, payload }),
    setTab: (tab) => {
      setTabValue = tab;
    },
    syncRewardCharacterImageState: () => {
      syncRewardImageCount += 1;
    },
    requestInventoryPage: (page) => {
      pageRequests.push(page);
    },
    getInventoryPage: () => inventory,
    setInventoryPageSize: (next) => {
      setPageSize.push(next);
      inventory.pageSize = next;
    },
    showConfirm: async () => true,
    t: (_key, fallback) => fallback,
    scheduleRenderQuick: () => {
      renderQuickInputs += 1;
    },
    scheduleRenderRegistered: () => {
      renderRegInputs += 1;
    },
    scheduleVirtualRender: () => {},
    renderRewards: () => {},
    onQuickBodyClick: () => {
      quickBodyClickCount += 1;
    },
    onSaveSettings: () => {
      saveSettingsCount += 1;
    },
    updateToggleKeyResolved: () => {},
    setCaptureToggleKey: (next) => {
      captureToggle = !!next;
    },
    showToast: (level) => {
      if (level === "info") toastInfoCount += 1;
    },
    closeLangMenu: () => {
      closeLangMenuCount += 1;
    },
    openLangMenu: () => {
      openLangMenuCount += 1;
    },
    syncLangDropdown: () => {
      syncLangDropdownCount += 1;
    },
    isLangMenuOpen: () => false,
    loadUiScalePrefs: () => {
      loadUiScalePrefsCount += 1;
    },
    syncUiScaleControls: () => {
      syncUiScaleControlsCount += 1;
    },
    applyUiScaleFromPrefs: () => {
      applyUiScaleFromPrefsCount += 1;
    },
    syncInputScaleControls: () => {
      syncInputScaleControlsCount += 1;
    },
    scheduleAutoUiScale: () => {},
    getPerfMode: () => "auto",
    onPerfModeChanged: (raw) => {
      perfModeRaw = raw;
    },
    onUiScaleModeChanged: (raw) => {
      uiScaleModeRaw = raw;
    },
    onManualScaleChange: (raw) => {
      manualScaleRaw.push(raw);
    },
    onInputScaleChange: (raw, opts) => {
      inputScaleRaw.push({ raw, opts: opts || {} });
    },
  });

  tabBtn.fire("click");
  assert.equal(setTabValue, "tabQuick");

  map.get("btnRefreshState").fire("click");
  assert.deepEqual(safeCalls.slice(0, 2), [
    { name: "copng_requestState", payload: {} },
    { name: "copng_getSettings", payload: {} },
  ]);

  map.get("btnClose").fire("click");
  assert.equal(safeCalls[2].name, "copng_requestToggle");

  invPageSizeEl.value = "300";
  invPageSizeEl.fire("change");
  assert.equal(setPageSize[0], 300);
  assert.equal(pageRequests[0], 0);

  btnInvPrev.fire("click");
  btnInvNext.fire("click");
  assert.deepEqual(pageRequests.slice(1, 3), [1, 3]);

  map.get("quickFilter").fire("input");
  map.get("regFilter").fire("input");
  assert.equal(renderQuickInputs, 1);
  assert.equal(renderRegInputs, 1);

  quickBody.fire("click");
  assert.equal(quickBodyClickCount, 1);

  map.get("btnSaveSettings").fire("click");
  assert.equal(saveSettingsCount, 1);

  map.get("btnCaptureToggleKey").fire("click");
  assert.equal(captureToggle, true);
  assert.equal(toastInfoCount, 1);

  map.get("setPerfMode").value = "on";
  map.get("setPerfMode").fire("change");
  assert.equal(perfModeRaw, "on");

  map.get("setUiScaleMode").value = "manual";
  map.get("setUiScaleMode").fire("change");
  assert.equal(uiScaleModeRaw, "manual");

  map.get("setUiScaleRange").value = "1.5";
  map.get("setUiScaleRange").fire("input");
  map.get("setUiScaleNumber").value = "1.6";
  map.get("setUiScaleNumber").fire("input");
  assert.deepEqual(manualScaleRaw, ["1.5", "1.6"]);

  map.get("setInputScaleRange").value = "1.75";
  map.get("setInputScaleRange").fire("input");
  map.get("setInputScaleNumber").value = "1.80";
  map.get("setInputScaleNumber").fire("input");
  map.get("setInputScalePreset").value = "2.0";
  map.get("setInputScalePreset").fire("change");
  assert.equal(inputScaleRaw.length, 3);
  assert.deepEqual(inputScaleRaw[2], { raw: "2.0", opts: { toast: true } });

  map.get("btnRefundRewards").fire("click");
  await Promise.resolve();
  const refundCall = safeCalls.find((it) => it.name === "copng_refundRewards");
  assert.ok(refundCall, "Refund action should call copng_refundRewards");

  map.get("btnRecoverCarryWeight").fire("click");
  await Promise.resolve();
  const recoverCall = safeCalls.find((it) => it.name === "copng_recoverCarryWeight");
  assert.ok(recoverCall, "Recover action should call copng_recoverCarryWeight");

  assert.equal(syncRewardImageCount, 1);
  assert.equal(syncLangDropdownCount, 1);
  assert.equal(loadUiScalePrefsCount, 1);
  assert.equal(syncUiScaleControlsCount, 1);
  assert.equal(applyUiScaleFromPrefsCount, 1);
  assert.equal(syncInputScaleControlsCount, 1);
  assert.equal(openLangMenuCount, 0);
  assert.equal(closeLangMenuCount, 0);
});
