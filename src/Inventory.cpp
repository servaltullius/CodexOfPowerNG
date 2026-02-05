#include "CodexOfPowerNG/Inventory.h"

#include <RE/Skyrim.h>

namespace CodexOfPowerNG::Inventory
{
	RemoveSelection SelectSafeRemoval(const RE::InventoryEntryData* entry, std::int32_t totalCount, bool protectFavorites) noexcept
	{
		RemoveSelection out{};
		if (!entry || totalCount <= 0) {
			return out;
		}

		out.isQuestObject = entry->IsQuestObject();

		std::int32_t extraCountTotal = 0;
		if (entry->extraLists) {
			for (auto* xList : *entry->extraLists) {
				if (!xList) {
					continue;
				}

				const auto count = xList->GetCount();
				if (count > 0) {
					extraCountTotal += count;
				}

				const bool worn = xList->HasType<RE::ExtraWorn>() || xList->HasType<RE::ExtraWornLeft>();
				const bool hotkey = protectFavorites && xList->HasType<RE::ExtraHotkey>();
				if (worn || hotkey) {
					continue;
				}

				if (count > 0) {
					if (!out.extraList) {
						out.extraList = xList;
					}
					out.safeCount += count;
				}
			}
		}

		const std::int32_t baseCount = totalCount - extraCountTotal;
		if (baseCount > 0) {
			out.baseCount = baseCount;
			out.safeCount += baseCount;
		}

		// If no safe ExtraDataList exists but base items exist, prefer base stack (nullptr extraList).
		if (!out.extraList && out.baseCount > 0) {
			out.extraList = nullptr;
		}

		return out;
	}
}
