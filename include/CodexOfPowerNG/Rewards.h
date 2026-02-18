#pragma once

#include <cstdint>
#include <cstddef>

namespace CodexOfPowerNG::Rewards
{
	enum class CarryWeightRecoveryStatus : std::uint8_t
	{
		kApplied = 0,
		kAlreadyUsed,
		kAlreadyRecorded,
		kUnavailable
	};

	struct CarryWeightRecoveryResult
	{
		CarryWeightRecoveryStatus status{ CarryWeightRecoveryStatus::kUnavailable };
		float appliedDelta{ 0.0f };
	};

	// Called after successful registration. Triggers rewards at the configured cadence.
	void MaybeGrantRegistrationReward(std::uint32_t group, std::int32_t totalRegistered) noexcept;

	// After loading a save, re-sync reward totals to runtime AV state if any bonus is missing.
	void SyncRewardTotalsToPlayer() noexcept;

	// Lightweight carry-weight-only resync used right after carry reward grants.
	void ScheduleCarryWeightQuickResync() noexcept;

	// Applies one-time carry-weight recovery (+5) for saves missing recorded carry reward.
	[[nodiscard]] CarryWeightRecoveryResult RecoverCarryWeightRewardOneTime() noexcept;

	// Refunds recorded rewards (does not restore consumed items, does not clear registrations).
	// Returns number of actor values refunded.
	[[nodiscard]] std::size_t RefundRewards() noexcept;
}
