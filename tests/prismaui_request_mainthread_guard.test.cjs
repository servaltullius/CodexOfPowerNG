const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const requestOpsPath = path.join(__dirname, "..", "src", "PrismaUIRequestOps.cpp");

function readRequestOps() {
  return fs.readFileSync(requestOpsPath, "utf8");
}

test("Prisma UI request ops avoid synchronous gameplay fallbacks", () => {
  const src = readRequestOps();

  assert.match(
    src,
    /ReportMainTaskQueueUnavailable\("Register item"\)/,
    "register path should report unavailable main task queue",
  );
  assert.match(
    src,
    /ReportMainTaskQueueUnavailable\("Undo register item"\)/,
    "undo path should report unavailable main task queue",
  );
  assert.match(
    src,
    /ReportMainTaskQueueUnavailable\("Refund rewards"\)/,
    "refund path should report unavailable main task queue",
  );

  assert.doesNotMatch(
    src,
    /PresentRegisterResult\(res,\s*true\)/,
    "register path should not execute synchronous fallback",
  );
  assert.doesNotMatch(
    src,
    /PresentUndoResult\(res,\s*true\)/,
    "undo path should not execute synchronous fallback",
  );
});
