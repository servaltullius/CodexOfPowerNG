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
  assert.match(src, /SerializationStateStore::Snapshot loadedState\{\};/);
  assert.match(src, /loadedState\.registeredItems/);
  assert.match(src, /loadedState\.blockedItems/);
  assert.match(src, /loadedState\.notifiedItems/);
  assert.match(src, /loadedState\.rewardTotals/);
  assert.match(src, /loadedState\.undoHistory/);
  assert.match(src, /loadedState\.undoNextActionId/);
  assert.match(src, /SerializationStateStore::ReplaceState\(std::move\(loadedState\)\);/);
});
