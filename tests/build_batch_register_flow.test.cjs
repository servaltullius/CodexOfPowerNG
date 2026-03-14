const test = require("node:test");
const assert = require("node:assert/strict");
const path = require("node:path");

const modulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_register_batch_panel.js");
const mod = require(modulePath);

test("grouped batch register view model preserves disabled reasons and one-row summary math", () => {
  const viewModel = mod.buildRegisterBatchViewModel(
    {
      sections: [
        {
          discipline: "attack",
          rows: [
            { formId: 46775, regKey: 46775, name: "Iron Sword", discipline: "attack", totalCount: 3, safeCount: 2, actionable: true, disabledReason: null, buildPoints: 0.7, buildPointsCenti: 70 },
            { formId: 51234, regKey: 51234, name: "Quest Blade", discipline: "attack", totalCount: 1, safeCount: 0, actionable: false, disabledReason: "quest_protected", buildPoints: 0.7, buildPointsCenti: 70 },
            { formId: 61234, regKey: 61234, name: "Steel Arrow", discipline: "attack", totalCount: 12, safeCount: 12, actionable: true, disabledReason: null, buildPoints: 0.2, buildPointsCenti: 20 },
          ],
        },
        {
          discipline: "utility",
          rows: [
            { formId: 71234, regKey: 71234, name: "Ruin Book", discipline: "utility", totalCount: 1, safeCount: 1, actionable: true, disabledReason: null, buildPoints: 0.08, buildPointsCenti: 8 },
            { formId: 81234, regKey: 81234, name: "Blessed Token", discipline: "utility", totalCount: 1, safeCount: 0, actionable: false, disabledReason: "favorite_protected", buildPoints: 0.05, buildPointsCenti: 5 },
          ],
        },
      ],
    },
    [46775, 61234, 71234],
    {
      actionableOnly: false,
      query: "",
    },
  );

  assert.equal(viewModel.listScope, "all_register_relevant");
  assert.equal(viewModel.summary.selectedRows, 3);
  assert.deepEqual(viewModel.summary.disciplineGain, { attack: 2, defense: 0, utility: 1 });
  assert.deepEqual(viewModel.summary.disciplinePointGain, { attack: 0.9, defense: 0, utility: 0.08 });
  assert.equal(viewModel.summary.formIds.length, 3);
  assert.equal(viewModel.rows[0].totalCount, 3);
  assert.equal(viewModel.rows[0].safeCount, 2);
  assert.equal(viewModel.rows[0].buildPoints, 0.7);
  assert.ok(Array.isArray(viewModel.rows[0].stateTags));
  assert.equal(viewModel.rows[0].canBatchSelect, true);
  assert.equal(viewModel.rows[1].singleRegisterAction, "disabled");
  assert.equal(viewModel.disabledRows[0].reasonTag, "quest_protected");
  assert.equal(viewModel.disabledRows[1].reasonTag, "favorite_protected");
});

test("grouped batch register view model merges duplicate discipline sections into one bucket", () => {
  const viewModel = mod.buildRegisterBatchViewModel(
    {
      sections: [
        {
          discipline: "attack",
          rows: [{ formId: 1, regKey: 1, name: "Iron Sword", discipline: "attack", totalCount: 1, safeCount: 1, actionable: true, disabledReason: null, buildPoints: 0.7, buildPointsCenti: 70 }],
        },
        {
          discipline: "utility",
          rows: [{ formId: 2, regKey: 2, name: "Potion", discipline: "utility", totalCount: 2, safeCount: 2, actionable: true, disabledReason: null, buildPoints: 0.25, buildPointsCenti: 25 }],
        },
        {
          discipline: "utility",
          rows: [{ formId: 3, regKey: 3, name: "Soul Gem", discipline: "utility", totalCount: 1, safeCount: 1, actionable: true, disabledReason: null, buildPoints: 0.3, buildPointsCenti: 30 }],
        },
      ],
    },
    [2, 3],
    {
      actionableOnly: false,
      query: "",
    },
  );

  assert.equal(viewModel.sections.length, 2);
  assert.equal(viewModel.sections[1].discipline, "utility");
  assert.equal(viewModel.sections[1].rows.length, 2);
  assert.deepEqual(
    viewModel.sections[1].rows.map((row) => row.name),
    ["Potion", "Soul Gem"],
  );
  assert.deepEqual(viewModel.summary.disciplineGain, { attack: 0, defense: 0, utility: 2 });
  assert.deepEqual(viewModel.summary.disciplinePointGain, { attack: 0, defense: 0, utility: 0.55 });
});

test("grouped batch register renderer shows an explicit empty state when no rows are available", () => {
  const html = mod.renderRegisterBatchRowsHtml(
    {
      sections: [],
      rows: [],
      summary: {
        selectedRows: 0,
        disciplineGain: { attack: 0, defense: 0, utility: 0 },
        disciplinePointGain: { attack: 0, defense: 0, utility: 0 },
        formIds: [],
      },
    },
    {
      t: (_key, fallback) => fallback,
      escapeHtml: (value) => String(value == null ? "" : value),
      toHex32: (value) => "0x" + Number(value >>> 0).toString(16).toUpperCase(),
    },
  );

  assert.match(html, /No items|No register-relevant items/i);
});

test("grouped batch register renderer exposes codex row structure and discipline hooks", () => {
  const html = mod.renderRegisterBatchRowsHtml(
    {
      sections: [
        {
          discipline: "attack",
          label: "Attack",
          rows: [
            {
              formId: 46775,
              regKey: 46775,
              name: "Iron Sword",
              discipline: "attack",
              totalCount: 3,
              safeCount: 2,
              actionable: true,
              canBatchSelect: true,
              singleRegisterAction: "enabled",
              buildPoints: 0.7,
              buildPointsCenti: 70,
              reasonTag: null,
              stateTags: [],
              isSelected: true,
            },
            {
              formId: 51234,
              regKey: 51234,
              name: "Quest Blade",
              discipline: "attack",
              totalCount: 1,
              safeCount: 0,
              actionable: false,
              canBatchSelect: false,
              singleRegisterAction: "disabled",
              buildPoints: 0.7,
              buildPointsCenti: 70,
              reasonTag: "quest_protected",
              stateTags: ["quest_protected", "not_actionable"],
              isSelected: false,
            },
          ],
        },
      ],
      rows: [],
      summary: {
        selectedRows: 1,
        disciplineGain: { attack: 1, defense: 0, utility: 0 },
        disciplinePointGain: { attack: 0.7, defense: 0, utility: 0 },
        formIds: [46775],
      },
    },
    {
      t: (_key, fallback) => fallback,
      escapeHtml: (value) => String(value == null ? "" : value),
      toHex32: (value) => "0x" + Number(value >>> 0).toString(16).toUpperCase(),
    },
  );

  assert.match(html, /sectionRow/);
  assert.match(html, /disc-attack/);
  assert.match(html, /colSelect/);
  assert.match(html, /colGroup/);
  assert.match(html, /groupMeta/);
  assert.match(html, /disciplineMark/);
  assert.match(html, /pointTag/);
  assert.match(html, /itemName/);
  assert.match(html, /stateTag/);
  assert.match(html, /reasonText/);
  assert.match(html, /quest_protected|not_actionable/);
  assert.match(html, /rowActionButton/);
  assert.match(html, /Weight \+0\.7 pt/);
});

test("grouped batch register summary favors point gain while preserving row counts", () => {
  const html = mod.renderRegisterBatchSummaryHtml(
    {
      sections: [],
      rows: [],
      summary: {
        selectedRows: 3,
        disciplineGain: { attack: 2, defense: 1, utility: 0 },
        disciplinePointGain: { attack: 0.9, defense: 0.3, utility: 0 },
        formIds: [46775, 61234, 90001],
      },
    },
    {
      t: (_key, fallback) => fallback,
      tFmt: (_key, fallback, vars) => String(fallback).replace(/\{([a-zA-Z0-9_]+)\}/g, (match, name) => (vars && Object.prototype.hasOwnProperty.call(vars, name) ? String(vars[name]) : match)),
      escapeHtml: (value) => String(value == null ? "" : value),
    },
  );

  assert.match(html, /Attack \+0\.9 pt \(2\)/);
  assert.match(html, /Defense \+0\.3 pt \(1\)/);
});
