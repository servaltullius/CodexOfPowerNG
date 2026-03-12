const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "input_correction.js");
const bootstrapModulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_bootstrap.js");

const html = fs.readFileSync(viewPath, "utf8");
const bootstrapSource = fs.readFileSync(bootstrapModulePath, "utf8");
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

function makeRafHarness() {
  const queue = [];
  return {
    requestAnimationFrame(cb) {
      queue.push(cb);
      return queue.length;
    },
    cancelAnimationFrame() {},
    flush(limit = 12) {
      let count = 0;
      while (queue.length > 0 && count < limit) {
        const cb = queue.shift();
        cb();
        count += 1;
      }
    },
  };
}

test("view loads input correction module", () => {
  assert.match(
    html,
    /<script src="input_correction\.js"><\/script>/,
    "index.html should load input_correction.js before inline script",
  );
  assert.match(
    html,
    /<script src="ui_bootstrap\.js"><\/script>/,
    "index.html should load ui_bootstrap.js before inline script",
  );
});

test("input correction module exports expected APIs", () => {
  assert.equal(typeof mod.installDirectWheelScroll, "function");
  assert.equal(typeof mod.installMouseEventCorrection, "function");
  assert.equal(typeof mod.installInputCorrection, "function");
});

test("view keeps observable fallback when module loading fails", () => {
  assert.match(
    bootstrapSource,
    /function installInputCorrectionFallback\(/,
    "ui_bootstrap.js should keep a fallback installer when input_correction.js is unavailable",
  );
  assert.match(
    bootstrapSource,
    /input_correction\.js unavailable; using fallback input correction/,
    "ui_bootstrap.js should log a warning when fallback input correction is used",
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
    requestAnimationFrameFn(cb) {
      cb();
      return 1;
    },
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

test("installDirectWheelScroll gives Quick Register a smoothed immediate step that settles to the full travel", () => {
  const rootEl = makeEventTarget({
    scrollTop: 0,
    scrollHeight: 1600,
    clientHeight: 400,
  });
  const raf = makeRafHarness();

  const detach = mod.installDirectWheelScroll({
    documentObj: {
      querySelector(selector) {
        if (selector === ".section.active") return { id: "tabQuick" };
        return rootEl;
      },
    },
    rootEl,
    clamp: (v, min, max) => Math.max(min, Math.min(max, Number(v))),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 100 },
    requestAnimationFrameFn: (cb) => raf.requestAnimationFrame(cb),
    cancelAnimationFrameFn: () => raf.cancelAnimationFrame(),
  });

  let prevented = false;
  rootEl.fire("wheel", {
    isTrusted: true,
    deltaY: 1,
    deltaMode: 0,
    timeStamp: 100,
    preventDefault: () => {
      prevented = true;
    },
    stopPropagation: () => {},
  });

  assert.equal(prevented, true);
  assert.ok(rootEl.scrollTop > 0 && rootEl.scrollTop < 96, "Quick Register should start moving immediately without snapping to the full travel in one tick");
  raf.flush();
  assert.ok(Math.abs(rootEl.scrollTop - 96) < 1, "Quick Register should settle to the full travel after the short smooth tail");

  detach();
  assert.equal(rootEl.listenerCount("wheel"), 0);
});

test("installDirectWheelScroll gives Quick Register a larger follow-up step during rapid wheel bursts", () => {
  const rootEl = makeEventTarget({
    scrollTop: 0,
    scrollHeight: 3200,
    clientHeight: 500,
  });
  const raf = makeRafHarness();

  const detach = mod.installDirectWheelScroll({
    documentObj: {
      querySelector(selector) {
        if (selector === ".section.active") return { id: "tabQuick" };
        return rootEl;
      },
    },
    rootEl,
    clamp: (v, min, max) => Math.max(min, Math.min(max, Number(v))),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 100 },
    requestAnimationFrameFn: (cb) => raf.requestAnimationFrame(cb),
    cancelAnimationFrameFn: () => raf.cancelAnimationFrame(),
  });

  rootEl.fire("wheel", {
    isTrusted: true,
    deltaY: 1,
    deltaMode: 0,
    timeStamp: 100,
    preventDefault: () => {},
    stopPropagation: () => {},
  });
  raf.flush();
  const firstScrollTop = rootEl.scrollTop;

  rootEl.fire("wheel", {
    isTrusted: true,
    deltaY: 1,
    deltaMode: 0,
    timeStamp: 136,
    preventDefault: () => {},
    stopPropagation: () => {},
  });
  raf.flush();
  const secondDelta = rootEl.scrollTop - firstScrollTop;

  assert.ok(Math.abs(firstScrollTop - 96) < 1);
  assert.ok(
    secondDelta > firstScrollTop,
    "Quick Register should accelerate slightly during rapid wheel bursts instead of feeling like fixed 1-step notches",
  );

  detach();
});

test("installDirectWheelScroll uses a smaller immediate step on Build", () => {
  const rootEl = makeEventTarget({
    scrollTop: 0,
    scrollHeight: 1600,
    clientHeight: 400,
  });

  const detach = mod.installDirectWheelScroll({
    documentObj: {
      querySelector(selector) {
        if (selector === ".section.active") return { id: "tabBuild" };
        return rootEl;
      },
    },
    rootEl,
    clamp: (v, min, max) => Math.max(min, Math.min(max, Number(v))),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 100 },
  });

  rootEl.fire("wheel", {
    isTrusted: true,
    deltaY: 1,
    deltaMode: 0,
    timeStamp: 100,
    preventDefault: () => {},
    stopPropagation: () => {},
  });

  assert.equal(rootEl.scrollTop, 24, "Build should use a smaller immediate reading step than Quick Register");

  detach();
  assert.equal(rootEl.listenerCount("wheel"), 0);
});

test("installDirectWheelScroll routes Build wheel input to the catalog scroller when available", () => {
  const rootEl = makeEventTarget({
    scrollTop: 0,
    scrollHeight: 1600,
    clientHeight: 400,
  });
  const buildCatalogScroller = {
    scrollTop: 0,
    scrollHeight: 2000,
    clientHeight: 360,
  };

  const detach = mod.installDirectWheelScroll({
    documentObj: {
      querySelector(selector) {
        if (selector === ".section.active") return { id: "tabBuild" };
        return rootEl;
      },
      getElementById(id) {
        if (id === "buildCatalogScroller") return buildCatalogScroller;
        return null;
      },
    },
    rootEl,
    clamp: (v, min, max) => Math.max(min, Math.min(max, Number(v))),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 100 },
  });

  rootEl.fire("wheel", {
    isTrusted: true,
    deltaY: 1,
    deltaMode: 0,
    timeStamp: 100,
    preventDefault: () => {},
    stopPropagation: () => {},
  });

  assert.equal(rootEl.scrollTop, 0, "Build root should stay fixed when the catalog scroller is available");
  assert.equal(buildCatalogScroller.scrollTop, 24, "Build wheel should advance the catalog scroller");

  detach();
  assert.equal(rootEl.listenerCount("wheel"), 0);
});

test("installDirectWheelScroll routes Quick Register wheel input to the quick table scroller when available", () => {
  const rootEl = makeEventTarget({
    scrollTop: 0,
    scrollHeight: 2200,
    clientHeight: 600,
  });
  const quickTableScroller = {
    scrollTop: 0,
    scrollHeight: 2600,
    clientHeight: 420,
  };
  const raf = makeRafHarness();

  const detach = mod.installDirectWheelScroll({
    documentObj: {
      querySelector(selector) {
        if (selector === ".section.active") return { id: "tabQuick" };
        return rootEl;
      },
      getElementById(id) {
        if (id === "quickTableScroller") return quickTableScroller;
        return null;
      },
    },
    rootEl,
    clamp: (v, min, max) => Math.max(min, Math.min(max, Number(v))),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 100 },
    requestAnimationFrameFn: (cb) => raf.requestAnimationFrame(cb),
    cancelAnimationFrameFn: () => raf.cancelAnimationFrame(),
  });

  rootEl.fire("wheel", {
    isTrusted: true,
    deltaY: 1,
    deltaMode: 0,
    timeStamp: 100,
    preventDefault: () => {},
    stopPropagation: () => {},
  });
  raf.flush();

  assert.equal(rootEl.scrollTop, 0, "Quick Register root should stay fixed when the quick table scroller is available");
  assert.ok(quickTableScroller.scrollTop > 0, "Quick Register wheel should advance the quick table scroller");

  detach();
  assert.equal(rootEl.listenerCount("wheel"), 0);
});

test("installDirectWheelScroll uses a larger tiny-wheel step for Quick Register than Build", () => {
  const quickRootEl = makeEventTarget({
    scrollTop: 0,
    scrollHeight: 2400,
    clientHeight: 500,
  });
  const buildRootEl = makeEventTarget({
    scrollTop: 0,
    scrollHeight: 2400,
    clientHeight: 500,
  });

  function makeDoc(rootEl, activeTabId) {
    return {
      querySelector(selector) {
        if (selector === ".root") return rootEl;
        if (selector === ".section.active") return { id: activeTabId };
        return null;
      },
    };
  }

  const detachQuick = mod.installDirectWheelScroll({
    documentObj: makeDoc(quickRootEl, "tabQuick"),
    clamp: (v, min, max) => Math.max(min, Math.min(max, Number(v))),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 100 },
  });
  const detachBuild = mod.installDirectWheelScroll({
    documentObj: makeDoc(buildRootEl, "tabBuild"),
    clamp: (v, min, max) => Math.max(min, Math.min(max, Number(v))),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 100 },
  });

  const wheelEvent = {
    isTrusted: true,
    deltaY: 1,
    deltaMode: 0,
    timeStamp: 100,
    preventDefault: () => {},
    stopPropagation: () => {},
  };

  quickRootEl.fire("wheel", wheelEvent);
  buildRootEl.fire("wheel", wheelEvent);

  assert.ok(quickRootEl.scrollTop > 0, "Quick Register should scroll immediately on a tiny wheel delta");
  assert.ok(buildRootEl.scrollTop > 0, "Build should still scroll immediately on a tiny wheel delta");
  assert.ok(
    quickRootEl.scrollTop > buildRootEl.scrollTop,
    "Quick Register should move farther than Build for the same tiny wheel input",
  );

  detachQuick();
  detachBuild();
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
