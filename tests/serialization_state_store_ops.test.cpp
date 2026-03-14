#include "CodexOfPowerNG/BuildTypes.h"
#include "CodexOfPowerNG/SerializationStateStoreOps.h"

#include <cassert>
#include <array>
#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace
{
	struct FakeSnapshot
	{
		std::unordered_map<std::uint32_t, std::uint32_t> registeredItems;
		std::unordered_set<std::uint32_t>                blockedItems;
		std::unordered_set<std::uint32_t>                notifiedItems;
		std::unordered_map<int, float>                   rewardTotals;
		std::unordered_map<int, float>                   buildAppliedEffectTotals;
		std::uint32_t                                    attackScore{ 0 };
		std::uint32_t                                    defenseScore{ 0 };
		std::uint32_t                                    utilityScore{ 0 };
		CodexOfPowerNG::Builds::BuildPointCenti          attackBuildPointsCenti{ 0 };
		CodexOfPowerNG::Builds::BuildPointCenti          defenseBuildPointsCenti{ 0 };
		CodexOfPowerNG::Builds::BuildPointCenti          utilityBuildPointsCenti{ 0 };
		std::array<std::string, CodexOfPowerNG::Builds::kBuildSlotCount> activeBuildSlots{};
		std::uint32_t                                    buildMigrationVersion{ 0 };
		CodexOfPowerNG::Builds::BuildMigrationState      buildMigrationState{ CodexOfPowerNG::Builds::BuildMigrationState::kNotStarted };
		CodexOfPowerNG::Builds::BuildMigrationNoticeSnapshot buildMigrationNotice{};
		std::deque<int>                                  undoHistory;
		std::uint64_t                                    undoNextActionId{ 1 };
	};

	struct FakeState
	{
		std::unordered_map<std::uint32_t, std::uint32_t> registeredItems;
		std::unordered_set<std::uint32_t>                blockedItems;
		std::unordered_set<std::uint32_t>                notifiedItems;
		std::unordered_map<int, float>                   rewardTotals;
		std::unordered_map<int, float>                   buildAppliedEffectTotals;
		std::uint32_t                                    attackScore{ 0 };
		std::uint32_t                                    defenseScore{ 0 };
		std::uint32_t                                    utilityScore{ 0 };
		CodexOfPowerNG::Builds::BuildPointCenti          attackBuildPointsCenti{ 0 };
		CodexOfPowerNG::Builds::BuildPointCenti          defenseBuildPointsCenti{ 0 };
		CodexOfPowerNG::Builds::BuildPointCenti          utilityBuildPointsCenti{ 0 };
		std::array<std::string, CodexOfPowerNG::Builds::kBuildSlotCount> activeBuildSlots{};
		std::uint32_t                                    buildMigrationVersion{ 0 };
		CodexOfPowerNG::Builds::BuildMigrationState      buildMigrationState{ CodexOfPowerNG::Builds::BuildMigrationState::kNotStarted };
		CodexOfPowerNG::Builds::BuildMigrationNoticeSnapshot buildMigrationNotice{};
		std::deque<int>                                  undoHistory;
		std::uint64_t                                    undoNextActionId{ 1 };
	};
}

int main()
{
	namespace Ops = CodexOfPowerNG::SerializationStateStore::Ops;

	FakeState state{};
	state.registeredItems.emplace(0x01020304u, 7u);
	state.blockedItems.insert(0x11111111u);
	state.notifiedItems.insert(0x22222222u);
	state.rewardTotals.insert_or_assign(4, 12.5f);
	state.buildAppliedEffectTotals.insert_or_assign(8, 0.75f);
	state.attackScore = 3;
	state.defenseScore = 4;
	state.utilityScore = 5;
	state.attackBuildPointsCenti = 800;
	state.defenseBuildPointsCenti = 1200;
	state.utilityBuildPointsCenti = 2400;
	state.activeBuildSlots[0] = "build.attack.ferocity";
	state.buildMigrationVersion = 2;
	state.buildMigrationState = CodexOfPowerNG::Builds::BuildMigrationState::kPendingCleanup;
	state.buildMigrationNotice = { true, true, 7u };
	state.undoHistory.push_back(99);
	state.undoNextActionId = 42;

	const auto snapshot = Ops::SnapshotState<FakeSnapshot>(state);
	assert(snapshot.registeredItems == state.registeredItems);
	assert(snapshot.blockedItems == state.blockedItems);
	assert(snapshot.notifiedItems == state.notifiedItems);
	assert(snapshot.rewardTotals == state.rewardTotals);
	assert(snapshot.buildAppliedEffectTotals == state.buildAppliedEffectTotals);
	assert(snapshot.attackScore == state.attackScore);
	assert(snapshot.defenseScore == state.defenseScore);
	assert(snapshot.utilityScore == state.utilityScore);
	assert(snapshot.attackBuildPointsCenti == state.attackBuildPointsCenti);
	assert(snapshot.defenseBuildPointsCenti == state.defenseBuildPointsCenti);
	assert(snapshot.utilityBuildPointsCenti == state.utilityBuildPointsCenti);
	assert(snapshot.activeBuildSlots == state.activeBuildSlots);
	assert(snapshot.buildMigrationVersion == state.buildMigrationVersion);
	assert(snapshot.buildMigrationState == state.buildMigrationState);
	assert(snapshot.buildMigrationNotice.needsNotice == state.buildMigrationNotice.needsNotice);
	assert(snapshot.buildMigrationNotice.legacyRewardsMigrated == state.buildMigrationNotice.legacyRewardsMigrated);
	assert(snapshot.buildMigrationNotice.unresolvedHistoricalRegistrations ==
	       state.buildMigrationNotice.unresolvedHistoricalRegistrations);
	assert(snapshot.undoHistory == state.undoHistory);
	assert(snapshot.undoNextActionId == 42);

	FakeSnapshot replacement{};
	replacement.registeredItems.emplace(9u, 3u);
	replacement.rewardTotals.insert_or_assign(7, 2.0f);
	replacement.buildAppliedEffectTotals.insert_or_assign(11, 1.25f);
	replacement.attackScore = 11;
	replacement.attackBuildPointsCenti = 1600;
	replacement.defenseBuildPointsCenti = 800;
	replacement.utilityBuildPointsCenti = 400;
	replacement.activeBuildSlots[1] = "build.attack.precision";
	replacement.buildMigrationVersion = 5;
	replacement.buildMigrationState = CodexOfPowerNG::Builds::BuildMigrationState::kComplete;
	replacement.buildMigrationNotice = { true, false, 2u };
	replacement.undoHistory.push_back(123);
	replacement.undoNextActionId = 77;
	Ops::ReplaceState(state, std::move(replacement));
	assert(state.registeredItems.contains(9u));
	assert(state.rewardTotals.contains(7));
	assert(state.buildAppliedEffectTotals.contains(11));
	assert(state.attackScore == 11);
	assert(state.attackBuildPointsCenti == 1600);
	assert(state.defenseBuildPointsCenti == 800);
	assert(state.utilityBuildPointsCenti == 400);
	assert(state.activeBuildSlots[1] == "build.attack.precision");
	assert(state.buildMigrationVersion == 5);
	assert(state.buildMigrationState == CodexOfPowerNG::Builds::BuildMigrationState::kComplete);
	assert(state.buildMigrationNotice.needsNotice);
	assert(!state.buildMigrationNotice.legacyRewardsMigrated);
	assert(state.buildMigrationNotice.unresolvedHistoricalRegistrations == 2u);
	assert(state.undoHistory.size() == 1);
	assert(state.undoNextActionId == 77);

	Ops::Clear(state);
	assert(state.registeredItems.empty());
	assert(state.blockedItems.empty());
	assert(state.notifiedItems.empty());
	assert(state.rewardTotals.empty());
	assert(state.buildAppliedEffectTotals.empty());
	assert(state.attackScore == 0);
	assert(state.defenseScore == 0);
	assert(state.utilityScore == 0);
	assert(state.attackBuildPointsCenti == 0);
	assert(state.defenseBuildPointsCenti == 0);
	assert(state.utilityBuildPointsCenti == 0);
	assert(state.activeBuildSlots[0].empty());
	assert(state.buildMigrationVersion == 0);
	assert(state.buildMigrationState == CodexOfPowerNG::Builds::BuildMigrationState::kNotStarted);
	assert(!state.buildMigrationNotice.needsNotice);
	assert(!state.buildMigrationNotice.legacyRewardsMigrated);
	assert(state.buildMigrationNotice.unresolvedHistoricalRegistrations == 0u);
	assert(state.undoHistory.empty());
	assert(state.undoNextActionId == 1);

	return 0;
}
