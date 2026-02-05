#include "CodexOfPowerNG/EventsNotifyGate.h"

#include <cassert>
#include <cstdint>

int main()
{
	using CodexOfPowerNG::Events::LootNotifyDecision;
	using CodexOfPowerNG::Events::DecideLootNotify;

	constexpr std::uint64_t kThrottleMs = 750;

	assert(DecideLootNotify(true, 2000, 1000, kThrottleMs) == LootNotifyDecision::kSkipAlreadyNotified);
	assert(DecideLootNotify(false, 1600, 1000, kThrottleMs) == LootNotifyDecision::kSkipThrottle);
	assert(DecideLootNotify(false, 1749, 1000, kThrottleMs) == LootNotifyDecision::kSkipThrottle);  // boundary -1
	assert(DecideLootNotify(false, 1750, 1000, kThrottleMs) == LootNotifyDecision::kNotify);         // boundary exact
	assert(DecideLootNotify(false, 1751, 1000, kThrottleMs) == LootNotifyDecision::kNotify);         // boundary +1
	assert(DecideLootNotify(false, 2500, 0, kThrottleMs) == LootNotifyDecision::kNotify);

	return 0;
}
