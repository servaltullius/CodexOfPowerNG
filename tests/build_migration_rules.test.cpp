#include "CodexOfPowerNG/BuildProgression.h"
#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/SerializationStateStore.h"

#include <RE/Skyrim.h>

#include <iostream>
#include <optional>
#include <string_view>

namespace
{
	using CodexOfPowerNG::BuildProgression::ConsumeMigrationNotice;
	using CodexOfPowerNG::BuildProgression::ConvertLegacyGroupToDiscipline;
	using CodexOfPowerNG::BuildProgression::NormalizeLoadedSnapshot;
	using CodexOfPowerNG::BuildProgression::TryFinalizePendingMigration;
	using CodexOfPowerNG::BuildProgression::TryResolveLegacyDiscipline;
	using CodexOfPowerNG::BuildProgression::kBuildMigrationVersion;
	using CodexOfPowerNG::Builds::BuildDiscipline;
	using CodexOfPowerNG::Builds::BuildMigrationState;
	using Snapshot = CodexOfPowerNG::SerializationStateStore::Snapshot;

	std::optional<BuildDiscipline> ResolveLegacyDisciplineForTest(RE::FormID formId) noexcept
	{
		switch (formId) {
		case 0x1003u:
			return BuildDiscipline::Attack;  // ammo fallback
		case 0x1004u:
			return BuildDiscipline::Defense;  // accessory fallback
		case 0x1006u:
			return BuildDiscipline::Utility;
		default:
			return std::nullopt;
		}
	}

	bool CleanupAlwaysFails(const Snapshot& /*snapshot*/) noexcept
	{
		return false;
	}

	bool CleanupSucceedsWithLegacyRewards(const Snapshot& snapshot) noexcept
	{
		return !snapshot.rewardTotals.empty();
	}

	Snapshot BuildLegacySnapshot()
	{
		Snapshot snapshot{};
		snapshot.registeredItems.emplace(0x1001u, 0u);
		snapshot.registeredItems.emplace(0x1002u, 1u);
		snapshot.registeredItems.emplace(0x1003u, CodexOfPowerNG::kGroupUnknown);
		snapshot.registeredItems.emplace(0x1004u, CodexOfPowerNG::kGroupUnknown);
		snapshot.registeredItems.emplace(0x1005u, CodexOfPowerNG::kGroupUnknown);
		snapshot.registeredItems.emplace(0x1006u, 5u);
		snapshot.registeredItems.emplace(0x1007u, 2u);
		snapshot.rewardTotals.emplace(RE::ActorValue::kDamageResist, 25.0f);
		snapshot.activeBuildSlots[0] = "build.attack.ferocity";
		snapshot.activeBuildSlots[3] = "build.utility.scout";

		CodexOfPowerNG::Registration::UndoRecord legacyUndo{};
		legacyUndo.actionId = 1u;
		legacyUndo.rewardDeltas.push_back({
			RE::ActorValue::kDamageResist,
			5.0f,
		});
		snapshot.undoHistory.push_back(legacyUndo);
		return snapshot;
	}

	bool LegacyGroupsMapIntoDisciplines()
	{
		return ConvertLegacyGroupToDiscipline(0u) == BuildDiscipline::Attack &&
		       ConvertLegacyGroupToDiscipline(1u) == BuildDiscipline::Defense &&
		       ConvertLegacyGroupToDiscipline(5u) == BuildDiscipline::Utility;
	}

	bool FallbackResolutionCoversAmmoAndAccessories()
	{
		const auto ammoDiscipline =
			TryResolveLegacyDiscipline(0x1003u, CodexOfPowerNG::kGroupUnknown, ResolveLegacyDisciplineForTest);
		const auto accessoryDiscipline =
			TryResolveLegacyDiscipline(0x1004u, CodexOfPowerNG::kGroupUnknown, ResolveLegacyDisciplineForTest);
		return ammoDiscipline.has_value() &&
		       ammoDiscipline.value() == BuildDiscipline::Attack &&
		       accessoryDiscipline.has_value() &&
		       accessoryDiscipline.value() == BuildDiscipline::Defense;
	}

	bool MigrationDerivesDeterministicScoresAndStagesCleanup()
	{
		auto snapshot = BuildLegacySnapshot();
		NormalizeLoadedSnapshot(snapshot, ResolveLegacyDisciplineForTest);

		return snapshot.attackScore == 2u &&
		       snapshot.defenseScore == 2u &&
		       snapshot.utilityScore == 2u &&
		       snapshot.buildMigrationVersion == kBuildMigrationVersion &&
		       snapshot.buildMigrationState == BuildMigrationState::kPendingCleanup &&
		       snapshot.buildMigrationNotice.needsNotice &&
		       snapshot.buildMigrationNotice.legacyRewardsMigrated &&
		       snapshot.buildMigrationNotice.unresolvedHistoricalRegistrations == 1u &&
		       snapshot.activeBuildSlots[0].empty() &&
		       snapshot.activeBuildSlots[3].empty();
	}

	bool MigrationRetryKeepsDeterministicScores()
	{
		auto snapshot = BuildLegacySnapshot();
		NormalizeLoadedSnapshot(snapshot, ResolveLegacyDisciplineForTest);
		const auto attackScore = snapshot.attackScore;
		const auto defenseScore = snapshot.defenseScore;
		const auto utilityScore = snapshot.utilityScore;

		NormalizeLoadedSnapshot(snapshot, ResolveLegacyDisciplineForTest);

		return snapshot.attackScore == attackScore &&
		       snapshot.defenseScore == defenseScore &&
		       snapshot.utilityScore == utilityScore &&
		       snapshot.buildMigrationState == BuildMigrationState::kPendingCleanup;
	}

	bool UnresolvedLegacyFormsAreCountedAndSkipped()
	{
		auto snapshot = BuildLegacySnapshot();
		NormalizeLoadedSnapshot(snapshot, ResolveLegacyDisciplineForTest);

		return snapshot.buildMigrationNotice.unresolvedHistoricalRegistrations == 1u &&
		       snapshot.attackScore + snapshot.defenseScore + snapshot.utilityScore == 6u;
	}

	bool MigrationStaysPendingUntilCleanupSucceeds()
	{
		auto snapshot = BuildLegacySnapshot();
		NormalizeLoadedSnapshot(snapshot, ResolveLegacyDisciplineForTest);

		if (TryFinalizePendingMigration(snapshot, CleanupAlwaysFails)) {
			return false;
		}

		return snapshot.buildMigrationState == BuildMigrationState::kPendingCleanup &&
		       snapshot.buildMigrationVersion == kBuildMigrationVersion &&
		       !snapshot.rewardTotals.empty() &&
		       snapshot.activeBuildSlots[0].empty();
	}

	bool MigrationCompleteClearsLegacyRewardStateAndUndoDeltas()
	{
		auto snapshot = BuildLegacySnapshot();
		NormalizeLoadedSnapshot(snapshot, ResolveLegacyDisciplineForTest);

		if (!TryFinalizePendingMigration(snapshot, CleanupSucceedsWithLegacyRewards)) {
			return false;
		}

		return snapshot.buildMigrationState == BuildMigrationState::kComplete &&
		       snapshot.rewardTotals.empty() &&
		       snapshot.undoHistory.size() == 1u &&
		       snapshot.undoHistory.front().rewardDeltas.empty();
	}

	bool MigrationNoticeIsOneShot()
	{
		auto snapshot = BuildLegacySnapshot();
		NormalizeLoadedSnapshot(snapshot, ResolveLegacyDisciplineForTest);

		const auto firstNotice = ConsumeMigrationNotice(snapshot);
		const auto secondNotice = ConsumeMigrationNotice(snapshot);
		return firstNotice.has_value() &&
		       firstNotice->legacyRewardsMigrated &&
		       firstNotice->unresolvedHistoricalRegistrations == 1u &&
		       !secondNotice.has_value();
	}

	bool CompletedMigrationReloadSkipsRerunAndDoesNotRepeatNotice()
	{
		auto snapshot = BuildLegacySnapshot();
		NormalizeLoadedSnapshot(snapshot, ResolveLegacyDisciplineForTest);
		if (!TryFinalizePendingMigration(snapshot, CleanupSucceedsWithLegacyRewards)) {
			return false;
		}
		if (!ConsumeMigrationNotice(snapshot).has_value()) {
			return false;
		}

		snapshot.attackScore = 9u;
		snapshot.defenseScore = 4u;
		snapshot.utilityScore = 7u;
		snapshot.undoHistory.front().rewardDeltas.push_back({
			RE::ActorValue::kHealth,
			3.0f,
		});

		NormalizeLoadedSnapshot(snapshot, ResolveLegacyDisciplineForTest);

		return snapshot.attackScore == 9u &&
		       snapshot.defenseScore == 4u &&
		       snapshot.utilityScore == 7u &&
		       snapshot.buildMigrationState == BuildMigrationState::kComplete &&
		       snapshot.undoHistory.front().rewardDeltas.empty() &&
		       !ConsumeMigrationNotice(snapshot).has_value();
	}

	bool LegacyResistanceSlotIdsRemapIntoBundledOptions()
	{
		Snapshot snapshot{};
		snapshot.buildMigrationVersion = kBuildMigrationVersion;
		snapshot.buildMigrationState = BuildMigrationState::kComplete;
		snapshot.activeBuildSlots[static_cast<std::size_t>(CodexOfPowerNG::Builds::BuildSlotId::Defense1)] = "build.defense.fireward";
		snapshot.activeBuildSlots[static_cast<std::size_t>(CodexOfPowerNG::Builds::BuildSlotId::Wildcard1)] = "build.defense.antidote";

		NormalizeLoadedSnapshot(snapshot, ResolveLegacyDisciplineForTest);

		return snapshot.activeBuildSlots[static_cast<std::size_t>(CodexOfPowerNG::Builds::BuildSlotId::Defense1)] == "build.defense.elementalWard" &&
		       snapshot.activeBuildSlots[static_cast<std::size_t>(CodexOfPowerNG::Builds::BuildSlotId::Wildcard1)] == "build.defense.purification";
	}
}

int main()
{
	const auto expect = [](bool condition, std::string_view message) {
		if (!condition) {
			std::cerr << "build_migration_rules: " << message << '\n';
			return false;
		}
		return true;
	};

	if (!expect(LegacyGroupsMapIntoDisciplines(), "legacy groups must map into attack/defense/utility")) {
		return 1;
	}
	if (!expect(FallbackResolutionCoversAmmoAndAccessories(), "fallback resolution must cover ammo and accessories")) {
		return 1;
	}
	if (!expect(MigrationDerivesDeterministicScoresAndStagesCleanup(), "migration must derive deterministic scores and stage cleanup")) {
		return 1;
	}
	if (!expect(MigrationRetryKeepsDeterministicScores(), "migration retry must keep deterministic scores")) {
		return 1;
	}
	if (!expect(UnresolvedLegacyFormsAreCountedAndSkipped(), "unresolved legacy forms must be counted and skipped")) {
		return 1;
	}
	if (!expect(MigrationStaysPendingUntilCleanupSucceeds(), "migration must stay pending until cleanup succeeds")) {
		return 1;
	}
	if (!expect(MigrationCompleteClearsLegacyRewardStateAndUndoDeltas(), "completed migration must clear legacy reward state and undo deltas")) {
		return 1;
	}
	if (!expect(MigrationNoticeIsOneShot(), "migration notice must emit once")) {
		return 1;
	}
	if (!expect(CompletedMigrationReloadSkipsRerunAndDoesNotRepeatNotice(), "completed migration reload must skip rerun and notice repeat")) {
		return 1;
	}
	if (!expect(LegacyResistanceSlotIdsRemapIntoBundledOptions(), "legacy resistance slot ids must remap into bundled options")) {
		return 1;
	}

	return 0;
}
