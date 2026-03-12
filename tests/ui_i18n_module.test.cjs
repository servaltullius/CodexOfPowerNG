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
  assert.match(moduleSource, /"build\.attack\.vitals\.title":\s*"Vitals"/);
  assert.match(moduleSource, /"build\.attack\.vitals\.description":\s*"\+3% critical chance while this option is slotted\."/);
  assert.match(moduleSource, /"build\.attack\.pinpoint\.title":\s*"Pinpoint"/);
  assert.match(moduleSource, /"build\.attack\.pinpoint\.description":\s*"\+2% critical chance while this option is slotted\."/);
  assert.match(moduleSource, /"build\.defense\.endurance\.title":\s*"Endurance"/);
  assert.match(moduleSource, /"build\.defense\.endurance\.description":\s*"\+15 max health while this option is slotted\."/);
  assert.match(moduleSource, /"build\.defense\.bulwark\.title":\s*"Bulwark"/);
  assert.match(moduleSource, /"build\.defense\.bulwark\.description":\s*"\+8 armor rating while this option is slotted\."/);
  assert.match(moduleSource, /"build\.defense\.warding\.title":\s*"Magic Ward"/);
  assert.match(moduleSource, /"build\.defense\.warding\.description":\s*"\+0\.75% magic resist while this option is slotted\."/);
  assert.match(moduleSource, /"build\.defense\.fireward\.title":\s*"Fire Ward"/);
  assert.match(moduleSource, /"build\.defense\.fireward\.description":\s*"\+1% fire resist while this option is slotted\."/);
  assert.match(moduleSource, /"build\.defense\.absorption\.title":\s*"Absorption"/);
  assert.match(moduleSource, /"build\.defense\.absorption\.description":\s*"\+0\.4% spell absorption chance while this option is slotted\."/);
  assert.match(moduleSource, /"build\.utility\.hauler\.title":\s*"Hauler"/);
  assert.match(moduleSource, /"build\.utility\.hauler\.description":\s*"\+15 carry weight while this option is slotted\."/);
  assert.match(moduleSource, /"build\.utility\.mobility\.title":\s*"Mobility"/);
  assert.match(moduleSource, /"build\.utility\.mobility\.description":\s*"\+3% movement speed while this option is slotted\."/);
  assert.match(moduleSource, /"build\.utility\.wayfinder\.title":\s*"Wayfinder"/);
  assert.match(moduleSource, /"build\.utility\.wayfinder\.description":\s*"\+1\.5% movement speed while this option is slotted\."/);
  assert.match(moduleSource, /"build\.attack\.vitals\.title":\s*"급소"/);
  assert.match(moduleSource, /"build\.attack\.vitals\.description":\s*"슬롯 활성 시 치명타 확률이 3% 증가합니다\."/);
  assert.match(moduleSource, /"build\.attack\.pinpoint\.title":\s*"정조준"/);
  assert.match(moduleSource, /"build\.attack\.pinpoint\.description":\s*"슬롯 활성 시 치명타 확률이 2% 증가합니다\."/);
  assert.match(moduleSource, /"build\.defense\.endurance\.title":\s*"인내"/);
  assert.match(moduleSource, /"build\.defense\.endurance\.description":\s*"슬롯 활성 시 최대 체력이 15 증가합니다\."/);
  assert.match(moduleSource, /"build\.defense\.bulwark\.title":\s*"방벽"/);
  assert.match(moduleSource, /"build\.defense\.bulwark\.description":\s*"슬롯 활성 시 방어력이 8 증가합니다\."/);
  assert.match(moduleSource, /"build\.defense\.warding\.title":\s*"마법결계"/);
  assert.match(moduleSource, /"build\.defense\.warding\.description":\s*"슬롯 활성 시 마법 저항이 0\.75% 증가합니다\."/);
  assert.match(moduleSource, /"build\.defense\.fireward\.title":\s*"화염결계"/);
  assert.match(moduleSource, /"build\.defense\.fireward\.description":\s*"슬롯 활성 시 화염 저항이 1% 증가합니다\."/);
  assert.match(moduleSource, /"build\.defense\.absorption\.title":\s*"주문흡수"/);
  assert.match(moduleSource, /"build\.defense\.absorption\.description":\s*"슬롯 활성 시 주문 흡수 확률이 0\.4% 증가합니다\."/);
  assert.match(moduleSource, /"build\.utility\.hauler\.title":\s*"짐꾼"/);
  assert.match(moduleSource, /"build\.utility\.hauler\.description":\s*"슬롯 활성 시 소지 한도가 15 증가합니다\."/);
  assert.match(moduleSource, /"build\.utility\.mobility\.title":\s*"기동"/);
  assert.match(moduleSource, /"build\.utility\.mobility\.description":\s*"슬롯 활성 시 이동 속도가 3% 증가합니다\."/);
  assert.match(moduleSource, /"build\.utility\.wayfinder\.title":\s*"길잡이"/);
  assert.match(moduleSource, /"build\.utility\.wayfinder\.description":\s*"슬롯 활성 시 이동 속도가 1\.5% 증가합니다\."/);
  assert.match(moduleSource, /"build\.utility\.barter\.description":\s*"슬롯 활성 시 상점 구매\/판매 가격이 10% 유리해집니다\."/);
  assert.match(moduleSource, /return base\.replace/);
});

test("translator normalizes language and formats variables", () => {
  const translator = mod.createTranslator({ getLanguage: () => "ko" });
  assert.equal(translator.t("btn.save"), "저장");
  assert.equal(translator.t("build.attack.pinpoint.title"), "정조준");
  assert.equal(translator.t("build.defense.warding.title"), "마법결계");
  assert.equal(translator.t("build.utility.wayfinder.description"), "슬롯 활성 시 이동 속도가 1.5% 증가합니다.");
  assert.equal(translator.tFmt("rewards.more", "+{n}개 더", { n: 5 }), "+5개 더");
  assert.equal(mod.normalizeLanguage("jp"), "en");
});
