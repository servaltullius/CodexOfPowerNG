#pragma once

#include <cstddef>
#include <cmath>
#include <cstdint>

namespace CodexOfPowerNG::Rewards
{
	enum class SyncRequestAction : std::uint8_t
	{
		kStartNewRun,
		kMarkRerun,
		kForceRestartAndStart
	};

	[[nodiscard]] inline SyncRequestAction DecideSyncRequestAction(
		bool scheduled,
		std::uint64_t scheduledSinceMs,
		std::uint64_t nowMs,
		std::uint64_t stuckThresholdMs) noexcept
	{
		if (!scheduled) {
			return SyncRequestAction::kStartNewRun;
		}

		if (nowMs >= scheduledSinceMs && (nowMs - scheduledSinceMs) > stuckThresholdMs) {
			return SyncRequestAction::kForceRestartAndStart;
		}

		return SyncRequestAction::kMarkRerun;
	}

	[[nodiscard]] inline std::uint32_t NextMissingStreak(
		float delta,
		std::uint32_t currentStreak,
		float epsilon = 0.001f) noexcept
	{
		if (std::abs(delta) <= epsilon) {
			return 0;
		}

		return currentStreak + 1;
	}

	[[nodiscard]] inline bool ShouldApplyAfterStreak(
		float delta,
		std::uint32_t streak,
		std::uint32_t requiredStreak,
		float epsilon = 0.001f) noexcept
	{
		return std::abs(delta) > epsilon && streak >= requiredStreak;
	}

	[[nodiscard]] inline bool ShouldStopSyncAfterPass(
		std::size_t correctedCount,
		std::size_t pendingCount) noexcept
	{
		return correctedCount == 0 && pendingCount == 0;
	}
}
