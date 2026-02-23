const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const settingsPath = path.join(__dirname, "..", "src", "PrismaUISettings.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("settings worker logs when UI feedback enqueue fails", () => {
  const src = read(settingsPath);

  assert.match(
    src,
    /if \(!QueueUITask\(\[ok,\s*needsMainThreadL10n\]\(\) \{/,
    "Settings save worker should explicitly check QueueUITask() result",
  );

  assert.match(
    src,
    /Settings save worker: failed to queue UI feedback task/,
    "Settings save worker should log enqueue failures for UI feedback tasks",
  );
});

