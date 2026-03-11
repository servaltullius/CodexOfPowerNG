const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

function read(relPath) {
  return fs.readFileSync(path.join(__dirname, "..", relPath), "utf8");
}

test("build graph includes quick register builder module", () => {
  const cmake = read("CMakeLists.txt");
  assert.match(cmake, /src\/RegistrationQuickListBuilder\.cpp/);
});

test("quick register delegates eligible-item scan to builder module", () => {
  const quickRegisterSrc = read("src/RegistrationQuickRegister.cpp");
  assert.match(quickRegisterSrc, /#include "RegistrationQuickListBuilder\.h"/);
  assert.match(quickRegisterSrc, /Internal::BuildQuickListEligibleItems\(/);
  assert.doesNotMatch(quickRegisterSrc, /std::unordered_set<RE::FormID>\s+seenRegKeys/);
});

test("quick register builder owns quest protection, TCC gating, and stable sorting", () => {
  const builderSrc = read("src/RegistrationQuickListBuilder.cpp");
  assert.match(builderSrc, /entry\.IsQuestObject\(\)/);
  assert.match(builderSrc, /Internal::EvaluateTccGate\(settings, tccLists, obj, regKey\)/);
  assert.match(builderSrc, /player\.GetItemCount\(obj\)/);
  assert.match(builderSrc, /std::sort\(allEligible\.begin\(\), allEligible\.end\(\)/);
});
