#include "CodexOfPowerNG/SerializationWriteFlow.h"

#include <algorithm>
#include <array>
#include <cassert>

int main()
{
	std::array<int, 4> order{ 0, 0, 0, 0 };
	int                idx = 0;
	bool               firstFailed = false;

	CodexOfPowerNG::Serialization::ExecuteAllSaveWriters(
		[&]() -> bool {
			order[idx++] = 1;
			firstFailed = true;
			return false;
		},
		[&]() { order[idx++] = 2; },
		[&]() { order[idx++] = 3; },
		[&]() { order[idx++] = 4; });

	assert(firstFailed);
	assert(idx == 4);
	assert(std::equal(order.begin(), order.end(), std::array<int, 4>{ 1, 2, 3, 4 }.begin()));

	return 0;
}
