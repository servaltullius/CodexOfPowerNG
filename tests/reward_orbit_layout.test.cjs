const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const html = fs.readFileSync(viewPath, "utf8");

test("rewards tab includes character image and orbit container", () => {
  assert.match(
    html,
    /id="rewardCharacterImg"[\s\S]*src="assets\/character\.png"/,
    "Rewards view should reference assets/character.png",
  );

  assert.match(
    html,
    /id="rewardOrbit"/,
    "Rewards view should include rewardOrbit container",
  );
});

test("reward orbit layout has high-DPI compact mode and dynamic node width", () => {
  assert.match(
    html,
    /function getRewardOrbitLayout\(\)\s*\{[\s\S]*scale >= 2\.0 \|\| viewportW < 1280[\s\S]*positions:\s*REWARD_ORBIT_POSITIONS_COMPACT[\s\S]*nodeWidthPx:\s*162[\s\S]*positions:\s*REWARD_ORBIT_POSITIONS_REGULAR[\s\S]*nodeWidthPx:\s*188[\s\S]*\}/,
    "Reward orbit should switch to compact layout for high DPI or smaller viewport",
  );

  assert.match(
    html,
    /renderRewardOrbit\(rows\)\s*\{[\s\S]*getRewardOrbitLayout\(\)[\s\S]*style\.setProperty\("--rewardNodeWidth", `\$\{layout\.nodeWidthPx\}px`\)[\s\S]*const maxNodes = layout\.positions\.length;[\s\S]*\}/,
    "Reward orbit renderer should apply dynamic node width and cap visible nodes by selected layout",
  );
});

test("reward orbit updates on resize and handles image load errors", () => {
  assert.match(
    html,
    /window\.addEventListener\("resize", \(\) => \{[\s\S]*renderRewards\(\);[\s\S]*\}\);/,
    "Resize handling should rerender rewards so orbit layout reflows",
  );

  assert.match(
    html,
    /rewardCharacterImgEl\.addEventListener\("load", syncRewardCharacterImageState\);[\s\S]*rewardCharacterImgEl\.addEventListener\("error", syncRewardCharacterImageState\);/,
    "Character image should toggle fallback on both load and error",
  );
});

test("reward orbit strings exist in both locales", () => {
  assert.match(
    html,
    /"rewards\.imageMissing": "Character image missing: assets\/character\.png"/,
    "English locale should include image-missing text",
  );
  assert.match(
    html,
    /"rewards\.imageMissing": "캐릭터 이미지 없음: assets\/character\.png"/,
    "Korean locale should include image-missing text",
  );
  assert.match(
    html,
    /"rewards\.more": "\+\{n\} more"/,
    "English locale should include +{n} more label",
  );
  assert.match(
    html,
    /"rewards\.more": "\+\{n\}개 더"/,
    "Korean locale should include +{n}개 더 label",
  );
});
