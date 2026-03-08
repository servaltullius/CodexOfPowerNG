const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const cmakePath = path.join(__dirname, "..", "CMakeLists.txt");
const headerPath = path.join(__dirname, "..", "include", "CodexOfPowerNG", "NotifiedStateStore.h");
const opsHeaderPath = path.join(__dirname, "..", "include", "CodexOfPowerNG", "NotifiedStateStoreOps.h");
const implPath = path.join(__dirname, "..", "src", "NotifiedStateStore.cpp");
const eventsPath = path.join(__dirname, "..", "src", "Events.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("build graph includes notified state store module", () => {
  const cmake = read(cmakePath);
  assert.match(cmake, /src\/NotifiedStateStore\.cpp/);
  assert.match(cmake, /include\/CodexOfPowerNG\/NotifiedStateStore\.h/);
  assert.match(cmake, /include\/CodexOfPowerNG\/NotifiedStateStoreOps\.h/);
});

test("notified state store exposes focused helpers and delegates pure set operations", () => {
  const header = read(headerPath);
  const opsHeader = read(opsHeaderPath);
  const impl = read(implPath);

  assert.match(header, /ContainsAny\(RE::FormID primaryId,\s*RE::FormID secondaryId\)/);
  assert.match(header, /MarkPair\(RE::FormID primaryId,\s*RE::FormID secondaryId\)/);
  assert.match(header, /ReplaceAll\(std::unordered_set<RE::FormID> notifiedItems\)/);
  assert.match(opsHeader, /namespace CodexOfPowerNG::NotifiedStateStore::Ops/);
  assert.match(opsHeader, /ContainsAny\(const Set& notifiedItems/);
  assert.match(opsHeader, /MarkPair\(Set& notifiedItems/);

  assert.match(impl, /Ops::ContainsAny\(state\.notifiedItems,\s*primaryId,\s*secondaryId\)/);
  assert.match(impl, /Ops::MarkPair\(state\.notifiedItems,\s*primaryId,\s*secondaryId\)/);
  assert.match(impl, /Ops::ReplaceAll\(state\.notifiedItems,\s*std::move\(notifiedItems\)\)/);
});

test("events path uses notified state store instead of touching notifiedItems directly", () => {
  const src = read(eventsPath);
  assert.match(src, /NotifiedStateStore::ContainsAny\(regKeyId,\s*baseId\)/);
  assert.match(src, /NotifiedStateStore::MarkPair\(regKeyId,\s*baseId\)/);
  assert.doesNotMatch(src, /state\.notifiedItems/);
});
