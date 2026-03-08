const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const cmakePath = path.join(__dirname, "..", "CMakeLists.txt");
const headerPath = path.join(__dirname, "..", "include", "CodexOfPowerNG", "NotifiedStateStore.h");
const implPath = path.join(__dirname, "..", "src", "NotifiedStateStore.cpp");
const eventsPath = path.join(__dirname, "..", "src", "Events.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("build graph includes notified state store module", () => {
  const cmake = read(cmakePath);
  assert.match(cmake, /src\/NotifiedStateStore\.cpp/);
  assert.match(cmake, /include\/CodexOfPowerNG\/NotifiedStateStore\.h/);
});

test("notified state store exposes focused notified-item helpers", () => {
  const header = read(headerPath);
  const impl = read(implPath);

  assert.match(header, /ContainsAny\(RE::FormID primaryId,\s*RE::FormID secondaryId\)/);
  assert.match(header, /MarkPair\(RE::FormID primaryId,\s*RE::FormID secondaryId\)/);
  assert.match(header, /ReplaceAll\(std::unordered_set<RE::FormID> notifiedItems\)/);

  assert.match(impl, /state\.notifiedItems\.contains\(primaryId\)/);
  assert.match(impl, /state\.notifiedItems\.insert\(primaryId\)/);
  assert.match(impl, /state\.notifiedItems = std::move\(notifiedItems\)/);
});

test("events path uses notified state store instead of touching notifiedItems directly", () => {
  const src = read(eventsPath);
  assert.match(src, /NotifiedStateStore::ContainsAny\(regKeyId,\s*baseId\)/);
  assert.match(src, /NotifiedStateStore::MarkPair\(regKeyId,\s*baseId\)/);
  assert.doesNotMatch(src, /state\.notifiedItems/);
});
