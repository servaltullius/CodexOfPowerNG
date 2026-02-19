#pragma once

#include <cstdint>
#include <cstddef>

namespace CodexOfPowerNG::Rewards
{
	// Called after successful registration. Triggers rewards at the configured cadence.
	void MaybeGrantRegistrationReward(std::uint32_t group, std::int32_t totalRegistered) noexcept;

	// After loading a save, re-sync reward totals to runtime AV state if any bonus is missing.
	void SyncRewardTotalsToPlayer() noexcept;

	// Clears in-flight reward sync workers at the load boundary.
	// Call from kPreLoadGame before serialization callbacks run.
	void ResetSyncSchedulersForLoad() noexcept;

	// Lightweight carry-weight-only resync used right after carry reward grants.
	void ScheduleCarryWeightQuickResync() noexcept;

	// Refunds recorded rewards (does not restore consumed items, does not clear registrations).
	// Returns number of actor values refunded.
	[[nodiscard]] std::size_t RefundRewards() noexcept;
}
