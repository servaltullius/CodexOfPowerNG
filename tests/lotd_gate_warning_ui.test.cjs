const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const renderingPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_rendering.js");
const i18nPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_i18n.js");

function readView() {
  return fs.readFileSync(viewPath, "utf8");
}

function readRendering() {
  return fs.readFileSync(renderingPath, "utf8");
}

function readI18n() {
  return fs.readFileSync(i18nPath, "utf8");
}

test("status bar includes LOTD gate pill and localized labels", () => {
  const html = readView();
  const i18n = readI18n();
  assert.match(html, /id="lotdGate"/);
  assert.match(i18n, /"status\.lotdGate":\s*"LOTD Gate"/);
  assert.match(i18n, /"status\.lotdOk":\s*"TCC ready"/);
  assert.match(i18n, /"status\.lotdBlocked":\s*"TCC missing"/);
  assert.match(i18n, /"status\.lotdGate":\s*"LOTD 게이트"/);
});

test("renderStatus handles LOTD gate blocking and toast", () => {
  const rendering = readRendering();
  assert.match(rendering, /state\s*&&\s*state\.lotdGate/);
  assert.match(rendering, /getLotdGateBlockingToastShown/);
  assert.match(rendering, /toast\.lotdGateBlocked/);
  assert.match(rendering, /classList\.toggle\("warn",\s*blocking\)/);
});

test("settings tab includes LOTD gate warning banner and localized text", () => {
  const html = readView();
  const i18n = readI18n();
  assert.match(html, /id="lotdGateWarnBanner"/);
  assert.match(i18n, /"settings\.lotdGateWarnTitle":\s*"LOTD gate warning"/);
  assert.match(i18n, /"settings\.lotdGateWarnBody":\s*"LOTD Display gate is enabled, but TCC lists are unavailable\./);
  assert.match(i18n, /"settings\.lotdGateWarnTitle":\s*"LOTD 게이트 경고"/);
});

test("renderStatus toggles settings warning banner visibility by gate state", () => {
  const rendering = readRendering();
  assert.match(rendering, /lotdGateWarnBannerEl/);
  assert.match(rendering, /style\.display\s*=\s*enabled\s*&&\s*blocking\s*\?\s*""\s*:\s*"none"/);
});
