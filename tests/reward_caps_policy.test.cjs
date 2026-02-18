const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const capsPath = path.join(__dirname, "..", "include", "CodexOfPowerNG", "RewardCaps.h");
const corePath = path.join(__dirname, "..", "src", "RewardsCore.cpp");
const rewardsPath = path.join(__dirname, "..", "src", "Rewards.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("reward cap table defines vanilla-safe thresholds", () => {
  const src = read(capsPath);
  assert.match(src, /kCriticalChance[\s\S]*outCap = 50\.0f/);
  assert.match(src, /kResistShock[\s\S]*outCap = 85\.0f/);
  assert.match(src, /kAbsorbChance[\s\S]*outCap = 80\.0f/);
  assert.match(src, /kSpeedMult[\s\S]*outCap = 0\.50f/);
  assert.doesNotMatch(
    src,
    /kCarryWeight/,
    "Carry weight should remain uncapped in the vanilla-safe preset",
  );
});

test("grant path clamps totals against AV caps before applying reward", () => {
  const src = read(corePath);
  assert.match(src, /const float clampedTotal = ClampRewardTotal\(av, total \+ delta\);/);
  assert.match(src, /Reward cap applied:/);
  assert.match(src, /Reward grant skipped by cap:/);
});

test("sync/refund paths normalize over-capped totals and include carry-weight diagnostics", () => {
  const src = read(rewardsPath);
  assert.match(src, /ClampRewardTotalsInState\(\)/);
  assert.match(src, /normalizeCapsOnFirstPass/);
  assert.match(src, /Reward cap normalize:/);
  assert.match(src, /Reward sync \(carry weight\):/);
});
