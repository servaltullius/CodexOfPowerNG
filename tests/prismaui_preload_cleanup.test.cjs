const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const internalHeaderPath = path.join(__dirname, "..", "src", "PrismaUIInternal.h");
const managerPath = path.join(__dirname, "..", "src", "PrismaUIManager.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("PreLoadGame forces Prisma UI cleanup to avoid stuck focus/cursor", () => {
  const header = read(internalHeaderPath);
  const manager = read(managerPath);

  assert.match(
    header,
    /void ForceCleanupForLoadBoundary\(\) noexcept;/,
    "PrismaUIInternal should declare a PreLoadGame cleanup hook",
  );

  assert.match(
    manager,
    /void OnPreLoadGame\(\) noexcept[\s\S]*?ForceCleanupForLoadBoundary\(\);/,
    "OnPreLoadGame should call ForceCleanupForLoadBoundary()",
  );
});

