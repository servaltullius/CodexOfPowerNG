#pragma once

#include <cstdint>

namespace CodexOfPowerNG
{
	inline constexpr auto kPluginName = "CodexOfPowerNG";
	inline constexpr auto kPrismaUIViewPath = "codexofpowerng/index.html";

	// Paths are relative to the game runtime directory (SkyrimSE.exe).
	inline constexpr auto kPluginDataDir = "Data/SKSE/Plugins/CodexOfPowerNG";
	inline constexpr auto kSettingsPath = "Data/SKSE/Plugins/CodexOfPowerNG/settings.json";
	inline constexpr auto kLangDir = "Data/SKSE/Plugins/CodexOfPowerNG/lang";
	inline constexpr auto kExcludeMapPath = "Data/SKSE/Plugins/CodexOfPowerNG/exclude_map.json";
	inline constexpr auto kExcludeUserPath = "Data/SKSE/Plugins/CodexOfPowerNG/exclude_user.json";
	inline constexpr auto kVariantMapPath = "Data/SKSE/Plugins/CodexOfPowerNG/variant_map.json";
	inline constexpr std::uint32_t kExcludePatchMax = 32;

	inline constexpr std::uint32_t kSerializationUniqueId = 'CPNG';
	inline constexpr std::uint32_t kSerializationVersion = 2;

	inline constexpr std::uint32_t kRecordRegisteredItems = 'REGI';
	inline constexpr std::uint32_t kRecordBlockedItems = 'BLCK';
	inline constexpr std::uint32_t kRecordNotifiedItems = 'NTFY';
	inline constexpr std::uint32_t kRecordRewards = 'RWDS';

	// Default: F4 (DIK_F4 = 0x3E)
	inline constexpr std::uint32_t kToggleKeyCode = 0x3E;
}
