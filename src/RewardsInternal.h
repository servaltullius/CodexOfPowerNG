#pragma once

#include "CodexOfPowerNG/RegistrationUndoTypes.h"

#include <RE/Skyrim.h>

#include <cstdint>
#include <string_view>
#include <vector>

namespace CodexOfPowerNG::Rewards::Internal
{
	[[nodiscard]] int   RandomInt(int minInclusive, int maxInclusive) noexcept;
	[[nodiscard]] float RewardMult() noexcept;

	[[nodiscard]] float RecordRewardDelta(RE::ActorValue av, float delta) noexcept;
	void BeginRewardDeltaCapture(std::vector<Registration::RewardDelta>& outDeltas) noexcept;
	void EndRewardDeltaCapture() noexcept;
	void CaptureAppliedRewardDelta(RE::ActorValue av, float delta) noexcept;
	void GrantReward(
		RE::ActorValue av,
		float amount,
		std::string_view labelKey,
		std::string_view fallbackLabel) noexcept;
	void GrantWeightedRandomReward(std::uint32_t group) noexcept;
}
