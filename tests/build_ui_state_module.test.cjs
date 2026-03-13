const test = require("node:test");
const assert = require("node:assert/strict");
const path = require("node:path");

const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_state.js");
const mod = require(modulePath);

test("ui state stores build payloads independently from rewards", () => {
  const state = mod.createUIState({
    documentObj: null,
    windowObj: null,
    refs: {},
    t: (_key, fallback) => fallback,
    safeCall: () => {},
  });

  state.setBuild({
    disciplines: {
      attack: { score: 12, currentTier: 1, nextTierScore: 20, scoreToNextTier: 8 },
      defense: { score: 4, currentTier: 0, nextTierScore: 10, scoreToNextTier: 6 },
      utility: { score: 7, currentTier: 0, nextTierScore: 10, scoreToNextTier: 3 },
    },
    options: [
      {
        id: "build.attack.ferocity",
        discipline: "attack",
        themeId: "devastation",
        currentMagnitude: 6,
        nextMagnitude: 7,
        currentTier: 1,
      },
    ],
    activeSlots: [{ slotId: "attack_1", optionId: "build.attack.ferocity" }],
  });

  const build = state.getBuild();
  assert.equal(build.disciplines.attack.score, 12);
  assert.equal(build.disciplines.attack.currentTier, 1);
  assert.equal(build.disciplines.defense.score, 4);
  assert.equal(Array.isArray(build.activeSlots), true);
  assert.equal(build.activeSlots[0].slotId, "attack_1");
  assert.equal(build.options[0].currentMagnitude, 6);

  state.setRewards({ totals: [{ label: "Legacy", total: 1 }] });
  assert.equal(state.getBuild().disciplines.attack.score, 12);
  assert.deepEqual(state.getRewards(), { totals: [{ label: "Legacy", total: 1 }] });
});

test("ui state keeps build catalog selection independently and normalizes it against the current payload", () => {
  const state = mod.createUIState({
    documentObj: null,
    windowObj: null,
    refs: {},
    t: (_key, fallback) => fallback,
    safeCall: () => {},
  });

  state.setBuild({
    disciplines: {
      attack: { score: 32, currentTier: 3, nextTierScore: 40, scoreToNextTier: 8 },
      defense: { score: 15, currentTier: 1, nextTierScore: 20, scoreToNextTier: 5 },
      utility: { score: 5, currentTier: 0, nextTierScore: 10, scoreToNextTier: 5 },
    },
    themeMap: {
      attack: [
        { id: "devastation", titleKey: "build.theme.attack.devastation", optionCount: 1 },
        { id: "precision", titleKey: "build.theme.attack.precision", optionCount: 2 },
        { id: "fury", titleKey: "build.theme.attack.fury", optionCount: 0 },
      ],
      defense: [{ id: "guard", titleKey: "build.theme.defense.guard", optionCount: 1 }],
      utility: [{ id: "livelihood", titleKey: "build.theme.utility.livelihood", optionCount: 1 }],
    },
    options: [
      { id: "build.attack.ferocity", discipline: "attack", themeId: "devastation", unlocked: true },
      { id: "build.attack.precision", discipline: "attack", themeId: "precision", unlocked: true },
      { id: "build.attack.vitals", discipline: "attack", themeId: "precision", unlocked: true },
      { id: "build.defense.guard", discipline: "defense", themeId: "guard", unlocked: true },
    ],
    activeSlots: [{ slotId: "attack_1", optionId: "build.attack.vitals" }],
  });

  assert.deepEqual(state.getBuildSelection(), {
    discipline: "attack",
    theme: "devastation",
    optionId: "build.attack.ferocity",
  });

  state.setBuildSelection({ discipline: "attack", theme: "precision", optionId: "build.attack.vitals" });
  assert.deepEqual(state.getBuildSelection(), {
    discipline: "attack",
    theme: "precision",
    optionId: "build.attack.vitals",
  });

  state.setBuild({
    disciplines: {
      attack: { score: 32, currentTier: 3, nextTierScore: 40, scoreToNextTier: 8 },
      defense: { score: 15, currentTier: 1, nextTierScore: 20, scoreToNextTier: 5 },
      utility: { score: 5, currentTier: 0, nextTierScore: 10, scoreToNextTier: 5 },
    },
    themeMap: {
      attack: [
        { id: "devastation", titleKey: "build.theme.attack.devastation", optionCount: 1 },
        { id: "precision", titleKey: "build.theme.attack.precision", optionCount: 1 },
      ],
      defense: [{ id: "guard", titleKey: "build.theme.defense.guard", optionCount: 1 }],
      utility: [{ id: "livelihood", titleKey: "build.theme.utility.livelihood", optionCount: 1 }],
    },
    options: [
      { id: "build.attack.ferocity", discipline: "attack", themeId: "devastation", unlocked: true },
      { id: "build.attack.precision", discipline: "attack", themeId: "precision", unlocked: true },
    ],
    activeSlots: [{ slotId: "attack_1", optionId: "build.attack.precision" }],
  });

  assert.deepEqual(state.getBuildSelection(), {
    discipline: "attack",
    theme: "precision",
    optionId: "build.attack.precision",
  });
});

test("ui state prefers grouped theme rows for the active build selection", () => {
  const state = mod.createUIState({
    documentObj: null,
    windowObj: null,
    refs: {},
    t: (_key, fallback) => fallback,
    safeCall: () => {},
  });

  state.setBuild({
    disciplines: {
      attack: { score: 32, currentTier: 3, nextTierScore: 40, scoreToNextTier: 8 },
      defense: { score: 15, currentTier: 1, nextTierScore: 20, scoreToNextTier: 5 },
      utility: { score: 5, currentTier: 0, nextTierScore: 10, scoreToNextTier: 5 },
    },
    themeMap: {
      attack: [
        { id: "devastation", titleKey: "build.theme.attack.devastation", optionCount: 1 },
        { id: "precision", titleKey: "build.theme.attack.precision", optionCount: 1 },
      ],
      defense: [{ id: "guard", titleKey: "build.theme.defense.guard", optionCount: 1 }],
      utility: [{ id: "livelihood", titleKey: "build.theme.utility.livelihood", optionCount: 1 }],
    },
    groupedCatalog: {
      attack: {
        discipline: "attack",
        themes: [
          {
            id: "precision",
            titleKey: "build.theme.attack.precision",
            optionCount: 1,
            rows: [{ id: "build.attack.vitals", discipline: "attack", themeId: "precision", unlocked: true }],
          },
        ],
      },
      defense: { discipline: "defense", themes: [] },
      utility: { discipline: "utility", themes: [] },
    },
    selectedDiscipline: "attack",
    selectedTheme: "precision",
    selectedThemeRows: [{ id: "build.attack.vitals", discipline: "attack", themeId: "precision", unlocked: true }],
    options: [
      { id: "build.attack.ferocity", discipline: "attack", themeId: "devastation", unlocked: true },
      { id: "build.attack.vitals", discipline: "attack", themeId: "precision", unlocked: true },
    ],
    activeSlots: [{ slotId: "attack_1", optionId: "build.attack.vitals" }],
  });

  state.setBuildSelection({ discipline: "attack", theme: "precision", optionId: "build.attack.vitals" });

  const rows = state.getBuildRowsForSelection({ discipline: "attack", theme: "precision" });
  assert.deepEqual(rows.map((row) => row.id), ["build.attack.vitals"]);
});

test("ui state honors payload-selected discipline/theme/option before any local build interaction", () => {
  const state = mod.createUIState({
    documentObj: null,
    windowObj: null,
    refs: {},
    t: (_key, fallback) => fallback,
    safeCall: () => {},
  });

  state.setBuild({
    disciplines: {
      attack: { score: 32, currentTier: 3, nextTierScore: 40, scoreToNextTier: 8 },
      defense: { score: 30, currentTier: 3, nextTierScore: 40, scoreToNextTier: 10 },
      utility: { score: 5, currentTier: 0, nextTierScore: 10, scoreToNextTier: 5 },
    },
    themeMap: {
      attack: [{ id: "devastation", titleKey: "build.theme.attack.devastation", optionCount: 1 }],
      defense: [{ id: "guard", titleKey: "build.theme.defense.guard", optionCount: 1 }],
      utility: [{ id: "livelihood", titleKey: "build.theme.utility.livelihood", optionCount: 1 }],
    },
    selectedDiscipline: "defense",
    selectedTheme: "guard",
    selectedOptionId: "build.defense.endurance",
    selectedThemeRows: [{ id: "build.defense.endurance", discipline: "defense", themeId: "guard", unlocked: true }],
    options: [
      { id: "build.attack.ferocity", discipline: "attack", themeId: "devastation", unlocked: true },
      { id: "build.defense.endurance", discipline: "defense", themeId: "guard", unlocked: true },
    ],
    activeSlots: [{ slotId: "defense_1", optionId: "build.defense.endurance" }],
  });

  assert.deepEqual(state.getBuildSelection(), {
    discipline: "defense",
    theme: "guard",
    optionId: "build.defense.endurance",
  });
});
