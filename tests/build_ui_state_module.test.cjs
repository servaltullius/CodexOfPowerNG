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
      attack: { score: 12, unlockedBaselineCount: 1 },
      defense: { score: 4, unlockedBaselineCount: 0 },
      utility: { score: 7, unlockedBaselineCount: 0 },
    },
    activeSlots: [{ slotId: "attack_1", optionId: "build.attack.ferocity" }],
  });

  const build = state.getBuild();
  assert.equal(build.disciplines.attack.score, 12);
  assert.equal(build.disciplines.defense.score, 4);
  assert.equal(Array.isArray(build.activeSlots), true);
  assert.equal(build.activeSlots[0].slotId, "attack_1");

  state.setRewards({ totals: [{ label: "Legacy", total: 1 }] });
  assert.equal(state.getBuild().disciplines.attack.score, 12);
  assert.deepEqual(state.getRewards(), { totals: [{ label: "Legacy", total: 1 }] });
});
