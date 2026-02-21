#include "CodexOfPowerNG/SerializationWriteFlow.h"

#include <algorithm>
#include <array>
#include <cassert>

int main()
{
	std::array<int, 5> order{ 0, 0, 0, 0, 0 };
	int                idx = 0;
	bool               firstFailed = false;

	const bool allOk = CodexOfPowerNG::Serialization::ExecuteAllSaveWriters(
		[&]() -> bool {
			order[idx++] = 1;
			firstFailed = true;
			return false;
		},
		[&]() { order[idx++] = 2; },
		[&]() { order[idx++] = 3; },
		[&]() { order[idx++] = 4; },
		[&]() { order[idx++] = 5; });

	assert(!allOk);
	assert(firstFailed);
	assert(idx == 5);
	assert(std::equal(order.begin(), order.end(), std::array<int, 5>{ 1, 2, 3, 4, 5 }.begin()));

	const bool allOkSecond = CodexOfPowerNG::Serialization::ExecuteAllSaveWriters(
		[]() -> bool { return true; },
		[]() {},
		[]() {},
		[]() {},
		[]() -> bool { return true; });
	assert(allOkSecond);

	return 0;
}
