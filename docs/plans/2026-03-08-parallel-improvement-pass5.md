# 2026-03-08 Parallel Improvement Pass 5

## Goal
- Move the remaining PrismaUI rendering layer out of `index.html` into a dedicated module.
- Leave `index.html` responsible for state ownership, native/bootstrap wiring, and non-render interaction helpers only.

## Non-goals
- UI redesign or behavior changes
- Moving input-correction or keyboard-shortcut logic again
- New native API contracts
- Packaging or release automation updates

## Affected files or modules
- Add: `PrismaUI/views/codexofpowerng/ui_rendering.js`
- Add: `tests/ui_rendering_module.test.cjs`
- Add: `docs/plans/2026-03-08-parallel-improvement-pass5.md`
- Modify: `PrismaUI/views/codexofpowerng/index.html`
- Modify: `tests/lotd_gate_warning_ui.test.cjs`
- Modify: `tests/virtual_refresh_resync.test.cjs`
- Modify: `tests/ui_wiring_module.test.cjs`
- Modify: `tests/interop_bridge_module.test.cjs`

## Constraints
- Preserve current status, inventory, registered, undo, rewards, and settings rendering behavior
- Keep LOTD warning/toast behavior unchanged
- Keep virtual-table refresh ordering unchanged
- Avoid leaving fallback rendering code in `index.html`

## Milestones
1. Freeze pass-5 scope in a plan document
2. Add `ui_rendering.js` and extract all inline rendering functions
3. Rewire `index.html` to consume the rendering module
4. Update regression tests to point at the new rendering boundary
5. Run targeted tests, full tests, and the release build

## Validation steps
1. `node --test tests/ui_rendering_module.test.cjs tests/lotd_gate_warning_ui.test.cjs tests/ui_wiring_module.test.cjs tests/virtual_refresh_resync.test.cjs tests/interop_bridge_module.test.cjs`
2. `bash scripts/test.sh`
3. `env VCPKG_ROOT=/mnt/c/Users/kdw73/vcpkg cmake --build --preset wsl-release`

## Risks
- `index.html` currently mixes render helpers with interaction state, so extracting without changing behavior requires careful dependency injection
- Regex-based tests will need synchronized updates when function ownership moves from HTML to a JS module
- UI wiring, native state bridge, and shortcut handling all depend on render functions, so module boundaries must stay explicit

## Rollback notes
- `ui_rendering.js` can be rolled back by moving the returned methods back into `index.html`
- If a regression appears, restore the previous inline render functions before changing native or wiring code
