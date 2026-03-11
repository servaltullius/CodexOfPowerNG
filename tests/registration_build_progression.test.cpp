#include "CodexOfPowerNG/BuildProgression.h"
#include "CodexOfPowerNG/BuildStateStore.h"
#include "CodexOfPowerNG/RegistrationStateStore.h"
#include "CodexOfPowerNG/SerializationStateStore.h"

#include <iostream>
#include <string_view>

namespace
{
	using CodexOfPowerNG::BuildProgression::ApplyRegistrationContribution;
	using CodexOfPowerNG::BuildProgression::BaselineMilestonesChanged;
	using CodexOfPowerNG::BuildProgression::CountUnlockedBaselineMilestones;
	using CodexOfPowerNG::BuildProgression::MakeRegistrationContribution;
	using CodexOfPowerNG::BuildProgression::RollbackContributionResult;
	using CodexOfPowerNG::BuildProgression::RollbackRegistrationContributionDetailed;
	using CodexOfPowerNG::BuildProgression::RollbackRegistrationContribution;
	using CodexOfPowerNG::BuildStateStore::GetActiveSlot;
	using CodexOfPowerNG::BuildStateStore::GetAttackScore;
	using CodexOfPowerNG::BuildStateStore::SetActiveSlot;
	using CodexOfPowerNG::Builds::BuildDiscipline;
	using CodexOfPowerNG::Builds::BuildSlotId;
	using CodexOfPowerNG::Registration::BuildScoreContribution;
	using CodexOfPowerNG::Registration::UndoRecord;

	bool RegisterGrantsOneScoreToMappedDiscipline()
	{
		CodexOfPowerNG::SerializationStateStore::Clear();
		const auto contribution = MakeRegistrationContribution(0u);
		if (!contribution.has_value()) {
			return false;
		}
		if (contribution->discipline != BuildDiscipline::Attack || contribution->scoreDelta != 1u) {
			return false;
		}

		if (!ApplyRegistrationContribution(contribution.value())) {
			return false;
		}
		return GetAttackScore() == 1u;
	}

	bool UndoRemovesThatScoreContribution()
	{
		CodexOfPowerNG::SerializationStateStore::Clear();
		const auto contribution = MakeRegistrationContribution(0u);
		if (!contribution.has_value()) {
			return false;
		}

		(void)ApplyRegistrationContribution(contribution.value());
		(void)RollbackRegistrationContribution(contribution.value());
		return GetAttackScore() == 0u;
	}

	bool UndoDeactivatesNowIneligibleOption()
	{
		CodexOfPowerNG::SerializationStateStore::Clear();
		const auto contribution = MakeRegistrationContribution(0u);
		if (!contribution.has_value()) {
			return false;
		}

		for (std::uint32_t i = 0; i < 5u; ++i) {
			if (!ApplyRegistrationContribution(contribution.value())) {
				return false;
			}
		}
		if (!SetActiveSlot(BuildSlotId::Attack1, "build.attack.ferocity")) {
			return false;
		}

		const auto deactivatedSlots = RollbackRegistrationContribution(contribution.value());
		return GetAttackScore() == 4u &&
		       deactivatedSlots == 1u &&
		       !GetActiveSlot(BuildSlotId::Attack1).has_value();
	}

	bool DetailedRollbackReportsScoreChangesAndClearedSlots()
	{
		CodexOfPowerNG::SerializationStateStore::Clear();
		const auto contribution = MakeRegistrationContribution(0u);
		if (!contribution.has_value()) {
			return false;
		}

		if (!ApplyRegistrationContribution(contribution.value())) {
			return false;
		}

		const auto result = RollbackRegistrationContributionDetailed(contribution.value());
		return result.scoreChanged &&
		       result.deactivatedSlots == 0u &&
		       GetAttackScore() == 0u;
	}

	bool DetailedRollbackClearsInvalidSlotsEvenWhenScoreIsAlreadyZero()
	{
		CodexOfPowerNG::SerializationStateStore::Clear();
		auto snapshot = CodexOfPowerNG::SerializationStateStore::SnapshotState();
		snapshot.attackScore = 0u;
		snapshot.activeBuildSlots[static_cast<std::size_t>(BuildSlotId::Attack1)] = "build.attack.ferocity";
		CodexOfPowerNG::SerializationStateStore::ReplaceState(std::move(snapshot));

		const auto contribution = MakeRegistrationContribution(0u);
		if (!contribution.has_value()) {
			return false;
		}

		const auto result = RollbackRegistrationContributionDetailed(contribution.value());
		return !result.scoreChanged &&
		       result.deactivatedSlots == 1u &&
		       !GetActiveSlot(BuildSlotId::Attack1).has_value();
	}

	bool UndoRecordRoundTripsBuildContributionMetadata()
	{
		CodexOfPowerNG::SerializationStateStore::Clear();

		UndoRecord record{};
		record.formId = 0x1234u;
		record.regKey = 0x5678u;
		record.group = 1u;
		record.buildContribution = BuildScoreContribution{
			.discipline = BuildDiscipline::Defense,
			.scoreDelta = 1u,
		};

		(void)CodexOfPowerNG::RegistrationStateStore::PushUndoRecord(record);
		auto snapshot = CodexOfPowerNG::SerializationStateStore::SnapshotState();
		CodexOfPowerNG::SerializationStateStore::Clear();
		CodexOfPowerNG::SerializationStateStore::ReplaceState(std::move(snapshot));

		const auto restored = CodexOfPowerNG::RegistrationStateStore::SnapshotUndoRecords(1u);
		return restored.size() == 1u &&
		       restored.front().buildContribution.has_value() &&
		       restored.front().buildContribution->discipline == BuildDiscipline::Defense &&
		       restored.front().buildContribution->scoreDelta == 1u;
	}

	bool BaselineMilestonesTrackThresholdCrossing()
	{
		return CountUnlockedBaselineMilestones(BuildDiscipline::Attack, 9u) == 0u &&
		       CountUnlockedBaselineMilestones(BuildDiscipline::Attack, 10u) == 1u &&
		       BaselineMilestonesChanged(BuildDiscipline::Attack, 9u, 10u) &&
		       !BaselineMilestonesChanged(BuildDiscipline::Attack, 10u, 11u);
	}
}

int main()
{
	const auto expect = [](bool condition, std::string_view message) {
		if (!condition) {
			std::cerr << "registration_build_progression: " << message << '\n';
			return false;
		}
		return true;
	};

	if (!expect(RegisterGrantsOneScoreToMappedDiscipline(), "register must grant one score to mapped discipline")) {
		return 1;
	}
	if (!expect(UndoRemovesThatScoreContribution(), "undo must remove the score contribution")) {
		return 1;
	}
	if (!expect(UndoDeactivatesNowIneligibleOption(), "undo must deactivate options that lose eligibility")) {
		return 1;
	}
	if (!expect(
			DetailedRollbackReportsScoreChangesAndClearedSlots(),
			"detailed rollback must report score changes separately from slot clears")) {
		return 1;
	}
	if (!expect(
			DetailedRollbackClearsInvalidSlotsEvenWhenScoreIsAlreadyZero(),
			"detailed rollback must clear invalid slots even when score is already zero")) {
		return 1;
	}
	if (!expect(UndoRecordRoundTripsBuildContributionMetadata(), "undo record must round-trip build contribution metadata")) {
		return 1;
	}
	if (!expect(BaselineMilestonesTrackThresholdCrossing(), "baseline milestones must track threshold crossing")) {
		return 1;
	}

	CodexOfPowerNG::SerializationStateStore::Clear();
	return 0;
}
