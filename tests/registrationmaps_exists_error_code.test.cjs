const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const mapsPath = path.join(__dirname, "..", "src", "RegistrationMaps.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("RegistrationMaps avoids throwing filesystem exists()", () => {
  const src = read(mapsPath);

  assert.match(
    src,
    /std::filesystem::exists\(patchPath, ec\)/,
    "LoadFromDisk should use the error_code overload of filesystem::exists",
  );
});

