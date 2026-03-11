(function (root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) {
    module.exports = api;
  }
  if (root) {
    root.COPNGRewardOrbit = api;
  }
})(typeof globalThis !== "undefined" ? globalThis : this, function () {
  "use strict";

  const EMPTY_POSITIONS = Object.freeze([]);

  function rewardMagnitude() {
    return 0;
  }

  function getRewardOrbitLayout() {
    return {
      positions: EMPTY_POSITIONS,
      nodeWidthPx: 0,
    };
  }

  function syncRewardCharacterImageState() {}

  function renderRewardOrbit(opts) {
    const onAfterRender = opts && typeof opts.onAfterRender === "function" ? opts.onAfterRender : null;
    if (onAfterRender) onAfterRender();
  }

  return {
    REWARD_ORBIT_POSITIONS_REGULAR: EMPTY_POSITIONS,
    REWARD_ORBIT_POSITIONS_COMPACT: EMPTY_POSITIONS,
    rewardMagnitude,
    getRewardOrbitLayout,
    syncRewardCharacterImageState,
    renderRewardOrbit,
  };
});
