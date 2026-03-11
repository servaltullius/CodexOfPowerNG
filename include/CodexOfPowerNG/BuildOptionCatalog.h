#pragma once

#include "CodexOfPowerNG/BuildTypes.h"

#include <span>

namespace CodexOfPowerNG::Builds
{
	[[nodiscard]] std::span<const BuildOptionDef> GetBuildOptionCatalog() noexcept;
	[[nodiscard]] std::span<const BuildBaselineMilestoneDef> GetBuildBaselineMilestones() noexcept;
	[[nodiscard]] std::span<const BuildSlotId> GetInitialBuildSlotLayout() noexcept;
}
