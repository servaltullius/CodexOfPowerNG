#pragma once

#include "CodexOfPowerNG/Config.h"
#include "PrismaUIInternal.h"

#include <optional>

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	[[nodiscard]] bool                   SettingsEquivalent(const Settings& a, const Settings& b) noexcept;
	[[nodiscard]] std::optional<Settings> ParseSettingsPayload(const char* argument, const Settings& current) noexcept;
}
