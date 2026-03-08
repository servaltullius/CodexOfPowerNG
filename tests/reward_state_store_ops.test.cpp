#include "CodexOfPowerNG/RewardStateStoreOps.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <unordered_map>

namespace
{
	bool NearlyEqual(float lhs, float rhs)
	{
		return std::abs(lhs - rhs) <= 0.0001f;
	}
}

int main()
{
	namespace Ops = CodexOfPowerNG::RewardStateStore::Ops;

	std::unordered_map<int, float> totals;
	constexpr float epsilon = 0.001f;
	auto clamp = [](int av, float total) {
		if (av == 1) {
			return std::clamp(total, -100.0f, 100.0f);
		}
		if (av == 3) {
			return 0.0f;
		}
		return total;
	};

	const auto first = Ops::AdjustClamped(totals, 1, 150.0f, clamp, epsilon);
	assert(!first.existedBefore);
	assert(NearlyEqual(first.previousTotal, 0.0f));
	assert(NearlyEqual(first.nextTotal, 100.0f));
	assert(NearlyEqual(*Ops::Get(totals, 1), 100.0f));

	const auto second = Ops::AdjustClamped(totals, 1, -100.0f, clamp, epsilon);
	assert(second.existedBefore);
	assert(NearlyEqual(second.previousTotal, 100.0f));
	assert(NearlyEqual(second.nextTotal, 0.0f));
	assert(!Ops::Get(totals, 1).has_value());

	Ops::Set(totals, 1, 25.0f, epsilon);
	Ops::Set(totals, 2, 40.0f, epsilon);
	const auto snapshot = Ops::Snapshot(totals);
	assert(snapshot.size() == 2);

	const auto taken = Ops::Take(totals, 2);
	assert(taken.has_value());
	assert(NearlyEqual(*taken, 40.0f));
	assert(!Ops::Get(totals, 2).has_value());

	Ops::Set(totals, 1, 130.0f, epsilon);
	Ops::Set(totals, 3, 5.0f, epsilon);
	const auto adjustments = Ops::ClampAll(totals, clamp, epsilon);
	assert(adjustments.size() == 2);
	assert(NearlyEqual(*Ops::Get(totals, 1), 100.0f));
	assert(!Ops::Get(totals, 3).has_value());

	return 0;
}
