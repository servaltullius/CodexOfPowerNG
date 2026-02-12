#pragma once

#include <RE/Skyrim.h>

#include <cstdint>
#include <string>
#include <vector>

namespace CodexOfPowerNG::Registration
{
	struct ListItem
	{
		RE::FormID      formId{ 0 };
		RE::FormID      regKey{ 0 };
		std::uint32_t   group{ 255 };
		std::int32_t    totalCount{ 0 };
		std::int32_t    safeCount{ 0 };
		bool            excluded{ false };
		bool            registered{ false };
		bool            blocked{ false };
		std::string     name;
	};

	struct QuickRegisterList
	{
		bool                  hasMore{ false };
		std::size_t           total{ 0 };
		std::vector<ListItem> items;
	};

	struct RegisterResult
	{
		bool            success{ false };
		std::string     message;
		RE::FormID      regKey{ 0 };
		std::uint32_t   group{ 255 };
		std::size_t     totalRegistered{ 0 };
	};

	[[nodiscard]] std::uint32_t GetDiscoveryGroup(const RE::TESForm* item) noexcept;
	[[nodiscard]] std::string   GetDiscoveryGroupName(std::uint32_t group);

	// Normalization key (regKey) for a form ID (returns 0 on failure).
	[[nodiscard]] RE::FormID GetRegisterKeyId(RE::FormID formId) noexcept;

	// Returns whether a FormID is already registered (uses regKey normalization).
	[[nodiscard]] bool IsRegistered(RE::FormID formId) noexcept;

	// Returns whether a FormID is excluded/blocked (uses regKey normalization).
	[[nodiscard]] bool IsExcluded(RE::FormID formId) noexcept;

	// Returns whether a FormID is discoverable (group 0..5 and not excluded).
	[[nodiscard]] bool IsDiscoverable(RE::FormID formId) noexcept;

	// Quick-register inventory: unregistered + owned + registerable + safe to consume.
	[[nodiscard]] QuickRegisterList BuildQuickRegisterList(std::size_t offset, std::size_t limit);

	// Registered items view (discovery mode): state map keys -> names + groups.
	[[nodiscard]] std::vector<ListItem> BuildRegisteredList();

	// Registers an inventory item (consumes 1) and updates co-save state.
	[[nodiscard]] RegisterResult TryRegisterItem(RE::FormID formId);

	// Loads exclude/variant maps early (safe to call multiple times).
	void Warmup() noexcept;
}
