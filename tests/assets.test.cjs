const { test } = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

function readText(relPath) {
  return fs.readFileSync(path.join(__dirname, "..", relPath), "utf8");
}

function readJson(relPath) {
  return JSON.parse(readText(relPath));
}

test("settings template includes corpseExplosion section", () => {
  const j = readJson("SKSE/Plugins/CodexOfPowerNG/settings.json");
  assert.ok(j && typeof j === "object");
  assert.ok(j.corpseExplosion && typeof j.corpseExplosion === "object");
  assert.equal(typeof j.corpseExplosion.enabled, "boolean");
  assert.equal(typeof j.corpseExplosion.chancePct, "number");
  assert.equal(typeof j.corpseExplosion.cooldownMs, "number");
  assert.equal(typeof j.corpseExplosion.vfxMode, "string");
  assert.equal(typeof j.corpseExplosion.vfxEditorID, "string");
  assert.equal(typeof j.corpseExplosion.notify, "boolean");
});

test("native localization includes corpseExploded message", () => {
  const en = readJson("SKSE/Plugins/CodexOfPowerNG/lang/en.json");
  const ko = readJson("SKSE/Plugins/CodexOfPowerNG/lang/ko.json");
  assert.equal(typeof en.msg.corpseExploded, "string");
  assert.equal(typeof ko.msg.corpseExploded, "string");
});

test("PrismaUI view contains corpse explosion settings controls", () => {
  const html = readText("PrismaUI/views/codexofpowerng/index.html");
  assert.match(html, /setCorpseExplEnabled/);
  assert.match(html, /setCorpseExplChance/);
  assert.match(html, /setCorpseExplCooldown/);
  assert.match(html, /setCorpseExplMode/);
  assert.match(html, /setCorpseExplEditorID/);
  assert.match(html, /setCorpseExplNotify/);
  assert.match(html, /corpseModeDropdown/);
});

