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
	using CodexOfPowerNG::Rewards::ComputeCapNormalizationDeltaFromSnapshot;

	// Over-capped observed total should be normalized downward immediately.
	AssertNear(
		ComputeCapNormalizationDeltaFromSnapshot(
			100.0f,  // base
			190.0f,  // current -> observed +90
			190.0f,  // permanent -> observed +90
			90.0f,   // permanent mod
			85.0f),  // capped expected total
		-5.0f);

	// Exactly capped total should produce no adjustment.
	AssertNear(
		ComputeCapNormalizationDeltaFromSnapshot(
			100.0f,
			185.0f,
			185.0f,
			85.0f,
			85.0f),
		0.0f);

	// Missing/under-capped totals should not be increased by cap normalization.
	AssertNear(
		ComputeCapNormalizationDeltaFromSnapshot(
			100.0f,
			170.0f,  // observed +70
			170.0f,
			70.0f,
			85.0f),
		0.0f);

	// When one snapshot source already matches capped expected total, do not adjust.
	AssertNear(
		ComputeCapNormalizationDeltaFromSnapshot(
			100.0f,
			210.0f,  // noisy +110
			185.0f,  // stable +85
			85.0f,
			85.0f),
		0.0f);

	// Epsilon boundary: tiny overage within epsilon should be ignored.
	AssertNear(
		ComputeCapNormalizationDeltaFromSnapshot(
			100.0f,
			185.0005f,
			185.0005f,
			85.0005f,
			85.0f,
			0.001f),
		0.0f);

	return 0;
}
