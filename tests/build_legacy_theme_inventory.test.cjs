const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const inventoryPath = path.join(__dirname, "..", "docs", "contracts", "build-legacy-theme-inventory.json");
const inventory = JSON.parse(fs.readFileSync(inventoryPath, "utf8"));

const EXPECTED_THEME_MAP = Object.freeze({
  attack: ["devastation", "fury", "precision"],
  defense: ["bastion", "guard", "resistance"],
  utility: ["exploration", "livelihood", "trickery"],
});

const EXPECTED_HIERARCHY = ["signpost", "special", "standard"];

test("legacy build inventory stays inside the approved discipline/theme taxonomy", () => {
  assert.equal(inventory.version, 1);
  assert.deepEqual(
    Object.keys(inventory.disciplineThemes).sort(),
    ["attack", "defense", "utility"],
    "Inventory should only declare the three approved build disciplines",
  );

  for (const [discipline, expectedThemes] of Object.entries(EXPECTED_THEME_MAP)) {
    assert.deepEqual(
      [...inventory.disciplineThemes[discipline]].sort(),
      expectedThemes,
      `${discipline} themes should stay pinned to the curated UI navigation set`,
    );
  }

  for (const entry of inventory.entries) {
    assert.ok(EXPECTED_THEME_MAP[entry.discipline], `Unknown discipline: ${entry.discipline}`);
    assert.ok(
      EXPECTED_THEME_MAP[entry.discipline].includes(entry.theme),
      `Theme ${entry.theme} must remain inside ${entry.discipline}`,
    );
    assert.ok(
      EXPECTED_HIERARCHY.includes(entry.hierarchy),
      `Hierarchy ${entry.hierarchy} must remain inside signpost/standard/special`,
    );
  }
});

test("legacy build inventory keeps candidate ids unique and all navigation themes populated", () => {
  const ids = new Set();
  const seenThemes = {
    attack: new Set(),
    defense: new Set(),
    utility: new Set(),
  };

  for (const entry of inventory.entries) {
    assert.ok(!ids.has(entry.id), `Duplicate legacy candidate id: ${entry.id}`);
    ids.add(entry.id);
    assert.ok(Array.isArray(entry.sources) && entry.sources.length > 0, `Missing sources for ${entry.id}`);
    assert.equal(typeof entry.notes, "string", `Missing notes field for ${entry.id}`);
    seenThemes[entry.discipline].add(entry.theme);
  }

  for (const [discipline, expectedThemes] of Object.entries(EXPECTED_THEME_MAP)) {
    assert.deepEqual(
      [...seenThemes[discipline]].sort(),
      expectedThemes,
      `${discipline} inventory should populate every curated theme so the UI taxonomy stays meaningful`,
    );
  }

  assert.ok(
    inventory.entries.length >= 35,
    "Legacy inventory should cover most existing reward candidates before broad catalog intake starts",
  );
});
