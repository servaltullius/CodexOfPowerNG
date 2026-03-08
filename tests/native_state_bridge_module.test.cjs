const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(
  __dirname,
  "..",
  "PrismaUI",
  "views",
  "codexofpowerng",
  "native_state_bridge.js",
);

const html = fs.readFileSync(viewPath, "utf8");
const mod = require(modulePath);

test("view loads native state bridge module", () => {
  assert.match(
    html,
    /<script src="native_state_bridge\.js"><\/script>/,
    "index.html should load native_state_bridge.js before inline bootstrap",
  );
});

test("native state bridge exports expected API", () => {
  assert.equal(typeof mod.applyDocumentLanguage, "function");
  assert.equal(typeof mod.createNativeStateBridge, "function");
});

test("native state bridge updates state, language, renders, and refresh resync hooks", () => {
  const documentObj = { documentElement: { lang: "en" } };
  const recorded = {
    uiLang: "en",
    state: null,
    inventory: null,
    registered: null,
    rewards: null,
    undoItems: null,
    settings: null,
    order: [],
  };

  const bridge = mod.createNativeStateBridge({
    documentObj,
    getUiLang: () => recorded.uiLang,
    setUiLang: (next) => {
      recorded.uiLang = next;
    },
    setStateValue: (next) => {
      recorded.state = next;
    },
    setInventoryPage: (next) => {
      recorded.inventory = next;
    },
    setRegisteredItems: (next) => {
      recorded.registered = next;
    },
    setRewardsValue: (next) => {
      recorded.rewards = next;
    },
    setUndoItems: (next) => {
      recorded.undoItems = next;
    },
    setSettingsValue: (next) => {
      recorded.settings = next;
    },
    applyI18n: () => recorded.order.push("applyI18n"),
    renderStatus: () => recorded.order.push("renderStatus"),
    renderQuick: () => recorded.order.push("renderQuick"),
    renderRegistered: () => recorded.order.push("renderRegistered"),
    renderUndo: () => recorded.order.push("renderUndo"),
    renderRewards: () => recorded.order.push("renderRewards"),
    renderSettings: () => recorded.order.push("renderSettings"),
    showToast: (level, message) => {
      recorded.toast = { level, message };
    },
    resetQuickVirtualWindow: () => recorded.order.push("resetQuick"),
    resetRegisteredVirtualWindow: () => recorded.order.push("resetRegistered"),
    schedulePostRefreshVirtualResync: () => recorded.order.push("resync"),
  });

  bridge.onState({ language: "ko", registeredCount: 3 });
  assert.equal(recorded.uiLang, "ko");
  assert.equal(documentObj.documentElement.lang, "ko");
  assert.deepEqual(recorded.order.slice(0, 6), [
    "applyI18n",
    "renderStatus",
    "renderQuick",
    "renderRegistered",
    "renderUndo",
    "renderRewards",
  ]);

  bridge.onInventory({ page: 2, items: [] });
  bridge.onRegistered([{ formId: 1 }]);
  assert.deepEqual(recorded.inventory, { page: 2, items: [] });
  assert.deepEqual(recorded.registered, [{ formId: 1 }]);
  assert.ok(recorded.order.includes("resetQuick"));
  assert.ok(recorded.order.includes("resetRegistered"));
  assert.equal(recorded.order.filter((x) => x === "resync").length, 2);

  bridge.onRewards(null);
  bridge.onUndoList({ nope: true });
  bridge.onSettings({ languageOverride: "en" });
  bridge.onToast({ level: "warn", message: "toast" });

  assert.deepEqual(recorded.rewards, { totals: [] });
  assert.deepEqual(recorded.undoItems, []);
  assert.equal(recorded.uiLang, "en");
  assert.equal(documentObj.documentElement.lang, "en");
  assert.deepEqual(recorded.toast, { level: "warn", message: "toast" });
  assert.ok(recorded.order.includes("renderSettings"));
});
