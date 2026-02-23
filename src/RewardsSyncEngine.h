#pragma once

#include "CodexOfPowerNG/State.h"

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::Rewards::Engine
{
	struct RewardSyncPassState
	{
		std::unordered_map<RE::ActorValue, std::uint32_t, ActorValueHash> missingStreaks;
		std::unordered_set<RE::ActorValue, ActorValueHash>                nonConvergingActorValues;
		bool normalizeCapsOnFirstPass{ true };
		bool weaponAbilityRefreshRequested{ false };
		std::uint64_t generation{ 0 };
		std::uint64_t readinessDeadlineMs{ 0 };
	};

	struct RewardSyncPassResult
	{
		std::size_t correctedCount{ 0 };
		std::size_t pendingCount{ 0 };
	};

	[[nodiscard]] float SnapshotRewardTotalForActorValue(RE::ActorValue av) noexcept;
	void NormalizeRewardCapsOnStateAndPlayer() noexcept;
	[[nodiscard]] std::vector<std::pair<RE::ActorValue, float>> SnapshotRewardTotals() noexcept;

	void PrepareRewardSyncPass(RewardSyncPassState& passState) noexcept;
	[[nodiscard]] RewardSyncPassResult ApplyRewardSyncPass(
		RewardSyncPassState& passState,
		std::uint32_t minMissingStreak) noexcept;
	void FinalizeRewardSyncPass(RewardSyncPassState& passState) noexcept;
}
