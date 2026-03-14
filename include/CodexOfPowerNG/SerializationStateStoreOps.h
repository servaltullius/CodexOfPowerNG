#pragma once

#include <cstdint>
#include <utility>

namespace CodexOfPowerNG::SerializationStateStore::Ops
{
	template <class Snapshot, class State>
	[[nodiscard]] Snapshot SnapshotState(const State& state)
	{
		Snapshot snapshot{};
		snapshot.registeredItems = state.registeredItems;
		snapshot.blockedItems = state.blockedItems;
		snapshot.notifiedItems = state.notifiedItems;
		snapshot.rewardTotals = state.rewardTotals;
		snapshot.buildAppliedEffectTotals = state.buildAppliedEffectTotals;
		snapshot.attackScore = state.attackScore;
		snapshot.defenseScore = state.defenseScore;
		snapshot.utilityScore = state.utilityScore;
		snapshot.attackBuildPointsCenti = state.attackBuildPointsCenti;
		snapshot.defenseBuildPointsCenti = state.defenseBuildPointsCenti;
		snapshot.utilityBuildPointsCenti = state.utilityBuildPointsCenti;
		snapshot.activeBuildSlots = state.activeBuildSlots;
		snapshot.buildMigrationVersion = state.buildMigrationVersion;
		snapshot.buildMigrationState = state.buildMigrationState;
		snapshot.buildMigrationNotice = state.buildMigrationNotice;
		snapshot.undoHistory = state.undoHistory;
		snapshot.undoNextActionId = state.undoNextActionId;
		return snapshot;
	}

	template <class State, class Snapshot>
	void ReplaceState(State& state, Snapshot snapshot) noexcept
	{
		state.registeredItems = std::move(snapshot.registeredItems);
		state.blockedItems = std::move(snapshot.blockedItems);
		state.notifiedItems = std::move(snapshot.notifiedItems);
		state.rewardTotals = std::move(snapshot.rewardTotals);
		state.buildAppliedEffectTotals = std::move(snapshot.buildAppliedEffectTotals);
		state.attackScore = snapshot.attackScore;
		state.defenseScore = snapshot.defenseScore;
		state.utilityScore = snapshot.utilityScore;
		state.attackBuildPointsCenti = snapshot.attackBuildPointsCenti;
		state.defenseBuildPointsCenti = snapshot.defenseBuildPointsCenti;
		state.utilityBuildPointsCenti = snapshot.utilityBuildPointsCenti;
		state.activeBuildSlots = std::move(snapshot.activeBuildSlots);
		state.buildMigrationVersion = snapshot.buildMigrationVersion;
		state.buildMigrationState = snapshot.buildMigrationState;
		state.buildMigrationNotice = snapshot.buildMigrationNotice;
		state.undoHistory = std::move(snapshot.undoHistory);
		state.undoNextActionId = snapshot.undoNextActionId;
	}

	template <class State>
	void Clear(State& state, std::uint64_t resetUndoNextActionId = 1) noexcept
	{
		state.registeredItems.clear();
		state.blockedItems.clear();
		state.notifiedItems.clear();
		state.rewardTotals.clear();
		state.buildAppliedEffectTotals.clear();
		state.attackScore = 0;
		state.defenseScore = 0;
		state.utilityScore = 0;
		state.attackBuildPointsCenti = 0;
		state.defenseBuildPointsCenti = 0;
		state.utilityBuildPointsCenti = 0;
		state.activeBuildSlots = {};
		state.buildMigrationVersion = 0;
		state.buildMigrationState = decltype(state.buildMigrationState)::kNotStarted;
		state.buildMigrationNotice = {};
		state.undoHistory.clear();
		state.undoNextActionId = resetUndoNextActionId;
	}
}
