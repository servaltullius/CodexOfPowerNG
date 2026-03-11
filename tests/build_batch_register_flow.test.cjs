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
