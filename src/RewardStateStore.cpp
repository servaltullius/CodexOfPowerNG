#include "CodexOfPowerNG/RewardStateStore.h"

#include "CodexOfPowerNG/RewardCaps.h"
#include "CodexOfPowerNG/RewardStateStoreOps.h"
#include "CodexOfPowerNG/State.h"

namespace CodexOfPowerNG::RewardStateStore
{
	RewardTotalTransition AdjustClamped(RE::ActorValue av, float delta) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return Ops::AdjustClamped(
			state.rewardTotals,
			av,
			delta,
			[](RE::ActorValue actorValue, float total) { return Rewards::ClampRewardTotal(actorValue, total); },
			Rewards::kRewardCapEpsilon);
	}

	std::optional<float> Get(RE::ActorValue av) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return Ops::Get(state.rewardTotals, av);
	}

	std::optional<float> Take(RE::ActorValue av) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return Ops::Take(state.rewardTotals, av);
	}

	void Set(RE::ActorValue av, float total) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		Ops::Set(state.rewardTotals, av, total, Rewards::kRewardCapEpsilon);
	}

	void Clear() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		state.rewardTotals.clear();
	}

	std::size_t Count() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return state.rewardTotals.size();
	}

	std::vector<std::pair<RE::ActorValue, float>> Snapshot() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return Ops::Snapshot(state.rewardTotals);
	}

	std::vector<RewardCapAdjustment> ClampAll() noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		return Ops::ClampAll(
			state.rewardTotals,
			[](RE::ActorValue actorValue, float total) { return Rewards::ClampRewardTotal(actorValue, total); },
			Rewards::kRewardCapEpsilon);
	}
}
