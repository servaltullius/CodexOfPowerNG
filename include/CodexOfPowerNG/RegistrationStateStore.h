#pragma once

#include <RE/Skyrim.h>

#include <cstdint>
#include <unordered_set>
#include <utility>
#include <vector>

namespace CodexOfPowerNG::RegistrationStateStore
{
	struct QuickListSnapshot
	{
		std::unordered_set<RE::FormID> blockedItems;
		std::unordered_set<RE::FormID> registeredKeys;
	};

	class IRegistrationStateStore
	{
public:
		virtual ~IRegistrationStateStore() = default;

		[[nodiscard]] virtual bool IsBlocked(RE::FormID formId) noexcept = 0;
		[[nodiscard]] virtual bool IsRegistered(RE::FormID formId) noexcept = 0;
		[[nodiscard]] virtual bool IsRegisteredEither(RE::FormID regKeyId, RE::FormID legacyId) noexcept = 0;

		virtual void              BlockPair(RE::FormID regKeyId, RE::FormID itemId) noexcept = 0;
		[[nodiscard]] virtual std::size_t InsertRegistered(RE::FormID regKeyId, std::uint32_t group) noexcept = 0;

		[[nodiscard]] virtual QuickListSnapshot SnapshotQuickList() noexcept = 0;
		[[nodiscard]] virtual std::vector<std::pair<RE::FormID, std::uint32_t>> SnapshotRegisteredItems() noexcept = 0;
	};

	[[nodiscard]] IRegistrationStateStore& GetStore() noexcept;
	void                                   SetStoreForTesting(IRegistrationStateStore* store) noexcept;

	[[nodiscard]] bool IsBlocked(RE::FormID formId) noexcept;
	[[nodiscard]] bool IsRegistered(RE::FormID formId) noexcept;
	[[nodiscard]] bool IsRegisteredEither(RE::FormID regKeyId, RE::FormID legacyId) noexcept;

	void BlockPair(RE::FormID regKeyId, RE::FormID itemId) noexcept;
	[[nodiscard]] std::size_t InsertRegistered(RE::FormID regKeyId, std::uint32_t group) noexcept;

	[[nodiscard]] QuickListSnapshot SnapshotQuickList() noexcept;
	[[nodiscard]] std::vector<std::pair<RE::FormID, std::uint32_t>> SnapshotRegisteredItems() noexcept;
}
