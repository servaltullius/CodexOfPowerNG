#pragma once

#include <array>
#include <cmath>

namespace CodexOfPowerNG::Rewards
{
	[[nodiscard]] inline float SelectObservedRewardTotal(
		float expectedTotal,
		float observedFromCurrent,
		float observedFromPermanent,
		float observedPermanentModifier) noexcept
	{
		const std::array<float, 3> candidates{
			observedFromCurrent,
			observedFromPermanent,
			observedPermanentModifier
		};

		float best = candidates[0];
		float bestDiff = std::abs(expectedTotal - best);
		for (std::size_t i = 1; i < candidates.size(); ++i) {
			const float candidate = candidates[i];
			const float diff = std::abs(expectedTotal - candidate);

			if (diff + 0.000001f < bestDiff) {
				best = candidate;
				bestDiff = diff;
				continue;
			}

			// Tie-breaker: prefer smaller magnitude to reduce over-correction risk.
			if (std::abs(diff - bestDiff) <= 0.000001f && std::abs(candidate) < std::abs(best)) {
				best = candidate;
			}
		}
		return best;
	}

	[[nodiscard]] inline float ComputeRewardSyncDelta(
		float observedTotal,
		float expectedTotal,
		float epsilon = 0.001f) noexcept
	{
		if (std::abs(expectedTotal) <= epsilon) {
			return 0.0f;
		}

		const float missing = expectedTotal - observedTotal;

		if (expectedTotal > 0.0f) {
			return (missing > epsilon) ? missing : 0.0f;
		}

		if (expectedTotal < 0.0f) {
			return (missing < -epsilon) ? missing : 0.0f;
		}

		return 0.0f;
	}

	[[nodiscard]] inline float ComputeRewardSyncDeltaFromSnapshot(
		float baseValue,
		float currentValue,
		float permanentValue,
		float permanentModifier,
		float expectedTotal,
		float epsilon = 0.001f) noexcept
	{
		const float observedFromCurrent = currentValue - baseValue;
		const float observedFromPermanent = permanentValue - baseValue;
		const float observed = SelectObservedRewardTotal(
			expectedTotal,
			observedFromCurrent,
			observedFromPermanent,
			permanentModifier);
		return ComputeRewardSyncDelta(observed, expectedTotal, epsilon);
	}

	[[nodiscard]] inline float ComputeRewardSyncDeltaFromCurrent(
		float baseValue,
		float currentValue,
		float expectedTotal,
		float epsilon = 0.001f) noexcept
	{
		const float observedFromCurrent = currentValue - baseValue;
		return ComputeRewardSyncDelta(observedFromCurrent, expectedTotal, epsilon);
	}

	[[nodiscard]] inline float ComputeCapNormalizationDeltaFromSnapshot(
		float baseValue,
		float currentValue,
		float permanentValue,
		float permanentModifier,
		float cappedExpectedTotal,
		float epsilon = 0.001f) noexcept
	{
		const float observedFromCurrent = currentValue - baseValue;
		const float observedFromPermanent = permanentValue - baseValue;
		const float observed = SelectObservedRewardTotal(
			cappedExpectedTotal,
			observedFromCurrent,
			observedFromPermanent,
			permanentModifier);
		const float delta = cappedExpectedTotal - observed;
		return (delta < -epsilon) ? delta : 0.0f;
	}
}
