const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const lifecyclePath = path.join(__dirname, "..", "src", "PrismaUIViewLifecycle.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("PrismaUI API is re-acquired lazily if missing at OnPostLoad", () => {
  const src = read(lifecyclePath);

  assert.match(
    src,
    /RequestPluginAPI\(PRISMA_UI_API::InterfaceVersion::V1\)/,
    "GetPrismaAPI should be able to request the Prisma UI API (late acquire)",
  );
});

