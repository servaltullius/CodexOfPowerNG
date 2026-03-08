#include "CodexOfPowerNG/SerializationStateStoreOps.h"

#include <cassert>
#include <cstdint>
#include <deque>
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
		std::deque<int>                                  undoHistory;
		std::uint64_t                                    undoNextActionId{ 1 };
	};

	struct FakeState
	{
		std::unordered_map<std::uint32_t, std::uint32_t> registeredItems;
		std::unordered_set<std::uint32_t>                blockedItems;
		std::unordered_set<std::uint32_t>                notifiedItems;
		std::unordered_map<int, float>                   rewardTotals;
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
	state.undoHistory.push_back(99);
	state.undoNextActionId = 42;

	const auto snapshot = Ops::SnapshotState<FakeSnapshot>(state);
	assert(snapshot.registeredItems == state.registeredItems);
	assert(snapshot.blockedItems == state.blockedItems);
	assert(snapshot.notifiedItems == state.notifiedItems);
	assert(snapshot.rewardTotals == state.rewardTotals);
	assert(snapshot.undoHistory == state.undoHistory);
	assert(snapshot.undoNextActionId == 42);

	FakeSnapshot replacement{};
	replacement.registeredItems.emplace(9u, 3u);
	replacement.rewardTotals.insert_or_assign(7, 2.0f);
	replacement.undoHistory.push_back(123);
	replacement.undoNextActionId = 77;
	Ops::ReplaceState(state, std::move(replacement));
	assert(state.registeredItems.contains(9u));
	assert(state.rewardTotals.contains(7));
	assert(state.undoHistory.size() == 1);
	assert(state.undoNextActionId == 77);

	Ops::Clear(state);
	assert(state.registeredItems.empty());
	assert(state.blockedItems.empty());
	assert(state.notifiedItems.empty());
	assert(state.rewardTotals.empty());
	assert(state.undoHistory.empty());
	assert(state.undoNextActionId == 1);

	return 0;
}
