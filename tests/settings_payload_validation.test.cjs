const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const interactionsPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_interactions.js");
const renderingPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_rendering.js");

function readInteractions() {
  return fs.readFileSync(interactionsPath, "utf8");
}

function readRendering() {
  return fs.readFileSync(renderingPath, "utf8");
}

test("settings save keeps rewardMultiplier=0 instead of coercing to 1.0", () => {
  const src = readInteractions();
  assert.match(src, /const rewardMultInput = parseFloat\(documentObj\.getElementById\("setRewardMult"\)\.value\);/);
  assert.match(src, /rewardMultiplier:\s*Number\.isFinite\(rewardMultInput\)\s*\?\s*rewardMultInput\s*:\s*1\.0/);
  assert.doesNotMatch(src, /rewardMultiplier:\s*parseFloat\([^)]*\)\s*\|\|\s*1\.0/);
});

test("settings save validates rewardEvery with explicit numeric check", () => {
  const src = readInteractions();
  assert.match(src, /const rewardEveryInput = parseInt\(documentObj\.getElementById\("setRewardEvery"\)\.value,\s*10\);/);
  assert.match(src, /rewardEvery:\s*Number\.isFinite\(rewardEveryInput\)\s*&&\s*rewardEveryInput\s*>\s*0\s*\?\s*rewardEveryInput\s*:\s*5/);
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
