#pragma once

#include "PrismaUIInternal.h"

#include "CodexOfPowerNG/BuildTypes.h"

#include <RE/Skyrim.h>

#include <optional>
#include <string>
#include <vector>

namespace CodexOfPowerNG::PrismaUIManager::Internal
{
	struct InventoryRequest
	{
		std::uint32_t page{ 0 };
		std::uint32_t pageSize{ 200 };
	};

	struct BuildActivationRequest
	{
		std::string         optionId;
		Builds::BuildSlotId slotId{ Builds::BuildSlotId::Attack1 };
	};

	struct BuildDeactivationRequest
	{
		Builds::BuildSlotId slotId{ Builds::BuildSlotId::Attack1 };
	};

	struct BuildSwapRequest
	{
		std::string         optionId;
		Builds::BuildSlotId fromSlotId{ Builds::BuildSlotId::Attack1 };
		Builds::BuildSlotId toSlotId{ Builds::BuildSlotId::Attack1 };
	};

	struct RegisterBatchRequest
	{
		std::vector<RE::FormID> formIds;
	};

	enum class BuildMutationGuard : std::uint8_t
	{
		kAllowed,
		kInvalidPayload,
		kCombatLocked,
	};

	[[nodiscard]] InventoryRequest ParseInventoryRequest(const char* argument) noexcept;
	[[nodiscard]] std::optional<RE::FormID> ParseFormIDFromJson(const json& j) noexcept;
	[[nodiscard]] std::optional<std::uint64_t> ParseActionIdFromJson(const json& j) noexcept;
	[[nodiscard]] std::optional<Builds::BuildSlotId> ParseBuildSlotId(std::string_view raw) noexcept;
	[[nodiscard]] std::optional<BuildActivationRequest> ParseBuildActivateRequest(const json& payload) noexcept;
	[[nodiscard]] std::optional<BuildDeactivationRequest> ParseBuildDeactivateRequest(const json& payload) noexcept;
	[[nodiscard]] std::optional<BuildSwapRequest> ParseBuildSwapRequest(const json& payload) noexcept;
	[[nodiscard]] std::optional<RegisterBatchRequest> ParseRegisterBatchRequest(const json& payload) noexcept;
	[[nodiscard]] BuildMutationGuard ValidateBuildMutationRequest(bool inCombat, bool payloadValid) noexcept;

	void QueueSendInventory(InventoryRequest req) noexcept;
	void QueueSendRegistered() noexcept;
	void QueueSendRewards() noexcept;
	void QueueSendBuild() noexcept;
	void QueueSendUndoList() noexcept;
	void FlushPendingUIRefresh() noexcept;

	void HandleRefundRewardsRequest() noexcept;
	void HandleRegisterItemRequest(const char* argument) noexcept;
	void HandleRequestBuild() noexcept;
	void HandleRegisterBatchRequest(const char* argument) noexcept;
	void HandleActivateBuildOptionRequest(const char* argument) noexcept;
	void HandleDeactivateBuildOptionRequest(const char* argument) noexcept;
	void HandleSwapBuildOptionRequest(const char* argument) noexcept;
	void HandleUndoRegisterRequest(const char* argument) noexcept;
}
