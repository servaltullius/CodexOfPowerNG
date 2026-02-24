const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const settingsPath = path.join(__dirname, "..", "src", "PrismaUISettings.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("settings save worker rolls back to last persisted snapshot on save failure", () => {
  const src = read(settingsPath);

  assert.match(
    src,
    /struct SettingsSaveJob[\s\S]*Settings persistedSettings\{\};/,
    "SettingsSaveJob should include a persisted settings snapshot for rollback",
  );

  assert.match(
    src,
    /void RevertSettingsAfterSaveFailure\(const SettingsSaveJob& job\) noexcept[\s\S]*SetSettings\(job\.persistedSettings\);[\s\S]*Registration::InvalidateQuickRegisterCache\(\);/,
    "Save failure rollback helper should restore last persisted settings and invalidate quick-register cache",
  );

  assert.match(
    src,
    /\[\[nodiscard\]\] Settings SnapshotLastPersistedSettings\(const Settings& fallback\) noexcept[\s\S]*if \(!g_lastPersistedSettings\)[\s\S]*g_lastPersistedSettings = ClampSettings\(fallback\);/,
    "Worker should lazily initialize a last-persisted snapshot from current clamped settings",
  );

  assert.match(
    src,
    /void RecordPersistedSettings\(const Settings& settings\) noexcept[\s\S]*g_lastPersistedSettings = ClampSettings\(settings\);/,
    "Successful save should update the last-persisted snapshot",
  );

  assert.match(
    src,
    /if \(!ok\)\s*\{[\s\S]*RevertSettingsAfterSaveFailure\(next\);/,
    "Save worker should invoke rollback helper when persistence fails",
  );

  assert.match(
    src,
    /if \(ok\)\s*\{[\s\S]*RecordPersistedSettings\(next\.settings\);/,
    "Save worker should record persisted snapshot when disk save succeeds",
  );

  assert.match(
    src,
    /return SaveSettingsSnapshotToDisk\(settings\);/,
    "Worker persistence should use snapshot-only disk save to avoid stale in-memory overwrite",
  );

  assert.match(
    src,
    /const auto persistedSettings = SnapshotLastPersistedSettings\(current\);[\s\S]*QueueSettingsSave\(next,\s*persistedSettings,\s*reloadL10n\);/,
    "Request handler should queue save with persisted snapshot instead of current in-memory state",
  );
});
