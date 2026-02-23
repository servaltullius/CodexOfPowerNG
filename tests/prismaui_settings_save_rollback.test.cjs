const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const settingsPath = path.join(__dirname, "..", "src", "PrismaUISettings.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("settings save job carries fallback snapshot and rolls back on save failure", () => {
  const src = read(settingsPath);

  assert.match(
    src,
    /struct SettingsSaveJob[\s\S]*Settings previousSettings\{\};/,
    "SettingsSaveJob should include a previousSettings snapshot for rollback",
  );

  assert.match(
    src,
    /void RevertSettingsAfterSaveFailure\(const SettingsSaveJob& job\) noexcept[\s\S]*SetSettings\(job\.previousSettings\);[\s\S]*Registration::InvalidateQuickRegisterCache\(\);/,
    "Save failure rollback helper should restore previous settings and invalidate quick-register cache",
  );

  assert.match(
    src,
    /if \(!ok\)\s*\{[\s\S]*RevertSettingsAfterSaveFailure\(next\);/,
    "Save worker should invoke rollback helper when persistence fails",
  );

  assert.match(
    src,
    /QueueSettingsSave\(next,\s*current,\s*reloadL10n\);/,
    "Request handler should pass the current settings as rollback snapshot",
  );
});
