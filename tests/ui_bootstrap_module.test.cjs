const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_bootstrap.js");

const html = fs.readFileSync(viewPath, "utf8");
const moduleSource = fs.readFileSync(modulePath, "utf8");
const mod = require(modulePath);

test("view loads ui_bootstrap module before inline bootstrap", () => {
  assert.match(html, /<script src="ui_bootstrap\.js"><\/script>/);
});

test("ui_bootstrap module exports bootstrap and fallback installers", () => {
  assert.equal(typeof mod.initializeUI, "function");
  assert.equal(typeof mod.installInputCorrectionFallback, "function");
  assert.equal(typeof mod.installEscapeCloseFallback, "function");
});

test("index delegates bootstrap layer to ui_bootstrap module", () => {
  assert.match(html, /uiBootstrapApi\.initializeUI\(\{/);
  assert.doesNotMatch(html, /function installInputCorrectionFallback\(/);
  assert.doesNotMatch(html, /safeCall\("copng_requestState", \{\}\);/);
  assert.doesNotMatch(html, /safeCall\("copng_requestUndoList", \{\}\);/);
});

test("ui_bootstrap module owns startup requests and fallback bootstrap", () => {
  assert.match(moduleSource, /safeCall\("copng_requestState", \{\}\);/);
  assert.match(moduleSource, /safeCall\("copng_getSettings", \{\}\);/);
  assert.match(moduleSource, /safeCall\("copng_requestInventory", \{ page: 0, pageSize: 200 \}\);/);
  assert.match(moduleSource, /safeCall\("copng_requestRegistered", \{\}\);/);
  assert.match(moduleSource, /safeCall\("copng_requestUndoList", \{\}\);/);
  assert.match(moduleSource, /safeCall\("copng_requestBuild", \{\}\);/);
  assert.match(moduleSource, /function installInputCorrectionFallback\(/);
  assert.match(moduleSource, /function installEscapeCloseFallback\(/);
});
