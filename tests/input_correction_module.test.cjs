const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "input_correction.js");

const html = fs.readFileSync(viewPath, "utf8");
const mod = require(modulePath);

function makeEventTarget(initial = {}) {
  const listeners = Object.create(null);
  const target = {
    ...initial,
    addEventListener(type, fn, options) {
      if (!listeners[type]) listeners[type] = [];
      listeners[type].push({ fn, options });
    },
    removeEventListener(type, fn) {
      const list = listeners[type] || [];
      listeners[type] = list.filter((it) => it.fn !== fn);
    },
    listenerCount(type) {
      return (listeners[type] || []).length;
    },
    fire(type, event = {}) {
      const list = listeners[type] || [];
      for (const it of list) it.fn(event);
    },
    _listeners: listeners,
  };
  return target;
}

test("view loads input correction module", () => {
  assert.match(
    html,
    /<script src="input_correction\.js"><\/script>/,
    "index.html should load input_correction.js before inline script",
  );
});

test("input correction module exports expected APIs", () => {
  assert.equal(typeof mod.installDirectWheelScroll, "function");
  assert.equal(typeof mod.installMouseEventCorrection, "function");
  assert.equal(typeof mod.installInputCorrection, "function");
});

test("view keeps observable fallback when module loading fails", () => {
  assert.match(
    html,
    /installInputCorrectionFallback\(\)/,
    "index.html should keep a fallback installer when input_correction.js is unavailable",
  );
  assert.match(
    html,
    /input_correction\.js unavailable; using fallback input correction/,
    "index.html should log a warning when fallback input correction is used",
  );
});

test("installDirectWheelScroll applies normalized wheel delta", () => {
  const rootEl = makeEventTarget({
    scrollTop: 0,
    scrollHeight: 1600,
    clientHeight: 400,
  });

  const detach = mod.installDirectWheelScroll({
    documentObj: { querySelector: () => rootEl },
    rootEl,
    clamp: (v, min, max) => Math.max(min, Math.min(max, Number(v))),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 100 },
  });

  let prevented = false;
  rootEl.fire("wheel", {
    isTrusted: true,
    deltaY: 120,
    deltaMode: 0,
    timeStamp: 100,
    preventDefault: () => {
      prevented = true;
    },
    stopPropagation: () => {},
  });

  assert.equal(prevented, true);
  assert.ok(rootEl.scrollTop > 0, "wheel handler should update scrollTop");
  assert.ok(rootEl.scrollTop <= 1200, "scrollTop should be clamped within scrollable range");

  detach();
  assert.equal(rootEl.listenerCount("wheel"), 0);
});

test("installMouseEventCorrection re-dispatches click for non-element mouseup target", () => {
  let dispatchedClick = 0;

  const clickable = {
    nodeType: 1,
    tagName: "BUTTON",
    matches: () => true,
    closest: () => clickable,
    dispatchEvent(ev) {
      if (ev && ev.type === "click") dispatchedClick += 1;
      return true;
    },
  };

  const rootEl = {
    contains: (node) => node === clickable,
    getBoundingClientRect: () => ({ left: 0, top: 0 }),
  };

  const docListeners = Object.create(null);
  const doc = {
    addEventListener(type, fn) {
      if (!docListeners[type]) docListeners[type] = [];
      docListeners[type].push(fn);
    },
    removeEventListener(type, fn) {
      const list = docListeners[type] || [];
      docListeners[type] = list.filter((it) => it !== fn);
    },
    elementFromPoint: () => clickable,
    querySelector: () => rootEl,
  };

  const win = {
    innerWidth: 800,
    innerHeight: 600,
    MouseEvent: function MockMouseEvent(type, init) {
      return { type, ...init };
    },
  };

  const detach = mod.installMouseEventCorrection({
    documentObj: doc,
    windowObj: win,
    rootEl,
    clamp: (v, min, max) => Math.max(min, Math.min(max, Number(v))),
    getInputScale: () => 1,
  });

  const mouseupHandlers = docListeners.mouseup || [];
  assert.equal(mouseupHandlers.length, 1, "mouseup correction handler should be registered");

  mouseupHandlers[0]({
    isTrusted: true,
    button: 0,
    clientX: 120,
    clientY: 80,
    target: { nodeType: 3, parentNode: clickable },
    detail: 1,
    screenX: 100,
    screenY: 50,
    buttons: 1,
    ctrlKey: false,
    shiftKey: false,
    altKey: false,
    metaKey: false,
  });

  assert.equal(dispatchedClick, 1, "mouseup fallback should dispatch synthetic click");

  detach();
  assert.equal((docListeners.mouseup || []).length, 0);
  assert.equal((docListeners.click || []).length, 0);
});

test("installMouseEventCorrection suppresses duplicate native click after synthetic mouseup fix", () => {
  let dispatchedClick = 0;

  const clickable = {
    nodeType: 1,
    tagName: "BUTTON",
    matches: () => true,
    closest: () => clickable,
    dispatchEvent(ev) {
      if (ev && ev.type === "click") dispatchedClick += 1;
      return true;
    },
  };

  const rootEl = {
    contains: (node) => node === clickable,
    getBoundingClientRect: () => ({ left: 0, top: 0 }),
  };

  const docListeners = Object.create(null);
  const doc = {
    addEventListener(type, fn) {
      if (!docListeners[type]) docListeners[type] = [];
      docListeners[type].push(fn);
    },
    removeEventListener(type, fn) {
      const list = docListeners[type] || [];
      docListeners[type] = list.filter((it) => it !== fn);
    },
    elementFromPoint: () => clickable,
    querySelector: () => rootEl,
  };

  const win = {
    innerWidth: 800,
    innerHeight: 600,
    MouseEvent: function MockMouseEvent(type, init) {
      return { type, ...init };
    },
  };

  const detach = mod.installMouseEventCorrection({
    documentObj: doc,
    windowObj: win,
    rootEl,
    clamp: (v, min, max) => Math.max(min, Math.min(max, Number(v))),
    getInputScale: () => 1,
  });

  const mouseup = (docListeners.mouseup || [])[0];
  const click = (docListeners.click || [])[0];
  assert.equal(typeof mouseup, "function");
  assert.equal(typeof click, "function");

  mouseup({
    isTrusted: true,
    button: 0,
    clientX: 64,
    clientY: 32,
    target: { nodeType: 3, parentNode: clickable },
    detail: 1,
    screenX: 100,
    screenY: 50,
    buttons: 1,
    ctrlKey: false,
    shiftKey: false,
    altKey: false,
    metaKey: false,
  });
  assert.equal(dispatchedClick, 1);

  let prevented = false;
  let stopped = false;
  click({
    isTrusted: true,
    button: 0,
    clientX: 64,
    clientY: 32,
    target: { nodeType: 3, parentNode: clickable },
    preventDefault: () => {
      prevented = true;
    },
    stopImmediatePropagation: () => {
      stopped = true;
    },
  });

  assert.equal(prevented, true, "duplicate native click should be prevented");
  assert.equal(stopped, true, "duplicate native click should be stopped");
  assert.equal(dispatchedClick, 1, "duplicate click should not dispatch another synthetic click");

  detach();
});

test("installMouseEventCorrection can recover clicks using scaled candidate points", () => {
  let dispatchedClick = 0;

  const clickable = {
    nodeType: 1,
    tagName: "BUTTON",
    matches: () => true,
    closest: () => clickable,
    dispatchEvent(ev) {
      if (ev && ev.type === "click") dispatchedClick += 1;
      return true;
    },
  };

  const rootEl = {
    contains: (node) => node === clickable,
    getBoundingClientRect: () => ({ left: 0, top: 0 }),
  };

  const docListeners = Object.create(null);
  const doc = {
    addEventListener(type, fn) {
      if (!docListeners[type]) docListeners[type] = [];
      docListeners[type].push(fn);
    },
    removeEventListener(type, fn) {
      const list = docListeners[type] || [];
      docListeners[type] = list.filter((it) => it !== fn);
    },
    // identity (10,10) misses, origin-mul (20,20) hits
    elementFromPoint: (x, y) => (x === 20 && y === 20 ? clickable : null),
    querySelector: () => rootEl,
  };

  const win = {
    innerWidth: 800,
    innerHeight: 600,
    MouseEvent: function MockMouseEvent(type, init) {
      return { type, ...init };
    },
  };

  const detach = mod.installMouseEventCorrection({
    documentObj: doc,
    windowObj: win,
    rootEl,
    clamp: (v, min, max) => Math.max(min, Math.min(max, Number(v))),
    getInputScale: () => 2,
  });

  const click = (docListeners.click || [])[0];
  assert.equal(typeof click, "function");

  let prevented = false;
  click({
    isTrusted: true,
    button: 0,
    clientX: 10,
    clientY: 10,
    target: {
      nodeType: 1,
      tagName: "DIV",
      matches: () => false,
      closest: () => null,
    },
    preventDefault: () => {
      prevented = true;
    },
    stopImmediatePropagation: () => {},
  });

  assert.equal(prevented, true, "scaled correction should prevent native click before re-dispatch");
  assert.equal(dispatchedClick, 1, "scaled candidate should recover target and dispatch synthetic click");

  detach();
});
