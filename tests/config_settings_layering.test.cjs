const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const constantsPath = path.join(__dirname, "..", "include", "CodexOfPowerNG", "Constants.h");
const configPath = path.join(__dirname, "..", "src", "Config.cpp");
const settingsPayloadPath = path.join(__dirname, "..", "src", "PrismaUISettingsPayload.cpp");
const settingsTemplatePath = path.join(__dirname, "..", "SKSE", "Plugins", "CodexOfPowerNG", "settings.json");

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

test("settings payload omits legacy reward cadence fields but parser still accepts old keys", () => {
  const src = read(settingsPayloadPath);
  assert.match(
    src,
    /if \(auto it = j\.find\("enableRewards"\); it != j\.end\(\) && it->is_boolean\(\)\)/,
    "Settings parser should remain backward compatible with legacy reward keys",
  );
  assert.match(
    src,
    /if \(auto it = j\.find\("rewardEvery"\); it != j\.end\(\) && it->is_number_integer\(\)\)/,
    "Settings parser should still accept rewardEvery from older payloads",
  );
  assert.match(
    src,
    /if \(auto it = j\.find\("allowSkillRewards"\); it != j\.end\(\) && it->is_boolean\(\)\)/,
    "Settings parser should still accept allowSkillRewards from older payloads",
  );
  assert.doesNotMatch(
    src,
    /\{\s*"enableRewards", settings\.enableRewards\s*\}/,
    "BuildSettingsPayload should not expose enableRewards to the current UI contract",
  );
  assert.doesNotMatch(
    src,
    /\{\s*"rewardEvery", settings\.rewardEvery\s*\}/,
    "BuildSettingsPayload should not expose rewardEvery to the current UI contract",
  );
  assert.doesNotMatch(
    src,
    /\{\s*"rewardMultiplier", settings\.rewardMultiplier\s*\}/,
    "BuildSettingsPayload should not expose rewardMultiplier to the current UI contract",
  );
  assert.doesNotMatch(
    src,
    /\{\s*"allowSkillRewards", settings\.allowSkillRewards\s*\}/,
    "BuildSettingsPayload should not expose allowSkillRewards to the current UI contract",
  );
});

test("saved and shipped settings no longer advertise legacy rewards controls", () => {
  const configSrc = read(configPath);
  const templateSrc = read(settingsTemplatePath);
  assert.doesNotMatch(
    configSrc,
    /j\["rewards"\]\s*=/,
    "SaveToDisk should stop writing the legacy rewards block into settings.user.json",
  );
  assert.doesNotMatch(
    templateSrc,
    /"rewards"\s*:/,
    "Distributed settings.json should no longer present rewards settings as active options",
  );
});
