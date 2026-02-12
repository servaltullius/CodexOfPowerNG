#pragma once

#include <chrono>
#include <cstdint>

namespace CodexOfPowerNG
{
	[[nodiscard]] inline std::uint64_t NowMs() noexcept
	{
		using namespace std::chrono;
		return static_cast<std::uint64_t>(
			duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
	}
}
