#include "CodexOfPowerNG/NotifiedStateStoreOps.h"

#include <cassert>
#include <cstdint>
#include <unordered_set>

int main()
{
	namespace Ops = CodexOfPowerNG::NotifiedStateStore::Ops;

	std::unordered_set<std::uint32_t> notifiedItems;
	assert(!Ops::ContainsAny(notifiedItems, 0u, 0u));

	Ops::MarkPair(notifiedItems, 0u, 0u);
	assert(notifiedItems.empty());

	Ops::MarkPair(notifiedItems, 0x01020304u, 0x05060708u);
	assert(Ops::ContainsAny(notifiedItems, 0x01020304u, 0u));
	assert(Ops::ContainsAny(notifiedItems, 0u, 0x05060708u));
	assert(Ops::Count(notifiedItems) == 2);

	auto snapshot = Ops::Snapshot(notifiedItems);
	assert(snapshot == notifiedItems);

	Ops::ReplaceAll(notifiedItems, std::unordered_set<std::uint32_t>{ 0xDEADBEEFu });
	assert(Ops::Count(notifiedItems) == 1);
	assert(Ops::ContainsAny(notifiedItems, 0xDEADBEEFu, 0u));

	return 0;
}
