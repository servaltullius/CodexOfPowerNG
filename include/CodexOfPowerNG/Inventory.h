#pragma once

#include <RE/Skyrim.h>

#include <cstdint>

namespace CodexOfPowerNG::Inventory
{
	struct RemoveSelection
	{
		// null means the "base stack" (no extra data list)
		RE::ExtraDataList* extraList{ nullptr };
		std::int32_t       safeCount{ 0 };
		std::int32_t       baseCount{ 0 };
	};

	// Computes a safe removal target for an inventory entry (never removes worn items; optionally protects hotkey items).
	[[nodiscard]] RemoveSelection SelectSafeRemoval(const RE::InventoryEntryData* entry, std::int32_t totalCount, bool protectFavorites) noexcept;
}

