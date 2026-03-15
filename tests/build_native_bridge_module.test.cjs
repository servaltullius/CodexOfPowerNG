const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

function read(relPath) {
  return fs.readFileSync(path.join(__dirname, "..", relPath), "utf8");
}

const interopBridge = require(path.join(
  __dirname,
  "..",
  "PrismaUI",
  "views",
  "codexofpowerng",
  "interop_bridge.js",
));
const nativeStateBridge = require(path.join(
  __dirname,
  "..",
  "PrismaUI",
  "views",
  "codexofpowerng",
  "native_state_bridge.js",
));
const nativeBridgeBootstrap = require(path.join(
  __dirname,
  "..",
  "PrismaUI",
  "views",
  "codexofpowerng",
  "native_bridge_bootstrap.js",
));

test("build bridge exports build normalization helpers", () => {
  assert.equal(typeof interopBridge.normalizeBuildPayload, "function");
  assert.equal(typeof nativeStateBridge.createNativeStateBridge, "function");
});

test("interop bridge normalizes build payloads and grouped quick-list sections", () => {
  const normalizedBuild = interopBridge.normalizeBuildPayload({
    disciplines: {
      attack: {
        score: 35,
        recordCount: 35,
        buildPoints: 28,
        currentTier: 3,
        nextTierPoints: 32,
        pointsToNextTier: 4,
        nextTierScore: 32,
        scoreToNextTier: 4,
      },
    },
    groupedCatalog: {
      attack: {
        themes: [{ id: "devastation", titleKey: "build.theme.attack.devastation", optionCount: 1 }],
      },
    },
    selectedThemeRows: [{ id: "build.attack.ferocity", discipline: "attack", themeId: "devastation" }],
    selectedOptionDetail: { id: "build.attack.ferocity", discipline: "attack", themeId: "devastation" },
    options: [
      {
        id: "build.attack.ferocity",
        discipline: "attack",
        themeId: "devastation",
        magnitude: 4.5,
        baseMagnitude: 3,
        magnitudePerTier: 0.5,
        currentMagnitude: 4.5,
        nextMagnitude: 5,
        effectParts: [{ effectKey: "attack_damage_mult", magnitude: 4.5 }],
        nextEffectParts: [{ effectKey: "attack_damage_mult", magnitude: 5 }],
        currentTier: 3,
        unlockPoints: 4,
        nextTierPoints: 32,
        pointsToNextTier: 4,
        nextTierScore: 32,
        scoreToNextTier: 4,
      },
    ],
    activeSlots: [{ slotId: "attack_1", optionId: "build.attack.ferocity" }],
  });
  assert.equal(normalizedBuild.disciplines.attack.score, 35);
  assert.equal(normalizedBuild.disciplines.attack.recordCount, 35);
  assert.equal(normalizedBuild.disciplines.attack.buildPoints, 28);
  assert.equal(normalizedBuild.disciplines.attack.currentTier, 3);
  assert.equal(normalizedBuild.disciplines.attack.nextTierPoints, 32);
  assert.equal(normalizedBuild.disciplines.attack.pointsToNextTier, 4);
  assert.equal(normalizedBuild.activeSlots.length, 6);
  assert.equal(normalizedBuild.themeMap.attack.length, 3);
  assert.equal(normalizedBuild.groupedCatalog.attack.themes.length, 1);
  assert.equal(normalizedBuild.selectedThemeRows.length, 1);
  assert.equal(normalizedBuild.selectedOptionDetail.id, "build.attack.ferocity");
  assert.equal(normalizedBuild.options[0].themeId, "devastation");
  assert.equal(normalizedBuild.options[0].hierarchy, "standard");
  assert.equal(normalizedBuild.options[0].currentMagnitude, 4.5);
  assert.equal(normalizedBuild.options[0].nextMagnitude, 5);
  assert.deepEqual(normalizedBuild.options[0].effectParts, [{ effectKey: "attack_damage_mult", magnitude: 4.5 }]);
  assert.deepEqual(normalizedBuild.options[0].nextEffectParts, [{ effectKey: "attack_damage_mult", magnitude: 5 }]);
  assert.equal(normalizedBuild.options[0].currentTier, 3);

  const normalizedQuickList = interopBridge.normalizeInventoryPayload({
    sections: [
      {
        discipline: "attack",
        rows: [
          { formId: 46775, disabledReason: null },
          { formId: 51234, disabledReason: "quest_protected" },
          { formId: 61234, disabledReason: "favorite_protected" },
        ],
      },
    ],
  });
  assert.equal(normalizedQuickList.sections[0].rows[1].disabledReason, "quest_protected");
  assert.equal(normalizedQuickList.sections[0].rows[2].disabledReason, "favorite_protected");
});

test("native state bridge forwards build payloads to build renderer", () => {
  const recorded = {
    build: null,
    renders: [],
  };

  const bridge = nativeStateBridge.createNativeStateBridge({
    setBuildValue: (next) => {
      recorded.build = next;
    },
    renderBuild: () => {
      recorded.renders.push("build");
    },
    isTabActive: (tabId) => tabId === "tabBuild",
  });

  bridge.onBuild({
    disciplines: {
      attack: {
        score: 35,
        recordCount: 35,
        buildPoints: 28,
        currentTier: 3,
        nextTierPoints: 32,
        pointsToNextTier: 4,
        nextTierScore: 32,
        scoreToNextTier: 4,
      },
    },
    groupedCatalog: {
      attack: {
        themes: [{ id: "devastation", titleKey: "build.theme.attack.devastation", optionCount: 1 }],
      },
    },
    selectedThemeRows: [{ id: "build.attack.ferocity", discipline: "attack", themeId: "devastation" }],
    selectedOptionDetail: { id: "build.attack.ferocity", discipline: "attack", themeId: "devastation" },
    options: [
      {
        id: "build.attack.ferocity",
        discipline: "attack",
        themeId: "devastation",
        magnitude: 4.5,
        currentMagnitude: 4.5,
        nextMagnitude: 5,
        effectParts: [{ effectKey: "attack_damage_mult", magnitude: 4.5 }],
        nextEffectParts: [{ effectKey: "attack_damage_mult", magnitude: 5 }],
        currentTier: 3,
      },
    ],
    activeSlots: [{ slotId: "attack_1", optionId: "build.attack.ferocity" }],
  });

  assert.equal(recorded.build.disciplines.attack.score, 35);
  assert.equal(recorded.build.disciplines.attack.recordCount, 35);
  assert.equal(recorded.build.disciplines.attack.buildPoints, 28);
  assert.equal(recorded.build.disciplines.attack.currentTier, 3);
  assert.equal(recorded.build.disciplines.attack.nextTierPoints, 32);
  assert.equal(recorded.build.disciplines.attack.pointsToNextTier, 4);
  assert.equal(recorded.build.activeSlots.length, 6);
  assert.equal(recorded.build.themeMap.attack.length, 3);
  assert.equal(recorded.build.groupedCatalog.attack.themes.length, 1);
  assert.equal(recorded.build.selectedThemeRows.length, 1);
  assert.equal(recorded.build.selectedOptionDetail.id, "build.attack.ferocity");
  assert.equal(recorded.build.options[0].currentMagnitude, 4.5);
  assert.deepEqual(recorded.build.options[0].effectParts, [{ effectKey: "attack_damage_mult", magnitude: 4.5 }]);
  assert.deepEqual(recorded.renders, ["build"]);
});

test("native bridge bootstrap installs build callback fallback", () => {
  const win = {};
  const recorded = { build: null };
  const detach = nativeBridgeBootstrap.installNativeBridge({
    windowObj: win,
    interopBridgeApi: null,
    onBuild: (next) => {
      recorded.build = next;
    },
  });

  assert.equal(typeof win.copng_setBuild, "function");
  win.copng_setBuild(
    JSON.stringify({
      disciplines: {
        attack: {
          score: 35,
          recordCount: 35,
          buildPoints: 28,
          currentTier: 3,
          nextTierPoints: 32,
          pointsToNextTier: 4,
          nextTierScore: 32,
          scoreToNextTier: 4,
        },
      },
      options: [
        {
          id: "build.attack.ferocity",
          discipline: "attack",
          themeId: "devastation",
          currentMagnitude: 4.5,
          nextMagnitude: 5,
          effectParts: [{ effectKey: "attack_damage_mult", magnitude: 4.5 }],
          nextEffectParts: [{ effectKey: "attack_damage_mult", magnitude: 5 }],
          currentTier: 3,
        },
      ],
      activeSlots: [{ slotId: "attack_1", optionId: "build.attack.ferocity" }],
    }),
  );
  assert.equal(recorded.build.disciplines.attack.score, 35);
  assert.equal(recorded.build.disciplines.attack.buildPoints, 28);
  assert.equal(recorded.build.disciplines.attack.currentTier, 3);
  assert.equal(recorded.build.activeSlots.length, 6);
  assert.equal(recorded.build.options[0].currentMagnitude, 4.5);
  assert.deepEqual(recorded.build.options[0].effectParts, [{ effectKey: "attack_damage_mult", magnitude: 4.5 }]);

  detach();
  assert.equal(win.copng_setBuild, undefined);
});

test("native build request endpoints are registered in request plumbing", () => {
  const requestSrc = read("src/PrismaUIRequests.cpp");
  assert.match(requestSrc, /copng_requestBuild/);
  assert.match(requestSrc, /copng_requestRegisterBatch/);
  assert.match(requestSrc, /copng_activateBuildOption/);
  assert.match(requestSrc, /copng_deactivateBuildOption/);
  assert.match(requestSrc, /copng_swapBuildOption/);
});

test("native build payload source uses build-point tier helpers", () => {
  const payloadSrc = read("src/PrismaUIPayloadsBuild.cpp");
  assert.match(payloadSrc, /buildPoints/);
  assert.match(payloadSrc, /unlockPointsCenti/);
  assert.match(payloadSrc, /GetBuildPointsTier/);
  assert.match(payloadSrc, /GetNextBuildPointsThresholdCenti/);
  assert.match(payloadSrc, /GetBuildPointsToNextTierCenti/);
  assert.match(payloadSrc, /GetResolvedBuildEffectBundle/);
  assert.match(payloadSrc, /GetNextTierResolvedBuildEffectBundle/);
  assert.match(payloadSrc, /effectParts/);
  assert.doesNotMatch(payloadSrc, /GetEffectiveBuildScalingTier/);
});
