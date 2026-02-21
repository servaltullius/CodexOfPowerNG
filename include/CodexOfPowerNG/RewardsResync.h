#pragma once

#include <algorithm>
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

	[[nodiscard]] inline float MergeRewardSyncDeltaCandidates(
		float expectedTotal,
		float deltaFromCurrent,
		float deltaFromSnapshot,
		float epsilon = 0.001f) noexcept
	{
		if (std::abs(expectedTotal) <= epsilon) {
			return 0.0f;
		}

		if (expectedTotal > 0.0f) {
			return (std::max)(0.0f, (std::max)(deltaFromCurrent, deltaFromSnapshot));
		}

		return (std::min)(0.0f, (std::min)(deltaFromCurrent, deltaFromSnapshot));
	}

	[[nodiscard]] inline float ComputeCarryWeightSyncDelta(
		float baseValue,
		float currentValue,
		float permanentValue,
		float permanentModifier,
		float expectedTotal,
		float epsilon = 0.001f) noexcept
	{
		const float deltaFromCurrent = ComputeRewardSyncDeltaFromCurrent(
			baseValue,
			currentValue,
			expectedTotal,
			epsilon);
		const float deltaFromSnapshot = ComputeRewardSyncDeltaFromSnapshot(
			baseValue,
			currentValue,
			permanentValue,
			permanentModifier,
			expectedTotal,
			epsilon);
		return MergeRewardSyncDeltaCandidates(
			expectedTotal,
			deltaFromCurrent,
			deltaFromSnapshot,
			epsilon);
	}

	// Conservative correction policy for capped AVs:
	// For positive expected totals, only apply a positive delta when both
	// current-based and snapshot-based views agree that reward is missing.
	// NOTE: Do not apply downward corrections based on cap comparisons here;
	// "currentValue" can be above cap due to external buffs/gear, and reward
	// sync must not interfere with those sources.
	[[nodiscard]] inline float ComputeCappedRewardSyncDeltaFromSnapshot(
		float baseValue,
		float currentValue,
		float permanentValue,
		float permanentModifier,
		float expectedTotal,
		float hardCapTotal,
		float epsilon = 0.001f) noexcept
	{
		const float cappedExpectedTotal = (std::min)(expectedTotal, hardCapTotal);

		const float deltaFromCurrent = ComputeRewardSyncDeltaFromCurrent(
			baseValue,
			currentValue,
			cappedExpectedTotal,
			epsilon);
		const float deltaFromSnapshot = ComputeRewardSyncDeltaFromSnapshot(
			baseValue,
			currentValue,
			permanentValue,
			permanentModifier,
			cappedExpectedTotal,
			epsilon);

		if (cappedExpectedTotal > 0.0f) {
			if (deltaFromCurrent <= epsilon || deltaFromSnapshot <= epsilon) {
				return 0.0f;
			}
			return (std::min)(deltaFromCurrent, deltaFromSnapshot);
		}

		return MergeRewardSyncDeltaCandidates(
			cappedExpectedTotal,
			deltaFromCurrent,
			deltaFromSnapshot,
			epsilon);
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
