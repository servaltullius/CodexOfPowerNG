#include "CodexOfPowerNG/Config.h"

#include "CodexOfPowerNG/Constants.h"

#include <RE/Skyrim.h>

#include <SKSE/Logger.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <mutex>
#include <string_view>

namespace CodexOfPowerNG
{
	namespace
	{
		std::mutex g_settingsMutex;
		Settings   g_settings{};

		[[nodiscard]] Settings Clamp(Settings settings)
		{
			if (settings.languageOverride != "auto" && settings.languageOverride != "en" && settings.languageOverride != "ko") {
				settings.languageOverride = "auto";
			}

			// Reward settings
			if (settings.rewardEvery <= 0) {
				settings.rewardEvery = 5;
			}

			// UI input correction scale (JS side)
			if (!std::isfinite(settings.uiInputScale) || settings.uiInputScale <= 0.0) {
				settings.uiInputScale = 1.0;
			}
			settings.uiInputScale = std::clamp(settings.uiInputScale, 0.50, 3.00);

			if (settings.rewardMultiplier <= 0.0) {
				settings.rewardMultiplier = 0.0;
			} else {
				if (settings.rewardMultiplier < 0.10) {
					settings.rewardMultiplier = 0.10;
				}
				if (settings.rewardMultiplier > 5.0) {
					settings.rewardMultiplier = 5.0;
				}
			}

			return settings;
		}

		[[nodiscard]] std::filesystem::path SettingsPath()
		{
			return std::filesystem::path(kSettingsPath);
		}

		[[nodiscard]] std::filesystem::path UserSettingsPath()
		{
			return std::filesystem::path(kSettingsUserPath);
		}

		[[nodiscard]] Settings Defaults()
		{
			return Settings{};
		}

		void ApplyJsonToSettings(const nlohmann::json& j, Settings& settings)
		{
			if (auto it = j.find("language"); it != j.end() && it->is_string()) {
				settings.languageOverride = it->get<std::string>();
			}

			if (auto it = j.find("hotkey"); it != j.end() && it->is_object()) {
				const auto& hk = *it;
				if (auto k = hk.find("toggleKeyCode"); k != hk.end() && k->is_number_unsigned()) {
					settings.toggleKeyCode = k->get<std::uint32_t>();
				}
			}

			if (auto it = j.find("ui"); it != j.end() && it->is_object()) {
				const auto& ui = *it;
				if (auto k = ui.find("disableFocusMenu"); k != ui.end() && k->is_boolean()) {
					settings.uiDisableFocusMenu = k->get<bool>();
				}
				if (auto k = ui.find("pauseGame"); k != ui.end() && k->is_boolean()) {
					settings.uiPauseGame = k->get<bool>();
				}
				if (auto k = ui.find("inputScale"); k != ui.end() && (k->is_number_float() || k->is_number_integer())) {
					settings.uiInputScale = k->get<double>();
				}
				if (auto k = ui.find("destroyOnClose"); k != ui.end() && k->is_boolean()) {
					settings.uiDestroyOnClose = k->get<bool>();
				}
			}

			if (auto it = j.find("registration"); it != j.end() && it->is_object()) {
				const auto& reg = *it;
				if (auto k = reg.find("normalize"); k != reg.end() && k->is_boolean()) {
					settings.normalizeRegistration = k->get<bool>();
				}
				if (auto k = reg.find("requireTccDisplayed"); k != reg.end() && k->is_boolean()) {
					settings.requireTccDisplayed = k->get<bool>();
				}
			}

			if (auto it = j.find("safety"); it != j.end() && it->is_object()) {
				const auto& safety = *it;
				if (auto k = safety.find("protectFavorites"); k != safety.end() && k->is_boolean()) {
					settings.protectFavorites = k->get<bool>();
				}
			}

			if (auto it = j.find("lootNotify"); it != j.end() && it->is_object()) {
				const auto& ln = *it;
				if (auto k = ln.find("enabled"); k != ln.end() && k->is_boolean()) {
					settings.enableLootNotify = k->get<bool>();
				}
			}

			if (auto it = j.find("rewards"); it != j.end() && it->is_object()) {
				const auto& r = *it;
				if (auto k = r.find("enabled"); k != r.end() && k->is_boolean()) {
					settings.enableRewards = k->get<bool>();
				}
				if (auto k = r.find("every"); k != r.end() && k->is_number_integer()) {
					settings.rewardEvery = k->get<std::int32_t>();
				}
				if (auto k = r.find("multiplier"); k != r.end() && (k->is_number_float() || k->is_number_integer())) {
					settings.rewardMultiplier = k->get<double>();
				}
				if (auto k = r.find("allowSkillRewards"); k != r.end() && k->is_boolean()) {
					settings.allowSkillRewards = k->get<bool>();
				}
			}
		}

		void ApplySettingsFromFile(const std::filesystem::path& path, Settings& settings, std::string_view label) noexcept
		{
			std::ifstream in(path, std::ios::binary);
			if (!in.is_open()) {
				return;
			}

			try {
				nlohmann::json j;
				in >> j;
				ApplyJsonToSettings(j, settings);
			} catch (const std::exception& e) {
				SKSE::log::warn("Failed to parse {} ({}): {}", label, path.string(), e.what());
			}
		}

		[[nodiscard]] Settings LoadFromDisk()
		{
			Settings settings = Defaults();
			ApplySettingsFromFile(SettingsPath(), settings, "settings.json");
			ApplySettingsFromFile(UserSettingsPath(), settings, "settings.user.json");
			return Clamp(std::move(settings));
		}

		[[nodiscard]] bool SaveToDisk(const Settings& settings)
		{
			const auto path = UserSettingsPath();
			std::error_code ec{};
			std::filesystem::create_directories(path.parent_path(), ec);

			nlohmann::json j;
			j["version"] = 2;
			j["language"] = settings.languageOverride;
			j["hotkey"] = { { "toggleKeyCode", settings.toggleKeyCode } };
			j["ui"] = {
				{ "disableFocusMenu", settings.uiDisableFocusMenu },
				{ "pauseGame", settings.uiPauseGame },
				{ "inputScale", settings.uiInputScale },
				{ "destroyOnClose", settings.uiDestroyOnClose },
			};
			j["registration"] = {
				{ "normalize", settings.normalizeRegistration },
				{ "requireTccDisplayed", settings.requireTccDisplayed },
			};
			j["safety"] = { { "protectFavorites", settings.protectFavorites } };
			j["lootNotify"] = { { "enabled", settings.enableLootNotify } };
			j["rewards"] = {
				{ "enabled", settings.enableRewards },
				{ "every", settings.rewardEvery },
				{ "multiplier", settings.rewardMultiplier },
				{ "allowSkillRewards", settings.allowSkillRewards },
			};

			std::ofstream out(path, std::ios::binary | std::ios::trunc);
			if (!out.is_open()) {
				SKSE::log::error("Failed to write settings.user.json at '{}'", path.string());
				return false;
			}

			out << j.dump(2);
			out << "\n";
			return true;
		}
	}

	Settings GetSettings()
	{
		std::scoped_lock lock(g_settingsMutex);
		return g_settings;
	}

	void SetSettings(const Settings& settings)
	{
		std::scoped_lock lock(g_settingsMutex);
		g_settings = Clamp(settings);
	}

	Settings ClampSettings(const Settings& settings)
	{
		return Clamp(settings);
	}

	void LoadSettingsFromDisk()
	{
		SetSettings(LoadFromDisk());
	}

	bool SaveSettingsToDisk()
	{
		const auto snapshot = GetSettings();
		return SaveToDisk(snapshot);
	}

	bool SaveSettingsToDisk(const Settings& settings)
	{
		SetSettings(settings);
		return SaveSettingsToDisk();
	}
}
