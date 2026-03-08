#include "CodexOfPowerNG/SerializationStateStore.h"

#include "CodexOfPowerNG/State.h"

#include <utility>

namespace CodexOfPowerNG::SerializationStateStore
{
	Snapshot SnapshotState() noexcept
	{
		Snapshot snapshot{};
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		snapshot.registeredItems = state.registeredItems;
		snapshot.blockedItems = state.blockedItems;
		snapshot.notifiedItems = state.notifiedItems;
		snapshot.rewardTotals = state.rewardTotals;
		snapshot.undoHistory = state.undoHistory;
		snapshot.undoNextActionId = state.undoNextActionId;
		return snapshot;
	}

	void ReplaceState(Snapshot snapshot) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		state.registeredItems = std::move(snapshot.registeredItems);
		state.blockedItems = std::move(snapshot.blockedItems);
		state.notifiedItems = std::move(snapshot.notifiedItems);
		state.rewardTotals = std::move(snapshot.rewardTotals);
		state.undoHistory = std::move(snapshot.undoHistory);
		state.undoNextActionId = snapshot.undoNextActionId;
	}

	void Clear() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		state.registeredItems.clear();
		state.blockedItems.clear();
		state.notifiedItems.clear();
		state.rewardTotals.clear();
		state.undoHistory.clear();
		state.undoNextActionId = 1;
	}
}
