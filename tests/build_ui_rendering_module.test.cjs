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
  assert.match(renderedHtml, /buildSummaryBar/);
  assert.match(renderedHtml, /buildSummaryCard disc-attack/);
  assert.match(renderedHtml, /buildSummaryCard disc-defense/);
  assert.match(renderedHtml, /buildSummaryCard disc-utility/);
  assert.match(renderedHtml, /buildPanels buildShrineGrid buildFixedSurface/);
  assert.match(renderedHtml, /buildSlotStage/);
  assert.match(renderedHtml, /buildSlotCluster/);
  assert.match(renderedHtml, /buildOptionRailHeader/);
  assert.match(renderedHtml, /Build score opens options permanently, but only your active slots apply to the current build\./);
  assert.match(renderedHtml, /buildOptionRailBody/);
  assert.match(renderedHtml, /id="buildCardsScroller"/);
  assert.match(renderedHtml, /buildOptionRail/);
  assert.match(renderedHtml, /buildAltarPanel buildStaticPanel/);
  assert.match(renderedHtml, /buildFocusPanel[\s\S]*buildStaticPanel/);
  assert.doesNotMatch(renderedHtml, /rewardCharacterImg/);
  assert.match(renderedHtml, /Compatible Slots|Slot Match|Slots/);
  assert.match(renderedHtml, /Locked|Unlocked|Active/);
  assert.match(renderedHtml, /Requires|Need .* Score/);
  assert.match(renderedHtml, /Legacy rewards migrated|historical registrations could not be converted/);
  assert.match(renderedHtml, /Activate/);
  assert.match(renderedHtml, /Deactivate|Swap/);
  assert.match(renderedHtml, /Attack|Defense|Utility/);
});

test("build view source includes a fixed, more opaque performance-first layout contract", () => {
  assert.match(html, /\.buildFixedBoard\s*\{[\s\S]*height:\s*var\(--buildViewportPx,\s*min\(calc\(560px \* var\(--uiScale\)\),\s*calc\(56vh\)\)\)/);
  assert.match(html, /\.buildFixedBoard\s*\{[\s\S]*display:\s*flex/);
  assert.match(html, /\.buildFixedBoard\s*\{[\s\S]*flex-direction:\s*column/);
  assert.match(html, /\.buildFixedBoard\s*\{[\s\S]*overflow:\s*hidden/);
  assert.match(html, /\.buildFixedSurface\s*\{[\s\S]*flex:\s*1 1 auto/);
  assert.match(html, /\.buildFixedSurface\s*\{[\s\S]*min-height:\s*0/);
  assert.match(html, /\.buildOptionRail\s*\{[\s\S]*grid-template-rows:\s*auto minmax\(0,\s*1fr\)/);
  assert.match(html, /\.buildOptionRailBody\s*\{[\s\S]*overflow:\s*auto/);
  assert.match(html, /\.buildOptionRailBody\s*\{[\s\S]*overscroll-behavior:\s*contain/);
  assert.match(html, /\.buildSummaryCard\s*\{[\s\S]*rgba\(12,\s*14,\s*22,\s*0\.94\)/);
  assert.doesNotMatch(html, /\.buildSummaryCard\s*\{[^}]*backdrop-filter:/);
  assert.match(html, /\.buildSummaryValue\s*\{[\s\S]*font-size:\s*calc\(32px \* var\(--uiScale\)\)/);
  assert.match(html, /\.buildOptionRail,\s*[\s\S]*\.buildFocusPanel\s*\{[\s\S]*height:\s*100%/);
  assert.match(html, /\.buildAltarPanel\s*\{[\s\S]*rgba\(11,\s*13,\s*19,\s*0\.96\)/);
  assert.match(html, /\.buildFocusPanel\s*\{[\s\S]*rgba\(10,\s*12,\s*18,\s*0\.94\)/);
  assert.match(html, /\.buildSlotCard\.isOccupied\s*\{[\s\S]*box-shadow:\s*inset 0 0 0 1px rgba\(255,\s*255,\s*255,\s*0\.04\)/);
  assert.match(html, /\.buildOptionCard\.isActive\s*\{[\s\S]*rgba\(255,\s*255,\s*255,\s*0\.015\)/);
  assert.match(html, /\.heroHeader\s*\{/);
  assert.match(html, /\.heroMain\s*\{[\s\S]*display:\s*grid/);
  assert.match(html, /\.heroAside\s*\{[\s\S]*display:\s*flex/);
  assert.match(html, /\.heroAside\s*\{[\s\S]*align-items:\s*flex-start/);
  assert.match(html, /\.heroAside\s*\{[\s\S]*justify-content:\s*flex-end/);
  assert.match(html, /\.heroCharacterPanel\s*\{/);
  assert.match(html, /\.heroCharacterPanel\s+\.rewardCharacterWrap\s*\{/);
  assert.match(html, /\.heroCharacterPanel\s+\.rewardCharacterWrap\s*\{[\s\S]*transform:\s*translateX\(calc\(-20px \* var\(--uiScale\)\)\)/);
  assert.match(
    html,
    /#rewardCharacterImg\s*\{[\s\S]*filter:\s*grayscale\(0\.04\)\s+brightness\(0\.92\)\s+contrast\(1\.08\)\s+drop-shadow\(0 calc\(6px \* var\(--uiScale\)\) calc\(12px \* var\(--uiScale\)\) rgba\(0,\s*0,\s*0,\s*0\.28\)\)/,
  );
});
