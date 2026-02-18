const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const constantsPath = path.join(__dirname, "..", "include", "CodexOfPowerNG", "Constants.h");
const configPath = path.join(__dirname, "..", "src", "Config.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("constants define dedicated user settings override path", () => {
  const src = read(constantsPath);
  assert.match(
    src,
    /kSettingsUserPath\s*=\s*"Data\/SKSE\/Plugins\/CodexOfPowerNG\/settings\.user\.json"/,
    "Config constants should expose a dedicated user override settings path",
  );
});

test("config loads default template then user override in deterministic order", () => {
  const src = read(configPath);
  assert.match(
    src,
    /Settings settings = Defaults\(\);\s*ApplySettingsFromFile\(SettingsPath\(\), settings, "settings\.json"\);\s*ApplySettingsFromFile\(UserSettingsPath\(\), settings, "settings\.user\.json"\);/,
    "LoadFromDisk should apply settings.json first, then settings.user.json as the final override layer",
  );
});

test("config saves only to settings.user.json and no longer bootstraps settings.json", () => {
  const src = read(configPath);
  assert.match(
    src,
    /const auto path = UserSettingsPath\(\);/,
    "SaveToDisk should target the user override path",
  );
  assert.match(
    src,
    /Failed to write settings\.user\.json/,
    "Write failures should mention settings.user.json to aid troubleshooting",
  );
  assert.match(
    src,
    /void LoadSettingsFromDisk\(\)\s*\{\s*SetSettings\(LoadFromDisk\(\)\);\s*\}/,
    "LoadSettingsFromDisk should load layered settings directly without creating template files",
  );
});
