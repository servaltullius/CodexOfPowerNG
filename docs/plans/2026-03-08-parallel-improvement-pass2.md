# 2026-03-08 Parallel Improvement Pass 2

## Goal
- Continue reducing shared-state coupling by isolating the rewards state access path.
- Make PrismaUI refresh behavior easier to change in parallel by documenting the current invalidation contract.

## Non-goals
- Full replacement of `RuntimeState`
- Serialization store split for all domains
- Behavior changes in rewards math, sync cadence, or UI refresh payload contents
- GitHub Actions expansion beyond the existing validation rail

## Affected files or modules
- Add: `include/CodexOfPowerNG/RewardStateStore.h`
- Add: `src/RewardStateStore.cpp`
- Add: `tests/reward_state_store_module.test.cjs`
- Add: `docs/plans/2026-03-08-parallel-improvement-pass2.md`
- Modify: `CMakeLists.txt`
- Modify: `src/RewardsCore.cpp`
- Modify: `src/RewardsSyncEngine.cpp`
- Modify: `src/Rewards.cpp`
- Modify: `docs/contracts/prismaui-js-api.md`
- Modify: `tests/reward_caps_policy.test.cjs`

## Constraints
- Preserve current reward application and sync semantics
- Keep changes incremental and low-risk
- Avoid changing serialization schema in this pass
- Do not weaken existing JS/C++ regression coverage

## Milestones
1. Add the pass-2 plan and lock scope
2. Introduce a dedicated reward state access module
3. Migrate reward runtime paths off direct `state.rewardTotals` access
4. Document the PrismaUI refresh/invalidation contract
5. Run tests and release build again

## Validation steps
1. `node --test tests/reward_state_store_module.test.cjs tests/reward_caps_policy.test.cjs`
2. `bash scripts/test.sh`
3. `env VCPKG_ROOT=/mnt/c/Users/kdw73/vcpkg cmake --build --preset wsl-release`

## Risks
- Regex-based source tests may need coordinated updates when internal structure moves
- Reward sync code is gameplay-sensitive, so even behavior-preserving refactors need full regression checks
- `RewardStateStore` is a transitional seam, not the final architecture

## Rollback notes
- `RewardStateStore` can be reverted independently from the PrismaUI contract docs
- If reward regressions appear, restore direct reward-state access and keep only the documentation updates
