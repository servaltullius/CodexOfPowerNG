#include "CodexOfPowerNG/Registration.h"

#include "CodexOfPowerNG/BuildEffectRuntime.h"
#include "CodexOfPowerNG/BuildProgression.h"
#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/Inventory.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/RegistrationQuestGuard.h"
#include "CodexOfPowerNG/RegistrationStateStore.h"
#include "CodexOfPowerNG/SerializationStateStore.h"
#include "CodexOfPowerNG/Util.h"

#include "RegistrationInternal.h"
#include "RegistrationQuickListBuilder.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::Registration
{
	namespace
	{
		inline constexpr std::uint64_t kQuickListCacheTtlMs = 750;

		struct QuickListCache
		{
			std::vector<ListItem> allEligible{};
			std::uint64_t generation{ 0 };
			std::uint64_t builtAtMs{ 0 };
			std::uint32_t settingsMask{ 0 };
		};

		std::mutex                g_quickListCacheMutex;
		QuickListCache            g_quickListCache{};
		std::atomic<std::uint64_t> g_quickListGeneration{ 1 };

		[[nodiscard]] std::uint32_t BuildQuickListSettingsMask(const Settings& settings) noexcept
		{
			std::uint32_t mask = 0;
			mask |= settings.normalizeRegistration ? (1u << 0) : 0u;
			mask |= settings.requireTccDisplayed ? (1u << 1) : 0u;
			mask |= settings.protectFavorites ? (1u << 2) : 0u;
			return mask;
		}

		void FillQuickListPage(
			const std::vector<ListItem>& allEligible,
			std::size_t offset,
			std::size_t limit,
			QuickRegisterList& result)
		{
			const auto totalEligible = allEligible.size();
			const auto clampedOffset = (std::min)(offset, totalEligible);
			const auto remaining = totalEligible - clampedOffset;
			const auto pageSize = (std::min)(limit, remaining);

			result.total = totalEligible;
			result.hasMore = (clampedOffset + pageSize) < totalEligible;
			result.items.assign(
				allEligible.begin() + static_cast<std::ptrdiff_t>(clampedOffset),
				allEligible.begin() + static_cast<std::ptrdiff_t>(clampedOffset + pageSize));
		}

		[[nodiscard]] bool TryBuildQuickListFromCache(
			std::size_t offset,
			std::size_t limit,
			std::uint64_t generation,
			std::uint32_t settingsMask,
			QuickRegisterList& result) noexcept
		{
			std::scoped_lock lock(g_quickListCacheMutex);
			if (g_quickListCache.generation != generation) {
				return false;
			}
			if (g_quickListCache.settingsMask != settingsMask) {
				return false;
			}
			const auto nowMs = NowMs();
			if (nowMs < g_quickListCache.builtAtMs || (nowMs - g_quickListCache.builtAtMs) > kQuickListCacheTtlMs) {
				return false;
			}

			FillQuickListPage(g_quickListCache.allEligible, offset, limit, result);
			return true;
		}

		void UpdateQuickListCache(
			std::vector<ListItem> allEligible,
			std::uint64_t generation,
			std::uint32_t settingsMask,
			std::uint64_t builtAtMs) noexcept
		{
			std::scoped_lock lock(g_quickListCacheMutex);
			g_quickListCache.allEligible = std::move(allEligible);
			g_quickListCache.generation = generation;
			g_quickListCache.settingsMask = settingsMask;
			g_quickListCache.builtAtMs = builtAtMs;
		}

		void ClearQuickListCacheStorage() noexcept
		{
			std::scoped_lock lock(g_quickListCacheMutex);
			g_quickListCache = QuickListCache{};
		}
	}

	void InvalidateQuickRegisterCache() noexcept
	{
		(void)g_quickListGeneration.fetch_add(1, std::memory_order_acq_rel);
		ClearQuickListCacheStorage();
	}

	QuickRegisterList BuildQuickRegisterList(std::size_t offset, std::size_t limit)
	{
		QuickRegisterList result{};

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return result;
		}

		const auto settings = GetSettings();
		const auto settingsMask = BuildQuickListSettingsMask(settings);
		const auto cacheGeneration = g_quickListGeneration.load(std::memory_order_acquire);
		if (TryBuildQuickListFromCache(offset, limit, cacheGeneration, settingsMask, result)) {
			return result;
		}

		const auto tccLists = Internal::ResolveTccLists();
		if (settings.requireTccDisplayed && (!tccLists.master || !tccLists.displayed)) {
			Internal::WarnMissingTccListsOnce();
		}

		Internal::EnsureMapsLoaded();

		const auto questProtected = QuestGuard::SnapshotProtectedForms();
		const auto quickListState = RegistrationStateStore::SnapshotQuickList();
		auto allEligible = Internal::BuildQuickListEligibleItems(
			*player,
			settings,
			tccLists,
			quickListState,
			questProtected);

		// Phase 2: paginate and update short-lived cache
		FillQuickListPage(allEligible, offset, limit, result);
		UpdateQuickListCache(std::move(allEligible), cacheGeneration, settingsMask, NowMs());

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

		if (QuestGuard::IsQuestProtected(item->GetFormID()) ||
			(regKey != item && QuestGuard::IsQuestProtected(regKey->GetFormID()))) {
			result.message = L10n::T("msg.registerQuestItem",
				"Codex of Power: Cannot register (active quest item)");
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

		if (foundEntry->IsQuestObject() ||
			QuestGuard::IsQuestProtected(item->GetFormID()) ||
			(regKey != item && QuestGuard::IsQuestProtected(regKey->GetFormID()))) {
			result.message = L10n::T("msg.registerQuestItem",
				"Codex of Power: Cannot register (active quest item)");
			RE::DebugNotification(result.message.c_str());
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
		const auto buildContribution = BuildProgression::MakeRegistrationContribution(group, regKey->GetFormType());
		if (buildContribution.has_value()) {
			(void)BuildProgression::ApplyRegistrationContribution(buildContribution.value());
			BuildEffectRuntime::SyncCurrentBuildEffectsToPlayer();
		}

		UndoRecord undoRecord{};
		undoRecord.formId = item->GetFormID();
		undoRecord.regKey = regKey->GetFormID();
		undoRecord.group = group;
		undoRecord.buildContribution = buildContribution;
		(void)RegistrationStateStore::PushUndoRecord(std::move(undoRecord));
		InvalidateQuickRegisterCache();

		result.success = true;
		result.message = msg;
		result.regKey = regKey->GetFormID();
		result.group = group;
		result.totalRegistered = totalRegistered;
		return result;
	}
}
