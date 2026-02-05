#include "CodexOfPowerNG/Registration.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/Inventory.h"
#include "CodexOfPowerNG/L10n.h"
#include "CodexOfPowerNG/Rewards.h"
#include "CodexOfPowerNG/State.h"

#include <RE/Skyrim.h>

#include <RE/M/Misc.h>

#include <SKSE/Logger.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <filesystem>
#include <fstream>
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

		[[nodiscard]] std::optional<RE::TESForm*> LookupFormFromEntry(std::string_view fileName, std::string_view idStr) noexcept
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

			return form;
		}

		void LoadExcludeFile(const std::filesystem::path& path, std::unordered_set<RE::FormID>& out) noexcept
		{
			std::ifstream in(path, std::ios::binary);
			if (!in.is_open()) {
				return;
			}

			try {
				nlohmann::json j;
				in >> j;

				auto it = j.find("forms");
				if (it == j.end() || !it->is_array()) {
					return;
				}

				for (const auto& entry : *it) {
					if (!entry.is_object()) {
						continue;
					}

					auto fileIt = entry.find("file");
					auto idIt = entry.find("id");
					if (fileIt == entry.end() || idIt == entry.end() || !fileIt->is_string() || !idIt->is_string()) {
						continue;
					}

					const auto file = fileIt->get<std::string>();
					const auto id = idIt->get<std::string>();

					auto formOpt = LookupFormFromEntry(file, id);
					if (!formOpt) {
						continue;
					}

					out.insert((*formOpt)->GetFormID());
				}
			} catch (const std::exception& e) {
				SKSE::log::warn("Failed to parse exclude file '{}': {}", path.string(), e.what());
			}
		}

		void LoadVariantMapFile(const std::filesystem::path& path, std::unordered_map<RE::FormID, RE::FormID>& out) noexcept
		{
			std::ifstream in(path, std::ios::binary);
			if (!in.is_open()) {
				return;
			}

			try {
				nlohmann::json j;
				in >> j;

				auto it = j.find("mappings");
				if (it == j.end() || !it->is_array()) {
					return;
				}

				for (const auto& entry : *it) {
					if (!entry.is_object()) {
						continue;
					}

					const auto v = entry.find("variant");
					const auto b = entry.find("base");
					if (v == entry.end() || b == entry.end() || !v->is_object() || !b->is_object()) {
						continue;
					}

					const auto vFileIt = v->find("file");
					const auto vIdIt = v->find("id");
					const auto bFileIt = b->find("file");
					const auto bIdIt = b->find("id");
					if (vFileIt == v->end() || vIdIt == v->end() || bFileIt == b->end() || bIdIt == b->end()) {
						continue;
					}
					if (!vFileIt->is_string() || !vIdIt->is_string() || !bFileIt->is_string() || !bIdIt->is_string()) {
						continue;
					}

					const auto vFormOpt = LookupFormFromEntry(vFileIt->get<std::string>(), vIdIt->get<std::string>());
					const auto bFormOpt = LookupFormFromEntry(bFileIt->get<std::string>(), bIdIt->get<std::string>());
					if (!vFormOpt || !bFormOpt) {
						continue;
					}

					auto* vForm = *vFormOpt;
					auto* bForm = *bFormOpt;
					if (!vForm || !bForm || vForm == bForm) {
						continue;
					}

					out.emplace(vForm->GetFormID(), bForm->GetFormID());
				}
			} catch (const std::exception& e) {
				SKSE::log::warn("Failed to parse variant map '{}': {}", path.string(), e.what());
			}
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

			LoadExcludeFile(std::filesystem::path(kExcludeMapPath), g_excluded);
			LoadExcludeFile(std::filesystem::path(kExcludeUserPath), g_excluded);

			for (std::uint32_t i = 1; i <= kExcludePatchMax; ++i) {
				char name[64];
				const auto written = std::snprintf(name, sizeof(name), "exclude_patch_%02u.json", i);
				if (written <= 0) {
					break;
				}
				const auto patchPath = std::filesystem::path(kPluginDataDir) / name;
				if (!std::filesystem::exists(patchPath)) {
					break;
				}
				LoadExcludeFile(patchPath, g_excluded);
			}

			LoadVariantMapFile(std::filesystem::path(kVariantMapPath), g_variantBase);

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
				if (baseID == 0 || baseID == regKey->GetFormID()) {
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

		[[nodiscard]] bool IsDragonClaw(const RE::TESForm* item) noexcept
		{
			auto* misc = item ? item->As<RE::TESObjectMISC>() : nullptr;
			if (!misc) {
				return false;
			}

			auto* n = misc->GetName();
			if (!n || n[0] == '\0') {
				return false;
			}

			std::string_view name = n;
			if (name.find("Dragon Claw") != std::string_view::npos || name.find("dragon claw") != std::string_view::npos) {
				return true;
			}
			// Korean (commonly "용발톱"); avoid matching generic "발톱" (e.g., bear claws ingredient).
			if (name.find("용발톱") != std::string_view::npos) {
				return true;
			}

			return false;
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

			auto& state = GetState();
			{
				std::scoped_lock lock(state.mutex);
				if (state.blockedItems.contains(id)) {
					return true;
				}
			}

			if (item->IsKey() || item->IsLockpick() || item->IsGold()) {
				return true;
			}

			if (IsDragonClaw(item)) {
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

			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			if (state.registeredItems.contains(regKey->GetFormID())) {
				return true;
			}

			// Backward compat: older data may have stored the non-template variant as the key.
			if (item != regKey) {
				return state.registeredItems.contains(item->GetFormID());
			}

			return false;
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
		if (!item) {
			return 255;
		}

		if (IsExcludedForm(item)) {
			return 255;
		}

		switch (item->GetFormType()) {
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
			return 255;
		}
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

			std::unordered_set<RE::FormID> blocked;
			std::unordered_set<RE::FormID> registered;
				{
					auto& state = GetState();
					std::scoped_lock lock(state.mutex);
					blocked = state.blockedItems;
					registered.reserve(state.registeredItems.size() * 2);
					for (const auto& [id, _] : state.registeredItems) {
						registered.insert(id);
					}
				}

				const auto isExcludedFast = [&](const RE::TESForm* item) noexcept -> bool {
					if (!item) {
						return true;
					}

					const auto id = item->GetFormID();
					if (g_excluded.contains(id) || blocked.contains(id)) {
						return true;
					}

					if (item->IsKey() || item->IsLockpick() || item->IsGold()) {
						return true;
					}

					if (IsDragonClaw(item)) {
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

					switch (item->GetFormType()) {
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
						return 255;
					}
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

							std::int32_t extraCountTotal = 0;
							std::int32_t safeExtraCount = 0;

							if (entry->extraLists) {
								for (auto* xList : *entry->extraLists) {
									if (!xList) {
									continue;
								}

								const auto count = xList->GetCount();
								if (count > 0) {
									extraCountTotal += count;
								}

								const bool worn = xList->HasType<RE::ExtraWorn>() || xList->HasType<RE::ExtraWornLeft>();
								const bool hotkey = settings.protectFavorites && xList->HasType<RE::ExtraHotkey>();
								if (worn || hotkey) {
									continue;
								}

								if (count > 0) {
									safeExtraCount += count;
								}
							}
						}

							const auto baseCount = totalCount - extraCountTotal;
							const auto safeCount = safeExtraCount + (baseCount > 0 ? baseCount : 0);
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
		std::vector<std::pair<RE::FormID, std::uint32_t>> ids;
		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			ids.reserve(state.registeredItems.size());
			for (const auto& [id, group] : state.registeredItems) {
				ids.emplace_back(id, group);
			}
		}

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

		if (IsRegisteredForm(item)) {
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
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);
				state.blockedItems.insert(regKey->GetFormID());
				if (item->GetFormID() != regKey->GetFormID()) {
					state.blockedItems.insert(item->GetFormID());
				}
			}

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
			{
				auto& state = GetState();
				std::scoped_lock lock(state.mutex);
				state.blockedItems.insert(regKey->GetFormID());
				if (item->GetFormID() != regKey->GetFormID()) {
					state.blockedItems.insert(item->GetFormID());
				}
			}

			result.message = L10n::T("msg.registerCantDestroy", "Codex of Power: Cannot register (cannot destroy item)");
			RE::DebugNotification(result.message.c_str());
			return result;
		}

		std::size_t totalRegistered = 0;
		{
			auto& state = GetState();
			std::scoped_lock lock(state.mutex);
			state.registeredItems.emplace(regKey->GetFormID(), group);
			totalRegistered = state.registeredItems.size();
		}

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
