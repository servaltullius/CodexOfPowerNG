const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const capsPath = path.join(__dirname, "..", "include", "CodexOfPowerNG", "RewardCaps.h");
const corePath = path.join(__dirname, "..", "src", "RewardsCore.cpp");
const rewardsPath = path.join(__dirname, "..", "src", "Rewards.cpp");
const enginePath = path.join(__dirname, "..", "src", "RewardsSyncEngine.cpp");
const runtimePath = path.join(__dirname, "..", "include", "CodexOfPowerNG", "RewardsSyncRuntime.h");
const randomTablesPath = path.join(__dirname, "..", "src", "RewardsRandomTables.cpp");
const mainPath = path.join(__dirname, "..", "src", "main.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("reward cap table defines vanilla-safe thresholds", () => {
  const src = read(capsPath);
  assert.match(src, /kAttackDamageMult[\s\S]*outCap = 1\.0f/);
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

test("skill rewards store fractional progress but apply whole-level actor deltas", () => {
  const capsSrc = read(capsPath);
  const coreSrc = read(corePath);
  const rewardsSrc = read(rewardsPath);
  const engineSrc = read(enginePath);

  assert.match(capsSrc, /bool IsSkillActorValue\(RE::ActorValue av\)/);
  assert.match(capsSrc, /kOneHanded/);
  assert.match(capsSrc, /kTwoHanded/);
  assert.match(capsSrc, /kArchery/);
  assert.match(capsSrc, /kHeavyArmor/);
  assert.match(capsSrc, /kLightArmor/);
  assert.match(capsSrc, /kBlock/);
  assert.match(capsSrc, /kAlchemy/);
  assert.match(capsSrc, /kLockpicking/);
  assert.match(capsSrc, /kPickpocket/);
  assert.match(capsSrc, /ActorAppliedRewardTotal\(\s*RE::ActorValue av,\s*float total/);
  assert.match(capsSrc, /std::trunc\(snapped\)/);

  assert.match(coreSrc, /actorDelta = ActorAppliedRewardTotal\(av, clampedTotal\) - ActorAppliedRewardTotal\(av, total\)/);
  assert.match(coreSrc, /CaptureAppliedRewardDelta\(av, outcome\.stateDelta\)/);
  assert.match(coreSrc, /ModActorValue\(av, outcome\.actorDelta\)/);

  assert.match(rewardsSrc, /const float previousApplied = ActorAppliedRewardTotal\(av, previousTotal\);/);
  assert.match(rewardsSrc, /const float nextApplied = ActorAppliedRewardTotal\(av, next\);/);
  assert.match(rewardsSrc, /const float appliedActorDelta = nextApplied - previousApplied;/);

  assert.match(engineSrc, /const float applied = ActorAppliedRewardTotal\(av, clamped\);/);
});

test("weapon reward table avoids AttackDamageMult percent grants", () => {
  const src = read(randomTablesPath);
  assert.doesNotMatch(src, /GrantReward\(RE::ActorValue::kAttackDamageMult,/);
  assert.match(src, /GrantReward\(RE::ActorValue::kOneHanded,/);
  assert.match(src, /GrantReward\(RE::ActorValue::kTwoHanded,/);
  assert.match(src, /GrantReward\(RE::ActorValue::kArchery,/);
});

test("sync/refund paths normalize over-capped totals and include carry-weight diagnostics", () => {
  const rewardsSrc = read(rewardsPath);
  const engineSrc = read(enginePath);
  const runtimeSrc = read(runtimePath);

  assert.match(rewardsSrc, /Engine::NormalizeRewardCapsOnStateAndPlayer\(\);/);
  assert.match(rewardsSrc, /Engine::PrepareRewardSyncPass\(\*passState\);/);
  assert.match(rewardsSrc, /Engine::ApplyRewardSyncPass\(\*passState,\s*kRewardSyncMinMissingStreak\)/);
  assert.match(rewardsSrc, /kCarryWeightQuickResyncMaxAttempts = 3/);
  assert.match(rewardsSrc, /kCarryWeightQuickResyncStuckMs = 3000/);
  assert.match(rewardsSrc, /ScheduleCarryWeightQuickResync\(\)/);

  assert.match(engineSrc, /ClampRewardTotalsInState\(\)/);
  assert.match(engineSrc, /normalizeCapsOnFirstPass/);
  assert.match(engineSrc, /Reward cap normalize:/);
  assert.match(engineSrc, /ComputeCappedRewardSyncDeltaFromSnapshot\(/);
  assert.match(engineSrc, /Reward sync \(carry weight\):/);
  assert.match(engineSrc, /ComputeCarryWeightSyncDelta\(/);
  assert.match(engineSrc, /ComputeRewardSyncDeltaFromSnapshot\(/);
  assert.match(engineSrc, /if \(av == RE::ActorValue::kCarryWeight\)[\s\S]*applyImmediately = std::abs\(delta\) > kRewardCapEpsilon;/);
  assert.match(engineSrc, /nonConvergingActorValues/);
  assert.match(engineSrc, /Reward sync guard: AV/);

  assert.match(runtimeSrc, /carryWeightQuickResync\.rerunRequested/);
});

test("carry weight reward schedules quick resync after grant", () => {
  const src = read(corePath);
  assert.match(src, /if \(av == RE::ActorValue::kCarryWeight[\s\S]*ScheduleCarryWeightQuickResync\(\)/);
  assert.doesNotMatch(src, /if \(av == RE::ActorValue::kCarryWeight\)[\s\S]*SyncRewardTotalsToPlayer\(\)/);
});

test("serialization load clamps over-capped reward totals", () => {
  const src = read(path.join(__dirname, "..", "src", "SerializationLoad.cpp"));
  assert.match(src, /ClampRewardTotal\(av,\s*total\)/);
  assert.match(src, /insert_or_assign\(av,\s*clamped\)/);
  assert.doesNotMatch(src, /SyncRewardTotalsToPlayer\(\)/);
});

test("post-load/new-game path schedules full + carry-weight quick reward resync", () => {
  const src = read(mainPath);
  assert.match(src, /case SKSE::MessagingInterface::kPreLoadGame:[\s\S]*Rewards::ResetSyncSchedulersForLoad\(\)/);
  assert.match(src, /case SKSE::MessagingInterface::kPostLoadGame:[\s\S]*case SKSE::MessagingInterface::kNewGame:/);
  assert.match(src, /if \(message->type == SKSE::MessagingInterface::kNewGame\)[\s\S]*Rewards::ResetSyncSchedulersForLoad\(\)/);
  assert.match(src, /Rewards::SyncRewardTotalsToPlayer\(\);/);
  assert.match(src, /Rewards::ScheduleCarryWeightQuickResync\(\);/);
});

test("reward sync runtime protects load boundary and readiness retries", () => {
  const rewardsSrc = read(rewardsPath);
  const runtimeSrc = read(runtimePath);
  const engineSrc = read(enginePath);

  assert.match(rewardsSrc, /void ResetSyncSchedulersForLoad\(\) noexcept/);
  assert.match(runtimeSrc, /BumpGenerationAndClearSchedulers\(\)/);
  assert.match(runtimeSrc, /state\.generation\.fetch_add/);
  assert.match(rewardsSrc, /SyncRuntime::IsCurrentGeneration\(passState->generation\)/);
  assert.match(rewardsSrc, /kRewardSyncReadinessTimeoutMs = 20000/);
  assert.match(rewardsSrc, /kCarryWeightQuickResyncReadinessTimeoutMs = 10000/);
  assert.match(rewardsSrc, /if \(!IsRewardSyncEnvironmentReady\(\)\)/);
  assert.match(rewardsSrc, /watchdog: force restarting stale sync worker \(generation \{\}\)/);
  assert.match(engineSrc, /weaponAbilityRefreshRequested/);
  assert.match(engineSrc, /UpdateWeaponAbility\(/);
  assert.match(engineSrc, /RefreshEquippedWeaponAbilities\(\)/);
  assert.match(engineSrc, /MigrateLegacyAttackDamageMultReward/);
  assert.match(engineSrc, /state\.rewardTotals\.erase\(it\)/);
  assert.match(engineSrc, /ModActorValue\(RE::ActorValue::kAttackDamageMult,\s*-removable\)/);
});
