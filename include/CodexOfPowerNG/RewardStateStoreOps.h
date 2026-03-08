#pragma once

#include <cmath>
#include <optional>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::RewardStateStore::Ops
{
	template <class Key>
	struct RewardTotalTransition
	{
		bool  existedBefore{ false };
		float previousTotal{ 0.0f };
		float nextTotal{ 0.0f };
	};

	template <class Key>
	struct RewardCapAdjustment
	{
		Key   av;
		float before;
		float after;
	};

	template <class Map, class Key, class ClampFn>
	[[nodiscard]] RewardTotalTransition<Key> AdjustClamped(
		Map& totals,
		const Key& av,
		float delta,
		ClampFn&& clampRewardTotal,
		float epsilon) noexcept
	{
		RewardTotalTransition<Key> transition{};
		const auto                it = totals.find(av);
		if (it != totals.end()) {
			transition.existedBefore = true;
			transition.previousTotal = it->second;
		}

		transition.nextTotal = clampRewardTotal(av, transition.previousTotal + delta);
		if (std::abs(transition.nextTotal) <= epsilon) {
			if (it != totals.end()) {
				totals.erase(it);
			}
		} else {
			totals.insert_or_assign(av, transition.nextTotal);
		}

		return transition;
	}

	template <class Map, class Key>
	[[nodiscard]] std::optional<float> Get(const Map& totals, const Key& av) noexcept
	{
		const auto it = totals.find(av);
		if (it == totals.end()) {
			return std::nullopt;
		}
		return it->second;
	}

	template <class Map, class Key>
	[[nodiscard]] std::optional<float> Take(Map& totals, const Key& av) noexcept
	{
		const auto it = totals.find(av);
		if (it == totals.end()) {
			return std::nullopt;
		}

		const float total = it->second;
		totals.erase(it);
		return total;
	}

	template <class Map, class Key>
	void Set(Map& totals, const Key& av, float total, float epsilon) noexcept
	{
		if (std::abs(total) <= epsilon) {
			totals.erase(av);
			return;
		}
		totals.insert_or_assign(av, total);
	}

	template <class Map>
	[[nodiscard]] std::vector<std::pair<typename Map::key_type, float>> Snapshot(const Map& totals)
	{
		std::vector<std::pair<typename Map::key_type, float>> snapshot;
		snapshot.reserve(totals.size());
		for (const auto& [av, total] : totals) {
			snapshot.emplace_back(av, total);
		}
		return snapshot;
	}

	template <class Map, class ClampFn>
	[[nodiscard]] std::vector<RewardCapAdjustment<typename Map::key_type>> ClampAll(
		Map& totals,
		ClampFn&& clampRewardTotal,
		float epsilon) noexcept
	{
		std::vector<RewardCapAdjustment<typename Map::key_type>> adjustments;
		adjustments.reserve(totals.size());

		for (auto it = totals.begin(); it != totals.end();) {
			const float clamped = clampRewardTotal(it->first, it->second);
			if (std::abs(clamped - it->second) <= epsilon) {
				++it;
				continue;
			}

			adjustments.push_back(RewardCapAdjustment<typename Map::key_type>{ it->first, it->second, clamped });
			if (std::abs(clamped) <= epsilon) {
				it = totals.erase(it);
				continue;
			}

			it->second = clamped;
			++it;
		}

		return adjustments;
	}
}
