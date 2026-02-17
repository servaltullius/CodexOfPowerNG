#pragma once

#include <RE/Skyrim.h>

#include <cstdint>
#include <string_view>

namespace CodexOfPowerNG::Rewards::Internal
{
	[[nodiscard]] int   RandomInt(int minInclusive, int maxInclusive) noexcept;
	[[nodiscard]] float RewardMult() noexcept;

	void RecordRewardDelta(RE::ActorValue av, float delta) noexcept;
	void GrantReward(
		RE::ActorValue av,
		float amount,
		std::string_view labelKey,
		std::string_view fallbackLabel) noexcept;
	void GrantWeightedRandomReward(std::uint32_t group) noexcept;
}
