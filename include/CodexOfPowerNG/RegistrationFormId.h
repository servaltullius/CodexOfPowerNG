#pragma once

#include <charconv>
#include <cstdint>
#include <optional>
#include <string_view>

namespace CodexOfPowerNG::Registration::FormId
{
	[[nodiscard]] inline std::optional<std::uint32_t> ParseUInt32Strict(std::string_view text, int base) noexcept
	{
		std::uint32_t value{};
		auto*         begin = text.data();
		auto*         end = text.data() + text.size();
		auto [ptr, ec] = std::from_chars(begin, end, value, base);
		if (ec != std::errc{} || ptr != end) {
			return std::nullopt;
		}
		return value;
	}

	[[nodiscard]] inline std::optional<std::uint32_t> ParseFormIDText(std::string_view text) noexcept
	{
		if (text.empty()) {
			return std::nullopt;
		}

		if (text.rfind("0x", 0) == 0 || text.rfind("0X", 0) == 0) {
			auto trimmed = text.substr(2);
			return ParseUInt32Strict(trimmed, 16);
		}

		return ParseUInt32Strict(text, 10);
	}

	[[nodiscard]] constexpr std::uint32_t ToLocalFormID(std::uint32_t anyID) noexcept
	{
		if (anyID == 0) {
			return 0;
		}

		if ((anyID & 0xFF000000u) == 0xFE000000u) {
			return anyID & 0x00000FFFu;
		}

		return anyID & 0x00FFFFFFu;
	}
}
