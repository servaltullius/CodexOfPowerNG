#include "CodexOfPowerNG/RewardStateStore.h"

#include "CodexOfPowerNG/RewardCaps.h"
#include "CodexOfPowerNG/State.h"

#include <cmath>

namespace CodexOfPowerNG::RewardStateStore
{
	RewardTotalTransition AdjustClamped(RE::ActorValue av, float delta) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);

		RewardTotalTransition transition{};
		const auto it = state.rewardTotals.find(av);
		if (it != state.rewardTotals.end()) {
			transition.existedBefore = true;
			transition.previousTotal = it->second;
		}

		transition.nextTotal = Rewards::ClampRewardTotal(av, transition.previousTotal + delta);
		if (std::abs(transition.nextTotal) <= Rewards::kRewardCapEpsilon) {
			if (it != state.rewardTotals.end()) {
				state.rewardTotals.erase(it);
			}
		} else {
			state.rewardTotals.insert_or_assign(av, transition.nextTotal);
		}

		return transition;
	}

	std::optional<float> Get(RE::ActorValue av) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		const auto it = state.rewardTotals.find(av);
		if (it == state.rewardTotals.end()) {
			return std::nullopt;
		}
		return it->second;
	}

	std::optional<float> Take(RE::ActorValue av) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		const auto it = state.rewardTotals.find(av);
		if (it == state.rewardTotals.end()) {
			return std::nullopt;
		}

		const float total = it->second;
		state.rewardTotals.erase(it);
		return total;
	}

	void Set(RE::ActorValue av, float total) noexcept
	{
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		if (std::abs(total) <= Rewards::kRewardCapEpsilon) {
			state.rewardTotals.erase(av);
			return;
		}
		state.rewardTotals.insert_or_assign(av, total);
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
		std::vector<std::pair<RE::ActorValue, float>> totals;
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		totals.reserve(state.rewardTotals.size());
		for (const auto& [av, total] : state.rewardTotals) {
			totals.emplace_back(av, total);
		}
		return totals;
	}

	std::vector<RewardCapAdjustment> ClampAll() noexcept
	{
		std::vector<RewardCapAdjustment> adjustments;
		auto& state = GetState();
		std::scoped_lock lock(state.mutex);
		adjustments.reserve(state.rewardTotals.size());

		for (auto it = state.rewardTotals.begin(); it != state.rewardTotals.end();) {
			const float clamped = Rewards::ClampRewardTotal(it->first, it->second);
			if (std::abs(clamped - it->second) <= Rewards::kRewardCapEpsilon) {
				++it;
				continue;
			}

			adjustments.push_back(RewardCapAdjustment{ it->first, it->second, clamped });
			if (std::abs(clamped) <= Rewards::kRewardCapEpsilon) {
				it = state.rewardTotals.erase(it);
				continue;
			}

			it->second = clamped;
			++it;
		}

		return adjustments;
	}
}
