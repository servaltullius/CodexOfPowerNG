#include "CodexOfPowerNG/L10n.h"

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/Constants.h"

#include <RE/Skyrim.h>

#include <RE/M/Misc.h>
#include <RE/S/Setting.h>

#include <SKSE/Logger.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace CodexOfPowerNG::L10n
{
	namespace
	{
		std::mutex    g_mutex;
		nlohmann::json g_lang;
		std::string    g_langCode{ "en" };

		[[nodiscard]] std::string ToUpper(std::string value)
		{
			std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
			return value;
		}

		[[nodiscard]] std::string DetectGameLanguage()
		{
			auto* setting = RE::GetINISetting("sLanguage:General");
			if (!setting) {
				return "en";
			}

			auto* value = setting->GetString();
			if (!value) {
				return "en";
			}

			auto up = ToUpper(value);
			if (up.find("KOREAN") != std::string::npos) {
				return "ko";
			}

			return "en";
		}

		[[nodiscard]] std::filesystem::path LangPath(std::string_view langCode)
		{
			return std::filesystem::path(kLangDir) / (std::string(langCode) + ".json");
		}

		[[nodiscard]] std::optional<nlohmann::json> LoadJsonFile(const std::filesystem::path& path)
		{
			std::ifstream in(path, std::ios::binary);
			if (!in.is_open()) {
				return std::nullopt;
			}

			try {
				nlohmann::json j;
				in >> j;
				return j;
			} catch (const std::exception& e) {
				SKSE::log::warn("Failed to parse lang file '{}': {}", path.string(), e.what());
				return std::nullopt;
			}
		}

		[[nodiscard]] std::vector<std::string_view> Split(std::string_view text, char delim)
		{
			std::vector<std::string_view> parts;
			while (!text.empty()) {
				auto pos = text.find(delim);
				if (pos == std::string_view::npos) {
					parts.push_back(text);
					break;
				}
				parts.push_back(text.substr(0, pos));
				text = text.substr(pos + 1);
			}
			return parts;
		}
	}

	void Load()
	{
		auto settings = GetSettings();
		std::string desired = settings.languageOverride;
		if (desired != "en" && desired != "ko") {
			desired = DetectGameLanguage();
		}

		auto load = LoadJsonFile(LangPath(desired));
		if (!load && desired != "en") {
			desired = "en";
			load = LoadJsonFile(LangPath(desired));
		}

		if (!load) {
			SKSE::log::warn("No localization file found; using fallbacks only");
			std::scoped_lock lock(g_mutex);
			g_lang = nlohmann::json::object();
			g_langCode = desired;
			return;
		}

		std::scoped_lock lock(g_mutex);
		g_lang = std::move(*load);
		g_langCode = desired;
	}

	std::string ActiveLanguage()
	{
		std::scoped_lock lock(g_mutex);
		return g_langCode;
	}

	std::string T(std::string_view dottedPath, std::string_view fallback)
	{
		if (dottedPath.empty()) {
			return std::string(fallback);
		}

		std::scoped_lock lock(g_mutex);
		const nlohmann::json* cur = &g_lang;
		for (auto key : Split(dottedPath, '.')) {
			if (!cur->is_object()) {
				return std::string(fallback);
			}

			auto it = cur->find(std::string(key));
			if (it == cur->end()) {
				return std::string(fallback);
			}
			cur = std::addressof(*it);
		}

		if (cur->is_string()) {
			return cur->get<std::string>();
		}

		return std::string(fallback);
	}
}
