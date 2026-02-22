const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const schedulerPath = path.join(__dirname, "..", "src", "TaskScheduler.cpp");

function read(p) {
  return fs.readFileSync(p, "utf8");
}

test("TaskScheduler guards scheduled tasks with try/catch", () => {
  const src = read(schedulerPath);

  assert.match(
    src,
    /AddTask\(\[task = std::move\(task\)\]\(\) mutable \{[\s\S]*?try[\s\S]*?task\(\);[\s\S]*?catch/,
    "AddMainTask should wrap task() in a try/catch",
  );

  assert.match(
    src,
    /AddUITask\(\[task = std::move\(task\)\]\(\) mutable \{[\s\S]*?try[\s\S]*?task\(\);[\s\S]*?catch/,
    "AddUITask should wrap task() in a try/catch",
  );
});

