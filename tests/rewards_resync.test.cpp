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
	using CodexOfPowerNG::Rewards::ComputeCarryWeightSyncDelta;
	using CodexOfPowerNG::Rewards::ComputeRewardSyncDeltaFromCurrent;
	using CodexOfPowerNG::Rewards::ComputeRewardSyncDeltaFromSnapshot;
	using CodexOfPowerNG::Rewards::MergeRewardSyncDeltaCandidates;
	using CodexOfPowerNG::Rewards::SelectObservedRewardTotal;

	// Positive reward already reflected.
	AssertNear(ComputeRewardSyncDelta(20.0f, 20.0f), 0.0f);

	// Positive reward partially missing -> apply only the missing amount.
	AssertNear(ComputeRewardSyncDelta(8.0f, 20.0f), 12.0f);

	// Extra external bonus present -> don't over-apply.
	AssertNear(ComputeRewardSyncDelta(40.0f, 20.0f), 0.0f);

	// Negative reward already reflected.
	AssertNear(ComputeRewardSyncDelta(-0.2f, -0.2f), 0.0f);

	// Negative reward partially missing -> push downward by missing amount.
	AssertNear(ComputeRewardSyncDelta(-0.1f, -0.2f), -0.1f);

	// Negative reward exceeded by external effects -> no correction.
	AssertNear(ComputeRewardSyncDelta(-0.4f, -0.2f), 0.0f);

	// Snapshot-based select: favor candidate closest to expected.
	AssertNear(SelectObservedRewardTotal(1.0f, 0.0f, 0.0f, 1.0f), 1.0f);
	// Tie case picks smaller-magnitude candidate to avoid over-correction.
	AssertNear(SelectObservedRewardTotal(1.0f, 2.0f, 0.0f, 0.0f), 0.0f);
	AssertNear(SelectObservedRewardTotal(1.0f, 3.0f, 0.9f, 0.2f), 0.9f);

	// Snapshot calc: use permanent/permanent-modifier data when current is noisy.
	AssertNear(
		ComputeRewardSyncDeltaFromSnapshot(
			300.0f,  // base
			350.0f,  // current (noisy temp buffs)
			312.0f,  // permanent
			12.0f,   // permanent mod
			20.0f),  // expected reward total
		8.0f);

	// Carry-weight style noise: prefer stable candidate, avoid over-correction.
	AssertNear(
		ComputeRewardSyncDeltaFromSnapshot(
			300.0f,  // base
			320.0f,  // current (external +20)
			305.0f,  // permanent (reward +5)
			25.0f,   // permanent mod (noisy)
			5.0f),   // expected reward total
		0.0f);

	// Carry-weight missing reward recovery under current-value noise.
	AssertNear(
		ComputeRewardSyncDeltaFromSnapshot(
			300.0f,  // base
			330.0f,  // current (noisy temp buffs)
			303.0f,  // permanent (only +3 reflected)
			3.0f,    // permanent mod
			8.0f),   // expected reward total
		5.0f);

	// Current-based sync is preferred for carry weight gameplay consistency.
	AssertNear(
		ComputeRewardSyncDeltaFromCurrent(
			300.0f,
			300.0f,  // current reward not reflected
			5.0f),
		5.0f);
	AssertNear(
		ComputeRewardSyncDeltaFromCurrent(
			300.0f,
			307.0f,  // external bonus above expected
			5.0f),
		0.0f);

	// Merge helper: for positive expected values, choose stronger positive correction.
	AssertNear(MergeRewardSyncDeltaCandidates(5.0f, 0.0f, 2.0f), 2.0f);
	AssertNear(MergeRewardSyncDeltaCandidates(5.0f, 3.0f, 1.0f), 3.0f);

	// Carry helper combines current/snapshot signals.
	AssertNear(
		ComputeCarryWeightSyncDelta(
			300.0f,  // base
			300.0f,  // current missing
			305.0f,  // permanent reflects reward
			5.0f,    // perm mod
			5.0f),   // expected
		5.0f);
	AssertNear(
		ComputeCarryWeightSyncDelta(
			300.0f,
			307.0f,  // current already above expected
			307.0f,
			7.0f,
			5.0f),
		0.0f);

	return 0;
}
