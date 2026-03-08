# 2026-03-08 Parallel Improvement Pass 1

## Goal
- Make parallel improvement work safer by reducing coupling in three low-risk seams:
- add a repeatable validation rail for JS and host-friendly C++ tests
- shrink PrismaUI inline interop/bootstrap duplication
- reduce PrismaUI bridge dependence on direct `RuntimeState` reads

## Non-goals
- Full `RuntimeState` breakup into separate domain stores
- Large Rewards/Serialization architectural rewrites
- Reworking Skyrim-dependent C++ runtime behavior
- Packaging/release artifact regeneration in this pass

## Affected files or modules
- Add: `.github/workflows/validate.yml`
- Add: `docs/plans/2026-03-08-parallel-improvement-pass1.md`
- Modify: `PrismaUI/views/codexofpowerng/interop_bridge.js`
- Modify: `PrismaUI/views/codexofpowerng/index.html`
- Modify: `include/CodexOfPowerNG/RegistrationStateStore.h`
- Modify: `src/RegistrationStateStore.cpp`
- Modify: `include/CodexOfPowerNG/Rewards.h`
- Modify: `src/Rewards.cpp`
- Modify: `src/PrismaUIStatePayload.cpp`
- Modify: `src/PrismaUIRequestOps.cpp`
- Modify: `tests/interop_bridge_module.test.cjs`

## Constraints
- Preserve runtime behavior
- Keep changes modular and low-risk
- Avoid introducing new dependencies
- Keep JS fallback behavior available when the interop helper is unavailable
- Prefer API extraction over broad rewrites

## Milestones
1. Document the pass and freeze the intended scope
2. Add CI validation for `scripts/test.sh`
3. Extract native callback fallback wiring into `interop_bridge.js`
4. Add narrow query APIs for registered count / reward snapshot and adopt them in PrismaUI bridge code
5. Run local validation

## Validation steps
1. `bash scripts/test.sh`
2. `node --test tests/interop_bridge_module.test.cjs tests/virtual_refresh_resync.test.cjs tests/ui_wiring_module.test.cjs`

## Risks
- `index.html` tests rely on regex snapshots, so even safe extraction can break brittle assertions
- New bridge query APIs must not change payload semantics
- GitHub Actions can drift from local environment if kept too ambitious

## Rollback notes
- Revert `.github/workflows/validate.yml` independently if CI needs to be disabled
- Revert JS interop extraction independently from C++ query API extraction
- If any bridge regression appears, restore the original inline fallback block in `index.html`
