#pragma once

#include <RE/Skyrim.h>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace CodexOfPowerNG::RegistrationMaps
{
	using ResolveEntryFn = std::function<std::optional<RE::FormID>(std::string_view fileName, std::string_view idText)>;

	struct Paths
	{
		std::filesystem::path excludeMapPath;
		std::filesystem::path excludeUserPath;
		std::filesystem::path pluginDataDir;
		std::filesystem::path variantMapPath;
		std::uint32_t         excludePatchMax{ 0 };
	};

	struct Data
	{
		std::unordered_set<RE::FormID>             excluded;
		std::unordered_map<RE::FormID, RE::FormID> variantBase;
	};

	[[nodiscard]] Data LoadFromDisk(const Paths& paths, const ResolveEntryFn& resolveEntry) noexcept;
}
