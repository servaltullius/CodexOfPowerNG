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

	void QueueSendInventory(InventoryRequest req) noexcept;
	void QueueSendRegistered() noexcept;
	void QueueSendRewards() noexcept;

	void HandleRefundRewardsRequest() noexcept;
	void HandleRecoverCarryWeightRequest() noexcept;
	void HandleRegisterItemRequest(const char* argument) noexcept;
}
