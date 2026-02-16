const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "interop_bridge.js");

const html = fs.readFileSync(viewPath, "utf8");
const mod = require(modulePath);

test("view loads interop bridge module", () => {
  assert.match(
    html,
    /<script src="interop_bridge\.js"><\/script>/,
    "index.html should load interop_bridge.js before inline script",
  );
});

test("interop bridge exports expected API", () => {
  assert.equal(typeof mod.parseJsonPayload, "function");
  assert.equal(typeof mod.normalizeInventoryPayload, "function");
  assert.equal(typeof mod.normalizeToastPayload, "function");
  assert.equal(typeof mod.installNativeCallbacks, "function");
});

test("inventory normalization supports array/object/fallback", () => {
  const fromArray = mod.normalizeInventoryPayload([{ formId: 1 }]);
  assert.deepEqual(fromArray, {
    page: 0,
    pageSize: 1,
    total: 1,
    hasMore: false,
    items: [{ formId: 1 }],
  });

  const fromObject = mod.normalizeInventoryPayload({
    page: 3,
    pageSize: 200,
    total: 777,
    hasMore: true,
    items: [{ formId: 2 }],
  });
  assert.equal(fromObject.page, 3);
  assert.equal(fromObject.pageSize, 200);
  assert.equal(fromObject.total, 777);
  assert.equal(fromObject.hasMore, true);
  assert.deepEqual(fromObject.items, [{ formId: 2 }]);

  const fallback = mod.normalizeInventoryPayload(null);
  assert.deepEqual(fallback, {
    page: 0,
    pageSize: 200,
    total: 0,
    hasMore: false,
    items: [],
  });
});

test("installNativeCallbacks wires global callbacks and forwards normalized payloads", () => {
  const win = {};
  const received = {
    state: null,
    inventory: null,
    registered: null,
    rewards: null,
    settings: null,
    toast: null,
  };

  const detach = mod.installNativeCallbacks({
    windowObj: win,
    onState: (v) => {
      received.state = v;
    },
    onInventory: (v) => {
      received.inventory = v;
    },
    onRegistered: (v) => {
      received.registered = v;
    },
    onRewards: (v) => {
      received.rewards = v;
    },
    onSettings: (v) => {
      received.settings = v;
    },
    onToast: (v) => {
      received.toast = v;
    },
  });

  assert.equal(typeof win.copng_setState, "function");
  assert.equal(typeof win.copng_setInventory, "function");
  assert.equal(typeof win.copng_setRegistered, "function");
  assert.equal(typeof win.copng_setRewards, "function");
  assert.equal(typeof win.copng_setSettings, "function");
  assert.equal(typeof win.copng_toast, "function");

  win.copng_setState('{"language":"ko"}');
  win.copng_setInventory('[{"formId":4660}]');
  win.copng_setRegistered('{"oops":1}');
  win.copng_setRewards('{"totals":[{"label":"Health","total":5}]}');
  win.copng_setSettings('{"languageOverride":"en"}');
  win.copng_toast('{"level":"warn","message":"hello"}');

  assert.equal(received.state.language, "ko");
  assert.equal(received.inventory.total, 1);
  assert.deepEqual(received.registered, []);
  assert.deepEqual(received.rewards.totals, [{ label: "Health", total: 5 }]);
  assert.equal(received.settings.languageOverride, "en");
  assert.deepEqual(received.toast, { level: "warn", message: "hello" });

  detach();
  assert.equal(win.copng_setState, undefined);
  assert.equal(win.copng_setInventory, undefined);
});
