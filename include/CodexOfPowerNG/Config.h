#pragma once

#include <cstdint>
#include <string>

namespace CodexOfPowerNG
{
	struct Settings
	{
		// UI / input
		std::uint32_t toggleKeyCode{ 0x3E };  // DIK_F4
		bool          uiDisableFocusMenu{ false };
		bool          uiPauseGame{ true };
		double        uiInputScale{ 1.0 };    // JS-side input coordinate correction
		bool          uiDestroyOnClose{ true };

		// Localization
		// "auto" | "en" | "ko"
		std::string languageOverride{ "auto" };

		// Registration
		bool normalizeRegistration{ false };
		bool requireTccDisplayed{ false };

		// Safety
		bool protectFavorites{ true };

		// Loot notify
		bool enableLootNotify{ true };

		// Rewards
		bool enableRewards{ true };
		std::int32_t rewardEvery{ 5 };
		double rewardMultiplier{ 1.0 };
		bool allowSkillRewards{ false };
	};

	[[nodiscard]] Settings GetSettings();
	void                  SetSettings(const Settings& settings);

	// Clamps setting values to their valid ranges.
	[[nodiscard]] Settings ClampSettings(const Settings& settings);

	// Loads from disk and replaces current settings (falls back to defaults on error).
	void LoadSettingsFromDisk();
	// Saves current settings to disk.
	bool SaveSettingsToDisk();

	// Saves a provided settings snapshot to disk and replaces current settings.
	bool SaveSettingsToDisk(const Settings& settings);
}
