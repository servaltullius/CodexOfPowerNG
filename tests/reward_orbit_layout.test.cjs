const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const buildPanelModulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_build_panel.js");
const uiWiringModulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_wiring.js");
const i18nModulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_i18n.js");
const html = fs.readFileSync(viewPath, "utf8");
const buildPanelModuleSource = fs.readFileSync(buildPanelModulePath, "utf8");
const uiWiringModuleSource = fs.readFileSync(uiWiringModulePath, "utf8");
const i18nModuleSource = fs.readFileSync(i18nModulePath, "utf8");
const buildPanel = require(buildPanelModulePath);

test("build tab includes slot and option containers", () => {
  assert.match(html, /id="buildSlotsPanel"/, "Build view should include active slots panel");
  assert.match(html, /id="buildCardsPanel"/, "Build view should include option card panel");
});

test("build view loads build panel module", () => {
  assert.match(
    html,
    /<script src="ui_build_panel\.js"><\/script>/,
    "Build view should load ui_build_panel.js before inline script",
  );
  assert.doesNotMatch(
    html,
    /<script src="reward_orbit\.js"><\/script>/,
    "Active UI bundle should no longer depend on reward_orbit.js",
  );
});

test("build panel module renders slot actions and grouped discipline sections", () => {
  assert.match(buildPanelModuleSource, /function renderBuildPanelHtml\(/);

  const htmlOut = buildPanel.renderBuildPanelHtml(
    {
      disciplines: {
        attack: { score: 12, unlockedBaselineCount: 1 },
        defense: { score: 4, unlockedBaselineCount: 0 },
        utility: { score: 7, unlockedBaselineCount: 0 },
      },
      selectedDiscipline: "attack",
      selectedTheme: "devastation",
      selectedOptionId: "build.attack.ferocity",
      themeMap: {
        attack: [
          { id: "devastation", titleKey: "build.theme.attack.devastation", optionCount: 1 },
          { id: "precision", titleKey: "build.theme.attack.precision", optionCount: 0 },
        ],
        defense: [{ id: "guard", titleKey: "build.theme.defense.guard", optionCount: 0 }],
        utility: [{ id: "livelihood", titleKey: "build.theme.utility.livelihood", optionCount: 0 }],
      },
      options: [
        {
          id: "build.attack.ferocity",
          discipline: "attack",
          themeId: "devastation",
          themeTitleKey: "build.theme.attack.devastation",
          hierarchy: "signpost",
          titleKey: "build.attack.ferocity.title",
          descriptionKey: "build.attack.ferocity.description",
          unlockScore: 5,
          unlocked: true,
          slotCompatibility: "same_or_wildcard",
        },
      ],
      activeSlots: [
        { slotId: "attack_1", slotKind: "attack", optionId: "build.attack.ferocity", occupied: true },
        { slotId: "attack_2", slotKind: "attack", optionId: null, occupied: false },
        { slotId: "defense_1", slotKind: "defense", optionId: null, occupied: false },
        { slotId: "utility_1", slotKind: "utility", optionId: null, occupied: false },
        { slotId: "utility_2", slotKind: "utility", optionId: null, occupied: false },
        { slotId: "wildcard_1", slotKind: "wildcard", optionId: null, occupied: false },
      ],
      migrationNotice: {
        needsNotice: false,
        legacyRewardsMigrated: false,
        unresolvedHistoricalRegistrations: 0,
      },
    },
    {
      t: (_key, fallback) => fallback,
      tFmt: (_key, fallback, vars) =>
        String(fallback).replace(/\{([a-zA-Z0-9_]+)\}/g, (_, name) => String(vars[name])),
      escapeHtml: (value) => String(value == null ? "" : value),
    },
  );
  assert.match(htmlOut, /buildDisciplineButton/);
  assert.match(htmlOut, /buildThemeTab/);
  assert.match(htmlOut, /buildCatalogScroller/);
  assert.match(htmlOut, /buildSelectedOptionPanel/);
  assert.match(htmlOut, /Active Slots/);
  assert.match(htmlOut, /Attack/);
  assert.match(htmlOut, /Deactivate/);
});

test("build panel rerenders on resize", () => {
  assert.match(
    uiWiringModuleSource,
    /addListener\(win, "resize", function \(\) \{[\s\S]*scheduleVirtualRender\(\{ force: true \}\);[\s\S]*renderBuild\(\);[\s\S]*\}\);/,
    "UI wiring should rerender build panel on resize",
  );
});

test("build panel strings exist in both locales", () => {
  assert.match(i18nModuleSource, /"build\.activeSlots": "Active Slots"/);
  assert.match(i18nModuleSource, /"build\.activeSlots": "활성 슬롯"/);
  assert.match(i18nModuleSource, /"build\.activate": "Activate"/);
  assert.match(i18nModuleSource, /"build\.activate": "활성화"/);
  assert.match(
    i18nModuleSource,
    /"build\.help": "Build score opens options permanently, but only your active slots apply to the current build\."/,
  );
  assert.match(
    i18nModuleSource,
    /"build\.help": "빌드 점수로 해금한 옵션은 영구 보유되지만, 현재 빌드에는 활성 슬롯에 넣은 효과만 적용됩니다\."/,
  );
  assert.doesNotMatch(html, /class="small buildHelp"/);
  assert.match(buildPanelModuleSource, /buildCatalogLead/);
});
