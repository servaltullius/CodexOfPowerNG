const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const repoRoot = path.join(__dirname, "..");
const mainSrc = fs.readFileSync(path.join(repoRoot, "src", "main.cpp"), "utf8");
const requestOpsSrc = fs.readFileSync(path.join(repoRoot, "src", "PrismaUIRequestOps.cpp"), "utf8");
const registerSrc = fs.readFileSync(path.join(repoRoot, "src", "RegistrationQuickRegister.cpp"), "utf8");
const undoSrc = fs.readFileSync(path.join(repoRoot, "src", "RegistrationUndo.cpp"), "utf8");

test("build runtime sync is wired at gameplay boundaries", () => {
  assert.match(mainSrc, /BuildEffectRuntime::ResetForLoad\(\)/);
  assert.match(mainSrc, /BuildEffectRuntime::SyncCurrentBuildEffectsToPlayer\(\)/);
  assert.match(requestOpsSrc, /BuildEffectRuntime::SyncCurrentBuildEffectsToPlayer\(\)/);
  assert.match(registerSrc, /BuildEffectRuntime::SyncCurrentBuildEffectsToPlayer\(\)/);
  assert.match(undoSrc, /BuildEffectRuntime::SyncCurrentBuildEffectsToPlayer\(\)/);
});
