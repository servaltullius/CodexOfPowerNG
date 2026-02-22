const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const configPath = path.join(__dirname, "..", "src", "Config.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("config saves settings.user.json atomically via tmp + rename", () => {
  const src = read(configPath);

  assert.match(
    src,
    /auto tmpPath = path;\s*tmpPath \+= "\.tmp";/,
    "SaveToDisk should write to a .tmp file first",
  );

  assert.match(
    src,
    /auto bakPath = path;\s*bakPath \+= "\.bak";/,
    "SaveToDisk should maintain a .bak backup file",
  );

  assert.match(
    src,
    /std::ofstream out\(tmpPath, std::ios::binary \| std::ios::trunc\);/,
    "SaveToDisk should write settings to tmpPath (not directly to the final file)",
  );

  assert.match(
    src,
    /std::filesystem::rename\(tmpPath, path,/,
    "SaveToDisk should commit tmpPath -> path via filesystem rename",
  );
});

