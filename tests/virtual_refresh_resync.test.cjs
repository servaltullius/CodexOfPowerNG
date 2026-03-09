const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const nativeStateBridgeModulePath = path.join(
  __dirname,
  "..",
  "PrismaUI",
  "views",
  "codexofpowerng",
  "native_state_bridge.js",
);
const uiRenderingModulePath = path.join(
  __dirname,
  "..",
  "PrismaUI",
  "views",
  "codexofpowerng",
  "ui_rendering.js",
);
const virtualTablesModulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "virtual_tables.js");
const html = fs.readFileSync(viewPath, "utf8");
const nativeStateBridgeModuleSource = fs.readFileSync(nativeStateBridgeModulePath, "utf8");
const uiRenderingModuleSource = fs.readFileSync(uiRenderingModulePath, "utf8");
const virtualTablesModuleSource = fs.readFileSync(virtualTablesModulePath, "utf8");

test("virtualized tables schedule a second forced pass after list refresh", () => {
  assert.match(
    html,
    /function schedulePostRefreshVirtualResync\(\)\s*\{[\s\S]*requestAnimationFrame\(\(\)\s*=>\s*scheduleVirtualRender\(\{\s*force:\s*true\s*\}\)\);[\s\S]*\}/,
    "A post-refresh rAF resync should exist so spacer math is recalculated after layout/scroll clamping settles",
  );

  assert.match(
    html,
    /const nativeHandlers = nativeStateBridgeApi\.createNativeStateBridge\(\{[\s\S]*isTabActive:\s*uiRendering\.isTabActive,[\s\S]*schedulePostRefreshVirtualResync[\s\S]*\}\);/,
    "Native state bridge wiring should receive active-tab gating and post-refresh virtual resync callbacks",
  );

  assert.match(
    nativeStateBridgeModuleSource,
    /onInventory\(nextInventoryPage\)\s*\{[\s\S]*if \(isTabActive\("tabQuick"\)\) \{[\s\S]*renderQuick\(\);[\s\S]*schedulePostRefreshVirtualResync\(\);[\s\S]*\}[\s\S]*\}/,
    "Inventory native callback should only re-render and resync when the quick tab is active",
  );

  assert.match(
    nativeStateBridgeModuleSource,
    /onRegistered\(nextRegistered\)\s*\{[\s\S]*if \(isTabActive\("tabRegistered"\)\) \{[\s\S]*renderRegistered\(\);[\s\S]*schedulePostRefreshVirtualResync\(\);[\s\S]*\}[\s\S]*\}/,
    "Registered native callback should only re-render and resync when the registered tab is active",
  );

  assert.match(
    html,
    /nativeBridgeBootstrapApi\.installNativeBridge\(\{[\s\S]*onInventory:\s*nativeHandlers\.onInventory[\s\S]*onRegistered:\s*nativeHandlers\.onRegistered[\s\S]*\}\);/,
    "Native bridge bootstrap wiring should connect inventory/registered callbacks from native handlers",
  );

  assert.match(
    html,
    /const VIRTUAL_MIN_ROWS = \d+;/,
    "A minimum-row threshold should exist so small lists skip spacer-based virtualization",
  );

  assert.match(
    html,
    /const uiRendering = uiRenderingApi\.createUIRendering\(\{[\s\S]*getVirtualTableManager:\s*ensureVirtualTablesManager[\s\S]*\}\);/,
    "ui_rendering wiring should receive the virtual table manager factory",
  );

  assert.match(
    uiRenderingModuleSource,
    /function renderQuickVirtual\(opts\)\s*\{[\s\S]*mgr\.renderQuickVirtual\(opts \|\| \{ force: false \}\);[\s\S]*\}/,
    "Quick render should delegate to virtual table manager via ui_rendering.js",
  );

  assert.match(
    uiRenderingModuleSource,
    /function renderRegisteredVirtual\(opts\)\s*\{[\s\S]*mgr\.renderRegisteredVirtual\(opts \|\| \{ force: false \}\);[\s\S]*\}/,
    "Registered render should delegate to virtual table manager via ui_rendering.js",
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
