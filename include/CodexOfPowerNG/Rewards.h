#pragma once

#include <cstdint>
#include <cstddef>

namespace CodexOfPowerNG::Rewards
{
	// Called after successful registration. Triggers rewards at the configured cadence.
	void MaybeGrantRegistrationReward(std::uint32_t group, std::int32_t totalRegistered) noexcept;

	// After loading a save, re-sync reward totals to runtime AV state if any bonus is missing.
	void SyncRewardTotalsToPlayer() noexcept;

	// Refunds recorded rewards (does not restore consumed items, does not clear registrations).
	// Returns number of actor values refunded.
	[[nodiscard]] std::size_t RefundRewards() noexcept;
}
