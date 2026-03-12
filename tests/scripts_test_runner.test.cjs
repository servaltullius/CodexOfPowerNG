const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const scriptPath = path.join(__dirname, "..", "scripts", "test.sh");
const src = fs.readFileSync(scriptPath, "utf8");

test("scripts/test.sh uses an explicit host-safe routing table for C++ tests", () => {
  assert.match(
    src,
    /case "\$\(basename "\$test_src"\)" in/,
    "Host test runner should route each C++ test through an explicit case table",
  );
});

test("scripts/test.sh links build catalog implementation for contract test", () => {
  assert.match(
    src,
    /build_option_catalog_contract\.test\.cpp\)[\s\S]*extra_sources=\(\s*"\$ROOT_DIR\/src\/BuildOptionCatalog\.cpp"\s*\)/,
    "Catalog contract test should link BuildOptionCatalog.cpp in host mode",
  );
});

test("scripts/test.sh skips RE-dependent build helper tests in host mode", () => {
  assert.match(
    src,
    /build_migration_rules\.test\.cpp\|build_state_store_ops\.test\.cpp\|build_request_guards\.test\.cpp\|registration_build_progression\.test\.cpp\|build_effect_runtime\.test\.cpp\)/,
    "Host test runner should skip the known RE-dependent build helper tests",
  );
});
