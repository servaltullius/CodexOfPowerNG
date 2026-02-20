#pragma once

#include "PrismaUIInternal.h"

#include <RE/Skyrim.h>

#include <optional>

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	struct InventoryRequest
	{
		std::uint32_t page{ 0 };
		std::uint32_t pageSize{ 200 };
	};

	[[nodiscard]] InventoryRequest ParseInventoryRequest(const char* argument) noexcept;
	[[nodiscard]] std::optional<RE::FormID> ParseFormIDFromJson(const json& j) noexcept;
	[[nodiscard]] std::optional<std::uint64_t> ParseActionIdFromJson(const json& j) noexcept;

	void QueueSendInventory(InventoryRequest req) noexcept;
	void QueueSendRegistered() noexcept;
	void QueueSendRewards() noexcept;
	void QueueSendUndoList() noexcept;

	void HandleRefundRewardsRequest() noexcept;
	void HandleRegisterItemRequest(const char* argument) noexcept;
	void HandleUndoRegisterRequest(const char* argument) noexcept;
}
