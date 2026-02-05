#include "CodexOfPowerNG/RegistrationRules.h"

namespace CodexOfPowerNG::RegistrationRules
{
	bool IsDragonClawName(std::string_view name) noexcept
	{
		if (name.empty()) {
			return false;
		}

		if (name.find("Dragon Claw") != std::string_view::npos || name.find("dragon claw") != std::string_view::npos) {
			return true;
		}
		// Korean (commonly "용발톱"); avoid matching generic "발톱" (e.g., bear claws ingredient).
		return name.find("용발톱") != std::string_view::npos;
	}

	bool IsIntrinsicExcluded(const RE::TESForm* item) noexcept
	{
		if (!item) {
			return true;
		}

		if (item->IsKey() || item->IsLockpick() || item->IsGold()) {
			return true;
		}

		const auto* name = item->GetName();
		return IsDragonClawName(name ? std::string_view(name) : std::string_view{});
	}

	std::uint32_t GroupFromFormType(RE::FormType formType) noexcept
	{
		switch (formType) {
		case RE::FormType::Weapon:
			return 0;
		case RE::FormType::Armor:
			return 1;
		case RE::FormType::AlchemyItem:
			return 2;
		case RE::FormType::Ingredient:
			return 3;
		case RE::FormType::Book:
			return 4;
		case RE::FormType::Ammo:
		case RE::FormType::Scroll:
		case RE::FormType::SoulGem:
		case RE::FormType::Misc:
			return 5;
		default:
			return kUnknownGroup;
		}
	}

	std::uint32_t DiscoveryGroupFor(const RE::TESForm* item, bool excluded) noexcept
	{
		if (!item || excluded) {
			return kUnknownGroup;
		}
		return GroupFromFormType(item->GetFormType());
	}

	bool IsValidVariantStep(RE::FormID currentId, RE::FormID nextId) noexcept
	{
		return nextId != 0 && nextId != currentId;
	}
}
