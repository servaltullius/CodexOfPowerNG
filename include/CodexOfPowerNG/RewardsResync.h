#pragma once

#include <cmath>

namespace CodexOfPowerNG::Rewards
{
	[[nodiscard]] inline float ComputeRewardSyncDelta(
		float baseValue,
		float currentValue,
		float expectedTotal,
		float epsilon = 0.001f) noexcept
	{
		if (std::abs(expectedTotal) <= epsilon) {
			return 0.0f;
		}

		const float currentTotal = currentValue - baseValue;
		const float missing = expectedTotal - currentTotal;

		if (expectedTotal > 0.0f) {
			return (missing > epsilon) ? missing : 0.0f;
		}

		if (expectedTotal < 0.0f) {
			return (missing < -epsilon) ? missing : 0.0f;
		}

		return 0.0f;
	}
}
