#include "RegistrationInternal.h"

#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/RegistrationStateStore.h"

#include <RE/Skyrim.h>

namespace CodexOfPowerNG::Registration::Internal
{
	std::string BestItemName(const RE::TESForm* primary, const RE::TESForm* fallback)
	{
		if (primary) {
			if (auto* name = primary->GetName(); name && name[0] != '\0') {
				return name;
			}
		}
		if (fallback) {
			if (auto* name = fallback->GetName(); name && name[0] != '\0') {
				return name;
			}
		}
		return {};
	}

	bool IsRegisteredForm(const RE::TESForm* item) noexcept
	{
		if (!item) {
			return false;
		}

		auto* regKey = GetRegisterKey(item);
		if (!regKey) {
			return false;
		}

		return RegistrationStateStore::IsRegisteredEither(regKey->GetFormID(), item->GetFormID());
	}

	std::uint32_t ClampGroup(std::uint32_t group, const RE::TESForm* item) noexcept
	{
		if (group <= 5) {
			return group;
		}

		const auto computed = GetDiscoveryGroup(item);
		if (computed <= 5) {
			return computed;
		}

		return 5;
	}
}
