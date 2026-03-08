#include "CodexOfPowerNG/NotifiedStateStore.h"

#include "CodexOfPowerNG/NotifiedStateStoreOps.h"
#include "CodexOfPowerNG/State.h"

#include <utility>

namespace CodexOfPowerNG::NotifiedStateStore
{
	bool ContainsAny(RE::FormID primaryId, RE::FormID secondaryId) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return Ops::ContainsAny(state.notifiedItems, primaryId, secondaryId);
	}

	void MarkPair(RE::FormID primaryId, RE::FormID secondaryId) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		Ops::MarkPair(state.notifiedItems, primaryId, secondaryId);
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
		Ops::ReplaceAll(state.notifiedItems, std::move(notifiedItems));
	}

	std::unordered_set<RE::FormID> Snapshot() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return Ops::Snapshot(state.notifiedItems);
	}

	std::size_t Count() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return Ops::Count(state.notifiedItems);
	}
}
