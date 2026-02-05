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

			if (settings.corpseExplosionVfxMode != "auto" && settings.corpseExplosionVfxMode != "art" &&
			    settings.corpseExplosionVfxMode != "shader" && settings.corpseExplosionVfxMode != "none") {
				settings.corpseExplosionVfxMode = "auto";
			}

			// Corpse explosion proc settings
			if (!std::isfinite(settings.corpseExplosionChancePct) || settings.corpseExplosionChancePct < 0.0) {
				settings.corpseExplosionChancePct = 0.0;
			}
			settings.corpseExplosionChancePct = std::clamp(settings.corpseExplosionChancePct, 0.0, 100.0);
			if (settings.corpseExplosionCooldownMs < 0) {
				settings.corpseExplosionCooldownMs = 0;
			}
			if (settings.corpseExplosionCooldownMs > 60000) {
				settings.corpseExplosionCooldownMs = 60000;
			}
			if (settings.corpseExplosionVfxEditorID.size() > 128) {
				settings.corpseExplosionVfxEditorID.resize(128);
			}

			// Reward settings
			if (settings.rewardEvery <= 0) {
				settings.rewardEvery = 10;
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

		[[nodiscard]] Settings Defaults()
		{
			return Settings{};
		}

		[[nodiscard]] Settings LoadFromDisk()
		{
			Settings settings = Defaults();
			const auto path = SettingsPath();

			std::ifstream in(path, std::ios::binary);
			if (!in.is_open()) {
				return settings;
			}

			try {
				nlohmann::json j;
				in >> j;

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

				if (auto it = j.find("corpseExplosion"); it != j.end() && it->is_object()) {
					const auto& p = *it;
					if (auto k = p.find("enabled"); k != p.end() && k->is_boolean()) {
						settings.corpseExplosionEnabled = k->get<bool>();
					}
					if (auto k = p.find("chancePct"); k != p.end() && (k->is_number_float() || k->is_number_integer())) {
						settings.corpseExplosionChancePct = k->get<double>();
					}
					if (auto k = p.find("cooldownMs"); k != p.end() && k->is_number_integer()) {
						settings.corpseExplosionCooldownMs = k->get<std::int32_t>();
					}
					if (auto k = p.find("vfxMode"); k != p.end() && k->is_string()) {
						settings.corpseExplosionVfxMode = k->get<std::string>();
					}
					if (auto k = p.find("vfxEditorID"); k != p.end() && k->is_string()) {
						settings.corpseExplosionVfxEditorID = k->get<std::string>();
					}
					if (auto k = p.find("notify"); k != p.end() && k->is_boolean()) {
						settings.corpseExplosionNotify = k->get<bool>();
					}
				}

			} catch (const std::exception& e) {
				SKSE::log::warn("Failed to parse settings.json: {}", e.what());
				return Defaults();
			}

			return Clamp(std::move(settings));
		}

		[[nodiscard]] bool SaveToDisk(const Settings& settings)
		{
			const auto path = SettingsPath();
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
			j["registration"] = { { "normalize", settings.normalizeRegistration } };
			j["safety"] = { { "protectFavorites", settings.protectFavorites } };
			j["lootNotify"] = { { "enabled", settings.enableLootNotify } };
			j["rewards"] = {
				{ "enabled", settings.enableRewards },
				{ "every", settings.rewardEvery },
				{ "multiplier", settings.rewardMultiplier },
				{ "allowSkillRewards", settings.allowSkillRewards },
			};
			j["corpseExplosion"] = {
				{ "enabled", settings.corpseExplosionEnabled },
				{ "chancePct", settings.corpseExplosionChancePct },
				{ "cooldownMs", settings.corpseExplosionCooldownMs },
				{ "vfxMode", settings.corpseExplosionVfxMode },
				{ "vfxEditorID", settings.corpseExplosionVfxEditorID },
				{ "notify", settings.corpseExplosionNotify },
			};

			std::ofstream out(path, std::ios::binary | std::ios::trunc);
			if (!out.is_open()) {
				SKSE::log::error("Failed to write settings.json at '{}'", path.string());
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

	void LoadSettingsFromDisk()
	{
		const auto path = std::filesystem::path(kSettingsPath);
		if (!std::filesystem::exists(path)) {
			const auto defaults = Defaults();
			SetSettings(defaults);
			(void)SaveToDisk(defaults);
			return;
		}

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
