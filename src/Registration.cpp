#include "CodexOfPowerNG/Registration.h"

#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/RegistrationRules.h"
#include "CodexOfPowerNG/RegistrationStateStore.h"

#include "RegistrationInternal.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::Registration
{
	std::uint32_t GetDiscoveryGroup(const RE::TESForm* item) noexcept
	{
		return RegistrationRules::DiscoveryGroupFor(item, Internal::IsExcludedForm(item));
	}

	std::string GetDiscoveryGroupName(std::uint32_t group)
	{
		switch (group) {
		case 0:
			return L10n::T("group.weapons", "Weapons");
		case 1:
			return L10n::T("group.armors", "Armors");
		case 2:
			return L10n::T("group.consumables", "Consumables");
		case 3:
			return L10n::T("group.ingredients", "Ingredients");
		case 4:
			return L10n::T("group.books", "Books");
		default:
			return L10n::T("group.misc", "Misc");
		}
	}

	RE::FormID GetRegisterKeyId(RE::FormID formId) noexcept
	{
		auto* form = RE::TESForm::LookupByID(formId);
		auto* regKey = Internal::GetRegisterKey(form);
		return regKey ? regKey->GetFormID() : 0;
	}

	bool IsRegistered(RE::FormID formId) noexcept
	{
		auto* form = RE::TESForm::LookupByID(formId);
		return Internal::IsRegisteredForm(form);
	}

	bool IsExcluded(RE::FormID formId) noexcept
	{
		auto* form = RE::TESForm::LookupByID(formId);
		if (!form) {
			return true;
		}

		auto* regKey = Internal::GetRegisterKey(form);
		return Internal::IsExcludedOrBlocked(form, regKey);
	}

	bool IsDiscoverable(RE::FormID formId) noexcept
	{
		auto* form = RE::TESForm::LookupByID(formId);
		if (!form) {
			return false;
		}

		auto* regKey = Internal::GetRegisterKey(form);
		if (!regKey) {
			return false;
		}

		if (Internal::IsExcludedOrBlocked(form, regKey)) {
			return false;
		}

		return GetDiscoveryGroup(regKey) <= 5;
	}

	std::vector<ListItem> BuildRegisteredList()
	{
		auto ids = RegistrationStateStore::SnapshotRegisteredItems();

		std::vector<ListItem> out;
		out.reserve(ids.size());

		for (const auto& [id, storedGroup] : ids) {
			auto* form = RE::TESForm::LookupByID(id);
			if (!form) {
				continue;
			}

			ListItem li{};
			li.formId = id;
			li.regKey = id;
			li.group = Internal::ClampGroup(storedGroup, form);
			li.name = Internal::BestItemName(form, nullptr);
			if (li.name.empty()) {
				li.name = L10n::T("ui.unnamed", "(unnamed)");
			}
			out.push_back(std::move(li));
		}

		std::sort(out.begin(), out.end(), [](const ListItem& a, const ListItem& b) {
			if (a.group != b.group) {
				return a.group < b.group;
			}
			return a.name < b.name;
		});

		return out;
	}

	bool IsTccDisplayedListsAvailable() noexcept
	{
		return Internal::IsTccDisplayedListsAvailable();
	}

	void Warmup() noexcept
	{
		Internal::EnsureMapsLoaded();
		(void)Internal::ResolveTccLists();
	}
}
