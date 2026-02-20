const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const loadPath = path.join(__dirname, "..", "src", "SerializationLoad.cpp");

function readLoad() {
  return fs.readFileSync(loadPath, "utf8");
}

test("serialization load stages records before committing runtime state", () => {
  const src = readLoad();
  assert.match(src, /loadedRegisteredItems/);
  assert.match(src, /loadedBlockedItems/);
  assert.match(src, /loadedNotifiedItems/);
  assert.match(src, /loadedRewardTotals/);
  assert.match(src, /loadedUndoHistory/);

  assert.match(src, /state\.registeredItems = std::move\(loadedRegisteredItems\)/);
  assert.match(src, /state\.blockedItems = std::move\(loadedBlockedItems\)/);
  assert.match(src, /state\.notifiedItems = std::move\(loadedNotifiedItems\)/);
  assert.match(src, /state\.rewardTotals = std::move\(loadedRewardTotals\)/);
  assert.match(src, /state\.undoHistory = std::move\(loadedUndoHistory\)/);
  assert.match(src, /state\.undoNextActionId = loadedUndoNextActionId/);
});
