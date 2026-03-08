const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const cmakePath = path.join(__dirname, "..", "CMakeLists.txt");
const headerPath = path.join(__dirname, "..", "include", "CodexOfPowerNG", "SerializationStateStore.h");
const implPath = path.join(__dirname, "..", "src", "SerializationStateStore.cpp");
const savePath = path.join(__dirname, "..", "src", "SerializationSave.cpp");
const loadPath = path.join(__dirname, "..", "src", "SerializationLoad.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("build graph includes serialization state store module", () => {
  const cmake = read(cmakePath);
  assert.match(cmake, /src\/SerializationStateStore\.cpp/);
  assert.match(cmake, /include\/CodexOfPowerNG\/SerializationStateStore\.h/);
});

test("serialization state store exposes snapshot and replace helpers", () => {
  const header = read(headerPath);
  const impl = read(implPath);

  assert.match(header, /struct Snapshot/);
  assert.match(header, /SnapshotState\(\) noexcept/);
  assert.match(header, /ReplaceState\(Snapshot snapshot\) noexcept/);
  assert.match(header, /Clear\(\) noexcept/);

  assert.match(impl, /snapshot\.registeredItems = state\.registeredItems/);
  assert.match(impl, /state\.registeredItems = std::move\(snapshot\.registeredItems\)/);
  assert.match(impl, /state\.undoNextActionId = snapshot\.undoNextActionId/);
});

test("serialization save/load paths use serialization state store boundary", () => {
  const saveSrc = read(savePath);
  const loadSrc = read(loadPath);

  assert.match(saveSrc, /const auto state = SerializationStateStore::SnapshotState\(\);/);
  assert.match(saveSrc, /SerializationStateStore::Clear\(\);/);
  assert.doesNotMatch(saveSrc, /auto& state = GetState\(\)/);

  assert.match(loadSrc, /SerializationStateStore::Snapshot loadedState\{\};/);
  assert.match(loadSrc, /SerializationStateStore::ReplaceState\(std::move\(loadedState\)\);/);
  assert.doesNotMatch(loadSrc, /auto& state = GetState\(\)/);
});
