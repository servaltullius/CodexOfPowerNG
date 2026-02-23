const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const workersPath = path.join(__dirname, "..", "src", "PrismaUIViewWorkers.cpp");

function readWorkers() {
  return fs.readFileSync(workersPath, "utf8");
}

test("Prisma UI workers use generation guards instead of in-path joins", () => {
  const src = readWorkers();

  assert.match(src, /closeRetryGeneration\.fetch_add/, "close retry flow should bump generation");
  assert.match(src, /focusDelayGeneration\.fetch_add/, "focus delay flow should bump generation");
  assert.match(src, /ReplaceWorkerThread\(/, "worker replacement should avoid blocking joins in request path");
});
