#include "CodexOfPowerNG/BuildStateStore.h"
#include "CodexOfPowerNG/SerializationStateStore.h"

#include <cstdint>
#include <iostream>
#include <string_view>

namespace
{
	using namespace CodexOfPowerNG::Builds;
	using namespace CodexOfPowerNG::BuildStateStore;

	bool SnapshotRoundTripPreservesBuildState()
	{
		CodexOfPowerNG::SerializationStateStore::Clear();

		SetAttackScore(15u);
		SetDefenseScore(30u);
		SetUtilityScore(30u);

		if (!SetActiveSlot(BuildSlotId::Attack1, "build.attack.precision")) {
			return false;
		}
		if (!SetActiveSlot(BuildSlotId::Defense1, "build.defense.stalwart")) {
			return false;
		}
		if (!SetActiveSlot(BuildSlotId::Wildcard1, "build.utility.scout")) {
			return false;
		}

		SetMigrationVersion(2u);
		SetMigrationState(BuildMigrationState::kPendingCleanup);
		SetMigrationNoticeSnapshot(BuildMigrationNoticeSnapshot{
			.needsNotice = true,
			.legacyRewardsMigrated = true,
			.unresolvedHistoricalRegistrations = 4u,
		});

		auto snapshot = CodexOfPowerNG::SerializationStateStore::SnapshotState();
		CodexOfPowerNG::SerializationStateStore::Clear();

		if (GetAttackScore() != 0u || GetDefenseScore() != 0u || GetUtilityScore() != 0u) {
			return false;
		}
		if (GetActiveSlot(BuildSlotId::Attack1).has_value()) {
			return false;
		}
		if (MigrationVersion() != 0u || MigrationState() != BuildMigrationState::kNotStarted) {
			return false;
		}

		const auto clearedNotice = GetMigrationNoticeSnapshot();
		if (clearedNotice.needsNotice || clearedNotice.legacyRewardsMigrated ||
		    clearedNotice.unresolvedHistoricalRegistrations != 0u) {
			return false;
		}

		CodexOfPowerNG::SerializationStateStore::ReplaceState(std::move(snapshot));

		const auto attackSlot = GetActiveSlot(BuildSlotId::Attack1);
		const auto defenseSlot = GetActiveSlot(BuildSlotId::Defense1);
		const auto wildcardSlot = GetActiveSlot(BuildSlotId::Wildcard1);
		const auto restoredNotice = GetMigrationNoticeSnapshot();
		return GetAttackScore() == 15u &&
		       GetDefenseScore() == 30u &&
		       GetUtilityScore() == 30u &&
		       attackSlot.has_value() &&
		       attackSlot.value() == "build.attack.precision" &&
		       defenseSlot.has_value() &&
		       defenseSlot.value() == "build.defense.stalwart" &&
		       wildcardSlot.has_value() &&
		       wildcardSlot.value() == "build.utility.scout" &&
		       MigrationVersion() == 2u &&
		       MigrationState() == BuildMigrationState::kPendingCleanup &&
		       restoredNotice.needsNotice &&
		       restoredNotice.legacyRewardsMigrated &&
		       restoredNotice.unresolvedHistoricalRegistrations == 4u;
	}

	bool MigrationNoticeFieldsRoundTrip()
	{
		CodexOfPowerNG::SerializationStateStore::Clear();

		SetMigrationNoticeSnapshot(BuildMigrationNoticeSnapshot{
			.needsNotice = true,
			.legacyRewardsMigrated = true,
			.unresolvedHistoricalRegistrations = 2u,
		});

		auto snapshot = CodexOfPowerNG::SerializationStateStore::SnapshotState();
		CodexOfPowerNG::SerializationStateStore::ReplaceState(CodexOfPowerNG::SerializationStateStore::Snapshot{});

		const auto clearedNotice = GetMigrationNoticeSnapshot();
		if (clearedNotice.needsNotice || clearedNotice.legacyRewardsMigrated ||
		    clearedNotice.unresolvedHistoricalRegistrations != 0u) {
			return false;
		}

		CodexOfPowerNG::SerializationStateStore::ReplaceState(std::move(snapshot));

		const auto restoredNotice = GetMigrationNoticeSnapshot();
		return restoredNotice.needsNotice &&
		       restoredNotice.legacyRewardsMigrated &&
		       restoredNotice.unresolvedHistoricalRegistrations == 2u;
	}
}

int main()
{
	const auto expect = [](bool condition, const char* message) {
		if (!condition) {
			std::cerr << "build_state_store_ops: " << message << '\n';
			return false;
		}
		return true;
	};

	CodexOfPowerNG::SerializationStateStore::Clear();

	if (!expect(GetAttackScore() == 0u, "attack score must default to zero")) {
		return 1;
	}

	SetAttackScore(15u);
	SetDefenseScore(5u);

	constexpr BuildSlotId      slotId = BuildSlotId::Attack1;
	constexpr std::string_view optionId = "build.attack.ferocity";
	constexpr std::string_view incompatibleOptionId = "build.defense.guard";

	if (!expect(SetActiveSlot(slotId, optionId), "compatible unlocked option must activate")) {
		return 1;
	}
	if (!expect(!SetActiveSlot(slotId, incompatibleOptionId), "incompatible option must be rejected")) {
		return 1;
	}
	if (!expect(MigrationState() == BuildMigrationState::kNotStarted, "migration state must default to not started")) {
		return 1;
	}
	if (!expect(SnapshotRoundTripPreservesBuildState(), "snapshot round-trip must preserve build state")) {
		return 1;
	}
	if (!expect(MigrationNoticeFieldsRoundTrip(), "migration notice fields must clear and restore via snapshot")) {
		return 1;
	}

	CodexOfPowerNG::SerializationStateStore::Clear();
	return 0;
}
