const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const capsPath = path.join(__dirname, "..", "include", "CodexOfPowerNG", "RewardCaps.h");
const corePath = path.join(__dirname, "..", "src", "RewardsCore.cpp");
const rewardsPath = path.join(__dirname, "..", "src", "Rewards.cpp");
const mainPath = path.join(__dirname, "..", "src", "main.cpp");

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
  assert.match(src, /ComputeCappedRewardSyncDeltaFromSnapshot\(/);
  assert.match(src, /Reward sync cap guard:/);
  assert.match(src, /Reward sync \(carry weight\):/);
  assert.match(src, /ComputeCarryWeightSyncDelta\(/);
  assert.match(src, /ComputeRewardSyncDeltaFromSnapshot\(/);
  assert.match(src, /kCarryWeightQuickResyncMaxAttempts = 3/);
  assert.match(src, /kCarryWeightQuickResyncStuckMs = 3000/);
  assert.match(src, /g_carryWeightQuickResyncRerunRequested/);
  assert.match(src, /ScheduleCarryWeightQuickResync\(\)/);
  assert.match(src, /if \(av == RE::ActorValue::kCarryWeight\)[\s\S]*forceImmediate = std::abs\(delta\) > kRewardCapEpsilon;/);
});

test("carry weight reward schedules quick resync after grant", () => {
  const src = read(corePath);
  assert.match(src, /if \(av == RE::ActorValue::kCarryWeight\)[\s\S]*ScheduleCarryWeightQuickResync\(\)/);
  assert.doesNotMatch(src, /if \(av == RE::ActorValue::kCarryWeight\)[\s\S]*SyncRewardTotalsToPlayer\(\)/);
});

test("serialization load clamps over-capped reward totals", () => {
  const src = read(path.join(__dirname, "..", "src", "SerializationLoad.cpp"));
  assert.match(src, /ClampRewardTotal\(av,\s*total\)/);
  assert.match(src, /insert_or_assign\(av,\s*clamped\)/);
});

test("post-load/new-game path schedules full + carry-weight quick reward resync", () => {
  const src = read(mainPath);
  assert.match(src, /case SKSE::MessagingInterface::kPostLoadGame:[\s\S]*case SKSE::MessagingInterface::kNewGame:/);
  assert.match(src, /Rewards::SyncRewardTotalsToPlayer\(\);/);
  assert.match(src, /Rewards::ScheduleCarryWeightQuickResync\(\);/);
});
