const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const configPath = path.join(__dirname, "..", "src", "Config.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("config repairs stale tmp/bak settings files on startup before loading", () => {
  const src = read(configPath);

  assert.match(
    src,
    /void RepairUserSettingsFiles\(\) noexcept/,
    "Config should include a repair helper for settings.user.json artifacts",
  );

  assert.match(
    src,
    /\[\[nodiscard\]\] Settings LoadFromDisk\(\)\s*\{\s*RepairUserSettingsFiles\(\);\s*Settings settings = Defaults\(\);/,
    "LoadFromDisk should call RepairUserSettingsFiles() before reading settings files",
  );
});
