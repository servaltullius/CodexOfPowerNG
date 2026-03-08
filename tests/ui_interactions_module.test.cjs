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
  assert.match(moduleSource, /function saveSettingsFromUi\(/);
  assert.match(moduleSource, /safeCall\("copng_saveSettings", payload\);/);
});
