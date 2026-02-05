const { test } = require("node:test");
const assert = require("node:assert/strict");

let ui = null;
try {
  ui = require("../PrismaUI/views/codexofpowerng/lang_ui.js");
} catch {
  ui = null;
}

test("lang_ui module exists", () => {
  assert.ok(ui, "Expected PrismaUI/views/codexofpowerng/lang_ui.js to exist and export functions");
});

test("normalizeLanguageValue accepts auto/en/ko and defaults to auto", () => {
  assert.equal(ui.normalizeLanguageValue("auto"), "auto");
  assert.equal(ui.normalizeLanguageValue("en"), "en");
  assert.equal(ui.normalizeLanguageValue("ko"), "ko");
  assert.equal(ui.normalizeLanguageValue("EN"), "en");
  assert.equal(ui.normalizeLanguageValue(""), "auto");
  assert.equal(ui.normalizeLanguageValue("fr"), "auto");
});

test("getLanguageItems uses provided translator", () => {
  const t = (key, fallback) => ({ "lang.auto": "AUTO", "lang.en": "EN", "lang.ko": "KO" }[key] || fallback || key);
  const items = ui.getLanguageItems(t);
  assert.deepEqual(items, [
    { value: "auto", label: "AUTO" },
    { value: "en", label: "EN" },
    { value: "ko", label: "KO" },
  ]);
});

