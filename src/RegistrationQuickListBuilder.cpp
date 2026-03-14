#include "RegistrationQuickListBuilder.h"

#include "CodexOfPowerNG/BuildProgression.h"
#include "CodexOfPowerNG/Inventory.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/RegistrationRules.h"

#include <algorithm>
#include <unordered_set>
#include <utility>

namespace CodexOfPowerNG::Registration::Internal
{
	namespace
	{
		[[nodiscard]] std::string DetermineDisabledReason(
			bool questProtected,
			TccGateDecision tccGate,
			const Inventory::RemoveSelection& removal) noexcept
		{
			if (questProtected) {
				return "quest_protected";
			}
			if (tccGate != TccGateDecision::kAllow) {
				return "not_actionable";
			}
			if (removal.safeCount > 0) {
				return {};
			}
			if (removal.blockedByFavorite) {
				return "favorite_protected";
			}
			return "not_actionable";
		}

		[[nodiscard]] bool IsExcludedFast(
			const RegistrationStateStore::QuickListSnapshot& quickListState,
			const RE::TESForm* item) noexcept
		{
			if (!item) {
				return true;
			}

			const auto id = item->GetFormID();
			if (Internal::IsExcludedByMap(id) || quickListState.blockedItems.contains(id)) {
				return true;
			}

			return RegistrationRules::IsIntrinsicExcluded(item);
		}

		[[nodiscard]] std::uint32_t GetDiscoveryGroupFast(
			const RegistrationStateStore::QuickListSnapshot& quickListState,
			const RE::TESForm* item) noexcept
		{
			if (!item || IsExcludedFast(quickListState, item)) {
				return 255;
			}

			return RegistrationRules::GroupFromFormType(item->GetFormType());
		}

		void AppendEligibleItem(
			RE::PlayerCharacter& player,
			const Settings& settings,
			const TccLists& tccLists,
			const RegistrationStateStore::QuickListSnapshot& quickListState,
			const std::unordered_set<RE::FormID>& questProtected,
			std::unordered_set<RE::FormID>& seenRegKeys,
			RE::InventoryEntryData& entry,
			std::vector<ListItem>& allEligible)
		{
			auto* obj = entry.GetObject();
			if (!obj) {
				return;
			}

			auto* regKey = Internal::GetRegisterKey(obj, settings);
			if (!regKey) {
				return;
			}

			const auto group = GetDiscoveryGroupFast(quickListState, regKey);
			if (group > 5) {
				return;
			}

			if (IsExcludedFast(quickListState, obj) || (regKey != obj && IsExcludedFast(quickListState, regKey))) {
				return;
			}

			const auto regKeyId = regKey->GetFormID();
			const auto objId = obj->GetFormID();

			if (quickListState.registeredKeys.contains(regKeyId) || quickListState.registeredKeys.contains(objId)) {
				return;
			}

			if (!seenRegKeys.insert(regKeyId).second) {
				return;
			}

			const auto totalCount = player.GetItemCount(obj);
			if (totalCount <= 0) {
				return;
			}

			const auto removal = Inventory::SelectSafeRemoval(&entry, totalCount, settings.protectFavorites);
			const bool isQuestProtected =
				entry.IsQuestObject() ||
				questProtected.contains(regKeyId) ||
				questProtected.contains(objId);
			const auto tccGate = Internal::EvaluateTccGate(settings, tccLists, obj, regKey);
			ListItem item{};
			item.formId = objId;
			item.regKey = regKeyId;
			item.group = group;
			item.totalCount = totalCount;
			item.safeCount = removal.safeCount;
			item.excluded = false;
			item.registered = false;
			item.blocked = false;
			item.buildPointsCenti = BuildProgression::ResolveBuildPointsForFormType(regKey->GetFormType());
			item.disabledReason = DetermineDisabledReason(isQuestProtected, tccGate, removal);
			if (!item.disabledReason.empty()) {
				item.safeCount = 0;
				item.blocked = true;
			}
			item.name = Internal::BestItemName(regKey, obj);
			if (item.name.empty()) {
				item.name = L10n::T("ui.unnamed", "(unnamed)");
			}

			allEligible.push_back(std::move(item));
		}
	}

	std::vector<ListItem> BuildQuickListEligibleItems(
		RE::PlayerCharacter& player,
		const Settings& settings,
		const TccLists& tccLists,
		const RegistrationStateStore::QuickListSnapshot& quickListState,
		const std::unordered_set<RE::FormID>& questProtected)
	{
		std::vector<ListItem>          allEligible;
		std::unordered_set<RE::FormID> seenRegKeys;

		if (auto* changes = player.GetInventoryChanges(); changes && changes->entryList) {
			for (auto* entry : *changes->entryList) {
				if (!entry) {
					continue;
				}

				AppendEligibleItem(
					player,
					settings,
					tccLists,
					quickListState,
					questProtected,
					seenRegKeys,
					*entry,
					allEligible);
			}
		}

		std::sort(allEligible.begin(), allEligible.end(), [](const ListItem& a, const ListItem& b) {
			if (a.group != b.group) {
				return a.group < b.group;
			}
			return a.name < b.name;
		});

		return allEligible;
	}
}
