const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const rulesPath = path.join(__dirname, "..", "src", "RegistrationRules.cpp");
const regPaths = [
  path.join(__dirname, "..", "src", "Registration.cpp"),
  path.join(__dirname, "..", "src", "RegistrationQuickRegister.cpp"),
];

test("ammo is grouped as weapons for discovery categories", () => {
  const src = fs.readFileSync(rulesPath, "utf8");
  assert.match(
    src,
    /case RE::FormType::Ammo:\s*return 0;/,
    "Ammo should be grouped with weapons to avoid confusing 'Misc' categorization",
  );
});

test("quick register list uses live inventory count API", () => {
  const src = regPaths
    .filter((p) => fs.existsSync(p))
    .map((p) => fs.readFileSync(p, "utf8"))
    .join("\n");
  assert.match(
    src,
    /GetItemCount\(obj\)/,
    "Quick register list should derive counts from PlayerCharacter::GetItemCount",
  );
});
