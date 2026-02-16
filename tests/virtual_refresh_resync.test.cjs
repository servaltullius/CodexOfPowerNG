const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const virtualTablesModulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "virtual_tables.js");
const html = fs.readFileSync(viewPath, "utf8");
const virtualTablesModuleSource = fs.readFileSync(virtualTablesModulePath, "utf8");

test("virtualized tables schedule a second forced pass after list refresh", () => {
  assert.match(
    html,
    /function schedulePostRefreshVirtualResync\(\)\s*\{[\s\S]*requestAnimationFrame\(\(\)\s*=>\s*scheduleVirtualRender\(\{\s*force:\s*true\s*\}\)\);[\s\S]*\}/,
    "A post-refresh rAF resync should exist so spacer math is recalculated after layout/scroll clamping settles",
  );

  assert.match(
    html,
    /function onNativeInventory\(nextInventoryPage\)\s*\{[\s\S]*renderQuick\(\);[\s\S]*schedulePostRefreshVirtualResync\(\);[\s\S]*\}/,
    "Inventory native callback should trigger post-refresh virtual resync",
  );

  assert.match(
    html,
    /function onNativeRegistered\(nextRegistered\)\s*\{[\s\S]*renderRegistered\(\);[\s\S]*schedulePostRefreshVirtualResync\(\);[\s\S]*\}/,
    "Registered native callback should trigger post-refresh virtual resync",
  );

  assert.match(
    html,
    /interopBridgeApi\.installNativeCallbacks\(\{[\s\S]*onInventory:\s*onNativeInventory[\s\S]*onRegistered:\s*onNativeRegistered[\s\S]*\}\);/,
    "Interop bridge wiring should connect native inventory/registered callbacks",
  );

  assert.match(
    html,
    /const VIRTUAL_MIN_ROWS = \d+;/,
    "A minimum-row threshold should exist so small lists skip spacer-based virtualization",
  );

  assert.match(
    html,
    /function renderQuickVirtual\(\{ force = false \} = \{\}\)\s*\{[\s\S]*mgr\.renderQuickVirtual\(\{ force \}\);[\s\S]*\}/,
    "Quick render should delegate to virtual table manager",
  );

  assert.match(
    html,
    /function renderRegisteredVirtual\(\{ force = false \} = \{\}\)\s*\{[\s\S]*mgr\.renderRegisteredVirtual\(\{ force \}\);[\s\S]*\}/,
    "Registered render should delegate to virtual table manager",
  );

  assert.match(
    virtualTablesModuleSource,
    /function renderQuickVirtual\([^)]*\)\s*\{[\s\S]*const shouldVirtualize = total > minRows;[\s\S]*if \(!shouldVirtualize\)\s*\{/,
    "Virtual tables module should bypass quick virtualization for small row counts",
  );

  assert.match(
    virtualTablesModuleSource,
    /function renderRegisteredVirtual\([^)]*\)\s*\{[\s\S]*const shouldVirtualize = total > minRows;[\s\S]*if \(!shouldVirtualize\)\s*\{/,
    "Virtual tables module should bypass registered virtualization for small row counts",
  );
});
