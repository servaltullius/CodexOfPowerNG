#pragma once

#include <cstdint>

namespace CodexOfPowerNG::Events
{
	enum class LootNotifyDecision : std::uint8_t
	{
		kSkipAlreadyNotified = 0,
		kSkipThrottle = 1,
		kNotify = 2,
	};

	[[nodiscard]] constexpr LootNotifyDecision DecideLootNotify(
		bool alreadyNotified,
		std::uint64_t nowMs,
		std::uint64_t lastNotifyMs,
		std::uint64_t throttleMs) noexcept
	{
		if (alreadyNotified) {
			return LootNotifyDecision::kSkipAlreadyNotified;
		}

		if (lastNotifyMs > 0 && nowMs < lastNotifyMs + throttleMs) {
			return LootNotifyDecision::kSkipThrottle;
		}

		return LootNotifyDecision::kNotify;
	}
}
