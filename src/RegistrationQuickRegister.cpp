#include "CodexOfPowerNG/Registration.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/Inventory.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/RegistrationRules.h"
#include "CodexOfPowerNG/RegistrationStateStore.h"
#include "CodexOfPowerNG/Rewards.h"

#include "RegistrationInternal.h"

#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::Registration
{
	QuickRegisterList BuildQuickRegisterList(std::size_t offset, std::size_t limit)
	{
		QuickRegisterList result{};

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return result;
		}

		const auto settings = GetSettings();
		const auto tccLists = Internal::ResolveTccLists();
		if (settings.requireTccDisplayed && (!tccLists.master || !tccLists.displayed)) {
			Internal::WarnMissingTccListsOnce();
		}

		Internal::EnsureMapsLoaded();

		const auto quickListState = RegistrationStateStore::SnapshotQuickList();
		const auto& blocked = quickListState.blockedItems;
		const auto& registered = quickListState.registeredKeys;

		const auto isExcludedFast = [&](const RE::TESForm* item) noexcept -> bool {
			if (!item) {
				return true;
			}

			const auto id = item->GetFormID();
			if (Internal::IsExcludedByMap(id) || blocked.contains(id)) {
				return true;
			}

			if (RegistrationRules::IsIntrinsicExcluded(item)) {
				return true;
			}

			return false;
		};

		const auto getDiscoveryGroupFast = [&](const RE::TESForm* item) noexcept -> std::uint32_t {
			if (!item) {
				return 255;
			}

			if (isExcludedFast(item)) {
				return 255;
			}

			return RegistrationRules::GroupFromFormType(item->GetFormType());
		};

		// Phase 1: collect ALL eligible items (stable pagination requires full scan)
		std::vector<ListItem> allEligible;
		std::unordered_set<RE::FormID> seenRegKeys;

		if (auto* changes = player->GetInventoryChanges(); changes && changes->entryList) {
			for (auto* entry : *changes->entryList) {
				if (!entry) {
					continue;
				}

				auto* obj = entry->GetObject();
				if (!obj) {
					continue;
				}

				auto* regKey = Internal::GetRegisterKey(obj, settings);
				if (!regKey) {
					continue;
				}

				const auto group = getDiscoveryGroupFast(regKey);
				if (group > 5) {
					continue;
				}

				if (isExcludedFast(obj) || (regKey != obj && isExcludedFast(regKey))) {
					continue;
				}

				if (Internal::EvaluateTccGate(settings, tccLists, obj, regKey) != TccGateDecision::kAllow) {
					continue;
				}

				const auto regKeyId = regKey->GetFormID();
				const auto objId = obj->GetFormID();
				if (registered.contains(regKeyId) || registered.contains(objId)) {
					continue;
				}

				if (!seenRegKeys.insert(regKeyId).second) {
					continue;
				}

				const auto totalCount = player->GetItemCount(obj);
				if (totalCount <= 0) {
					continue;
				}

				const auto removal =
					Inventory::SelectSafeRemoval(entry, totalCount, settings.protectFavorites);
				const auto safeCount = removal.safeCount;
				if (safeCount <= 0) {
					continue;
				}

				ListItem li{};
				li.formId = objId;
				li.regKey = regKeyId;
				li.group = group;
				li.totalCount = totalCount;
				li.safeCount = safeCount;
				li.excluded = false;
				li.registered = false;
				li.blocked = false;
				li.name = Internal::BestItemName(regKey, obj);
				if (li.name.empty()) {
					li.name = L10n::T("ui.unnamed", "(unnamed)");
				}

				allEligible.push_back(std::move(li));
			}
		}

		// Phase 2: sort ALL eligible items for stable ordering
		std::sort(allEligible.begin(), allEligible.end(), [](const ListItem& a, const ListItem& b) {
			if (a.group != b.group) {
				return a.group < b.group;
			}
			return a.name < b.name;
		});

		// Phase 3: paginate from sorted result
		const auto totalEligible = allEligible.size();
		const auto clampedOffset = (std::min)(offset, totalEligible);
		const auto remaining = totalEligible - clampedOffset;
		const auto pageSize = (std::min)(limit, remaining);

		result.total = totalEligible;
		result.hasMore = (clampedOffset + pageSize) < totalEligible;
		result.items.assign(
			std::make_move_iterator(allEligible.begin() + static_cast<std::ptrdiff_t>(clampedOffset)),
			std::make_move_iterator(allEligible.begin() + static_cast<std::ptrdiff_t>(clampedOffset + pageSize)));

		return result;
	}

	RegisterResult TryRegisterItem(RE::FormID formId)
	{
		RegisterResult result{};

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			result.message = "Player unavailable";
			return result;
		}

		auto* item = RE::TESForm::LookupByID<RE::TESBoundObject>(formId);
		if (!item) {
			result.message = "Invalid FormID";
			return result;
		}

		const auto settings = GetSettings();
		const auto tccLists = Internal::ResolveTccLists();
		if (settings.requireTccDisplayed && (!tccLists.master || !tccLists.displayed)) {
			Internal::WarnMissingTccListsOnce();
		}

		auto* regKey = Internal::GetRegisterKey(item, settings);
		if (!regKey) {
			result.message = "Invalid register key";
			return result;
		}

		if (Internal::IsExcludedOrBlocked(item, regKey)) {
			result.message = L10n::T("msg.registerExcluded", "Codex of Power: Cannot register (excluded item)");
			RE::DebugNotification(result.message.c_str());
			return result;
		}

		if (RegistrationStateStore::IsRegisteredEither(regKey->GetFormID(), item->GetFormID())) {
			result.message = "Already registered";
			return result;
		}

		const auto group = GetDiscoveryGroup(regKey);
		if (group > 5) {
			result.message = "Not discoverable";
			return result;
		}

		const auto tccGate = Internal::EvaluateTccGate(settings, tccLists, item, regKey);
		if (tccGate == TccGateDecision::kBlockNotDisplayed) {
			result.message = L10n::T("msg.registerLotdNotDisplayed", "Codex of Power: Cannot register (LOTD item not displayed yet)");
			RE::DebugNotification(result.message.c_str());
			return result;
		}
		if (tccGate == TccGateDecision::kBlockUnavailable) {
			result.message = L10n::T(
				"msg.registerLotdGateUnavailable",
				"Codex of Power: Cannot register (LOTD gate unavailable; check TCC install/load order)");
			RE::DebugNotification(result.message.c_str());
			return result;
		}

		RE::InventoryEntryData* foundEntry = nullptr;
		if (auto* changes = player->GetInventoryChanges(); changes && changes->entryList) {
			for (auto* entry : *changes->entryList) {
				if (!entry) {
					continue;
				}

				if (entry->GetObject() == item) {
					foundEntry = entry;
					break;
				}
			}
		}

		if (!foundEntry) {
			result.message = "Not in inventory";
			return result;
		}

		const auto totalCount = player->GetItemCount(item);
		const auto removal = Inventory::SelectSafeRemoval(foundEntry, totalCount, settings.protectFavorites);
		const auto displayName = Internal::BestItemName(regKey, item);
		if (displayName.empty()) {
			RegistrationStateStore::BlockPair(regKey->GetFormID(), item->GetFormID());

			result.message = L10n::T("msg.registerUnnamed", "Codex of Power: Cannot register (unnamed item)");
			RE::DebugNotification(result.message.c_str());
			return result;
		}

		if (removal.safeCount <= 0) {
			result.message = L10n::T("msg.registerProtected", "Codex of Power: Cannot register (equipped/favorited)");
			RE::DebugNotification(result.message.c_str());
			return result;
		}

		const auto oldHave = player->GetItemCount(item);

		player->RemoveItem(item, 1, RE::ITEM_REMOVE_REASON::kRemove, removal.extraList, nullptr);

		const auto newHave = player->GetItemCount(item);
		if (newHave >= oldHave) {
			RegistrationStateStore::BlockPair(regKey->GetFormID(), item->GetFormID());

			result.message = L10n::T("msg.registerCantDestroy", "Codex of Power: Cannot register (cannot destroy item)");
			RE::DebugNotification(result.message.c_str());
			return result;
		}

		const auto totalRegistered = RegistrationStateStore::InsertRegistered(regKey->GetFormID(), group);

		const auto msg =
			L10n::T("msg.registerOkPrefix", "Registered: ") + displayName +
			" (" + GetDiscoveryGroupName(group) + ", " +
			L10n::T("msg.totalPrefix", "total ") + std::to_string(totalRegistered) +
			L10n::T("msg.totalSuffix", " items") + ")";
		RE::DebugNotification(msg.c_str());

		Rewards::MaybeGrantRegistrationReward(group, static_cast<std::int32_t>(totalRegistered));

		result.success = true;
		result.message = msg;
		result.regKey = regKey->GetFormID();
		result.group = group;
		result.totalRegistered = totalRegistered;
		return result;
	}
}
