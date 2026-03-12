#pragma once

#include "CodexOfPowerNG/BuildTypes.h"

#include <RE/Skyrim.h>

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::Builds
{
	struct BuildRuntimeSnapshot
	{
		std::uint32_t                                    attackScore{ 0 };
		std::uint32_t                                    defenseScore{ 0 };
		std::uint32_t                                    utilityScore{ 0 };
		std::array<std::string, kBuildSlotCount> activeBuildSlots{};
	};

	[[nodiscard]] std::vector<std::pair<RE::ActorValue, float>> ComputeDerivedBuildActorValueTotals(
		const BuildRuntimeSnapshot& snapshot) noexcept;
	[[nodiscard]] float ClampBuildSyncDeltaForActorValue(
		RE::ActorValue av,
		float          currentActorValue,
		float          desiredDelta) noexcept;

	void SyncCurrentBuildEffectsToPlayer() noexcept;
	void ResetForLoad() noexcept;
}

namespace CodexOfPowerNG
{
	namespace BuildEffectRuntime = Builds;
}
