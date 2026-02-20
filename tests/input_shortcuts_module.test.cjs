const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "input_shortcuts.js");

const html = fs.readFileSync(viewPath, "utf8");
const mod = require(modulePath);

test("view loads input shortcuts module", () => {
  assert.match(
    html,
    /<script src="input_shortcuts\.js"><\/script>/,
    "index.html should load input_shortcuts.js before inline script",
  );
});

test("input shortcuts module exports key APIs", () => {
  assert.equal(typeof mod.createKeydownHandler, "function");
  assert.equal(typeof mod.installKeydownHandler, "function");
});

test("toggle key triggers native toggle request", () => {
  const calls = [];
  const handler = mod.createKeydownHandler({
    KC: {
      jsCodeToDik: (code) => (code === "F4" ? 0x3e : null),
      jsKeyCodeToDik: () => null,
      parseKeybindInput: () => null,
      dikToKeyName: () => "F4",
    },
    getToggleKeyDik: () => 0x3e,
    safeCall: (name, payload) => calls.push({ name, payload }),
    documentObj: { querySelector: () => ({ id: "tabQuick" }), getElementById: () => null },
  });

  let prevented = false;
  handler({
    key: "F4",
    code: "F4",
    keyCode: 115,
    preventDefault: () => {
      prevented = true;
    },
    target: { tagName: "DIV", isContentEditable: false },
  });

  assert.equal(prevented, true);
  assert.deepEqual(calls[0], { name: "copng_requestToggle", payload: {} });
});

test("capture mode escape cancels capture and shows hint toast", () => {
  const toasts = [];
  let capture = true;

  const handler = mod.createKeydownHandler({
    KC: {},
    t: (_key, fallback) => fallback,
    showToast: (level, message) => toasts.push({ level, message }),
    getCaptureToggleKey: () => capture,
    setCaptureToggleKey: (next) => {
      capture = !!next;
    },
  });

  handler({
    key: "Escape",
    code: "Escape",
    keyCode: 27,
    preventDefault: () => {},
    target: { tagName: "DIV", isContentEditable: false },
  });

  assert.equal(capture, false);
  assert.equal(toasts.length, 1);
  assert.equal(toasts[0].level, "info");
});

test("arrow down cycles quick selection and schedules virtual render", () => {
  let keyNavRaf = 0;
  let selected = 10;
  let scheduled = 0;
  const rafQueue = [];

  const handler = mod.createKeydownHandler({
    KC: {},
    documentObj: { querySelector: () => ({ id: "tabQuick" }), getElementById: () => null },
    getQuickVisibleIds: () => [10, 20, 30],
    getQuickSelectedId: () => selected,
    setQuickSelected: (id) => {
      selected = id;
    },
    scrollQuickIndexIntoView: () => {},
    scheduleVirtualRender: () => {
      scheduled += 1;
    },
    getKeyNavRaf: () => keyNavRaf,
    setKeyNavRaf: (next) => {
      keyNavRaf = next;
    },
    requestAnimationFrameFn: (cb) => {
      rafQueue.push(cb);
      return 1;
    },
  });

  handler({
    key: "ArrowDown",
    code: "ArrowDown",
    keyCode: 40,
    preventDefault: () => {},
    target: { tagName: "DIV", isContentEditable: false },
  });

  assert.equal(keyNavRaf, 1);
  assert.equal(selected, 10);
  assert.equal(scheduled, 0);

  assert.equal(rafQueue.length, 1);
  rafQueue.shift()();

  assert.equal(selected, 20);
  assert.equal(scheduled, 1);
  assert.equal(keyNavRaf, 0);
});

test("numeric shortcuts switch tabs including undo tab", () => {
  const tabs = [];
  const handler = mod.createKeydownHandler({
    KC: {},
    setTab: (tabId) => tabs.push(tabId),
    documentObj: { querySelector: () => ({ id: "tabQuick" }), getElementById: () => null },
  });

  handler({ key: "1", code: "Digit1", keyCode: 49, target: { tagName: "DIV", isContentEditable: false } });
  handler({ key: "2", code: "Digit2", keyCode: 50, target: { tagName: "DIV", isContentEditable: false } });
  handler({ key: "3", code: "Digit3", keyCode: 51, target: { tagName: "DIV", isContentEditable: false } });
  handler({ key: "4", code: "Digit4", keyCode: 52, target: { tagName: "DIV", isContentEditable: false } });
  handler({ key: "5", code: "Digit5", keyCode: 53, target: { tagName: "DIV", isContentEditable: false } });

  assert.deepEqual(tabs, ["tabQuick", "tabRegistered", "tabUndo", "tabRewards", "tabSettings"]);
});
