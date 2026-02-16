const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const viewPath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "index.html");
const orbitModulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "reward_orbit.js");
const uiWiringModulePath = path.join(__dirname, "..", "PrismaUI", "views", "codexofpowerng", "ui_wiring.js");
const html = fs.readFileSync(viewPath, "utf8");
const orbitModuleSource = fs.readFileSync(orbitModulePath, "utf8");
const uiWiringModuleSource = fs.readFileSync(uiWiringModulePath, "utf8");
const rewardOrbit = require(orbitModulePath);

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

test("rewards view loads reward orbit module", () => {
  assert.match(
    html,
    /<script src="reward_orbit\.js"><\/script>/,
    "Rewards view should load reward_orbit.js before inline script",
  );
});

test("reward orbit module has high-DPI compact mode and dynamic node width", () => {
  assert.match(
    orbitModuleSource,
    /function getRewardOrbitLayout\([^)]*\)\s*\{[\s\S]*scale >= 2\.0 \|\| viewportW < 1280[\s\S]*positions:\s*REWARD_ORBIT_POSITIONS_COMPACT[\s\S]*nodeWidthPx:\s*162[\s\S]*positions:\s*REWARD_ORBIT_POSITIONS_REGULAR[\s\S]*nodeWidthPx:\s*188[\s\S]*\}/,
    "Reward orbit should switch to compact layout for high DPI or smaller viewport",
  );

  assert.match(
    orbitModuleSource,
    /function renderRewardOrbit\([^)]*\)\s*\{[\s\S]*getRewardOrbitLayout\([\s\S]*\)[\s\S]*style\.setProperty\("--rewardNodeWidth", `\$\{layout\.nodeWidthPx\}px`\)[\s\S]*const maxNodes = layout\.positions\.length;[\s\S]*\}/,
    "Reward orbit renderer should apply dynamic node width and cap visible nodes by selected layout",
  );
});

test("reward orbit module exports behavior helpers", () => {
  assert.equal(typeof rewardOrbit.getRewardOrbitLayout, "function");
  assert.equal(typeof rewardOrbit.renderRewardOrbit, "function");
  assert.equal(typeof rewardOrbit.syncRewardCharacterImageState, "function");

  const regular = rewardOrbit.getRewardOrbitLayout({ scale: 1.0, viewportW: 1920 });
  const compact = rewardOrbit.getRewardOrbitLayout({ scale: 2.0, viewportW: 1920 });
  assert.equal(regular.nodeWidthPx, 188);
  assert.equal(compact.nodeWidthPx, 162);
  assert.equal(regular.positions.length, 8);
  assert.equal(compact.positions.length, 6);
});

test("reward orbit updates on resize and handles image load errors", () => {
  assert.match(
    uiWiringModuleSource,
    /addListener\(win, "resize", function \(\) \{[\s\S]*scheduleVirtualRender\(\{ force: true \}\);[\s\S]*renderRewards\(\);[\s\S]*\}\);/,
    "UI wiring should rerender rewards on resize so orbit layout reflows",
  );

  assert.match(
    uiWiringModuleSource,
    /addListener\(rewardCharacterImgEl, "load", syncRewardCharacterImageState\);[\s\S]*addListener\(rewardCharacterImgEl, "error", syncRewardCharacterImageState\);/,
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
