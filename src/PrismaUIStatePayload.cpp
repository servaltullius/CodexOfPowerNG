#include "PrismaUIInternal.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/Registration.h"
#include "CodexOfPowerNG/State.h"

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	json BuildRuntimeStatePayload()
	{
		std::size_t registeredCount = 0;
		std::size_t rewardCount = 0;
		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			registeredCount = state.registeredItems.size();
			rewardCount = state.rewardTotals.size();
		}

		// Avoid querying PrismaUI view state on-demand here; Focus/Show timing can
		// conflict with internal view task processing and appear as a stall.
		const bool focused = IsViewFocused();
		const bool hidden = IsViewHidden();

		const auto settings = GetSettings();

		json j;
		j["ui"] = {
			{ "ready", IsDomReady() },
			{ "focused", focused },
			{ "hidden", hidden },
		};
		j["registeredCount"] = registeredCount;
		j["rewardCount"] = rewardCount;
		j["language"] = L10n::ActiveLanguage();
		j["toggleKeyCode"] = settings.toggleKeyCode;
		const bool tccListsAvailable = Registration::IsTccDisplayedListsAvailable();
		j["lotdGate"] = {
			{ "requireTccDisplayed", settings.requireTccDisplayed },
			{ "tccListsAvailable", tccListsAvailable },
			{ "blocking", settings.requireTccDisplayed && !tccListsAvailable },
		};
		return j;
	}
}
