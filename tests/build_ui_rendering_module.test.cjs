const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_build_panel.js");

const html = fs.readFileSync(viewPath, "utf8");
const mod = require(modulePath);

test("view loads build panel module before inline bootstrap", () => {
  assert.match(html, /<script src="ui_build_panel\.js"><\/script>/);
});

test("build panel module exports renderer", () => {
  assert.equal(typeof mod.renderBuildPanelHtml, "function");
});

test("build panel renders slots, states, actions, and migration notice", () => {
  const renderedHtml = mod.renderBuildPanelHtml(
    {
      disciplines: {
        attack: { score: 12, unlockedBaselineCount: 1 },
        defense: { score: 4, unlockedBaselineCount: 0 },
        utility: { score: 7, unlockedBaselineCount: 0 },
      },
      options: [
        {
          id: "build.attack.ferocity",
          discipline: "attack",
          unlockScore: 5,
          unlocked: true,
          titleKey: "build.attack.ferocity.title",
          descriptionKey: "build.attack.ferocity.description",
          slotCompatibility: "same_or_wildcard",
        },
        {
          id: "build.defense.guard",
          discipline: "defense",
          unlockScore: 10,
          unlocked: false,
          titleKey: "build.defense.guard.title",
          descriptionKey: "build.defense.guard.description",
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
        needsNotice: true,
        legacyRewardsMigrated: true,
        unresolvedHistoricalRegistrations: 2,
      },
    },
    {
      t: (key, fallback) => fallback || key,
      tFmt: (_key, fallback, vars) =>
        String(fallback).replace(/\{([a-zA-Z0-9_]+)\}/g, (_, name) => String(vars[name])),
      escapeHtml: (value) => String(value == null ? "" : value),
    },
  );

  assert.match(renderedHtml, /Active Slots/);
  assert.match(renderedHtml, /Locked|Unlocked|Active/);
  assert.match(renderedHtml, /Requires|Need .* Score/);
  assert.match(renderedHtml, /Legacy rewards migrated|historical registrations could not be converted/);
  assert.match(renderedHtml, /Activate/);
  assert.match(renderedHtml, /Deactivate|Swap/);
  assert.match(renderedHtml, /Attack|Defense|Utility/);
});


