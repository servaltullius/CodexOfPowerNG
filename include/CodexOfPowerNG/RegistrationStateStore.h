#pragma once

#include "CodexOfPowerNG/RegistrationUndoTypes.h"

#include <RE/Skyrim.h>

#include <cstdint>
#include <optional>
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
		[[nodiscard]] virtual bool        RemoveRegistered(RE::FormID regKeyId) noexcept = 0;

		[[nodiscard]] virtual std::uint64_t PushUndoRecord(Registration::UndoRecord record) noexcept = 0;
		[[nodiscard]] virtual std::optional<Registration::UndoRecord> PopLatestUndoRecord(std::uint64_t actionId) noexcept = 0;
		virtual void                                               RestoreUndoRecord(Registration::UndoRecord record) noexcept = 0;
		[[nodiscard]] virtual std::vector<Registration::UndoRecord> SnapshotUndoRecords(std::size_t limit) noexcept = 0;

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
	[[nodiscard]] bool        RemoveRegistered(RE::FormID regKeyId) noexcept;

	[[nodiscard]] std::uint64_t PushUndoRecord(Registration::UndoRecord record) noexcept;
	[[nodiscard]] std::optional<Registration::UndoRecord> PopLatestUndoRecord(std::uint64_t actionId) noexcept;
	void                                                  RestoreUndoRecord(Registration::UndoRecord record) noexcept;
	[[nodiscard]] std::vector<Registration::UndoRecord>   SnapshotUndoRecords(std::size_t limit) noexcept;

	[[nodiscard]] QuickListSnapshot SnapshotQuickList() noexcept;
	[[nodiscard]] std::vector<std::pair<RE::FormID, std::uint32_t>> SnapshotRegisteredItems() noexcept;
}
