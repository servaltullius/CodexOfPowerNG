const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_i18n.js");

const html = fs.readFileSync(viewPath, "utf8");
const moduleSource = fs.readFileSync(modulePath, "utf8");
const mod = require(modulePath);

test("view loads ui_i18n module before inline bootstrap", () => {
  assert.match(html, /<script src="ui_i18n\.js"><\/script>/);
});

test("ui_i18n module exports message tables and translator factory", () => {
  assert.equal(typeof mod.createTranslator, "function");
  assert.equal(typeof mod.normalizeLanguage, "function");
  assert.equal(typeof mod.MESSAGES.en["status.lotdGate"], "string");
  assert.equal(typeof mod.MESSAGES.ko["settings.lotdGateWarnTitle"], "string");
});

test("index delegates translation helpers to ui_i18n module", () => {
  assert.match(html, /const uiI18nApi = .*window\.COPNGUII18n/);
  assert.match(html, /const \{ t, tFmt \} = uiI18nApi\.createTranslator\(/);
  assert.doesNotMatch(html, /const I18N = \{/);
});

test("ui_i18n module keeps LOTD and formatting messages in dedicated dictionaries", () => {
  assert.match(moduleSource, /"status\.lotdGate":\s*"LOTD Gate"/);
  assert.match(moduleSource, /"status\.lotdGate":\s*"LOTD 게이트"/);
  assert.match(moduleSource, /"settings\.lotdGateWarnTitle":\s*"LOTD gate warning"/);
  assert.match(moduleSource, /"settings\.lotdGateWarnBody":\s*"LOTD Display gate is enabled, but TCC lists are unavailable\./);
  assert.match(moduleSource, /"build\.focusTitle":\s*"Focused Option"/);
  assert.match(moduleSource, /"build\.focusTitle":\s*"선택 옵션"/);
  assert.match(moduleSource, /"build\.altarTitle":\s*"Build Shrine"/);
  assert.match(moduleSource, /"build\.altarTitle":\s*"빌드 제단"/);
  assert.match(moduleSource, /"build\.compatibleSlots":\s*"Compatible Slots: \{slots\}"/);
  assert.match(moduleSource, /"build\.compatibleSlots":\s*"호환 슬롯: \{slots\}"/);
  assert.match(moduleSource, /return base\.replace/);
});

test("translator normalizes language and formats variables", () => {
  const translator = mod.createTranslator({ getLanguage: () => "ko" });
  assert.equal(translator.t("btn.save"), "저장");
  assert.equal(translator.tFmt("rewards.more", "+{n}개 더", { n: 5 }), "+5개 더");
  assert.equal(mod.normalizeLanguage("jp"), "en");
});
