const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_rendering.js");

const html = fs.readFileSync(viewPath, "utf8");
const moduleSource = fs.readFileSync(modulePath, "utf8");
const mod = require(modulePath);

test("view loads ui_rendering module before inline bootstrap", () => {
  assert.match(
    html,
    /<script src="ui_rendering\.js"><\/script>/,
    "index.html should load ui_rendering.js before the inline bootstrap",
  );
});

test("ui_rendering module exports rendering factory and helpers", () => {
  assert.equal(typeof mod.createUIRendering, "function");
  assert.equal(typeof mod.escapeHtml, "function");
  assert.equal(typeof mod.sanitizeI18nHtml, "function");
});

test("index delegates render layer to ui_rendering module", () => {
  assert.match(html, /const uiRendering = uiRenderingApi\.createUIRendering\(\{/);
  assert.match(html, /const \{\s*applyI18n,[\s\S]*renderSettings,[\s\S]*\} = uiRendering;/);
  assert.doesNotMatch(html, /function renderStatus\(/);
  assert.doesNotMatch(html, /function renderQuick\(/);
  assert.doesNotMatch(html, /function renderRegistered\(/);
  assert.doesNotMatch(html, /function renderUndo\(/);
  assert.doesNotMatch(html, /function renderRewards\(/);
  assert.doesNotMatch(html, /function renderSettings\(/);
  assert.doesNotMatch(html, /function applyI18n\(/);
});

test("ui_rendering module owns i18n, tab, and list/reward/settings renderers", () => {
  assert.match(moduleSource, /function applyI18n\(/);
  assert.match(moduleSource, /function setTab\(/);
  assert.match(moduleSource, /function renderStatus\(/);
  assert.match(moduleSource, /function renderQuick\(/);
  assert.match(moduleSource, /function renderRegistered\(/);
  assert.match(moduleSource, /function renderUndo\(/);
  assert.match(moduleSource, /function renderRewards\(/);
  assert.match(moduleSource, /function renderSettings\(/);
});
