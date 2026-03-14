#pragma once

#include "CodexOfPowerNG/BuildTypes.h"

#include <span>

namespace CodexOfPowerNG::Builds
{
	[[nodiscard]] std::span<const BuildOptionDef> GetBuildOptionCatalog() noexcept;
	[[nodiscard]] std::span<const BuildBaselineMilestoneDef> GetBuildBaselineMilestones() noexcept;
	[[nodiscard]] std::span<const BuildSlotId> GetInitialBuildSlotLayout() noexcept;
	[[nodiscard]] std::uint32_t GetBuildPointsTier(BuildPointCenti pointsCenti) noexcept;
	[[nodiscard]] BuildPointCenti GetNextBuildPointsThresholdCenti(BuildPointCenti pointsCenti) noexcept;
	[[nodiscard]] BuildPointCenti GetBuildPointsToNextTierCenti(BuildPointCenti pointsCenti) noexcept;
	[[nodiscard]] BuildMagnitude GetScaledBuildMagnitude(
		const BuildOptionDef& option,
		BuildPointCenti       pointsCenti) noexcept;
	[[nodiscard]] BuildMagnitude GetNextTierBuildMagnitude(
		const BuildOptionDef& option,
		BuildPointCenti       pointsCenti) noexcept;
}
