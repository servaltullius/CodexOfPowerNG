const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

function read(relPath) {
  return fs.readFileSync(path.join(__dirname, "..", relPath), "utf8");
}

test("ui exposes undo tab and requests undo list", () => {
  const html = read("PrismaUI/views/codexofpowerng/index.html");
  const bootstrapSrc = read("PrismaUI/views/codexofpowerng/ui_bootstrap.js");
  const interactionsSrc = read("PrismaUI/views/codexofpowerng/ui_interactions.js");
  assert.match(html, /data-tab="tabUndo"/, "Undo tab button should exist");
  assert.match(html, /id="tabUndo"/, "Undo section should exist");
  assert.match(html, /id="undoBody"/, "Undo table body should exist");
  assert.match(bootstrapSrc, /safeCall\("copng_requestUndoList", \{\}\);/, "Bootstrap should request undo list at startup");
  assert.match(
    interactionsSrc,
    /safeCall\("copng_undoRegisterItem", \{ actionId: Math\.trunc\(actionId\) \}\);/,
    "Interaction module should call undo action listener",
  );
});

test("native listeners expose undo list and undo action endpoints", () => {
  const src = read("src/PrismaUIRequests.cpp");
  assert.match(src, /copng_requestUndoList/);
  assert.match(src, /copng_undoRegisterItem/);
});

test("request ops route undo payload and refresh relevant panels", () => {
  const src = read("src/PrismaUIRequestOps.cpp");
  assert.match(src, /void QueueSendUndoList\(\) noexcept/);
  assert.match(src, /void HandleUndoRegisterRequest\(const char\* argument\) noexcept/);
  assert.match(src, /Registration::TryUndoRegistration\(actionId\)/);
  assert.match(src, /SendJS\("copng_setUndoList", PrismaUIPayloads::BuildUndoPayload\(items\)\)/);
  assert.match(src, /QueueSendInventory\(SnapshotLastInventoryRequest\(\)\)/);
  assert.doesNotMatch(src, /QueueSendInventory\(InventoryRequest\{\}\)/);
});

test("registration path records undo metadata using exact applied reward deltas", () => {
  const src = read("src/RegistrationQuickRegister.cpp");
  assert.match(src, /auto rewardDeltas = Rewards::MaybeGrantRegistrationReward\(group, static_cast<std::int32_t>\(totalRegistered\)\);/);
  assert.match(src, /undoRecord\.rewardDeltas = std::move\(rewardDeltas\);/);
  assert.doesNotMatch(src, /SnapshotRewardTotals\(\)/);
  assert.doesNotMatch(src, /BuildRewardDeltas\(/);
  assert.match(src, /RegistrationStateStore::PushUndoRecord\(std::move\(undoRecord\)\)/);
});

test("undo records are serialized and restored", () => {
  const saveSrc = read("src/SerializationSave.cpp");
  const loadSrc = read("src/SerializationLoad.cpp");
  assert.match(saveSrc, /kRecordUndoHistory/);
  assert.match(saveSrc, /WriteUndoRecord/);
  assert.match(loadSrc, /case kRecordUndoHistory:/);
  assert.match(loadSrc, /ResolveFormID\(oldRegKey, newRegKey\)/);
  assert.match(loadSrc, /ResolveFormID\(oldFormId, newFormId\)/);
  assert.match(loadSrc, /if \(!regKeyResolved \|\| !formIdResolved\)/);
  assert.doesNotMatch(loadSrc, /falling back to regKey/);
  assert.match(loadSrc, /loadedState\.undoHistory/);
  assert.match(loadSrc, /SerializationStateStore::ReplaceState\(std::move\(loadedState\)\)/);
});

test("undo reward rollback applies actual clamped delta to actor values", () => {
  const rewardsSrc = read("src/Rewards.cpp");
  assert.match(rewardsSrc, /previousApplied = ActorAppliedRewardTotal\(av, transition\.previousTotal\)/);
  assert.match(rewardsSrc, /nextApplied = ActorAppliedRewardTotal\(av, transition\.nextTotal\)/);
  assert.match(rewardsSrc, /appliedActorDelta = nextApplied - previousApplied/);
  assert.match(rewardsSrc, /RewardDelta\{ av, appliedActorDelta \}/);
});

test("undo checks rollback result and emits warning path when no actor deltas were applied", () => {
  const undoSrc = read("src/RegistrationUndo.cpp");
  assert.match(undoSrc, /const auto rollbackApplied = Rewards::RollbackRewardDeltas\(record\.rewardDeltas\);/);
  assert.match(undoSrc, /if \(hadRollbackTarget && rollbackApplied == 0\)/);
  assert.match(undoSrc, /msg\.undoRewardRollbackWarning/);
});
