#include "RegistrationInternal.h"

#include <SKSE/Logger.h>

#include <atomic>

namespace CodexOfPowerNG::Registration::Internal
{
	namespace
	{
		std::atomic_bool g_tccListsAvailable{ false };
		std::atomic_bool g_tccListsKnown{ false };

		[[nodiscard]] bool ListHasEitherForm(
			const RE::BGSListForm* list,
			const RE::TESForm* first,
			const RE::TESForm* second) noexcept
		{
			if (!list) {
				return false;
			}

			if (first && list->HasForm(first)) {
				return true;
			}

			if (second && second != first && list->HasForm(second)) {
				return true;
			}

			return false;
		}
	}

	void WarnMissingTccListsOnce() noexcept
	{
		static std::atomic_bool warned{ false };
		bool                    expected = false;
		if (!warned.compare_exchange_strong(expected, true, std::memory_order_relaxed)) {
			return;
		}

		SKSE::log::warn(
			"registration.requireTccDisplayed is enabled, but TCC FormLists (dbmMaster/dbmDisp) were not found. "
			"Registration is now fail-closed until TCC lists are available.");
	}

	TccLists ResolveTccLists() noexcept
	{
		TccLists lists{};
		lists.master = RE::TESForm::LookupByEditorID<RE::BGSListForm>("dbmMaster");
		lists.displayed = RE::TESForm::LookupByEditorID<RE::BGSListForm>("dbmDisp");

		const bool available = lists.master != nullptr && lists.displayed != nullptr;
		g_tccListsAvailable.store(available, std::memory_order_relaxed);
		g_tccListsKnown.store(true, std::memory_order_relaxed);
		return lists;
	}

	TccGateDecision EvaluateTccGate(
		const Settings& settings,
		const TccLists& lists,
		const RE::TESForm* item,
		const RE::TESForm* regKey) noexcept
	{
		const bool trackedByLotd = ListHasEitherForm(lists.master, regKey, item);
		const bool displayed = ListHasEitherForm(lists.displayed, regKey, item);
		return DecideTccGate(
			settings.requireTccDisplayed,
			lists.master != nullptr,
			lists.displayed != nullptr,
			trackedByLotd,
			displayed);
	}

	bool IsTccDisplayedListsAvailable() noexcept
	{
		if (!g_tccListsKnown.load(std::memory_order_relaxed)) {
			(void)ResolveTccLists();
		}
		return g_tccListsAvailable.load(std::memory_order_relaxed);
	}
}
