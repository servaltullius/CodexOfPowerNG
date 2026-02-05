#pragma once

#include <RE/Skyrim.h>

#include <cstdint>
#include <string_view>

namespace CodexOfPowerNG::RegistrationRules
{
	inline constexpr std::uint32_t kUnknownGroup = 255;

	[[nodiscard]] bool IsDragonClawName(std::string_view name) noexcept;
	[[nodiscard]] bool IsIntrinsicExcluded(const RE::TESForm* item) noexcept;

	[[nodiscard]] std::uint32_t GroupFromFormType(RE::FormType formType) noexcept;
	[[nodiscard]] std::uint32_t DiscoveryGroupFor(const RE::TESForm* item, bool excluded) noexcept;

	[[nodiscard]] bool IsValidVariantStep(RE::FormID currentId, RE::FormID nextId) noexcept;
}
