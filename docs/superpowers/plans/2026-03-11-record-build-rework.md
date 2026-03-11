# Record-Build Rework Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the legacy random permanent reward system with a score-driven build system built around Attack / Defense / Utility disciplines, automatic threshold unlocks, active slots, grouped batch registration, and legacy reward migration.

**Architecture:** Keep the existing modular monolith structure, but replace reward authority with a new build-progression domain. Use deterministic discipline scores as the source of truth, derive passive milestones and unlock availability from those scores, and route all UI build actions through new native payloads instead of the old rewards contract. Migration must preserve registered progress, clear legacy reward state exactly once, and leave build slots empty after conversion.

**Tech Stack:** C++23 SKSE plugin, CommonLibSSE, nlohmann/json, Prisma UI web overlay, Node-based UI module tests, standalone C++ tests via CTest.

---

## File Structure

- Create: `include/CodexOfPowerNG/BuildTypes.h`
  Responsibility: shared enums and POD types for discipline IDs, slot IDs, option descriptors, score snapshots, and activation records.
- Create: `include/CodexOfPowerNG/BuildOptionCatalog.h`
  Responsibility: read-only access to the first-release build catalog and baseline milestone definitions.
- Create: `src/BuildOptionCatalog.cpp`
  Responsibility: native catalog tables, threshold lookup helpers, and slot/exclusivity rules.
- Create: `include/CodexOfPowerNG/BuildStateStore.h`
  Responsibility: runtime accessors for discipline scores, active slots, migration state, and derived activation helpers.
- Create: `src/BuildStateStore.cpp`
  Responsibility: synchronized mutation/read APIs over the new build slice inside persistent runtime state, including migration notice snapshots.
- Create: `include/CodexOfPowerNG/BuildProgression.h`
  Responsibility: public runtime API for registration score gain, undo score rollback, build payload queries, and slot activation requests.
- Create: `src/BuildProgression.cpp`
  Responsibility: orchestration for score changes, automatic unlock eligibility, slot activation/deactivation, and undo side effects.
- Create: `src/PrismaUIPayloadsBuild.cpp`
  Responsibility: JSON payload builders for build summary, build cards, slot panel, and grouped inventory summaries.
- Create: `src/PrismaUIRequestOpsBuild.cpp`
  Responsibility: build-specific request parsing and handling for activation, deactivation, batch registration, and combat-lock rejection paths.
- Modify: `include/CodexOfPowerNG/RegistrationUndoTypes.h`
  Responsibility: replace legacy reward-delta undo metadata with build-discipline contribution metadata that can round-trip through serialization.
- Modify: `include/CodexOfPowerNG/State.h`
  Responsibility: add persistent build-state fields without reusing the legacy reward-total map for new progression.
- Modify: `include/CodexOfPowerNG/SerializationStateStore.h`
  Responsibility: extend saved snapshot to hold build scores, active slots, and migration state.
- Modify: `src/SerializationStateStore.cpp`
  Responsibility: snapshot/replace/clear support for the new build fields.
- Modify: `src/SerializationLoad.cpp`
  Responsibility: read new build records, detect legacy reward data, stage deterministic migration, and keep retries idempotent.
- Modify: `src/SerializationSave.cpp`
  Responsibility: write new build records and stop treating legacy random rewards as the authoritative progression format.
- Modify: `src/main.cpp`
  Responsibility: switch post-load and preload runtime hooks from reward sync flows to build migration/runtime hooks.
- Modify: `include/CodexOfPowerNG/Rewards.h`
  Responsibility: narrow the legacy rewards surface to migration-cleanup helpers instead of gameplay reward grants.
- Modify: `src/Rewards.cpp`
  Responsibility: preserve one-time legacy reward cleanup behavior while removing random reward progression as authoritative gameplay logic.
- Modify: `src/RegistrationQuickRegister.cpp`
  Responsibility: add one-per-row discipline score gain and remove random reward grants.
- Modify: `src/RegistrationUndo.cpp`
  Responsibility: rollback score contribution, recompute eligibility, and deactivate invalid active slots on undo.
- Modify: `src/PrismaUIRequestOps.h`
  Responsibility: replace reward-centric request handlers with build-centric request handlers, plus batch register requests.
- Modify: `src/PrismaUIRequestOps.cpp`
  Responsibility: serve build payloads, handle slot activation/deactivation, handle batch register requests, and remove legacy reward refund flow.
- Modify: `src/PrismaUIRequests.cpp`
  Responsibility: register new JS listeners for build requests and batch registration.
- Modify: `src/PrismaUIPayloads.h`
  Responsibility: declare build payload helpers and inventory grouping payloads.
- Modify: `src/PrismaUIStatePayload.cpp`
  Responsibility: replace reward-count runtime summaries with build-aware runtime state and migration notice payloads.
- Modify: `include/CodexOfPowerNG/Config.h`
  Responsibility: remove or deprecate legacy reward cadence settings from the user-facing settings surface.
- Modify: `src/Config.cpp`
  Responsibility: keep backward-compatible parsing of legacy reward keys while no longer surfacing them as active gameplay settings.
- Modify: `src/PrismaUISettingsPayload.cpp`
  Responsibility: remove legacy reward-only settings from the settings payload contract.
- Modify: `docs/contracts/prismaui-js-api.md`
  Responsibility: document the new native/JS build contract and batch register flow.
- Modify: `PrismaUI/views/codexofpowerng/index.html`
  Responsibility: replace the Rewards tab with the Build tab and rework the Quick Register area into grouped batch-friendly sections.
- Create: `PrismaUI/views/codexofpowerng/ui_build_panel.js`
  Responsibility: render build cards, slot panel, migration notices, and build action affordances without growing `ui_rendering.js`.
- Create: `PrismaUI/views/codexofpowerng/ui_register_batch_panel.js`
  Responsibility: render grouped register sections, disabled-row reason tags, and batch summary helpers without growing `ui_rendering.js`.
- Modify: `PrismaUI/views/codexofpowerng/ui_state.js`
  Responsibility: store build payloads, grouped quick-list selection state, and batch register summary state.
- Modify: `PrismaUI/views/codexofpowerng/ui_rendering.js`
  Responsibility: coordinate shared rendering only, delegating build-card markup to `ui_build_panel.js` and grouped register markup to `ui_register_batch_panel.js`.
- Modify: `PrismaUI/views/codexofpowerng/ui_interactions.js`
  Responsibility: drive batch selection, build activation/deactivation, and register confirmation flows.
- Modify: `PrismaUI/views/codexofpowerng/ui_wiring.js`
  Responsibility: wire new buttons, tabs, and handlers for build and batch registration actions.
- Modify: `PrismaUI/views/codexofpowerng/native_state_bridge.js`
  Responsibility: accept new build payloads and grouped inventory payload shapes from native code.
- Modify: `PrismaUI/views/codexofpowerng/interop_bridge.js`
  Responsibility: install build-aware native callbacks such as `copng_setBuild` and normalize grouped inventory payloads.
- Modify: `PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js`
  Responsibility: keep fallback native callback wiring aligned with the new build payload contract.
- Modify: `PrismaUI/views/codexofpowerng/ui_bootstrap.js`
  Responsibility: swap reward-centric bootstrap wiring for build-centric rendering and interaction wiring.
- Modify: `PrismaUI/views/codexofpowerng/reward_orbit.js`
  Responsibility: remove or retire obsolete reward-orbit behavior now that the build screen is card/slot based.
- Modify: `PrismaUI/views/codexofpowerng/input_shortcuts.js`
  Responsibility: retarget shortcuts and tab navigation to the Build screen.
- Modify: `PrismaUI/views/codexofpowerng/virtual_tables.js`
  Responsibility: keep grouped register virtualization aligned with the new full inventory default scope.
- Modify: `PrismaUI/views/codexofpowerng/ui_i18n.js`
  Responsibility: add build-specific labels, slot text, migration notices, and batch-register strings.
- Test: `tests/build_option_catalog_contract.test.cpp`
  Responsibility: prove catalog IDs, threshold rules, slot compatibility, and baseline milestone structure.
- Test: `tests/build_state_store_ops.test.cpp`
  Responsibility: prove build score mutation, active-slot persistence, and migration-state helpers.
- Test: `tests/build_migration_rules.test.cpp`
  Responsibility: prove deterministic conversion from legacy registered items into discipline scores, fallback rules, and migration idempotency.
- Test: `tests/build_ui_state_module.test.cjs`
  Responsibility: prove JS-side build payload storage and batch-selection state.
- Test: `tests/build_ui_rendering_module.test.cjs`
  Responsibility: prove grouped register rendering, disabled-row labeling, and build-card slot rendering.
- Test: `tests/build_native_bridge_module.test.cjs`
  Responsibility: prove native build payload normalization and tab refresh handling.
- Test: `tests/build_batch_register_flow.test.cjs`
  Responsibility: prove select-then-confirm behavior and one-row-equals-one-registration summary math.

## Chunk 1: Data and Migration Contracts

### Task 1: Add Build Domain Types and Catalog

**Files:**
- Create: `include/CodexOfPowerNG/BuildTypes.h`
- Create: `include/CodexOfPowerNG/BuildOptionCatalog.h`
- Create: `src/BuildOptionCatalog.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/build_option_catalog_contract.test.cpp`

- [ ] **Step 1: Write the failing catalog contract test**

Create `tests/build_option_catalog_contract.test.cpp` with assertions for:

```cpp
EXPECT_TRUE(HasUniqueOptionIds());
EXPECT_TRUE(AllOptionsUseValidDiscipline());
EXPECT_TRUE(AllOptionsUseThresholdUnlocks());
EXPECT_TRUE(AllOptionsUseOnceOnlyStackRule());
EXPECT_TRUE(AllBaselineMilestonesUseDisciplinesOnly());
EXPECT_TRUE(HasExpectedInitialSlotLayout());
EXPECT_TRUE(AllOptionsDeclareRequiredPresentationFields());
EXPECT_TRUE(AllOptionsDeclareEffectPayloads());
```

- [ ] **Step 2: Run the test to verify it fails**

Run:

```bash
cmake --preset wsl-release -DCOPNG_BUILD_CPP_TEST_TARGETS=ON
cmake --build --preset wsl-release --target CodexOfPowerNG_build_option_catalog_contract_test
ctest --test-dir build/wsl-release --output-on-failure -R build_option_catalog_contract
```

Expected: test target fails to compile because build catalog/types do not exist yet.

- [ ] **Step 3: Add the build type and catalog files**

Implement the minimum contract:

```cpp
enum class BuildDiscipline : std::uint8_t { Attack, Defense, Utility };
enum class BuildSlotKind : std::uint8_t { Attack, Defense, Utility, Wildcard };
enum class BuildLayer : std::uint8_t { Baseline, Slotted };
enum class BuildSlotId : std::uint8_t {
  Attack1,
  Attack2,
  Defense1,
  Utility1,
  Utility2,
  Wildcard1,
};
enum class BuildSlotCompatibility : std::uint8_t { SameDisciplineOnly, WildcardOnly, SameOrWildcard };
enum class BuildEffectType : std::uint8_t { ActorValue, CarryWeight, Economy, UtilityFlag };
enum class BuildStackRule : std::uint8_t { OnceOnly };
using BuildMagnitude = std::variant<float, std::int32_t>;

struct BuildOptionDef {
  std::string_view id;
  BuildDiscipline discipline;
  BuildLayer layer;
  std::uint32_t unlockScore;
  BuildSlotCompatibility slotCompatibility;
  BuildEffectType effectType;
  BuildMagnitude magnitude;
  std::string_view exclusivityGroup;
  BuildStackRule stackRule;
  std::string_view titleKey;
  std::string_view descriptionKey;
};

struct BuildBaselineMilestoneDef {
  BuildDiscipline discipline;
  std::uint32_t threshold;
  BuildEffectType effectType;
  BuildMagnitude magnitude;
};
```

Seed an MVP catalog with a deliberately small option set:
- 3 slotted options per discipline
- 2 passive baseline milestones per discipline
- fixed slot layout: `Attack1`, `Attack2`, `Defense1`, `Utility1`, `Utility2`, `Wildcard1`

- [ ] **Step 4: Re-run the catalog contract test**

Run:

```bash
cmake --build --preset wsl-release --target CodexOfPowerNG_build_option_catalog_contract_test
ctest --test-dir build/wsl-release --output-on-failure -R build_option_catalog_contract
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt include/CodexOfPowerNG/BuildTypes.h include/CodexOfPowerNG/BuildOptionCatalog.h src/BuildOptionCatalog.cpp tests/build_option_catalog_contract.test.cpp
git commit -m "feat: add build option catalog contract"
```

### Task 2: Add Build State Store and Save Snapshot Fields

**Files:**
- Create: `include/CodexOfPowerNG/BuildStateStore.h`
- Create: `src/BuildStateStore.cpp`
- Modify: `include/CodexOfPowerNG/State.h`
- Modify: `include/CodexOfPowerNG/SerializationStateStore.h`
- Modify: `src/SerializationStateStore.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/build_state_store_ops.test.cpp`

- [ ] **Step 1: Write the failing build state store test**

Cover:

```cpp
EXPECT_EQ(GetAttackScore(), 0u);
EXPECT_TRUE(SetActiveSlot(slotId, optionId));
EXPECT_FALSE(SetActiveSlot(slotId, incompatibleOptionId));
EXPECT_EQ(MigrationState(), BuildMigrationState::kNotStarted);
EXPECT_TRUE(SnapshotRoundTripPreservesBuildState());
EXPECT_TRUE(MigrationNoticeFieldsRoundTrip());
```

- [ ] **Step 2: Run the test to verify it fails**

Run:

```bash
cmake --build --preset wsl-release --target CodexOfPowerNG_build_state_store_ops_test
ctest --test-dir build/wsl-release --output-on-failure -R build_state_store_ops
```

Expected: FAIL because build state store APIs and serialization fields do not exist yet.

- [ ] **Step 3: Implement build state storage**

Add dedicated fields for:
- `attackScore`
- `defenseScore`
- `utilityScore`
- fixed active slot map keyed by `BuildSlotId`
- migration version/state
- migration notice snapshot fields such as `needsNotice`, `legacyRewardsMigrated`, and `unresolvedHistoricalRegistrations`

Do not overload the legacy reward-total map for new build progression.

- [ ] **Step 4: Extend serialization snapshot helpers**

Persist and restore the build fields in `SerializationStateStore::Snapshot`.
Keep unlock availability derived from scores rather than persisted separately.
Also prove snapshot clear/replace behavior for migration notice fields.

- [ ] **Step 5: Re-run the state store test**

Run:

```bash
cmake --build --preset wsl-release --target CodexOfPowerNG_build_state_store_ops_test
ctest --test-dir build/wsl-release --output-on-failure -R build_state_store_ops
```

Expected: PASS.

- [ ] **Step 6: Commit**

```bash
git add include/CodexOfPowerNG/BuildStateStore.h src/BuildStateStore.cpp include/CodexOfPowerNG/State.h include/CodexOfPowerNG/SerializationStateStore.h src/SerializationStateStore.cpp CMakeLists.txt tests/build_state_store_ops.test.cpp
git commit -m "feat: add build state persistence"
```

### Task 3: Add Deterministic Migration and Legacy Cleanup Wiring

**Files:**
- Create: `include/CodexOfPowerNG/BuildProgression.h`
- Create: `src/BuildProgression.cpp`
- Modify: `src/SerializationLoad.cpp`
- Modify: `src/SerializationSave.cpp`
- Modify: `src/main.cpp`
- Modify: `include/CodexOfPowerNG/Rewards.h`
- Modify: `src/Rewards.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/build_migration_rules.test.cpp`

- [ ] **Step 1: Write the failing migration rules test**

Cover:

```cpp
EXPECT_EQ(ConvertLegacyGroupToDiscipline(0), BuildDiscipline::Attack);
EXPECT_EQ(ConvertLegacyGroupToDiscipline(1), BuildDiscipline::Defense);
EXPECT_EQ(ConvertLegacyGroupToDiscipline(5), BuildDiscipline::Utility);
EXPECT_EQ(ConvertMissingLegacyGroupResolvedAmmoToDiscipline(), BuildDiscipline::Attack);
EXPECT_EQ(ConvertMissingLegacyGroupResolvedAccessoryToDiscipline(), BuildDiscipline::Defense);
EXPECT_TRUE(MigrationRetryKeepsDeterministicScores());
EXPECT_TRUE(UnresolvedLegacyFormsAreCountedAndSkipped());
EXPECT_TRUE(MigrationStartsWithEmptySlots());
EXPECT_TRUE(MigrationStaysPendingUntilCleanupSucceeds());
EXPECT_TRUE(MigrationNoticeCapturesSkippedHistory());
EXPECT_TRUE(MigrationEmitsLegacyMigratedNoticeOnce());
EXPECT_TRUE(MigrationCompleteClearsLegacyUndoRewardDeltas());
EXPECT_TRUE(CompletedMigrationReloadSkipsRerun());
EXPECT_TRUE(CompletedReloadDoesNotRepeatMigrationNotice());
```

- [ ] **Step 2: Run the test to verify it fails**

Run:

```bash
cmake --build --preset wsl-release --target CodexOfPowerNG_build_migration_rules_test
ctest --test-dir build/wsl-release --output-on-failure -R build_migration_rules
```

Expected: FAIL because migration helpers do not exist.

- [ ] **Step 3: Implement deterministic score derivation and migration states**

Implement:
- legacy group -> discipline conversion
- fallback resolve-then-map behavior for old group-less saves
- explicit ammo/accessory fallback coverage for resolved legacy entries whose old group is missing or unknown
- unresolved entry counting
- migration states: `kNotStarted`, `kPendingCleanup`, `kComplete`
- one-time migration notice snapshot containing `legacyRewardsMigrated` and `unresolvedHistoricalRegistrations`
- legacy undo reward-delta disposal once migration reaches `kComplete`

- [ ] **Step 4: Replace the old reward runtime hook with build migration hook**

Update `main.cpp` to call build migration/runtime entry points on preload and post-load instead of kicking off the old reward sync workflow as authoritative gameplay progression.

- [ ] **Step 5: Lock legacy reward cleanup to one idempotent path**

Use stored legacy reward totals as the only cleanup source.
If player/AV access is unavailable, leave migration pending and retry.
Only mark migration complete after cleanup succeeds.
Keep active slots empty until migration reaches `kComplete`.
After `kComplete`, future loads must treat migration as a no-op and must not rehydrate legacy undo reward deltas.
Do not repeat the same migration notice on completed-save reloads.

- [ ] **Step 6: Re-run the migration test**

Run:

```bash
cmake --build --preset wsl-release --target CodexOfPowerNG_build_migration_rules_test
ctest --test-dir build/wsl-release --output-on-failure -R build_migration_rules
```

Expected: PASS.

- [ ] **Step 7: Commit**

```bash
git add include/CodexOfPowerNG/BuildProgression.h src/BuildProgression.cpp src/SerializationLoad.cpp src/SerializationSave.cpp src/main.cpp include/CodexOfPowerNG/Rewards.h src/Rewards.cpp CMakeLists.txt tests/build_migration_rules.test.cpp
git commit -m "feat: migrate legacy rewards into build scores"
```

## Chunk 2: Native Runtime and API Transition

### Task 4: Replace Random Reward Grants With Score Gain and Undo Rollback

**Files:**
- Modify: `src/RegistrationQuickRegister.cpp`
- Modify: `src/RegistrationUndo.cpp`
- Modify: `include/CodexOfPowerNG/Registration.h`
- Modify: `include/CodexOfPowerNG/RegistrationUndoTypes.h`
- Modify: `include/CodexOfPowerNG/BuildProgression.h`
- Modify: `src/BuildProgression.cpp`
- Modify: `src/SerializationLoad.cpp`
- Modify: `src/SerializationSave.cpp`
- Test: `tests/registration_build_progression.test.cpp`

- [ ] **Step 1: Write the failing registration/build integration test**

Cover:

```cpp
EXPECT_TRUE(RegisterGrantsOneScoreToMappedDiscipline());
EXPECT_TRUE(UndoRemovesThatScoreContribution());
EXPECT_TRUE(UndoDeactivatesNowIneligibleOption());
EXPECT_TRUE(UndoRecordRoundTripsBuildContributionMetadata());
EXPECT_TRUE(BaselineMilestonesTrackThresholdCrossing());
```

- [ ] **Step 2: Run the test to verify it fails**

Run:

```bash
cmake --build --preset wsl-release --target CodexOfPowerNG_registration_build_progression_test
ctest --test-dir build/wsl-release --output-on-failure -R registration_build_progression
```

Expected: FAIL because registration still grants random rewards.

- [ ] **Step 3: Replace reward grant logic with score contribution**

Each successful registration should contribute exactly one score unit for the mapped discipline.
Keep the one-row-equals-one-registration rule.
Recompute baseline passive milestones whenever a discipline score crosses a threshold.

- [ ] **Step 4: Rewrite undo to rollback score contribution**

Undo must:
- restore the item
- remove the original score contribution
- recalculate unlock eligibility for the affected discipline
- drop invalid active slots automatically

- [ ] **Step 5: Update undo serialization metadata**

Replace legacy `rewardDeltas` persistence with build contribution metadata that records:
- contributing discipline
- score delta
- discard-only legacy load fallback for pre-rework reward-delta undo entries once migration is complete

Do not keep writing new undo records in the old reward-delta format.

- [ ] **Step 6: Re-run the integration test**

Run:

```bash
cmake --build --preset wsl-release --target CodexOfPowerNG_registration_build_progression_test
ctest --test-dir build/wsl-release --output-on-failure -R registration_build_progression
```

Expected: PASS.

- [ ] **Step 7: Commit**

```bash
git add src/RegistrationQuickRegister.cpp src/RegistrationUndo.cpp include/CodexOfPowerNG/Registration.h include/CodexOfPowerNG/RegistrationUndoTypes.h include/CodexOfPowerNG/BuildProgression.h src/BuildProgression.cpp src/SerializationLoad.cpp src/SerializationSave.cpp tests/registration_build_progression.test.cpp
git commit -m "feat: route registration into build scores"
```

### Task 5: Replace Reward Payloads and Requests With Build Payloads

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `src/PrismaUIRequestOps.h`
- Modify: `src/PrismaUIRequestOps.cpp`
- Create: `src/PrismaUIRequestOpsBuild.cpp`
- Modify: `src/PrismaUIRequests.cpp`
- Modify: `src/PrismaUIPayloads.h`
- Modify: `src/PrismaUIPayloadsInventory.cpp`
- Modify: `src/PrismaUIStatePayload.cpp`
- Create: `src/PrismaUIPayloadsBuild.cpp`
- Modify: `src/RegistrationQuickListBuilder.h`
- Modify: `src/RegistrationQuickListBuilder.cpp`
- Modify: `docs/contracts/prismaui-js-api.md`
- Modify: `PrismaUI/views/codexofpowerng/native_state_bridge.js`
- Modify: `PrismaUI/views/codexofpowerng/interop_bridge.js`
- Modify: `PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js`
- Test: `tests/build_native_bridge_module.test.cjs`
- Test: `tests/build_request_guards.test.cpp`

- [ ] **Step 1: Write the failing build bridge/module and guard tests**

Cover:

```js
assert.equal(typeof handlers.onBuild, "function");
assert.equal(typeof win.copng_setBuild, "function");
assert.equal(typeof win.copng_requestBuild, "function");
assert.equal(typeof win.copng_requestRegisterBatch, "function");
assert.equal(typeof win.copng_deactivateBuildOption, "function");
assert.equal(typeof win.copng_swapBuildOption, "function");
assert.equal(normalizedBuild.activeSlots.length, 6);
assert.equal(normalizedQuickList.sections[0].rows[1].disabledReason, "quest_protected");
assert.equal(normalizedQuickList.sections[0].rows[2].disabledReason, "favorite_protected");
assert.deepEqual(batchRequest.formIds, [46775, 51234, 61234]);
```

```cpp
EXPECT_TRUE(RejectsBuildActivationWhileInCombat());
EXPECT_TRUE(RejectsInvalidBuildPayload());
EXPECT_TRUE(BatchRequestKeepsOneRowEqualsOneRegistration());
```

- [ ] **Step 2: Run the tests to verify they fail**

Run:

```bash
cmake --build --preset wsl-release --target CodexOfPowerNG_build_request_guards_test
ctest --test-dir build/wsl-release --output-on-failure -R build_request_guards
node --test tests/build_native_bridge_module.test.cjs tests/native_state_bridge_module.test.cjs tests/interop_bridge_module.test.cjs
```

Expected: FAIL because build bridge handlers, request guards, and grouped payload normalization do not exist yet.

- [ ] **Step 3: Add native build payload builders and request handlers**

Expose:
- build summary payload
- build option list payload
- active slot payload
- grouped quick-list payload with disabled rows, quest/favorite protection reason tags, and non-actionable state
- runtime state payload with build counts and migration notice summary
- batch register request handler
- activate / deactivate / swap build option handlers
- out-of-combat guard for activate / deactivate / swap requests

Use `src/PrismaUIRequestOpsBuild.cpp` for the new build-specific request code so `src/PrismaUIRequestOps.cpp` stays a coordinator instead of absorbing another full feature surface.

- [ ] **Step 4: Freeze the legacy refund path until the Build tab replaces the Rewards tab**

Do not add any new build behavior on top of `copng_refundRewards`.
Keep the old route only as temporary compatibility until Task 6 removes the Rewards tab and its UI wiring in the same chunk.

- [ ] **Step 5: Update the JS API contract document**

Document the new bindings, for example:

```json
window.copng_requestBuild({})
window.copng_activateBuildOption({ "optionId": "atk_sharpened_record", "slotId": "attack_1" })
window.copng_deactivateBuildOption({ "slotId": "attack_1" })
window.copng_swapBuildOption({ "fromSlotId": "attack_1", "toSlotId": "wildcard_1", "optionId": "atk_sharpened_record" })
window.copng_requestRegisterBatch({ "formIds": [46775, 51234] })
```

- [ ] **Step 6: Re-run the tests**

Run:

```bash
cmake --build --preset wsl-release --target CodexOfPowerNG_build_request_guards_test
ctest --test-dir build/wsl-release --output-on-failure -R build_request_guards
node --test tests/build_native_bridge_module.test.cjs tests/native_state_bridge_module.test.cjs tests/interop_bridge_module.test.cjs
```

Expected: PASS.

- [ ] **Step 7: Commit**

```bash
git add CMakeLists.txt src/PrismaUIRequestOps.h src/PrismaUIRequestOps.cpp src/PrismaUIRequestOpsBuild.cpp src/PrismaUIRequests.cpp src/PrismaUIPayloads.h src/PrismaUIPayloadsInventory.cpp src/PrismaUIStatePayload.cpp src/PrismaUIPayloadsBuild.cpp src/RegistrationQuickListBuilder.h src/RegistrationQuickListBuilder.cpp docs/contracts/prismaui-js-api.md PrismaUI/views/codexofpowerng/native_state_bridge.js PrismaUI/views/codexofpowerng/interop_bridge.js PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js tests/build_native_bridge_module.test.cjs tests/build_request_guards.test.cpp
git commit -m "feat: add native build API contract"
```

## Chunk 3: UI Rework

### Task 6: Rebuild the Rewards Tab Into the Build Tab

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`
- Create: `PrismaUI/views/codexofpowerng/ui_build_panel.js`
- Modify: `PrismaUI/views/codexofpowerng/ui_state.js`
- Modify: `PrismaUI/views/codexofpowerng/ui_rendering.js`
- Modify: `PrismaUI/views/codexofpowerng/ui_wiring.js`
- Modify: `PrismaUI/views/codexofpowerng/ui_interactions.js`
- Modify: `PrismaUI/views/codexofpowerng/ui_bootstrap.js`
- Modify: `PrismaUI/views/codexofpowerng/native_state_bridge.js`
- Modify: `PrismaUI/views/codexofpowerng/interop_bridge.js`
- Modify: `PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js`
- Modify: `PrismaUI/views/codexofpowerng/reward_orbit.js`
- Modify: `PrismaUI/views/codexofpowerng/input_shortcuts.js`
- Modify: `PrismaUI/views/codexofpowerng/ui_i18n.js`
- Test: `tests/build_ui_state_module.test.cjs`
- Test: `tests/build_ui_rendering_module.test.cjs`
- Test: `tests/build_native_bridge_module.test.cjs`
- Test: `tests/interop_bridge_module.test.cjs`
- Test: `tests/native_state_bridge_module.test.cjs`
- Test: `tests/ui_wiring_module.test.cjs`
- Test: `tests/input_shortcuts_module.test.cjs`
- Test: `tests/reward_orbit_layout.test.cjs`

- [ ] **Step 1: Write the failing JS state/rendering tests**

Cover:

```js
assert.equal(state.getBuild().disciplines.attack.score, 12);
assert.match(renderedHtml, /Active Slots/);
assert.match(renderedHtml, /Locked|Unlocked|Active/);
assert.match(renderedHtml, /Requires|Need .* Score/);
assert.match(renderedHtml, /Legacy rewards migrated|historical registrations could not be converted/);
assert.match(renderedHtml, /Activate/);
assert.match(renderedHtml, /Deactivate|Swap/);
assert.match(renderedHtml, /Combat lock|Unavailable in combat/);
assert.match(renderedHtml, /Attack|Defense|Utility/);
```

- [ ] **Step 2: Run the tests to verify they fail**

Run:

```bash
node --test tests/build_ui_state_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/build_native_bridge_module.test.cjs tests/interop_bridge_module.test.cjs tests/native_state_bridge_module.test.cjs tests/ui_wiring_module.test.cjs tests/input_shortcuts_module.test.cjs tests/reward_orbit_layout.test.cjs
```

Expected: FAIL because the UI still stores and renders `Rewards`.

- [ ] **Step 3: Extract the build-tab renderer and replace the rewards tab markup/state**

Create `ui_build_panel.js` and keep `ui_rendering.js` as a coordinator.
Update the UI so the old Rewards tab becomes the Build tab with:
- discipline tab strip
- build cards
- active slot panel
- current build summary
- locked cards that show their unlock requirement
- one-time migration notice banner for legacy reward conversion and unresolved historical registrations
- no reward-orbit-only markup or image wiring left in the active path

- [ ] **Step 4: Wire activate / deactivate / swap flows and combat lock**

Add the full build interaction loop:
- unlocked option -> activate
- active option -> deactivate
- active option -> swap into another compatible slot
- combat state -> disable and explain build changes
- activate payload -> `{ optionId, slotId }`
- deactivate payload -> `{ slotId }`
- swap payload -> `{ fromSlotId, toSlotId, optionId }`
- combat-locked action -> no native request is fired

Also switch bridge callbacks and tab navigation from `Rewards` to `Build` without mixed `tabRewards` / `tabBuild` naming in the same codepath.

- [ ] **Step 5: Re-run the JS tests**

Run:

```bash
node --test tests/build_ui_state_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/build_native_bridge_module.test.cjs tests/interop_bridge_module.test.cjs tests/native_state_bridge_module.test.cjs tests/ui_wiring_module.test.cjs tests/input_shortcuts_module.test.cjs tests/reward_orbit_layout.test.cjs
```

Expected: PASS.

- [ ] **Step 6: Commit**

```bash
git add PrismaUI/views/codexofpowerng/index.html PrismaUI/views/codexofpowerng/ui_build_panel.js PrismaUI/views/codexofpowerng/ui_state.js PrismaUI/views/codexofpowerng/ui_rendering.js PrismaUI/views/codexofpowerng/ui_wiring.js PrismaUI/views/codexofpowerng/ui_interactions.js PrismaUI/views/codexofpowerng/ui_bootstrap.js PrismaUI/views/codexofpowerng/native_state_bridge.js PrismaUI/views/codexofpowerng/interop_bridge.js PrismaUI/views/codexofpowerng/native_bridge_bootstrap.js PrismaUI/views/codexofpowerng/reward_orbit.js PrismaUI/views/codexofpowerng/input_shortcuts.js PrismaUI/views/codexofpowerng/ui_i18n.js tests/build_ui_state_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/build_native_bridge_module.test.cjs tests/interop_bridge_module.test.cjs tests/native_state_bridge_module.test.cjs tests/ui_wiring_module.test.cjs tests/input_shortcuts_module.test.cjs tests/reward_orbit_layout.test.cjs
git commit -m "feat: rebuild rewards tab into build screen"
```

### Task 7: Rework Quick Register Into a Grouped Batch-Register List

**Files:**
- Modify: `PrismaUI/views/codexofpowerng/index.html`
- Create: `PrismaUI/views/codexofpowerng/ui_register_batch_panel.js`
- Modify: `PrismaUI/views/codexofpowerng/ui_state.js`
- Modify: `PrismaUI/views/codexofpowerng/ui_rendering.js`
- Modify: `PrismaUI/views/codexofpowerng/ui_wiring.js`
- Modify: `PrismaUI/views/codexofpowerng/ui_interactions.js`
- Modify: `PrismaUI/views/codexofpowerng/native_state_bridge.js`
- Modify: `PrismaUI/views/codexofpowerng/virtual_tables.js`
- Modify: `PrismaUI/views/codexofpowerng/ui_i18n.js`
- Test: `tests/build_batch_register_flow.test.cjs`
- Test: `tests/ui_interactions_module.test.cjs`

- [ ] **Step 1: Write the failing batch-register flow test**

Cover:

```js
assert.equal(summary.selectedRows, 3);
assert.deepEqual(summary.disciplineGain, { attack: 2, defense: 0, utility: 1 });
assert.equal(batchRequest.formIds.length, 3);
assert.equal(disabledRows[0].reasonTag, "quest_protected");
assert.equal(disabledRows[1].reasonTag, "favorite_protected");
assert.equal(listScope, "all_register_relevant");
assert.equal(rows[0].totalCount, 3);
assert.equal(rows[0].safeCount, 2);
assert.ok(Array.isArray(rows[0].stateTags));
assert.equal(rows[0].canBatchSelect, true);
assert.equal(rows[1].singleRegisterAction, "disabled");
```

- [ ] **Step 2: Run the test to verify it fails**

Run:

```bash
node --test tests/build_batch_register_flow.test.cjs tests/ui_interactions_module.test.cjs
```

Expected: FAIL because the register screen has no grouped sections or batch-confirm flow.

- [ ] **Step 3: Extract grouped register rendering and disabled-row states**

Create `ui_register_batch_panel.js` and keep `ui_rendering.js` as a coordinator.
Render one vertical list grouped by Attack / Defense / Utility.
Default the list scope to the full register-relevant inventory rather than safe-only/actionable-only rows.
Each row must surface total count, safe count, state tags, single-item action state, and batch-select eligibility.
Visible disabled rows must show why they are not actionable.
Keep both quest-item protection and favorite protection visible and non-actionable.
Add an `actionable only` filter rather than a hard safe-only list.

- [ ] **Step 4: Add checkbox selection and explicit batch confirmation**

Each checked row represents exactly one registration action.
The summary panel must not multiply by `safeCount`.

- [ ] **Step 5: Re-run the batch-flow test**

Run:

```bash
node --test tests/build_batch_register_flow.test.cjs tests/ui_interactions_module.test.cjs
```

Expected: PASS.

- [ ] **Step 6: Commit**

```bash
git add PrismaUI/views/codexofpowerng/index.html PrismaUI/views/codexofpowerng/ui_register_batch_panel.js PrismaUI/views/codexofpowerng/ui_state.js PrismaUI/views/codexofpowerng/ui_rendering.js PrismaUI/views/codexofpowerng/ui_wiring.js PrismaUI/views/codexofpowerng/ui_interactions.js PrismaUI/views/codexofpowerng/native_state_bridge.js PrismaUI/views/codexofpowerng/virtual_tables.js PrismaUI/views/codexofpowerng/ui_i18n.js tests/build_batch_register_flow.test.cjs tests/ui_interactions_module.test.cjs
git commit -m "feat: add grouped batch register ui"
```

### Task 8: Remove Legacy Reward Settings and Finish Regression Coverage

**Files:**
- Modify: `include/CodexOfPowerNG/Config.h`
- Modify: `src/Config.cpp`
- Modify: `src/PrismaUISettingsPayload.cpp`
- Modify: `PrismaUI/views/codexofpowerng/index.html`
- Modify: `PrismaUI/views/codexofpowerng/ui_i18n.js`
- Modify: `PrismaUI/views/codexofpowerng/reward_orbit.js`
- Modify: `README.md`
- Test: `tests/config_settings_layering.test.cjs`
- Test: `tests/build_native_bridge_module.test.cjs`
- Test: `tests/build_ui_state_module.test.cjs`
- Test: `tests/build_ui_rendering_module.test.cjs`
- Test: `tests/build_batch_register_flow.test.cjs`
- Test: `tests/interop_bridge_module.test.cjs`
- Test: `tests/native_state_bridge_module.test.cjs`
- Test: `tests/ui_wiring_module.test.cjs`
- Test: `tests/input_shortcuts_module.test.cjs`
- Test: `tests/ui_interactions_module.test.cjs`
- Test: `tests/reward_orbit_layout.test.cjs`

- [ ] **Step 1: Write or update failing tests for legacy reward-setting removal**

Cover:

```js
assert.ok(!settingsPayload.enableRewards);
assert.ok(!settingsPayload.rewardEvery);
assert.ok(!settingsPayload.allowSkillRewards);
```

- [ ] **Step 2: Run the tests to verify they fail**

Run:

```bash
node --test tests/config_settings_layering.test.cjs tests/build_native_bridge_module.test.cjs tests/build_ui_state_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/build_batch_register_flow.test.cjs tests/interop_bridge_module.test.cjs tests/native_state_bridge_module.test.cjs tests/ui_wiring_module.test.cjs tests/input_shortcuts_module.test.cjs tests/ui_interactions_module.test.cjs tests/reward_orbit_layout.test.cjs
```

Expected: FAIL because the old reward settings and labels are still present.

- [ ] **Step 3: Remove the user-facing legacy reward settings surface**

Keep backward-compatible parsing of old keys in `Config.cpp`, but do not continue exposing cadence/random-reward settings in Settings UI or payloads.
Convert `reward_orbit.js` into an inert compatibility shim and update `reward_orbit_layout.test.cjs` so the active UI bundle explicitly no longer depends on the removed Rewards screen.

- [ ] **Step 4: Run the Node regression set**

Run:

```bash
node --test tests/config_settings_layering.test.cjs tests/build_native_bridge_module.test.cjs tests/build_ui_state_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/build_batch_register_flow.test.cjs tests/interop_bridge_module.test.cjs tests/native_state_bridge_module.test.cjs tests/ui_wiring_module.test.cjs tests/input_shortcuts_module.test.cjs tests/ui_interactions_module.test.cjs tests/reward_orbit_layout.test.cjs
```

Expected: PASS.

- [ ] **Step 5: Run the broader regression pass**

Run:

```bash
node --test tests/*.test.cjs
scripts/test.sh
```

Expected: PASS for both commands with zero unexpected failures.

- [ ] **Step 6: Commit**

```bash
git add include/CodexOfPowerNG/Config.h src/Config.cpp src/PrismaUISettingsPayload.cpp PrismaUI/views/codexofpowerng/index.html PrismaUI/views/codexofpowerng/ui_i18n.js PrismaUI/views/codexofpowerng/reward_orbit.js README.md tests/config_settings_layering.test.cjs tests/build_native_bridge_module.test.cjs tests/build_ui_state_module.test.cjs tests/build_ui_rendering_module.test.cjs tests/build_batch_register_flow.test.cjs tests/interop_bridge_module.test.cjs tests/native_state_bridge_module.test.cjs tests/ui_wiring_module.test.cjs tests/input_shortcuts_module.test.cjs tests/ui_interactions_module.test.cjs tests/reward_orbit_layout.test.cjs
git commit -m "feat: finish record-build rework surface"
```
