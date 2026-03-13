#pragma once

#include "CodexOfPowerNG/BuildTypes.h"

#include <span>

namespace CodexOfPowerNG::Builds
{
	[[nodiscard]] std::span<const BuildOptionDef> GetBuildOptionCatalog() noexcept;
	[[nodiscard]] std::span<const BuildBaselineMilestoneDef> GetBuildBaselineMilestones() noexcept;
	[[nodiscard]] std::span<const BuildSlotId> GetInitialBuildSlotLayout() noexcept;
	[[nodiscard]] std::uint32_t GetBuildScalingTier(std::uint32_t score) noexcept;
	[[nodiscard]] std::uint32_t GetNextBuildScalingScore(std::uint32_t score) noexcept;
	[[nodiscard]] std::uint32_t GetScoreToNextBuildScalingTier(std::uint32_t score) noexcept;
	[[nodiscard]] BuildMagnitude GetScaledBuildMagnitude(
		const BuildOptionDef& option,
		std::uint32_t         score) noexcept;
	[[nodiscard]] BuildMagnitude GetNextTierBuildMagnitude(
		const BuildOptionDef& option,
		std::uint32_t         score) noexcept;
}
