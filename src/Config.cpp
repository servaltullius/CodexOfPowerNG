#include "CodexOfPowerNG/Config.h"

#include "CodexOfPowerNG/Constants.h"

#include <RE/Skyrim.h>

#include <SKSE/Logger.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <exception>
#include <mutex>
#include <string_view>

namespace CodexOfPowerNG
{
	namespace
	{
		std::mutex g_settingsMutex;
		Settings   g_settings{};

		[[nodiscard]] bool IsValidJsonFile(const std::filesystem::path& path) noexcept
		{
			std::error_code sizeEc{};
			const auto size = std::filesystem::file_size(path, sizeEc);
			if (!sizeEc) {
				// A settings file should remain tiny; avoid parsing pathological files.
				constexpr std::uintmax_t kMaxBytes = 1024 * 1024;  // 1 MiB
				if (size > kMaxBytes) {
					return false;
				}
			}

			std::ifstream in(path, std::ios::binary);
			if (!in.is_open()) {
				return false;
			}

			try {
				nlohmann::json j;
				in >> j;
				return true;
			} catch (...) {
				return false;
			}
		}

		void RepairUserSettingsFiles() noexcept
		{
			const auto path = std::filesystem::path(kSettingsUserPath);
			auto tmpPath = path;
			tmpPath += ".tmp";
			auto bakPath = path;
			bakPath += ".bak";

			std::error_code ec{};
			const bool finalExists = std::filesystem::exists(path, ec) && !ec;
			ec.clear();
			const bool tmpExists = std::filesystem::exists(tmpPath, ec) && !ec;
			ec.clear();
			const bool bakExists = std::filesystem::exists(bakPath, ec) && !ec;

			// If the final file is missing but a temp exists, try recovering the temp first.
			// (Conservative: do not overwrite a valid final settings.user.json automatically.)
			if (!finalExists && tmpExists && IsValidJsonFile(tmpPath)) {
				std::error_code recoverEc{};
				std::filesystem::rename(tmpPath, path, recoverEc);
				if (!recoverEc) {
					SKSE::log::warn("Recovered settings.user.json from stale temp file '{}'", tmpPath.string());
				}
			}

			// Remove stale temp file (best effort).
			if (tmpExists) {
				std::error_code rmEc{};
				(void)std::filesystem::remove(tmpPath, rmEc);
				if (!rmEc) {
					SKSE::log::info("Removed stale settings.user.json.tmp at '{}'", tmpPath.string());
				}
			}

			// If the final file is still missing, try restoring the backup.
			ec.clear();
			const bool finalExistsAfter = std::filesystem::exists(path, ec) && !ec;
			if (!finalExistsAfter && bakExists && IsValidJsonFile(bakPath)) {
				std::error_code restoreEc{};
				std::filesystem::rename(bakPath, path, restoreEc);
				if (!restoreEc) {
					SKSE::log::warn("Restored settings.user.json from backup file '{}'", bakPath.string());
				}
			}
		}

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
			RepairUserSettingsFiles();
			Settings settings = Defaults();
			ApplySettingsFromFile(SettingsPath(), settings, "settings.json");
			ApplySettingsFromFile(UserSettingsPath(), settings, "settings.user.json");
			return Clamp(std::move(settings));
		}

		[[nodiscard]] bool SaveToDisk(const Settings& settings)
		{
			const auto path = UserSettingsPath();
			auto tmpPath = path;
			tmpPath += ".tmp";
			auto bakPath = path;
			bakPath += ".bak";

			std::error_code ec{};
			std::filesystem::create_directories(path.parent_path(), ec);
			if (ec) {
				SKSE::log::error(
					"Failed to create settings directory '{}' ({}): {}",
					path.parent_path().string(),
					ec.value(),
					ec.message());
				return false;
			}

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

			std::string serialized;
			try {
				serialized = j.dump(2);
			} catch (const std::exception& e) {
				SKSE::log::error("Failed to serialize settings JSON for settings.user.json: {}", e.what());
				return false;
			} catch (...) {
				SKSE::log::error("Failed to serialize settings JSON for settings.user.json");
				return false;
			}

			{
				std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
				if (!out.is_open()) {
					SKSE::log::error("Failed to write settings.user.json.tmp at '{}'", tmpPath.string());
					return false;
				}

				out << serialized;
				out << "\n";
				out.flush();
				if (!out) {
					SKSE::log::error("Failed to flush settings.user.json.tmp at '{}'", tmpPath.string());
					std::error_code rmEc{};
					(void)std::filesystem::remove(tmpPath, rmEc);
					return false;
				}
			}

			// Rotate: settings.user.json -> settings.user.json.bak (best effort).
			ec.clear();
			if (std::filesystem::exists(path, ec) && !ec) {
				std::error_code bakRemoveEc{};
				(void)std::filesystem::remove(bakPath, bakRemoveEc);

				std::error_code bakEc{};
				std::filesystem::rename(path, bakPath, bakEc);
				if (bakEc) {
					SKSE::log::warn(
						"Failed to rotate settings.user.json to backup '{}' ({}): {}",
						bakPath.string(),
						bakEc.value(),
						bakEc.message());
				}
			}

			// Commit: settings.user.json.tmp -> settings.user.json
			std::error_code commitEc{};
			std::filesystem::rename(tmpPath, path, commitEc);
			if (!commitEc) {
				return true;
			}

			SKSE::log::error(
				"Failed to replace settings.user.json with temp file '{}' ({}): {}",
				path.string(),
				commitEc.value(),
				commitEc.message());

			std::error_code rmEc{};
			(void)std::filesystem::remove(tmpPath, rmEc);

			// Restore backup if we moved the original away and the final path is now missing.
			std::error_code existsEc{};
			const bool finalExists = std::filesystem::exists(path, existsEc) && !existsEc;
			if (!finalExists) {
				std::error_code restoreEc{};
				std::filesystem::rename(bakPath, path, restoreEc);
				if (restoreEc) {
					SKSE::log::error(
						"Failed to restore settings.user.json from backup '{}' ({}): {}",
						bakPath.string(),
						restoreEc.value(),
						restoreEc.message());
				}
			}

			return false;
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
