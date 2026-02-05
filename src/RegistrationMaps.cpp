#include "CodexOfPowerNG/RegistrationMaps.h"

#include <SKSE/Logger.h>

#include <nlohmann/json.hpp>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

namespace CodexOfPowerNG::RegistrationMaps
{
	namespace
	{
		void LoadExcludeFile(const std::filesystem::path& path,
			std::unordered_set<RE::FormID>& out,
			const ResolveEntryFn& resolveEntry) noexcept
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

					auto formIdOpt = resolveEntry(fileIt->get<std::string>(), idIt->get<std::string>());
					if (!formIdOpt) {
						continue;
					}

					out.insert(*formIdOpt);
				}
			} catch (const std::exception& e) {
				SKSE::log::warn("Failed to parse exclude file '{}': {}", path.string(), e.what());
			}
		}

		void LoadVariantMapFile(const std::filesystem::path& path,
			std::unordered_map<RE::FormID, RE::FormID>& out,
			const ResolveEntryFn& resolveEntry) noexcept
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

					auto variantIdOpt = resolveEntry(vFileIt->get<std::string>(), vIdIt->get<std::string>());
					auto baseIdOpt = resolveEntry(bFileIt->get<std::string>(), bIdIt->get<std::string>());
					if (!variantIdOpt || !baseIdOpt || *variantIdOpt == *baseIdOpt) {
						continue;
					}

					out.emplace(*variantIdOpt, *baseIdOpt);
				}
			} catch (const std::exception& e) {
				SKSE::log::warn("Failed to parse variant map '{}': {}", path.string(), e.what());
			}
		}
	}

	Data LoadFromDisk(const Paths& paths, const ResolveEntryFn& resolveEntry) noexcept
	{
		Data out{};
		if (!resolveEntry) {
			return out;
		}

		LoadExcludeFile(paths.excludeMapPath, out.excluded, resolveEntry);
		LoadExcludeFile(paths.excludeUserPath, out.excluded, resolveEntry);

		for (std::uint32_t i = 1; i <= paths.excludePatchMax; ++i) {
			char name[64];
			const auto written = std::snprintf(name, sizeof(name), "exclude_patch_%02u.json", i);
			if (written <= 0) {
				break;
			}
			const auto patchPath = paths.pluginDataDir / name;
			if (!std::filesystem::exists(patchPath)) {
				break;
			}
			LoadExcludeFile(patchPath, out.excluded, resolveEntry);
		}

		LoadVariantMapFile(paths.variantMapPath, out.variantBase, resolveEntry);
		return out;
	}
}
