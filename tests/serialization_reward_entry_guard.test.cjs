const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const loadPath = path.join(__dirname, "..", "src", "SerializationLoad.cpp");

function readLoad() {
  return fs.readFileSync(loadPath, "utf8");
}

test("serialization load hardens reward actor value decoding", () => {
  const src = readLoad();

  assert.match(src, /kMaxSerializedRewardEntries/, "reward record entry cap should exist");
  assert.match(src, /kMaxSerializedUndoRewardDeltas/, "undo reward delta cap should exist");
  assert.match(src, /IsSupportedRewardActorValue/, "reward actor value allowlist should exist");
  assert.match(src, /skipping unsupported reward actor value/, "reward record should skip unsupported AVs");
  assert.match(src, /dropping unsupported undo reward actor value/, "undo record should skip unsupported AVs");
});
