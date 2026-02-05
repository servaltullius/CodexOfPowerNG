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

		// Safety
		bool protectFavorites{ true };

		// Loot notify
		bool enableLootNotify{ true };

		// Rewards
		bool enableRewards{ true };
		std::int32_t rewardEvery{ 10 };
		double rewardMultiplier{ 1.0 };
		bool allowSkillRewards{ false };

		// Proc: corpse explosion (VFX only)
		bool        corpseExplosionEnabled{ true };
		double      corpseExplosionChancePct{ 10.0 };
		std::int32_t corpseExplosionCooldownMs{ 2000 };
		// "auto" | "art" | "shader" | "none"
		std::string corpseExplosionVfxMode{ "auto" };
		// Optional override: EditorID of ARTO/EFSH to use.
		std::string corpseExplosionVfxEditorID{};
		// Optional: show a notification when it procs.
		bool corpseExplosionNotify{ false };
	};

	[[nodiscard]] Settings GetSettings();
	void                  SetSettings(const Settings& settings);

	// Loads from disk and replaces current settings (falls back to defaults on error).
	void LoadSettingsFromDisk();
	// Saves current settings to disk.
	bool SaveSettingsToDisk();

	// Saves a provided settings snapshot to disk and replaces current settings.
	bool SaveSettingsToDisk(const Settings& settings);
}
