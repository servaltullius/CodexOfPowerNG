const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

function read(relPath) {
  return fs.readFileSync(path.join(__dirname, "..", relPath), "utf8");
}

test("quick register list uses short-lived cache with generation invalidation", () => {
  const src = read("src/RegistrationQuickRegister.cpp");
  assert.match(src, /kQuickListCacheTtlMs/);
  assert.match(src, /TryBuildQuickListFromCache/);
  assert.match(src, /UpdateQuickListCache/);
  assert.match(src, /std::atomic<std::uint64_t>\s+g_quickListGeneration/);
  assert.match(src, /void InvalidateQuickRegisterCache\(\) noexcept/);
  assert.match(src, /g_quickListGeneration\.fetch_add\(1/);
});

test("register\/undo\/serialization paths invalidate quick register cache", () => {
  const regSrc = read("src/RegistrationQuickRegister.cpp");
  const undoSrc = read("src/RegistrationUndo.cpp");
  const loadSrc = read("src/SerializationLoad.cpp");
  const saveSrc = read("src/SerializationSave.cpp");
  const settingsSrc = read("src/PrismaUISettings.cpp");

  assert.match(regSrc, /InvalidateQuickRegisterCache\(\);/);
  assert.match(undoSrc, /InvalidateQuickRegisterCache\(\);/);
  assert.match(loadSrc, /Registration::InvalidateQuickRegisterCache\(\);/);
  assert.match(saveSrc, /Registration::InvalidateQuickRegisterCache\(\);/);
  assert.match(settingsSrc, /Registration::InvalidateQuickRegisterCache\(\);/);
});
