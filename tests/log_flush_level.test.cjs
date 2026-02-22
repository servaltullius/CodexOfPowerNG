const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const mainPath = path.join(__dirname, "..", "src", "main.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("logging avoids flush-on-info to reduce I/O stalls", () => {
  const src = read(mainPath);

  assert.match(
    src,
    /spdlog::flush_on\(spdlog::level::warn\);/,
    "SetupLogging should flush on warn (not info)",
  );
});

