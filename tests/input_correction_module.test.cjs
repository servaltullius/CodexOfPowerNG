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

// --- Mock rAF helpers ---
function makeMockRaf() {
  let nextId = 1;
  const pending = new Map();
  return {
    raf(cb) { const id = nextId++; pending.set(id, cb); return id; },
    caf(id) { pending.delete(id); },
    flush(ts) {
      // Execute all pending callbacks once (snapshot to avoid infinite loop)
      const cbs = [...pending.values()];
      pending.clear();
      for (const cb of cbs) cb(ts);
    },
    pendingCount() { return pending.size; },
  };
}

function makeClamp() {
  return (v, min, max) => Math.max(min, Math.min(max, Number(v)));
}

test("installDirectWheelScroll applies normalized wheel delta", () => {
  const mock = makeMockRaf();
  const rootEl = makeEventTarget({
    scrollTop: 0,
    scrollHeight: 1600,
    clientHeight: 400,
  });

  const detach = mod.installDirectWheelScroll({
    documentObj: { querySelector: () => rootEl },
    rootEl,
    clamp: makeClamp(),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 100 },
    requestAnimationFrameFn: mock.raf,
    cancelAnimationFrameFn: mock.caf,
    nowFn: () => 100,
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
  // Flush one rAF frame to apply the scroll
  mock.flush(116.67);

  assert.ok(rootEl.scrollTop > 0, "wheel handler should update scrollTop after rAF flush");
  assert.ok(rootEl.scrollTop <= 1200, "scrollTop should be clamped within scrollable range");

  detach();
  assert.equal(rootEl.listenerCount("wheel"), 0);
});

test("smooth scroll converges to target", () => {
  const mock = makeMockRaf();
  const rootEl = makeEventTarget({
    scrollTop: 0,
    scrollHeight: 2000,
    clientHeight: 400,
  });

  const detach = mod.installDirectWheelScroll({
    documentObj: { querySelector: () => rootEl },
    rootEl,
    clamp: makeClamp(),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 100 },
    requestAnimationFrameFn: mock.raf,
    cancelAnimationFrameFn: mock.caf,
    nowFn: () => 100,
  });

  rootEl.fire("wheel", {
    isTrusted: true,
    deltaY: 120,
    deltaMode: 0,
    timeStamp: 100,
    preventDefault: () => {},
    stopPropagation: () => {},
  });

  // Flush 60 frames at ~60fps intervals (allows full convergence even with low LERP_BASE)
  let ts = 100;
  for (let i = 0; i < 60; i++) {
    ts += 16.67;
    mock.flush(ts);
  }

  // Should have converged close to target
  assert.ok(rootEl.scrollTop > 0, "should have scrolled");
  // After 60 frames the animation should have settled (no more pending rAFs)
  assert.equal(mock.pendingCount(), 0, "animation should have converged and stopped");

  detach();
});

test("rapid wheel events accumulate delta", () => {
  const mock = makeMockRaf();
  const rootEl = makeEventTarget({
    scrollTop: 0,
    scrollHeight: 4000,
    clientHeight: 400,
  });

  const detach = mod.installDirectWheelScroll({
    documentObj: { querySelector: () => rootEl },
    rootEl,
    clamp: makeClamp(),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 100 },
    requestAnimationFrameFn: mock.raf,
    cancelAnimationFrameFn: mock.caf,
    nowFn: () => 100,
  });

  // Single wheel event
  rootEl.scrollTop = 0;
  rootEl.fire("wheel", {
    isTrusted: true,
    deltaY: 120,
    deltaMode: 0,
    timeStamp: 100,
    preventDefault: () => {},
    stopPropagation: () => {},
  });

  // Flush all frames to convergence
  let ts = 100;
  for (let i = 0; i < 30; i++) { ts += 16.67; mock.flush(ts); }
  const singleResult = rootEl.scrollTop;

  // Reset and do two rapid wheel events
  rootEl.scrollTop = 0;
  detach();

  const mock2 = makeMockRaf();
  const detach2 = mod.installDirectWheelScroll({
    documentObj: { querySelector: () => rootEl },
    rootEl,
    clamp: makeClamp(),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 200 },
    requestAnimationFrameFn: mock2.raf,
    cancelAnimationFrameFn: mock2.caf,
    nowFn: () => 200,
  });

  rootEl.fire("wheel", {
    isTrusted: true, deltaY: 120, deltaMode: 0, timeStamp: 200,
    preventDefault: () => {}, stopPropagation: () => {},
  });
  rootEl.fire("wheel", {
    isTrusted: true, deltaY: 120, deltaMode: 0, timeStamp: 205,
    preventDefault: () => {}, stopPropagation: () => {},
  });

  ts = 200;
  for (let i = 0; i < 30; i++) { ts += 16.67; mock2.flush(ts); }
  const doubleResult = rootEl.scrollTop;

  assert.ok(doubleResult > singleResult, "two rapid wheel events should scroll further than one");

  detach2();
});

test("external scrollTop change resets target", () => {
  const mock = makeMockRaf();
  const rootEl = makeEventTarget({
    scrollTop: 200,
    scrollHeight: 2000,
    clientHeight: 400,
  });

  const detach = mod.installDirectWheelScroll({
    documentObj: { querySelector: () => rootEl },
    rootEl,
    clamp: makeClamp(),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 100 },
    requestAnimationFrameFn: mock.raf,
    cancelAnimationFrameFn: mock.caf,
    nowFn: () => 100,
  });

  rootEl.fire("wheel", {
    isTrusted: true, deltaY: 120, deltaMode: 0, timeStamp: 100,
    preventDefault: () => {}, stopPropagation: () => {},
  });

  // Flush one frame to start animation
  mock.flush(116.67);
  const scrollAfterFirstFrame = rootEl.scrollTop;

  // Simulate external scrollTop change (keyboard navigation)
  rootEl.scrollTop = 800;

  // Flush more frames — animation should adopt the new position
  let ts = 116.67;
  for (let i = 0; i < 20; i++) { ts += 16.67; mock.flush(ts); }

  // Should NOT have returned to the original target area
  assert.ok(rootEl.scrollTop >= 750, "should stay near externally set scrollTop, not revert to old target");

  detach();
});

test("detach cancels animation", () => {
  const mock = makeMockRaf();
  const rootEl = makeEventTarget({
    scrollTop: 0,
    scrollHeight: 2000,
    clientHeight: 400,
  });

  const detach = mod.installDirectWheelScroll({
    documentObj: { querySelector: () => rootEl },
    rootEl,
    clamp: makeClamp(),
    getCurrentUiScale: () => 1,
    performanceObj: { now: () => 100 },
    requestAnimationFrameFn: mock.raf,
    cancelAnimationFrameFn: mock.caf,
    nowFn: () => 100,
  });

  rootEl.fire("wheel", {
    isTrusted: true, deltaY: 120, deltaMode: 0, timeStamp: 100,
    preventDefault: () => {}, stopPropagation: () => {},
  });

  assert.ok(mock.pendingCount() > 0, "rAF should be scheduled after wheel event");

  detach();

  // After detach, pending rAF should be cancelled
  assert.equal(mock.pendingCount(), 0, "detach should cancel pending rAF");

  // Flushing should not throw or change scrollTop
  const scrollBefore = rootEl.scrollTop;
  mock.flush(200);
  assert.equal(rootEl.scrollTop, scrollBefore, "flush after detach should not change scrollTop");
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
