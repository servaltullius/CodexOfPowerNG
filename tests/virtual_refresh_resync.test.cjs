const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const html = fs.readFileSync(viewPath, "utf8");

test("virtualized tables schedule a second forced pass after list refresh", () => {
  assert.match(
    html,
    /function schedulePostRefreshVirtualResync\(\)\s*\{[\s\S]*requestAnimationFrame\(\(\)\s*=>\s*scheduleVirtualRender\(\{\s*force:\s*true\s*\}\)\);[\s\S]*\}/,
    "A post-refresh rAF resync should exist so spacer math is recalculated after layout/scroll clamping settles",
  );

  assert.match(
    html,
    /window\.copng_setInventory\s*=\s*\(jsonStr\)\s*=>\s*\{[\s\S]*renderQuick\(\);[\s\S]*schedulePostRefreshVirtualResync\(\);[\s\S]*\}/,
    "Inventory refresh should trigger post-refresh virtual resync",
  );

  assert.match(
    html,
    /window\.copng_setRegistered\s*=\s*\(jsonStr\)\s*=>\s*\{[\s\S]*renderRegistered\(\);[\s\S]*schedulePostRefreshVirtualResync\(\);[\s\S]*\}/,
    "Registered refresh should trigger post-refresh virtual resync",
  );

  assert.match(
    html,
    /const VIRTUAL_MIN_ROWS = \d+;/,
    "A minimum-row threshold should exist so small lists skip spacer-based virtualization",
  );

  assert.match(
    html,
    /function renderQuickVirtual\(\{ force = false \} = \{\}\)\s*\{[\s\S]*const shouldVirtualize = total > VIRTUAL_MIN_ROWS;[\s\S]*if \(!shouldVirtualize\)\s*\{/,
    "Quick list should bypass virtualization for small row counts",
  );

  assert.match(
    html,
    /function renderRegisteredVirtual\(\{ force = false \} = \{\}\)\s*\{[\s\S]*const shouldVirtualize = total > VIRTUAL_MIN_ROWS;[\s\S]*if \(!shouldVirtualize\)\s*\{/,
    "Registered list should bypass virtualization for small row counts",
  );
});
