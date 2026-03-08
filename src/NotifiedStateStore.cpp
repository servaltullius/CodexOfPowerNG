#include "CodexOfPowerNG/NotifiedStateStore.h"

#include "CodexOfPowerNG/State.h"

#include <utility>

namespace CodexOfPowerNG::NotifiedStateStore
{
	bool ContainsAny(RE::FormID primaryId, RE::FormID secondaryId) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return state.notifiedItems.contains(primaryId) || state.notifiedItems.contains(secondaryId);
	}

	void MarkPair(RE::FormID primaryId, RE::FormID secondaryId) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		if (primaryId != 0) {
			state.notifiedItems.insert(primaryId);
		}
		if (secondaryId != 0) {
			state.notifiedItems.insert(secondaryId);
		}
	}

	void Clear() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		state.notifiedItems.clear();
	}

	void ReplaceAll(std::unordered_set<RE::FormID> notifiedItems) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		state.notifiedItems = std::move(notifiedItems);
	}

	std::unordered_set<RE::FormID> Snapshot() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return state.notifiedItems;
	}

	std::size_t Count() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return state.notifiedItems.size();
	}
}
