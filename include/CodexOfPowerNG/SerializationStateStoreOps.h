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
		state.undoHistory.clear();
		state.undoNextActionId = resetUndoNextActionId;
	}
}
