#pragma once

#include "CodexOfPowerNG/Registration.h"

#include <RE/Skyrim.h>

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::PrismaUIPayloads
{
	using json = nlohmann::json;

	[[nodiscard]] json BuildInventoryPayload(
		std::uint32_t page,
		std::uint32_t pageSize,
		Registration::QuickRegisterList& pageData) noexcept;

	[[nodiscard]] json BuildRegisteredPayload(
		const std::vector<Registration::ListItem>& items) noexcept;

	[[nodiscard]] json BuildRewardTotalsArray(
		const std::vector<std::pair<RE::ActorValue, float>>& totals,
		bool useL10n) noexcept;

	[[nodiscard]] std::string FormatReward(float total, std::string_view fmt) noexcept;
}
