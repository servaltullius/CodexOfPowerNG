const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

function read(relPath) {
  return fs.readFileSync(path.join(__dirname, "..", relPath), "utf8");
}

test("ui exposes undo tab and requests undo list", () => {
  const html = read("PrismaUI/views/codexofpowerng/index.html");
  assert.match(html, /data-tab="tabUndo"/, "Undo tab button should exist");
  assert.match(html, /id="tabUndo"/, "Undo section should exist");
  assert.match(html, /id="undoBody"/, "Undo table body should exist");
  assert.match(html, /safeCall\("copng_requestUndoList", \{\}\);/, "UI should request undo list at startup");
  assert.match(html, /safeCall\("copng_undoRegisterItem", \{ actionId:/, "UI should call undo action listener");
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
});

test("registration path records undo metadata and reward delta snapshot", () => {
  const src = read("src/RegistrationQuickRegister.cpp");
  assert.match(src, /SnapshotRewardTotals\(\)/);
  assert.match(src, /BuildRewardDeltas\(rewardBefore, rewardAfter\)/);
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
  assert.match(loadSrc, /state\.undoHistory/);
});

test("undo reward rollback applies actual clamped delta to actor values", () => {
  const rewardsSrc = read("src/Rewards.cpp");
  assert.match(rewardsSrc, /appliedActorDelta = next - previousTotal/);
  assert.match(rewardsSrc, /RewardDelta\{ av, appliedActorDelta \}/);
});
