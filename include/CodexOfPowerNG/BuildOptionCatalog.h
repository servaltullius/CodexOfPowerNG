#pragma once

#include "CodexOfPowerNG/BuildTypes.h"

#include <array>
#include <span>

namespace CodexOfPowerNG::Builds
{
	struct BuildResolvedEffectPart
	{
		BuildEffectType effectType{ BuildEffectType::ActorValue };
		std::string_view effectKey{};
		BuildMagnitude magnitude{ 0.0f };
	};

	struct BuildResolvedEffectBundle
	{
		std::array<BuildResolvedEffectPart, 2> parts{};
		std::size_t count{ 0u };
	};

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
	[[nodiscard]] BuildResolvedEffectBundle GetResolvedBuildEffectBundle(
		const BuildOptionDef& option,
		BuildPointCenti       pointsCenti) noexcept;
	[[nodiscard]] BuildResolvedEffectBundle GetNextTierResolvedBuildEffectBundle(
		const BuildOptionDef& option,
		BuildPointCenti       pointsCenti) noexcept;
}
