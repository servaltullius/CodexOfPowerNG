const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");

function readView() {
  return fs.readFileSync(viewPath, "utf8");
}

test("status bar includes LOTD gate pill and localized labels", () => {
  const html = readView();
  assert.match(html, /id="lotdGate"/);
  assert.match(html, /"status\.lotdGate":\s*"LOTD Gate"/);
  assert.match(html, /"status\.lotdOk":\s*"TCC ready"/);
  assert.match(html, /"status\.lotdBlocked":\s*"TCC missing"/);
  assert.match(html, /"status\.lotdGate":\s*"LOTD 게이트"/);
});

test("renderStatus handles LOTD gate blocking and toast", () => {
  const html = readView();
  assert.match(html, /state\s*&&\s*state\.lotdGate/);
  assert.match(html, /lotdGateBlockingToastShown/);
  assert.match(html, /toast\.lotdGateBlocked/);
  assert.match(html, /classList\.toggle\("warn",\s*blocking\)/);
});

test("settings tab includes LOTD gate warning banner and localized text", () => {
  const html = readView();
  assert.match(html, /id="lotdGateWarnBanner"/);
  assert.match(html, /"settings\.lotdGateWarnTitle":\s*"LOTD gate warning"/);
  assert.match(html, /"settings\.lotdGateWarnBody":\s*"LOTD Display gate is enabled, but TCC lists are unavailable\./);
  assert.match(html, /"settings\.lotdGateWarnTitle":\s*"LOTD 게이트 경고"/);
});

test("renderStatus toggles settings warning banner visibility by gate state", () => {
  const html = readView();
  assert.match(html, /lotdGateWarnBannerEl/);
  assert.match(html, /lotdGateWarnBannerEl\.style\.display\s*=\s*enabled\s*&&\s*blocking\s*\?\s*""\s*:\s*"none"/);
});
