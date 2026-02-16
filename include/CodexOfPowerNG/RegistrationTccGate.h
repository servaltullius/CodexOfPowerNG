#pragma once

#include <cstdint>

namespace CodexOfPowerNG::Registration
{
	enum class TccGateDecision : std::uint8_t
	{
		kAllow = 0,
		kBlockNotDisplayed = 1,
		kBlockUnavailable = 2,
	};

	[[nodiscard]] constexpr TccGateDecision DecideTccGate(
		bool requireTccDisplayed,
		bool hasMasterList,
		bool hasDisplayedList,
		bool trackedByLotd,
		bool isDisplayed) noexcept
	{
		if (!requireTccDisplayed) {
			return TccGateDecision::kAllow;
		}

		if (!hasMasterList || !hasDisplayedList) {
			return TccGateDecision::kBlockUnavailable;
		}

		if (!trackedByLotd) {
			return TccGateDecision::kAllow;
		}

		return isDisplayed ? TccGateDecision::kAllow : TccGateDecision::kBlockNotDisplayed;
	}
}
