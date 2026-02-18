#include "CodexOfPowerNG/RewardsResync.h"

#include <cassert>
#include <cmath>

namespace
{
	void AssertNear(float lhs, float rhs, float eps = 0.0001f)
	{
		assert(std::fabs(lhs - rhs) <= eps);
	}
}

int main()
{
	using CodexOfPowerNG::Rewards::ComputeRewardSyncDelta;

	// Positive reward already reflected.
	AssertNear(ComputeRewardSyncDelta(300.0f, 320.0f, 20.0f), 0.0f);

	// Positive reward partially missing -> apply only the missing amount.
	AssertNear(ComputeRewardSyncDelta(300.0f, 308.0f, 20.0f), 12.0f);

	// Extra external bonus present -> don't over-apply.
	AssertNear(ComputeRewardSyncDelta(300.0f, 340.0f, 20.0f), 0.0f);

	// Negative reward already reflected.
	AssertNear(ComputeRewardSyncDelta(1.0f, 0.8f, -0.2f), 0.0f);

	// Negative reward partially missing -> push downward by missing amount.
	AssertNear(ComputeRewardSyncDelta(1.0f, 0.9f, -0.2f), -0.1f);

	// Negative reward exceeded by external effects -> no correction.
	AssertNear(ComputeRewardSyncDelta(1.0f, 0.6f, -0.2f), 0.0f);

	return 0;
}
