const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const inventoryPath = path.join(__dirname, "..", "docs", "contracts", "build-legacy-expansion-inventory.json");
const inventory = JSON.parse(fs.readFileSync(inventoryPath, "utf8"));

const EXPECTED_THEME_MAP = Object.freeze({
  attack: ["devastation", "fury", "precision"],
  defense: ["bastion", "guard", "resistance"],
  utility: ["exploration", "finesse", "livelihood"],
});

const EXPECTED_HIERARCHY = ["signpost", "special", "standard"];
const EXPECTED_EFFECT_TYPES = ["actorValue", "carryWeight", "economy", "utilityFlag"];
const EXPECTED_RUNTIME_STATUS = ["supported", "needs-runtime", "deferred"];
const EXPECTED_SUPPORTED_RUNTIME_KEYS = new Set([
  "attack_damage_mult",
  "weapon_speed_mult",
  "critical_chance",
  "damage_resist",
  "block_power_modifier",
  "health",
  "magic_resist",
  "fire_resist",
  "frost_resist",
  "shock_resist",
  "poison_resist",
  "disease_resist",
  "absorb_chance",
  "speed_mult",
  "carry_weight",
  "speechcraft_modifier",
]);
const EXPECTED_MIGRATION_ANCHORS = [
  "build.attack.ferocity",
  "build.attack.precision",
  "build.attack.vitals",
  "build.defense.guard",
  "build.defense.bastion",
  "build.defense.endurance",
  "build.utility.cache",
  "build.utility.barter",
  "build.utility.mobility",
];

test("legacy build expansion inventory stays inside the approved taxonomy and schema", () => {
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
      `${discipline} themes should stay pinned to the curated navigation set`,
    );
  }

  for (const entry of inventory.entries) {
    assert.equal(typeof entry.legacyKey, "string", "legacyKey is required");
    assert.equal(typeof entry.sourceGroup, "string", `sourceGroup is required for ${entry.legacyKey}`);
    assert.equal(typeof entry.label, "string", `label is required for ${entry.legacyKey}`);
    assert.equal(typeof entry.discipline, "string", `discipline is required for ${entry.legacyKey}`);
    assert.equal(typeof entry.theme, "string", `theme is required for ${entry.legacyKey}`);
    assert.equal(typeof entry.hierarchy, "string", `hierarchy is required for ${entry.legacyKey}`);
    assert.equal(typeof entry.effectType, "string", `effectType is required for ${entry.legacyKey}`);
    assert.equal(typeof entry.runtimeKey, "string", `runtimeKey is required for ${entry.legacyKey}`);
    assert.ok(typeof entry.magnitude === "number" || entry.magnitude === null, `magnitude is required for ${entry.legacyKey}`);
    assert.equal(typeof entry.runtimeStatus, "string", `runtimeStatus is required for ${entry.legacyKey}`);
    assert.ok(Array.isArray(entry.migrationFrom), `migrationFrom array is required for ${entry.legacyKey}`);
    assert.equal(typeof entry.notes, "string", `notes is required for ${entry.legacyKey}`);

    assert.ok(EXPECTED_THEME_MAP[entry.discipline], `Unknown discipline: ${entry.discipline}`);
    assert.ok(
      EXPECTED_THEME_MAP[entry.discipline].includes(entry.theme),
      `Theme ${entry.theme} must remain inside ${entry.discipline}`,
    );
    assert.ok(
      EXPECTED_HIERARCHY.includes(entry.hierarchy),
      `Hierarchy ${entry.hierarchy} must remain inside signpost/standard/special`,
    );
    assert.ok(
      EXPECTED_EFFECT_TYPES.includes(entry.effectType),
      `effectType ${entry.effectType} must stay inside the approved runtime categories`,
    );
    assert.ok(
      EXPECTED_RUNTIME_STATUS.includes(entry.runtimeStatus),
      `runtimeStatus ${entry.runtimeStatus} must remain inside supported/needs-runtime/deferred`,
    );
  }
});

test("legacy build expansion inventory excludes skill rewards and only marks supported runtime keys when the runtime already handles them", () => {
  const seenThemes = {
    attack: new Set(),
    defense: new Set(),
    utility: new Set(),
  };
  const ids = new Set();

  for (const entry of inventory.entries) {
    assert.ok(!ids.has(entry.legacyKey), `Duplicate legacy candidate id: ${entry.legacyKey}`);
    ids.add(entry.legacyKey);
    assert.ok(
      !entry.legacyKey.startsWith("skill."),
      `Skill reward ${entry.legacyKey} should stay out of the build expansion inventory`,
    );
    seenThemes[entry.discipline].add(entry.theme);

    if (entry.runtimeStatus === "supported") {
      assert.ok(
        EXPECTED_SUPPORTED_RUNTIME_KEYS.has(entry.runtimeKey),
        `${entry.legacyKey} cannot be marked supported until BuildEffectRuntime handles ${entry.runtimeKey}`,
      );
    }
  }

  for (const [discipline, expectedThemes] of Object.entries(EXPECTED_THEME_MAP)) {
    assert.deepEqual(
      [...seenThemes[discipline]].sort(),
      expectedThemes,
      `${discipline} inventory should populate every curated theme so the navigation taxonomy stays meaningful`,
    );
  }

  assert.ok(
    inventory.entries.length >= 25,
    "Legacy expansion inventory should cover most non-skill AV/modifier candidates before runtime implementation starts",
  );
});

test("legacy build expansion inventory includes current catalog migration anchors and staged implementation batches", () => {
  const anchors = new Map(inventory.catalogMigrationAnchors.map((anchor) => [anchor.optionId, anchor]));

  for (const optionId of EXPECTED_MIGRATION_ANCHORS) {
    assert.ok(anchors.has(optionId), `Missing migration anchor for ${optionId}`);
    const anchor = anchors.get(optionId);
    assert.ok(EXPECTED_THEME_MAP[anchor.discipline].includes(anchor.theme), `${optionId} anchor must use an approved theme`);
    assert.ok(EXPECTED_HIERARCHY.includes(anchor.hierarchy), `${optionId} anchor must use an approved hierarchy`);
    assert.equal(typeof anchor.runtimeKey, "string", `${optionId} anchor needs a runtimeKey`);
    assert.equal(typeof anchor.notes, "string", `${optionId} anchor needs notes`);
  }

  assert.ok(Array.isArray(inventory.implementationBatches.supportedFirst), "supportedFirst batch list is required");
  assert.ok(Array.isArray(inventory.implementationBatches.needsRuntimeFirst), "needsRuntimeFirst batch list is required");
  assert.ok(inventory.implementationBatches.supportedFirst.length > 0, "supportedFirst should not be empty");
  assert.ok(inventory.implementationBatches.needsRuntimeFirst.length > 0, "needsRuntimeFirst should not be empty");
});
