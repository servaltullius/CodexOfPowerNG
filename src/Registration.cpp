#include "CodexOfPowerNG/Registration.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/Inventory.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/RegistrationMaps.h"
#include "CodexOfPowerNG/RegistrationRules.h"
#include "CodexOfPowerNG/RegistrationStateStore.h"
#include "CodexOfPowerNG/Rewards.h"

#include <RE/Skyrim.h>

#include <RE/M/Misc.h>

#include <SKSE/Logger.h>

#include <algorithm>
#include <charconv>
#include <mutex>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::Registration
{
	namespace
	{
		std::mutex g_mapsMutex;
		bool       g_mapsLoaded{ false };

		std::unordered_set<RE::FormID>            g_excluded{};
		std::unordered_map<RE::FormID, RE::FormID> g_variantBase{};

		[[nodiscard]] std::optional<std::uint32_t> ParseUInt32(std::string_view text, int base) noexcept
		{
			std::uint32_t value{};
			auto* begin = text.data();
			auto* end = text.data() + text.size();
			auto [ptr, ec] = std::from_chars(begin, end, value, base);
			if (ec != std::errc{} || ptr != end) {
				return std::nullopt;
			}
			return value;
		}

		[[nodiscard]] std::optional<RE::FormID> ParseFormID(std::string_view text) noexcept
		{
			if (text.empty()) {
				return std::nullopt;
			}

			if (text.rfind("0x", 0) == 0 || text.rfind("0X", 0) == 0) {
				auto trimmed = text.substr(2);
				if (auto v = ParseUInt32(trimmed, 16)) {
					return static_cast<RE::FormID>(*v);
				}
				return std::nullopt;
			}

			if (auto v = ParseUInt32(text, 10)) {
				return static_cast<RE::FormID>(*v);
			}

			return std::nullopt;
		}

		[[nodiscard]] RE::FormID ToLocalFormID(RE::FormID anyID) noexcept
		{
			if (anyID == 0) {
				return 0;
			}

			if ((anyID & 0xFF000000u) == 0xFE000000u) {
				return anyID & 0x00000FFFu;
			}

			return anyID & 0x00FFFFFFu;
		}

		[[nodiscard]] std::optional<RE::FormID> LookupFormFromEntry(std::string_view fileName, std::string_view idStr) noexcept
		{
			auto idRawOpt = ParseFormID(idStr);
			if (!idRawOpt) {
				return std::nullopt;
			}

			const auto localID = ToLocalFormID(*idRawOpt);
			if (localID == 0) {
				return std::nullopt;
			}

			auto* data = RE::TESDataHandler::GetSingleton();
			if (!data) {
				return std::nullopt;
			}

			auto* form = data->LookupForm(localID, fileName);
			if (!form) {
				return std::nullopt;
			}

			return form->GetFormID();
		}

		void EnsureMapsLoaded() noexcept
		{
			std::scoped_lock lock(g_mapsMutex);
			if (g_mapsLoaded) {
				return;
			}
			g_mapsLoaded = true;

			g_excluded.clear();
			g_variantBase.clear();

			const RegistrationMaps::Paths paths{
				.excludeMapPath = kExcludeMapPath,
				.excludeUserPath = kExcludeUserPath,
				.pluginDataDir = kPluginDataDir,
				.variantMapPath = kVariantMapPath,
				.excludePatchMax = kExcludePatchMax
			};
			auto loaded = RegistrationMaps::LoadFromDisk(paths, LookupFormFromEntry);
			g_excluded = std::move(loaded.excluded);
			g_variantBase = std::move(loaded.variantBase);

			SKSE::log::info("Exclude map loaded: {} forms", g_excluded.size());
			SKSE::log::info("Variant map loaded: {} mappings", g_variantBase.size());
		}

		[[nodiscard]] std::string BestItemName(const RE::TESForm* primary, const RE::TESForm* fallback)
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

		[[nodiscard]] RE::TESForm* GetRegisterKey(const RE::TESForm* item, const Settings& settings) noexcept
		{
			if (!item) {
				return nullptr;
			}

			if (!settings.normalizeRegistration) {
				return const_cast<RE::TESForm*>(item);
			}

			RE::TESForm* regKey = const_cast<RE::TESForm*>(item);

			if (auto* weap = regKey->As<RE::TESObjectWEAP>(); weap && weap->templateWeapon) {
				regKey = weap->templateWeapon;
			}

			EnsureMapsLoaded();

			for (int safety = 0; safety < 8; ++safety) {
				const auto it = g_variantBase.find(regKey->GetFormID());
				if (it == g_variantBase.end()) {
					break;
				}

				const auto baseID = it->second;
				if (!RegistrationRules::IsValidVariantStep(regKey->GetFormID(), baseID)) {
					break;
				}

				auto* baseForm = RE::TESForm::LookupByID(baseID);
				if (!baseForm || baseForm == regKey) {
					break;
				}

				regKey = baseForm;
			}

			return regKey;
		}

		[[nodiscard]] RE::TESForm* GetRegisterKey(const RE::TESForm* item) noexcept
		{
			const auto settings = GetSettings();
			return GetRegisterKey(item, settings);
		}

		[[nodiscard]] bool IsExcludedForm(const RE::TESForm* item) noexcept
		{
			if (!item) {
				return true;
			}

			EnsureMapsLoaded();

			const auto id = item->GetFormID();

			if (g_excluded.contains(id)) {
				return true;
			}

			if (RegistrationStateStore::IsBlocked(id)) {
				return true;
			}

			if (RegistrationRules::IsIntrinsicExcluded(item)) {
				return true;
			}

			return false;
		}

		[[nodiscard]] bool IsRegisteredForm(const RE::TESForm* item) noexcept
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

		[[nodiscard]] std::uint32_t ClampGroup(std::uint32_t group, const RE::TESForm* item) noexcept
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

		[[nodiscard]] bool IsExcludedOrBlocked(const RE::TESForm* item, const RE::TESForm* regKey) noexcept
		{
			if (IsExcludedForm(item)) {
				return true;
			}
			if (regKey && regKey != item && IsExcludedForm(regKey)) {
				return true;
			}
			return false;
		}
	}

	std::uint32_t GetDiscoveryGroup(const RE::TESForm* item) noexcept
	{
		return RegistrationRules::DiscoveryGroupFor(item, IsExcludedForm(item));
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
		auto* regKey = GetRegisterKey(form);
		return regKey ? regKey->GetFormID() : 0;
	}

	bool IsRegistered(RE::FormID formId) noexcept
	{
		auto* form = RE::TESForm::LookupByID(formId);
		return IsRegisteredForm(form);
	}

	bool IsExcluded(RE::FormID formId) noexcept
	{
		auto* form = RE::TESForm::LookupByID(formId);
		if (!form) {
			return true;
		}

		auto* regKey = GetRegisterKey(form);
		return IsExcludedOrBlocked(form, regKey);
	}

	bool IsDiscoverable(RE::FormID formId) noexcept
	{
		auto* form = RE::TESForm::LookupByID(formId);
		if (!form) {
			return false;
		}

		auto* regKey = GetRegisterKey(form);
		if (!regKey) {
			return false;
		}

		if (IsExcludedOrBlocked(form, regKey)) {
			return false;
		}

		return GetDiscoveryGroup(regKey) <= 5;
	}

	QuickRegisterList BuildQuickRegisterList(std::size_t offset, std::size_t limit)
	{
		QuickRegisterList result{};
		std::vector<ListItem> pageItems;
		pageItems.reserve(limit);
		std::size_t eligibleSeen = 0;
		bool hasMore = false;

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return result;
		}

		const auto settings = GetSettings();

		EnsureMapsLoaded();

		const auto quickListState = RegistrationStateStore::SnapshotQuickList();
		const auto& blocked = quickListState.blockedItems;
		const auto& registered = quickListState.registeredKeys;

		const auto isExcludedFast = [&](const RE::TESForm* item) noexcept -> bool {
			if (!item) {
				return true;
			}

			const auto id = item->GetFormID();
			if (g_excluded.contains(id) || blocked.contains(id)) {
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

		if (auto* changes = player->GetInventoryChanges(); changes && changes->entryList) {
			for (auto* entry : *changes->entryList) {
				if (!entry) {
					continue;
				}

				if (entry->IsQuestObject()) {
					continue;
				}

				auto* obj = entry->GetObject();
				if (!obj) {
					continue;
				}

				auto* regKey = GetRegisterKey(obj, settings);
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

				const auto regKeyId = regKey->GetFormID();
				const auto objId = obj->GetFormID();
				if (registered.contains(regKeyId) || registered.contains(objId)) {
					continue;
				}

				const auto totalCount = entry->countDelta;
				if (totalCount <= 0) {
					continue;
				}

				// Keep quick-list safe-count logic aligned with actual consume logic.
				const auto removal =
					Inventory::SelectSafeRemoval(entry, totalCount, settings.protectFavorites);
				const auto safeCount = removal.safeCount;
				if (safeCount <= 0) {
					continue;
				}

				if (eligibleSeen < offset) {
					++eligibleSeen;
					continue;
				}

				if (pageItems.size() >= limit) {
					hasMore = true;
					break;
				}

				++eligibleSeen;

				ListItem li{};
				li.formId = objId;
				li.regKey = regKeyId;
				li.group = group;
				li.totalCount = totalCount;
				li.safeCount = safeCount;
				li.excluded = false;
				li.registered = false;
				li.blocked = false;
				li.name = BestItemName(regKey, obj);
				if (li.name.empty()) {
					li.name = L10n::T("ui.unnamed", "(unnamed)");
				}

				pageItems.push_back(std::move(li));
			}
		}

		std::sort(pageItems.begin(), pageItems.end(), [](const ListItem& a, const ListItem& b) {
			if (a.group != b.group) {
				return a.group < b.group;
			}
			return a.name < b.name;
		});

		result.hasMore = hasMore;
		result.total = hasMore ? 0 : eligibleSeen;
		result.items = std::move(pageItems);
		return result;
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
			li.group = ClampGroup(storedGroup, form);
			li.name = BestItemName(form, nullptr);
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

		auto* regKey = GetRegisterKey(item, settings);
		if (!regKey) {
			result.message = "Invalid register key";
			return result;
		}

		if (IsExcludedOrBlocked(item, regKey)) {
			result.message = L10n::T("msg.registerQuestItem", "Codex of Power: Cannot register (quest item)");
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
		if (removal.isQuestObject) {
			result.message = L10n::T("msg.registerQuestItem", "Codex of Power: Cannot register (quest item)");
			RE::DebugNotification(result.message.c_str());
			return result;
		}

		const auto displayName = BestItemName(regKey, item);
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

	void Warmup() noexcept
	{
		EnsureMapsLoaded();
	}
}
