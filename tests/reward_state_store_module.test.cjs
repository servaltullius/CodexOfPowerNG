const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const cmakePath = path.join(__dirname, "..", "CMakeLists.txt");
const storeHeaderPath = path.join(__dirname, "..", "include", "CodexOfPowerNG", "RewardStateStore.h");
const storeImplPath = path.join(__dirname, "..", "src", "RewardStateStore.cpp");
const corePath = path.join(__dirname, "..", "src", "RewardsCore.cpp");
const enginePath = path.join(__dirname, "..", "src", "RewardsSyncEngine.cpp");
const rewardsPath = path.join(__dirname, "..", "src", "Rewards.cpp");
const contractPath = path.join(__dirname, "..", "docs", "contracts", "prismaui-js-api.md");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("build graph includes reward state store module", () => {
  const cmake = read(cmakePath);
  assert.match(cmake, /src\/RewardStateStore\.cpp/);
  assert.match(cmake, /include\/CodexOfPowerNG\/RewardStateStore\.h/);
});

test("reward state store exposes dedicated reward total helpers", () => {
  const header = read(storeHeaderPath);
  const impl = read(storeImplPath);

  assert.match(header, /struct RewardTotalTransition/);
  assert.match(header, /AdjustClamped\(RE::ActorValue av,\s*float delta\)/);
  assert.match(header, /std::optional<float>\s+Take\(RE::ActorValue av\)/);
  assert.match(header, /std::vector<RewardCapAdjustment>\s+ClampAll\(\)/);

  assert.match(impl, /state\.rewardTotals\.insert_or_assign/);
  assert.match(impl, /state\.rewardTotals\.erase/);
});

test("reward runtime paths use reward state store instead of direct rewardTotals access", () => {
  const core = read(corePath);
  const engine = read(enginePath);
  const rewards = read(rewardsPath);

  assert.match(core, /RewardStateStore::AdjustClamped\(av,\s*delta\)/);
  assert.doesNotMatch(core, /state\.rewardTotals/);

  assert.match(engine, /RewardStateStore::Take\(RE::ActorValue::kAttackDamageMult\)/);
  assert.match(engine, /RewardStateStore::ClampAll\(\)/);
  assert.match(engine, /RewardStateStore::Get\(av\)/);
  assert.match(engine, /RewardStateStore::Snapshot\(\)/);
  assert.doesNotMatch(engine, /state\.rewardTotals/);

  assert.match(rewards, /RewardStateStore::Clear\(\)/);
  assert.match(rewards, /RewardStateStore::AdjustClamped\(av,\s*-delta\)/);
  assert.match(rewards, /RewardStateStore::Snapshot\(\)/);
});

test("PrismaUI contract documents refresh invalidation behavior", () => {
  const contract = read(contractPath);
  assert.match(contract, /## Refresh Contract/);
  assert.match(contract, /copng_registerItem.*copng_setState/s);
  assert.match(contract, /main-thread queue is temporarily unavailable.*pending refresh/s);
});
