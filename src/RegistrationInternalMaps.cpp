#include "RegistrationInternal.h"

#include "CodexOfPowerNG/Constants.h"
#include "CodexOfPowerNG/RegistrationFormId.h"
#include "CodexOfPowerNG/RegistrationMaps.h"
#include "CodexOfPowerNG/RegistrationRules.h"
#include "CodexOfPowerNG/RegistrationStateStore.h"

#include <RE/Skyrim.h>

#include <SKSE/Logger.h>

#include <mutex>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace CodexOfPowerNG::Registration::Internal
{
	namespace
	{
		std::mutex g_mapsMutex;
		bool       g_mapsLoaded{ false };

		// Write-once maps: populated by EnsureMapsLoaded(), then immutable.
		// Thread safety: every public function that reads these calls
		// EnsureMapsLoaded() first. The mutex acquire/release there provides
		// a happens-before relationship with the initial write, so subsequent
		// lock-free reads are safe. Do NOT modify after initialization.
		std::unordered_set<RE::FormID>              g_excluded{};
		std::unordered_map<RE::FormID, RE::FormID> g_variantBase{};

		[[nodiscard]] std::optional<RE::FormID> LookupFormFromEntry(
			std::string_view fileName,
			std::string_view idStr) noexcept
		{
			auto idRawOpt = FormId::ParseFormIDText(idStr);
			if (!idRawOpt) {
				return std::nullopt;
			}

			const auto localID = static_cast<RE::FormID>(FormId::ToLocalFormID(*idRawOpt));
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

	bool IsExcludedByMap(RE::FormID formId) noexcept
	{
		if (formId == 0) {
			return true;
		}
		EnsureMapsLoaded();
		return g_excluded.contains(formId);
	}

	RE::TESForm* GetRegisterKey(const RE::TESForm* item, const Settings& settings) noexcept
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

	RE::TESForm* GetRegisterKey(const RE::TESForm* item) noexcept
	{
		const auto settings = GetSettings();
		return GetRegisterKey(item, settings);
	}

	bool IsExcludedForm(const RE::TESForm* item) noexcept
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

	bool IsExcludedOrBlocked(const RE::TESForm* item, const RE::TESForm* regKey) noexcept
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
