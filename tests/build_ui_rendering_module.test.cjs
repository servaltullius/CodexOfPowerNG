const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_build_panel.js");

const html = fs.readFileSync(viewPath, "utf8");
const mod = require(modulePath);
const i18n = require(path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_i18n.js"));

test("view loads build panel module before inline bootstrap", () => {
  assert.match(html, /<script src="ui_build_panel\.js"><\/script>/);
});

test("build panel module exports renderer", () => {
  assert.equal(typeof mod.renderBuildPanelHtml, "function");
});

test("build panel renders catalog-first layout with discipline/theme navigation and detail-first rail", () => {
  const translator = i18n.createTranslator({ getLanguage: () => "ko" });
  const renderedHtml = mod.renderBuildPanelHtml(
    {
      disciplines: {
        attack: {
          score: 32,
          currentTier: 3,
          nextTierScore: 40,
          scoreToNextTier: 8,
        },
        defense: {
          score: 30,
          currentTier: 3,
          nextTierScore: 40,
          scoreToNextTier: 10,
        },
        utility: {
          score: 30,
          currentTier: 3,
          nextTierScore: 40,
          scoreToNextTier: 10,
        },
      },
      selectedDiscipline: "attack",
      selectedTheme: "precision",
      themeMap: {
        attack: [
          { id: "devastation", titleKey: "build.theme.attack.devastation", optionCount: 1 },
          { id: "precision", titleKey: "build.theme.attack.precision", optionCount: 1 },
          { id: "fury", titleKey: "build.theme.attack.fury", optionCount: 1 },
        ],
        defense: [
          { id: "guard", titleKey: "build.theme.defense.guard", optionCount: 1 },
          { id: "bastion", titleKey: "build.theme.defense.bastion", optionCount: 1 },
          { id: "resistance", titleKey: "build.theme.defense.resistance", optionCount: 1 },
        ],
        utility: [
          { id: "exploration", titleKey: "build.theme.utility.exploration", optionCount: 1 },
          { id: "livelihood", titleKey: "build.theme.utility.livelihood", optionCount: 1 },
          { id: "trickery", titleKey: "build.theme.utility.trickery", optionCount: 1 },
        ],
      },
      groupedCatalog: {
        attack: {
          themes: [
            {
              id: "precision",
              titleKey: "build.theme.attack.precision",
              optionCount: 1,
            },
          ],
        },
      },
      selectedThemeRows: [
        {
          id: "build.attack.vitals",
          discipline: "attack",
          themeId: "precision",
          hierarchy: "signpost",
          unlockScore: 30,
          unlocked: true,
          titleKey: "build.attack.vitals.title",
          descriptionKey: "build.attack.vitals.description",
          effectKey: "attack_damage_mult",
          slotCompatibility: "same_or_wildcard",
          magnitude: 6,
          baseMagnitude: 3,
          magnitudePerTier: 1,
          currentMagnitude: 6,
          nextMagnitude: 7,
          currentTier: 3,
          nextTierScore: 40,
          scoreToNextTier: 8,
        },
      ],
      selectedOptionDetail: {
        id: "build.attack.vitals",
        discipline: "attack",
        themeId: "precision",
        hierarchy: "signpost",
        unlockScore: 30,
        unlocked: true,
        titleKey: "build.attack.vitals.title",
        descriptionKey: "build.attack.vitals.description",
        effectKey: "attack_damage_mult",
        slotCompatibility: "same_or_wildcard",
        magnitude: 6,
        baseMagnitude: 3,
        magnitudePerTier: 1,
        currentMagnitude: 6,
        nextMagnitude: 7,
        currentTier: 3,
        nextTierScore: 40,
        scoreToNextTier: 8,
      },
      options: [
        {
          id: "build.attack.vitals",
          discipline: "attack",
          themeId: "precision",
          hierarchy: "signpost",
          unlockScore: 30,
          unlocked: true,
          titleKey: "build.attack.vitals.title",
          descriptionKey: "build.attack.vitals.description",
          effectKey: "attack_damage_mult",
          slotCompatibility: "same_or_wildcard",
          magnitude: 6,
          baseMagnitude: 3,
          magnitudePerTier: 1,
          currentMagnitude: 6,
          nextMagnitude: 7,
          currentTier: 3,
          nextTierScore: 40,
          scoreToNextTier: 8,
        },
        {
          id: "build.defense.endurance",
          discipline: "defense",
          themeId: "guard",
          hierarchy: "standard",
          unlockScore: 30,
          unlocked: true,
          titleKey: "build.defense.endurance.title",
          descriptionKey: "build.defense.endurance.description",
          effectKey: "health",
          slotCompatibility: "same_or_wildcard",
          magnitude: 30,
          baseMagnitude: 15,
          magnitudePerTier: 5,
          currentMagnitude: 30,
          nextMagnitude: 35,
          currentTier: 3,
          nextTierScore: 40,
          scoreToNextTier: 10,
        },
        {
          id: "build.utility.mobility",
          discipline: "utility",
          themeId: "exploration",
          hierarchy: "special",
          unlockScore: 30,
          unlocked: true,
          titleKey: "build.utility.mobility.title",
          descriptionKey: "build.utility.mobility.description",
          effectKey: "speed_mult",
          slotCompatibility: "same_or_wildcard",
          magnitude: 5,
          baseMagnitude: 2,
          magnitudePerTier: 1,
          currentMagnitude: 5,
          nextMagnitude: 6,
          currentTier: 3,
          nextTierScore: 40,
          scoreToNextTier: 10,
        },
      ],
      activeSlots: [
        { slotId: "attack_1", slotKind: "attack", optionId: "build.attack.vitals", occupied: true },
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
      t: translator.t,
      tFmt: translator.tFmt,
      escapeHtml: (value) => String(value == null ? "" : value),
    },
  );

  assert.match(renderedHtml, /buildCatalogShell/);
  assert.match(renderedHtml, /buildDisciplineRail/);
  assert.match(renderedHtml, /buildThemeTabs/);
  assert.match(renderedHtml, /buildCatalogScroller/);
  assert.match(renderedHtml, /buildDetailRail/);
  assert.match(renderedHtml, /buildSlotMatrix/);
  assert.match(renderedHtml, /buildSummaryBar/);
  assert.match(renderedHtml, /buildSummaryCard disc-attack/);
  assert.match(renderedHtml, /buildSummaryCard disc-defense/);
  assert.match(renderedHtml, /buildSummaryCard disc-utility/);
  assert.match(renderedHtml, /현재 단계|티어/);
  assert.match(renderedHtml, /다음 강화|다음 단계/);
  assert.match(renderedHtml, /8점/);
  assert.match(renderedHtml, /buildCatalogHeader/);
  assert.match(renderedHtml, /buildThemeTab isActive/);
  assert.match(renderedHtml, /buildThemeTab isActive[\s\S]*정밀/);
  assert.doesNotMatch(renderedHtml, /buildCatalogRow [^"]*isSignpost/);
  assert.doesNotMatch(renderedHtml, /buildCatalogRow [^"]*isSpecial/);
  assert.match(renderedHtml, /buildCatalogState/);
  assert.match(renderedHtml, /buildSelectedOptionPanel/);
  assert.match(renderedHtml, /buildSelectedOptionPanel [^>]*data-wheel-surface="build-detail"/);
  assert.match(renderedHtml, /buildSlotMatrixCard/);
  assert.doesNotMatch(renderedHtml, /buildCatalogActions/);
  assert.match(renderedHtml, /활성 슬롯 효과는 계통 점수 10마다 한 단계씩 강화됩니다\./);
  assert.doesNotMatch(renderedHtml, /rewardCharacterImg/);
  assert.match(renderedHtml, /호환 슬롯|슬롯/);
  assert.match(renderedHtml, /잠김|해금됨|활성 중/);
  assert.match(renderedHtml, /점수 \d+ 필요/);
  assert.match(renderedHtml, /buildFocusActions[\s\S]*buildFocusMeta/);
  assert.match(renderedHtml, /기존 보상이 새 빌드 진행도로 이관되었습니다|과거 등록/);
  assert.match(renderedHtml, /활성화/);
  assert.match(renderedHtml, /비활성화|교체/);
  assert.match(renderedHtml, /공격|방어|유틸/);
  assert.match(renderedHtml, /파괴|정밀|격노/);
  assert.match(renderedHtml, /급소/);
  assert.match(renderedHtml, /슬롯 활성 시 공격 피해가 3% 증가합니다\./);
  assert.match(renderedHtml, /현재 효과/);
  assert.match(renderedHtml, /다음 단계 효과|다음 효과/);
  assert.doesNotMatch(renderedHtml, /인내/);
  assert.doesNotMatch(renderedHtml, /기동/);
  assert.doesNotMatch(renderedHtml, /핵심 표지|일반|특수/);
});

test("build panel renders supported-first option descriptions from grouped catalog rows", () => {
  const translator = i18n.createTranslator({ getLanguage: () => "en" });
  const baseDisciplines = {
    attack: { score: 32, currentTier: 3, nextTierScore: 40, scoreToNextTier: 8 },
    defense: { score: 30, currentTier: 3, nextTierScore: 40, scoreToNextTier: 10 },
    utility: { score: 30, currentTier: 3, nextTierScore: 40, scoreToNextTier: 10 },
  };
  const baseActiveSlots = [
    { slotId: "attack_1", slotKind: "attack", optionId: null, occupied: false },
    { slotId: "attack_2", slotKind: "attack", optionId: null, occupied: false },
    { slotId: "defense_1", slotKind: "defense", optionId: null, occupied: false },
    { slotId: "utility_1", slotKind: "utility", optionId: null, occupied: false },
    { slotId: "utility_2", slotKind: "utility", optionId: null, occupied: false },
    { slotId: "wildcard_1", slotKind: "wildcard", optionId: null, occupied: false },
  ];

  function renderSelection(selectedDiscipline, selectedTheme, rows, focusedOptionId) {
    const selectedOptionDetail = rows.find((row) => row.id === focusedOptionId) || rows[0] || null;
    const activeSlots = baseActiveSlots.map((slot) =>
      slot.slotKind === selectedDiscipline && slot.optionId == null && selectedOptionDetail
        ? { ...slot, optionId: selectedOptionDetail.id, occupied: true }
        : slot,
    );

    return mod.renderBuildPanelHtml(
      {
        disciplines: baseDisciplines,
        selectedDiscipline,
        selectedTheme,
        selectedOptionId: selectedOptionDetail ? selectedOptionDetail.id : "",
        themeMap: {
          attack: [{ id: "precision", titleKey: "build.theme.attack.precision", optionCount: 2 }],
          defense: [{ id: "guard", titleKey: "build.theme.defense.guard", optionCount: 2 }],
          utility: [
            { id: "livelihood", titleKey: "build.theme.utility.livelihood", optionCount: 2 },
            { id: "exploration", titleKey: "build.theme.utility.exploration", optionCount: 2 },
            { id: "trickery", titleKey: "build.theme.utility.trickery", optionCount: 2 },
          ],
        },
        groupedCatalog: {
          [selectedDiscipline]: {
            themes: [{ id: selectedTheme, titleKey: "build.theme." + selectedDiscipline + "." + selectedTheme, optionCount: rows.length }],
          },
        },
        selectedThemeRows: rows,
        selectedOptionDetail,
        options: rows,
        activeSlots,
        migrationNotice: {
          needsNotice: false,
          legacyRewardsMigrated: false,
          unresolvedHistoricalRegistrations: 0,
        },
      },
      {
        t: translator.t,
        tFmt: translator.tFmt,
        escapeHtml: (value) => String(value == null ? "" : value),
      },
    );
  }

  const attackHtml = renderSelection(
    "attack",
    "precision",
    [
      {
        id: "build.attack.pinpoint",
        discipline: "attack",
        themeId: "precision",
        hierarchy: "standard",
        unlockScore: 30,
        unlocked: true,
        titleKey: "build.attack.pinpoint.title",
        descriptionKey: "build.attack.pinpoint.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.attack.vitals",
        discipline: "attack",
        themeId: "precision",
        hierarchy: "special",
        unlockScore: 20,
        unlocked: true,
        titleKey: "build.attack.vitals.title",
        descriptionKey: "build.attack.vitals.description",
        slotCompatibility: "same_or_wildcard",
      },
    ],
    "build.attack.pinpoint",
  );
  assert.match(attackHtml, /Pinpoint/);
  assert.match(attackHtml, /\+2% critical chance while this option is slotted\./);
  assert.match(attackHtml, /\+3% attack damage while this option is slotted\./);

  const devastationHtml = renderSelection(
    "attack",
    "devastation",
    [
      {
        id: "build.attack.brawler",
        discipline: "attack",
        themeId: "devastation",
        hierarchy: "special",
        unlockScore: 25,
        unlocked: true,
        titleKey: "build.attack.brawler.title",
        descriptionKey: "build.attack.brawler.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.attack.destruction",
        discipline: "attack",
        themeId: "devastation",
        hierarchy: "special",
        unlockScore: 30,
        unlocked: true,
        titleKey: "build.attack.destruction.title",
        descriptionKey: "build.attack.destruction.description",
        slotCompatibility: "same_or_wildcard",
      },
    ],
    "build.attack.brawler",
  );
  assert.match(devastationHtml, /Destruction Adept|파괴숙련/);
  assert.match(devastationHtml, /Destruction spells cost less while this option is slotted\.|슬롯 활성 시 파괴 주문 비용이 감소합니다\./);

  const furyHtml = renderSelection(
    "attack",
    "fury",
    [
      {
        id: "build.attack.reserve",
        discipline: "attack",
        themeId: "fury",
        hierarchy: "signpost",
        unlockScore: 10,
        unlocked: true,
        titleKey: "build.attack.reserve.title",
        descriptionKey: "build.attack.reserve.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.attack.secondwind",
        discipline: "attack",
        themeId: "fury",
        hierarchy: "standard",
        unlockScore: 15,
        unlocked: true,
        titleKey: "build.attack.secondwind.title",
        descriptionKey: "build.attack.secondwind.description",
        slotCompatibility: "same_or_wildcard",
      },
    ],
    "build.attack.reserve",
  );
  assert.match(furyHtml, /Reserve/);
  assert.match(furyHtml, /\+2 max stamina while this option is slotted\./);

  const defenseHtml = renderSelection(
    "defense",
    "guard",
    [
      {
        id: "build.defense.bulwark",
        discipline: "defense",
        themeId: "guard",
        hierarchy: "standard",
        unlockScore: 20,
        unlocked: true,
        titleKey: "build.defense.bulwark.title",
        descriptionKey: "build.defense.bulwark.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.defense.endurance",
        discipline: "defense",
        themeId: "guard",
        hierarchy: "special",
        unlockScore: 30,
        unlocked: true,
        titleKey: "build.defense.endurance.title",
        descriptionKey: "build.defense.endurance.description",
        slotCompatibility: "same_or_wildcard",
      },
    ],
    "build.defense.bulwark",
  );
  assert.match(defenseHtml, /Stalwart/);
  assert.match(defenseHtml, /\+8 max health while this option is slotted\./);

  const guardSustainHtml = renderSelection(
    "defense",
    "guard",
    [
      {
        id: "build.defense.recovery",
        discipline: "defense",
        themeId: "guard",
        hierarchy: "standard",
        unlockScore: 25,
        unlocked: true,
        titleKey: "build.defense.recovery.title",
        descriptionKey: "build.defense.recovery.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.defense.restoration",
        discipline: "defense",
        themeId: "guard",
        hierarchy: "special",
        unlockScore: 35,
        unlocked: true,
        titleKey: "build.defense.restoration.title",
        descriptionKey: "build.defense.restoration.description",
        slotCompatibility: "same_or_wildcard",
      },
    ],
    "build.defense.recovery",
  );
  assert.match(guardSustainHtml, /Recovery/);
  assert.match(guardSustainHtml, /Health regenerates faster while this option is slotted\./);

  const bastionSpecialHtml = renderSelection(
    "defense",
    "bastion",
    [
      {
        id: "build.defense.reprisal",
        discipline: "defense",
        themeId: "bastion",
        hierarchy: "special",
        unlockScore: 25,
        unlocked: true,
        titleKey: "build.defense.reprisal.title",
        descriptionKey: "build.defense.reprisal.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.defense.alteration",
        discipline: "defense",
        themeId: "bastion",
        hierarchy: "special",
        unlockScore: 30,
        unlocked: true,
        titleKey: "build.defense.alteration.title",
        descriptionKey: "build.defense.alteration.description",
        slotCompatibility: "same_or_wildcard",
      },
    ],
    "build.defense.reprisal",
  );
  assert.match(bastionSpecialHtml, /Reprisal/);
  assert.match(bastionSpecialHtml, /A portion of incoming damage is reflected while this option is slotted\./);

  const resistanceHtml = renderSelection(
    "defense",
    "resistance",
    [
      {
        id: "build.defense.warding",
        discipline: "defense",
        themeId: "resistance",
        hierarchy: "signpost",
        unlockScore: 10,
        unlocked: true,
        titleKey: "build.defense.warding.title",
        descriptionKey: "build.defense.warding.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.defense.elementalWard",
        discipline: "defense",
        themeId: "resistance",
        hierarchy: "standard",
        unlockScore: 20,
        unlocked: true,
        titleKey: "build.defense.elementalWard.title",
        descriptionKey: "build.defense.elementalWard.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.defense.purification",
        discipline: "defense",
        themeId: "resistance",
        hierarchy: "standard",
        unlockScore: 25,
        unlocked: true,
        titleKey: "build.defense.purification.title",
        descriptionKey: "build.defense.purification.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.defense.absorption",
        discipline: "defense",
        themeId: "resistance",
        hierarchy: "special",
        unlockScore: 35,
        unlocked: false,
        titleKey: "build.defense.absorption.title",
        descriptionKey: "build.defense.absorption.description",
        slotCompatibility: "same_or_wildcard",
      },
    ],
    "build.defense.warding",
  );
  assert.match(resistanceHtml, /Magic Ward/);
  assert.match(resistanceHtml, /Magic resistance increases while this option is slotted\./);
  assert.match(resistanceHtml, /Elemental Ward/);
  assert.match(resistanceHtml, /Fire, frost, and shock resistance increase while this option is slotted\./);
  assert.match(resistanceHtml, /Purification Ward/);
  assert.match(resistanceHtml, /Poison and disease resistance increase while this option is slotted\./);
  assert.match(resistanceHtml, /Absorption/);
  assert.match(resistanceHtml, /Need 35 Score/);

  const livelihoodHtml = renderSelection(
    "utility",
    "livelihood",
    [
      {
        id: "build.utility.cache",
        discipline: "utility",
        themeId: "livelihood",
        hierarchy: "signpost",
        unlockScore: 10,
        unlocked: true,
        titleKey: "build.utility.cache.title",
        descriptionKey: "build.utility.cache.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.utility.smithing",
        discipline: "utility",
        themeId: "livelihood",
        hierarchy: "standard",
        unlockScore: 20,
        unlocked: true,
        titleKey: "build.utility.smithing.title",
        descriptionKey: "build.utility.smithing.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.utility.hauler",
        discipline: "utility",
        themeId: "livelihood",
        hierarchy: "standard",
        unlockScore: 30,
        unlocked: true,
        titleKey: "build.utility.hauler.title",
        descriptionKey: "build.utility.hauler.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.utility.barter",
        discipline: "utility",
        themeId: "livelihood",
        hierarchy: "standard",
        unlockScore: 25,
        unlocked: true,
        titleKey: "build.utility.barter.title",
        descriptionKey: "build.utility.barter.description",
        slotCompatibility: "same_or_wildcard",
      },
    ],
    "build.utility.cache",
  );
  assert.match(livelihoodHtml, /Cache|비축/);
  assert.match(livelihoodHtml, /Need 10 Score/);
  assert.match(livelihoodHtml, /Smithing/);
  assert.match(livelihoodHtml, /Smithing results improve while this option is slotted\./);
  assert.match(livelihoodHtml, /Need 25 Score/);
  assert.match(livelihoodHtml, /Need 30 Score/);

  const livelihoodResourceHtml = renderSelection(
    "utility",
    "livelihood",
    [
      {
        id: "build.utility.magicka",
        discipline: "utility",
        themeId: "livelihood",
        hierarchy: "special",
        unlockScore: 30,
        unlocked: true,
        titleKey: "build.utility.magicka.title",
        descriptionKey: "build.utility.magicka.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.utility.meditation",
        discipline: "utility",
        themeId: "livelihood",
        hierarchy: "standard",
        unlockScore: 30,
        unlocked: true,
        titleKey: "build.utility.meditation.title",
        descriptionKey: "build.utility.meditation.description",
        slotCompatibility: "same_or_wildcard",
      },
    ],
    "build.utility.magicka",
  );
  assert.match(livelihoodResourceHtml, /Magicka Well/);
  assert.match(livelihoodResourceHtml, /\+2 max magicka while this option is slotted\./);

  const explorationHtml = renderSelection(
    "utility",
    "exploration",
    [
      {
        id: "build.utility.wayfinder",
        discipline: "utility",
        themeId: "exploration",
        hierarchy: "standard",
        unlockScore: 20,
        unlocked: true,
        titleKey: "build.utility.wayfinder.title",
        descriptionKey: "build.utility.wayfinder.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.utility.mobility",
        discipline: "utility",
        themeId: "exploration",
        hierarchy: "signpost",
        unlockScore: 30,
        unlocked: true,
        titleKey: "build.utility.mobility.title",
        descriptionKey: "build.utility.mobility.description",
        slotCompatibility: "same_or_wildcard",
      },
    ],
    "build.utility.wayfinder",
  );
  assert.match(explorationHtml, /Wayfinder/);
  assert.match(explorationHtml, /\+1\.5% movement speed while this option is slotted\./);

  const explorationSpecialHtml = renderSelection(
    "utility",
    "exploration",
    [
      {
        id: "build.utility.echo",
        discipline: "utility",
        themeId: "exploration",
        hierarchy: "special",
        unlockScore: 35,
        unlocked: true,
        titleKey: "build.utility.echo.title",
        descriptionKey: "build.utility.echo.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.utility.mobility",
        discipline: "utility",
        themeId: "exploration",
        hierarchy: "signpost",
        unlockScore: 30,
        unlocked: true,
        titleKey: "build.utility.mobility.title",
        descriptionKey: "build.utility.mobility.description",
        slotCompatibility: "same_or_wildcard",
      },
    ],
    "build.utility.echo",
  );
  assert.match(explorationSpecialHtml, /Echo/);
  assert.match(explorationSpecialHtml, /Shout cooldown recovers faster while this option is slotted\./);

  const trickeryHtml = renderSelection(
    "utility",
    "trickery",
    [
      {
        id: "build.utility.sneak",
        discipline: "utility",
        themeId: "trickery",
        hierarchy: "signpost",
        unlockScore: 10,
        unlocked: true,
        titleKey: "build.utility.sneak.title",
        descriptionKey: "build.utility.sneak.description",
        slotCompatibility: "same_or_wildcard",
      },
      {
        id: "build.utility.illusion",
        discipline: "utility",
        themeId: "trickery",
        hierarchy: "special",
        unlockScore: 30,
        unlocked: true,
        titleKey: "build.utility.illusion.title",
        descriptionKey: "build.utility.illusion.description",
        slotCompatibility: "same_or_wildcard",
      },
    ],
    "build.utility.sneak",
  );
  assert.match(trickeryHtml, /Sneak/);
  assert.match(trickeryHtml, /You become harder to detect while this option is slotted\./);
});

test("build panel prefers grouped theme rows and selected detail payloads over refiltering the flat catalog", () => {
  const translator = i18n.createTranslator({ getLanguage: () => "en" });
  const renderedHtml = mod.renderBuildPanelHtml(
    {
      disciplines: {
        attack: { score: 32, currentTier: 3, nextTierScore: 40, scoreToNextTier: 8 },
        defense: { score: 30, currentTier: 3, nextTierScore: 40, scoreToNextTier: 10 },
        utility: { score: 30, currentTier: 3, nextTierScore: 40, scoreToNextTier: 10 },
      },
      selectedDiscipline: "attack",
      selectedTheme: "precision",
      selectedOptionId: "build.attack.vitals",
      themeMap: {
        attack: [
          { id: "devastation", titleKey: "build.theme.attack.devastation", optionCount: 1 },
          { id: "precision", titleKey: "build.theme.attack.precision", optionCount: 1 },
          { id: "fury", titleKey: "build.theme.attack.fury", optionCount: 1 },
        ],
        defense: [{ id: "guard", titleKey: "build.theme.defense.guard", optionCount: 1 }],
        utility: [{ id: "exploration", titleKey: "build.theme.utility.exploration", optionCount: 1 }],
      },
      groupedCatalog: {
        attack: {
          themes: [{ id: "precision", titleKey: "build.theme.attack.precision", optionCount: 1 }],
        },
      },
      selectedThemeRows: [
        {
          id: "build.attack.vitals",
          discipline: "attack",
          themeId: "precision",
          hierarchy: "signpost",
          unlockScore: 30,
          unlocked: true,
          titleKey: "build.attack.vitals.title",
          descriptionKey: "build.attack.vitals.description",
          effectKey: "attack_damage_mult",
          slotCompatibility: "same_or_wildcard",
          currentMagnitude: 6,
          nextMagnitude: 7,
          currentTier: 3,
          nextTierScore: 40,
          scoreToNextTier: 8,
        },
      ],
      selectedOptionDetail: {
        id: "build.attack.vitals",
        discipline: "attack",
        themeId: "precision",
        hierarchy: "signpost",
        unlockScore: 30,
        unlocked: true,
        titleKey: "build.attack.vitals.title",
        descriptionKey: "build.attack.vitals.description",
        effectKey: "attack_damage_mult",
        slotCompatibility: "same_or_wildcard",
        currentMagnitude: 6,
        nextMagnitude: 7,
        currentTier: 3,
        nextTierScore: 40,
        scoreToNextTier: 8,
      },
      options: [
        {
          id: "build.attack.vitals",
          discipline: "attack",
          themeId: "precision",
          hierarchy: "signpost",
          unlockScore: 30,
          unlocked: true,
          titleKey: "build.attack.vitals.title",
          descriptionKey: "build.attack.vitals.description",
          effectKey: "attack_damage_mult",
          slotCompatibility: "same_or_wildcard",
        },
        {
          id: "build.attack.ferocity",
          discipline: "attack",
          themeId: "devastation",
          hierarchy: "signpost",
          unlockScore: 5,
          unlocked: true,
          titleKey: "build.attack.ferocity.title",
          descriptionKey: "build.attack.ferocity.description",
          slotCompatibility: "same_or_wildcard",
        },
      ],
      activeSlots: [
        { slotId: "attack_1", slotKind: "attack", optionId: "build.attack.vitals", occupied: true },
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
      t: translator.t,
      tFmt: translator.tFmt,
      escapeHtml: (value) => String(value == null ? "" : value),
    },
  );

  assert.match(renderedHtml, /Vitals/);
  assert.match(renderedHtml, /buildThemeTab isActive[\s\S]*Precision/);
  assert.doesNotMatch(renderedHtml, /Ferocity/);
});

test("build view source includes a fixed catalog-first layout contract", () => {
  assert.match(html, /\.buildFixedBoard\s*\{[\s\S]*height:\s*var\(--buildViewportPx,\s*min\(calc\(560px \* var\(--uiScale\)\),\s*calc\(56vh\)\)\)/);
  assert.match(html, /\.buildFixedBoard\s*\{[\s\S]*display:\s*flex/);
  assert.match(html, /\.buildFixedBoard\s*\{[\s\S]*flex-direction:\s*column/);
  assert.match(html, /\.buildFixedBoard\s*\{[\s\S]*overflow:\s*hidden/);
  assert.match(html, /\.buildFixedSurface\s*\{[\s\S]*flex:\s*1 1 auto/);
  assert.match(html, /\.buildFixedSurface\s*\{[\s\S]*min-height:\s*0/);
  assert.match(html, /\.buildCatalogShell\s*\{/);
  assert.match(html, /\.buildCatalogLayout\s*\{[\s\S]*grid-template-columns:[\s\S]*minmax\(calc\(136px \* var\(--uiScale\)\),\s*0\.42fr\)[\s\S]*minmax\(0,\s*1\.75fr\)[\s\S]*minmax\(calc\(260px \* var\(--uiScale\)\),\s*0\.78fr\)/);
  assert.match(html, /\.buildDisciplineRail\s*\{/);
  assert.match(html, /\.buildCatalogPanel\s*\{[\s\S]*grid-template-rows:\s*auto auto minmax\(0,\s*1fr\)/);
  assert.match(html, /\.buildThemeTabs\s*\{[\s\S]*flex-wrap:\s*nowrap/);
  assert.match(html, /\.buildThemeTabs\s*\{[\s\S]*overflow-x:\s*auto/);
  assert.match(html, /\.buildCatalogScroller\s*\{[\s\S]*overflow:\s*auto/);
  assert.match(html, /\.buildCatalogScroller\s*\{[\s\S]*overscroll-behavior:\s*contain/);
  assert.match(html, /\.buildDetailRail\s*\{/);
  assert.match(html, /\.buildSlotMatrix\s*\{/);
  assert.match(html, /\.buildSummaryCard\s*\{[\s\S]*rgba\(12,\s*14,\s*22,\s*0\.94\)/);
  assert.match(html, /\.buildSummaryValue\s*\{[\s\S]*font-size:\s*calc\(28px \* var\(--uiScale\)\)/);
  assert.match(html, /\.buildDisciplineButton\s*\{[\s\S]*min-height:\s*calc\(44px \* var\(--uiScale\)\)/);
  assert.match(html, /\.buildSlotMatrixCard\s*\{[\s\S]*padding:\s*calc\(10px \* var\(--uiScale\)\)/);
  assert.match(html, /\.buildCatalogEffect\s*\{[\s\S]*white-space:\s*nowrap/);
  assert.match(html, /\.buildCatalogEffect\s*\{[\s\S]*text-overflow:\s*ellipsis/);
  assert.doesNotMatch(html, /\.buildCatalogRow\.isSignpost\s*\{/);
  assert.doesNotMatch(html, /\.buildCatalogRow\.isSpecial\s*\{/);
  assert.match(html, /\.heroHeader\s*\{/);
  assert.match(html, /\.heroMain\s*\{[\s\S]*display:\s*grid/);
  assert.match(html, /\.heroAside\s*\{[\s\S]*display:\s*flex/);
  assert.match(html, /\.heroAside\s*\{[\s\S]*align-items:\s*flex-start/);
  assert.match(html, /\.heroAside\s*\{[\s\S]*justify-content:\s*flex-end/);
  assert.match(html, /\.heroCharacterPanel\s*\{/);
  assert.match(html, /\.heroCharacterPanel\s+\.rewardCharacterWrap\s*\{/);
  assert.match(html, /\.heroCharacterPanel\s+\.rewardCharacterWrap\s*\{[\s\S]*transform:\s*translateX\(calc\(-20px \* var\(--uiScale\)\)\)/);
  assert.match(html, /\.buildCatalogLayout\s*\{[\s\S]*min-height:\s*0/);
});
