#pragma once

#include <string>
#include <string_view>

namespace CodexOfPowerNG::L10n
{
	// Loads localization JSON based on current Settings (auto/en/ko).
	void Load();

	// Lookup a dotted path (e.g. "msg.rewardPrefix") with fallback.
	[[nodiscard]] std::string T(std::string_view dottedPath, std::string_view fallback);

	[[nodiscard]] std::string ActiveLanguage();
}

