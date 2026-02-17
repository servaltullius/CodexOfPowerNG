#pragma once

#include "CodexOfPowerNG/Config.h"
#include "CodexOfPowerNG/RegistrationTccGate.h"

#include <RE/Skyrim.h>

#include <cstdint>
#include <string>

namespace CodexOfPowerNG::Registration::Internal
{
	struct TccLists
	{
		RE::BGSListForm* master{ nullptr };
		RE::BGSListForm* displayed{ nullptr };
	};

	[[nodiscard]] TccLists ResolveTccLists() noexcept;
	void                  WarnMissingTccListsOnce() noexcept;

	[[nodiscard]] TccGateDecision EvaluateTccGate(
		const Settings& settings,
		const TccLists& lists,
		const RE::TESForm* item,
		const RE::TESForm* regKey) noexcept;

	void EnsureMapsLoaded() noexcept;

	[[nodiscard]] bool IsExcludedByMap(RE::FormID formId) noexcept;
	[[nodiscard]] std::string BestItemName(const RE::TESForm* primary, const RE::TESForm* fallback);
	[[nodiscard]] RE::TESForm* GetRegisterKey(const RE::TESForm* item, const Settings& settings) noexcept;
	[[nodiscard]] RE::TESForm* GetRegisterKey(const RE::TESForm* item) noexcept;
	[[nodiscard]] bool IsExcludedForm(const RE::TESForm* item) noexcept;
	[[nodiscard]] bool IsRegisteredForm(const RE::TESForm* item) noexcept;
	[[nodiscard]] std::uint32_t ClampGroup(std::uint32_t group, const RE::TESForm* item) noexcept;
	[[nodiscard]] bool IsExcludedOrBlocked(const RE::TESForm* item, const RE::TESForm* regKey) noexcept;
	[[nodiscard]] bool IsTccDisplayedListsAvailable() noexcept;
}
