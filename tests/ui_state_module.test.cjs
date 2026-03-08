const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_state.js");

const html = fs.readFileSync(viewPath, "utf8");
const moduleSource = fs.readFileSync(modulePath, "utf8");
const mod = require(modulePath);

test("view loads ui_state module before inline bootstrap", () => {
  assert.match(html, /<script src="ui_state\.js"><\/script>/);
});

test("ui_state module exports state factory", () => {
  assert.equal(typeof mod.createUIState, "function");
});

test("index delegates state layer to ui_state module", () => {
  assert.match(html, /uiState = uiStateApi\.createUIState\(\{/);
  assert.doesNotMatch(html, /function showToast\(/);
  assert.doesNotMatch(html, /function updateToggleKeyResolved\(/);
  assert.doesNotMatch(html, /function setToggleKeyInputFromDik\(/);
  assert.doesNotMatch(html, /function setInputScale\(/);
  assert.doesNotMatch(html, /function applyUiScaleFromPrefs\(/);
});

test("ui_state module owns preferences, toast, and keybind helpers", () => {
  assert.match(moduleSource, /function loadInputScalePref\(/);
  assert.match(moduleSource, /function applyUiScaleFromPrefs\(/);
  assert.match(moduleSource, /function showToast\(/);
  assert.match(moduleSource, /function getToggleKeyDik\(/);
  assert.match(moduleSource, /function updateToggleKeyResolved\(/);
});
