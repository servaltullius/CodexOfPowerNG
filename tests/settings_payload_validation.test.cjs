const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const interactionsPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_interactions.js");
const renderingPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_rendering.js");
const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");

function readInteractions() {
  return fs.readFileSync(interactionsPath, "utf8");
}

function readRendering() {
  return fs.readFileSync(renderingPath, "utf8");
}

function readView() {
  return fs.readFileSync(viewPath, "utf8");
}

test("settings save payload no longer references legacy reward controls", () => {
  const src = readInteractions();
  assert.doesNotMatch(src, /setRewardsEnabled/);
  assert.doesNotMatch(src, /setRewardEvery/);
  assert.doesNotMatch(src, /setRewardMult/);
  assert.doesNotMatch(src, /setSkillRewards/);
  assert.doesNotMatch(src, /enableRewards:/);
  assert.doesNotMatch(src, /rewardEvery:/);
  assert.doesNotMatch(src, /rewardMultiplier:/);
  assert.doesNotMatch(src, /allowSkillRewards:/);
});

test("settings rendering no longer populates legacy reward inputs", () => {
  const src = readRendering();
  assert.doesNotMatch(src, /assignChecked\("setRewardsEnabled"/);
  assert.doesNotMatch(src, /assignValue\("setRewardEvery"/);
  assert.doesNotMatch(src, /assignValue\("setRewardMult"/);
  assert.doesNotMatch(src, /assignChecked\("setSkillRewards"/);
});

test("settings view no longer renders the legacy rewards card", () => {
  const src = readView();
  assert.doesNotMatch(src, /id="setRewardsEnabled"/);
  assert.doesNotMatch(src, /id="setRewardEvery"/);
  assert.doesNotMatch(src, /id="setRewardMult"/);
  assert.doesNotMatch(src, /id="setSkillRewards"/);
  assert.doesNotMatch(src, /data-i18n="settings\.rewardsTitle"/);
});

test("settings payload uiInputScale is always applied authoritatively", () => {
  const src = readRendering();
  assert.match(src, /const desiredInputScale = parseFloat\(String\(coalesce\(settings\.uiInputScale,\s*1\.0\)\)\);/);
  assert.match(
    src,
    /setInputScale\(Number\.isFinite\(desiredInputScale\)\s*&&\s*desiredInputScale\s*>\s*0\s*\?\s*desiredInputScale\s*:\s*1\.0,\s*\{\s*persist:\s*false\s*\}\);/,
  );
  assert.doesNotMatch(src, /if\s*\(!inputScaleHasLocal\)/);
});
