#include "PrismaUISettingsInternal.h"

#include <cmath>

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	namespace
	{
		[[nodiscard]] bool NearlyEqual(double a, double b) noexcept
		{
			return std::fabs(a - b) <= 1e-9;
		}

		[[nodiscard]] Settings SettingsFromJson(const json& j, Settings base)
		{
			try {
				if (auto it = j.find("toggleKeyCode"); it != j.end() && it->is_number_unsigned()) {
					base.toggleKeyCode = it->get<std::uint32_t>();
				}
				if (auto it = j.find("uiDisableFocusMenu"); it != j.end() && it->is_boolean()) {
					base.uiDisableFocusMenu = it->get<bool>();
				}
				if (auto it = j.find("uiPauseGame"); it != j.end() && it->is_boolean()) {
					base.uiPauseGame = it->get<bool>();
				}
				if (auto it = j.find("uiInputScale"); it != j.end() && (it->is_number_float() || it->is_number_integer())) {
					base.uiInputScale = it->get<double>();
				}
				if (auto it = j.find("uiDestroyOnClose"); it != j.end() && it->is_boolean()) {
					base.uiDestroyOnClose = it->get<bool>();
				}
				if (auto it = j.find("languageOverride"); it != j.end() && it->is_string()) {
					base.languageOverride = it->get<std::string>();
				}
				if (auto it = j.find("normalizeRegistration"); it != j.end() && it->is_boolean()) {
					base.normalizeRegistration = it->get<bool>();
				}
				if (auto it = j.find("requireTccDisplayed"); it != j.end() && it->is_boolean()) {
					base.requireTccDisplayed = it->get<bool>();
				}
				if (auto it = j.find("protectFavorites"); it != j.end() && it->is_boolean()) {
					base.protectFavorites = it->get<bool>();
				}
				if (auto it = j.find("enableLootNotify"); it != j.end() && it->is_boolean()) {
					base.enableLootNotify = it->get<bool>();
				}
				if (auto it = j.find("enableRewards"); it != j.end() && it->is_boolean()) {
					base.enableRewards = it->get<bool>();
				}
				if (auto it = j.find("rewardEvery"); it != j.end() && it->is_number_integer()) {
					base.rewardEvery = it->get<std::int32_t>();
				}
				if (auto it = j.find("rewardMultiplier"); it != j.end() && (it->is_number_float() || it->is_number_integer())) {
					base.rewardMultiplier = it->get<double>();
				}
				if (auto it = j.find("allowSkillRewards"); it != j.end() && it->is_boolean()) {
					base.allowSkillRewards = it->get<bool>();
				}
			} catch (...) {
			}
			return base;
		}
	}

	bool SettingsEquivalent(const Settings& a, const Settings& b) noexcept
	{
		return a.toggleKeyCode == b.toggleKeyCode &&
		       a.uiDisableFocusMenu == b.uiDisableFocusMenu &&
		       a.uiPauseGame == b.uiPauseGame &&
		       NearlyEqual(a.uiInputScale, b.uiInputScale) &&
		       a.uiDestroyOnClose == b.uiDestroyOnClose &&
		       a.languageOverride == b.languageOverride &&
		       a.normalizeRegistration == b.normalizeRegistration &&
		       a.requireTccDisplayed == b.requireTccDisplayed &&
		       a.protectFavorites == b.protectFavorites &&
		       a.enableLootNotify == b.enableLootNotify &&
		       a.enableRewards == b.enableRewards &&
		       a.rewardEvery == b.rewardEvery &&
		       NearlyEqual(a.rewardMultiplier, b.rewardMultiplier) &&
		       a.allowSkillRewards == b.allowSkillRewards;
	}

	std::optional<Settings> ParseSettingsPayload(const char* argument, const Settings& current) noexcept
	{
		json payload;
		try {
			payload = json::parse(argument ? argument : "{}");
		} catch (...) {
			return std::nullopt;
		}

		const auto next = ClampSettings(SettingsFromJson(payload, current));
		return next;
	}

	json BuildSettingsPayload(const Settings& settings)
	{
		return json{
			{ "toggleKeyCode", settings.toggleKeyCode },
			{ "uiDisableFocusMenu", settings.uiDisableFocusMenu },
			{ "uiPauseGame", settings.uiPauseGame },
			{ "uiInputScale", settings.uiInputScale },
			{ "uiDestroyOnClose", settings.uiDestroyOnClose },
			{ "languageOverride", settings.languageOverride },
			{ "normalizeRegistration", settings.normalizeRegistration },
			{ "requireTccDisplayed", settings.requireTccDisplayed },
			{ "protectFavorites", settings.protectFavorites },
			{ "enableLootNotify", settings.enableLootNotify },
			{ "enableRewards", settings.enableRewards },
			{ "rewardEvery", settings.rewardEvery },
			{ "rewardMultiplier", settings.rewardMultiplier },
			{ "allowSkillRewards", settings.allowSkillRewards },
		};
	}
}
