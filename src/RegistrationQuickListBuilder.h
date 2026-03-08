#pragma once

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/RegistrationStateStore.h"

#include "RegistrationInternal.h"

#include <RE/Skyrim.h>

#include <unordered_set>
#include <vector>

namespace CodexOfPowerNG::Registration::Internal
{
	[[nodiscard]] std::vector<ListItem> BuildQuickListEligibleItems(
		RE::PlayerCharacter& player,
		const Settings& settings,
		const TccLists& tccLists,
		const RegistrationStateStore::QuickListSnapshot& quickListState,
		const std::unordered_set<RE::FormID>& questProtected);
}
