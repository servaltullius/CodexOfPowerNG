const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const lifecyclePath = path.join(__dirname, "..", "src", "PrismaUIViewLifecycle.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("PrismaUI interop guards JSON dump exceptions in noexcept path", () => {
  const src = read(lifecyclePath);

  assert.match(
    src,
    /try\s*\{\s*s = payload\.dump\(\);/,
    "CallJS should wrap payload.dump() in a try block",
  );

  assert.match(
    src,
    /catch\s*\(const std::exception& e\)\s*\{\s*SKSE::log::error\("InteropCall: failed to serialize payload/,
    "CallJS should catch std::exception and log an error",
  );
});

