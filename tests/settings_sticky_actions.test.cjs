const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const html = fs.readFileSync(viewPath, "utf8");

test("settings actions are sticky and include close/save/reload", () => {
  assert.match(
    html,
    /<div class="row settingsActions">[\s\S]*id="btnReloadSettings"[\s\S]*id="btnSaveSettings"[\s\S]*id="btnCloseSettings"[\s\S]*<\/div>/,
    "Settings action bar should include Reload, Save, and Close buttons in one sticky row",
  );

  assert.match(
    html,
    /\.settingsActions\s*\{[\s\S]*position:\s*sticky;[\s\S]*bottom:/,
    "Settings action bar should be sticky at the bottom of the settings panel",
  );
});
