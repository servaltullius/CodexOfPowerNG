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
            { formId: 46775, regKey: 46775, name: "Iron Sword", discipline: "attack", totalCount: 3, safeCount: 2, actionable: true, disabledReason: null },
            { formId: 51234, regKey: 51234, name: "Quest Blade", discipline: "attack", totalCount: 1, safeCount: 0, actionable: false, disabledReason: "quest_protected" },
            { formId: 61234, regKey: 61234, name: "Steel Arrow", discipline: "attack", totalCount: 12, safeCount: 12, actionable: true, disabledReason: null },
          ],
        },
        {
          discipline: "utility",
          rows: [
            { formId: 71234, regKey: 71234, name: "Ruin Book", discipline: "utility", totalCount: 1, safeCount: 1, actionable: true, disabledReason: null },
            { formId: 81234, regKey: 81234, name: "Blessed Token", discipline: "utility", totalCount: 1, safeCount: 0, actionable: false, disabledReason: "favorite_protected" },
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
  assert.equal(viewModel.summary.formIds.length, 3);
  assert.equal(viewModel.rows[0].totalCount, 3);
  assert.equal(viewModel.rows[0].safeCount, 2);
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
          rows: [{ formId: 1, regKey: 1, name: "Iron Sword", discipline: "attack", totalCount: 1, safeCount: 1, actionable: true, disabledReason: null }],
        },
        {
          discipline: "utility",
          rows: [{ formId: 2, regKey: 2, name: "Potion", discipline: "utility", totalCount: 2, safeCount: 2, actionable: true, disabledReason: null }],
        },
        {
          discipline: "utility",
          rows: [{ formId: 3, regKey: 3, name: "Soul Gem", discipline: "utility", totalCount: 1, safeCount: 1, actionable: true, disabledReason: null }],
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
});

test("grouped batch register renderer shows an explicit empty state when no rows are available", () => {
  const html = mod.renderRegisterBatchRowsHtml(
    {
      sections: [],
      rows: [],
      summary: {
        selectedRows: 0,
        disciplineGain: { attack: 0, defense: 0, utility: 0 },
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
  assert.match(html, /disciplineMark/);
  assert.match(html, /itemName/);
  assert.match(html, /stateTag/);
  assert.match(html, /reasonText/);
  assert.match(html, /quest_protected|not_actionable/);
  assert.match(html, /rowActionButton/);
});
