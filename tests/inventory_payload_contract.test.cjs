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
    builderSrc,
    /quest_protected/,
    "Quick register builder should mark quest-protected rows explicitly",
  );
  assert.match(
    builderSrc,
    /favorite_protected/,
    "Quick register builder should mark favorite-protected rows explicitly",
  );
});
