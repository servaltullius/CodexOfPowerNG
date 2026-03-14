const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

function read(relPath) {
  return fs.readFileSync(path.join(__dirname, "..", relPath), "utf8");
}

test("inventory payload groups sections by discipline and forwards disabled reasons", () => {
  const payloadSrc = read("src/PrismaUIPayloadsInventory.cpp");

  assert.doesNotMatch(
    payloadSrc,
    /currentGroup != it\.group/,
    "Grouped quick-register payload should not split sections on legacy group boundaries",
  );
  assert.doesNotMatch(
    payloadSrc,
    /\{\s*"disabledReason",\s*nullptr\s*\}/,
    "Inventory payload should forward the row's disabled reason instead of hardcoding null",
  );
  assert.match(
    payloadSrc,
    /\{\s*"buildPoints",\s*Builds::FromBuildPointCenti\(it\.buildPointsCenti\)\s*\}/,
    "Inventory payload should expose per-row build points for quick-register weighting",
  );
  assert.match(
    payloadSrc,
    /\{\s*"buildPointsCenti",\s*it\.buildPointsCenti\s*\}/,
    "Inventory payload should expose precise centi-point values for quick-register rows",
  );
});

test("quick register builder preserves disabled rows with explicit reason tags", () => {
  const builderSrc = read("src/RegistrationQuickListBuilder.cpp");
  const registrationHeader = read("include/CodexOfPowerNG/Registration.h");

  assert.match(
    registrationHeader,
    /std::string\s+disabledReason;/,
    "ListItem should carry a disabled reason through the payload pipeline",
  );
  assert.match(
    registrationHeader,
    /Builds::BuildPointCenti\s+buildPointsCenti\{ 0 \};/,
    "ListItem should carry weighted build points for quick-register display",
  );
  assert.match(
    builderSrc,
    /quest_protected/,
    "Quick register builder should mark quest-protected rows explicitly",
  );
  assert.match(
    builderSrc,
    /favorite_protected/,
    "Quick register builder should mark favorite-protected rows explicitly",
  );
  assert.match(
    builderSrc,
    /item\.buildPointsCenti\s*=\s*BuildProgression::ResolveBuildPointsForFormType/,
    "Quick register builder should attach the build-point weight for each candidate row",
  );
});
