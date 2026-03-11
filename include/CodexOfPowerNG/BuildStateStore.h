#pragma once

#include "CodexOfPowerNG/BuildTypes.h"

#include <optional>
#include <string>

namespace CodexOfPowerNG::BuildStateStore
{
	[[nodiscard]] std::uint32_t GetAttackScore() noexcept;
	[[nodiscard]] std::uint32_t GetDefenseScore() noexcept;
	[[nodiscard]] std::uint32_t GetUtilityScore() noexcept;

	void SetAttackScore(std::uint32_t score) noexcept;
	void SetDefenseScore(std::uint32_t score) noexcept;
	void SetUtilityScore(std::uint32_t score) noexcept;

	[[nodiscard]] std::optional<std::string> GetActiveSlot(Builds::BuildSlotId slotId) noexcept;
	[[nodiscard]] bool                       SetActiveSlot(Builds::BuildSlotId slotId, std::string_view optionId) noexcept;
	void                                     ClearActiveSlot(Builds::BuildSlotId slotId) noexcept;
	void                                     ClearActiveSlots() noexcept;

	[[nodiscard]] Builds::BuildMigrationState          MigrationState() noexcept;
	[[nodiscard]] std::uint32_t                        MigrationVersion() noexcept;
	[[nodiscard]] Builds::BuildMigrationNoticeSnapshot GetMigrationNoticeSnapshot() noexcept;

	void SetMigrationState(Builds::BuildMigrationState state) noexcept;
	void SetMigrationVersion(std::uint32_t version) noexcept;
	void SetMigrationNoticeSnapshot(Builds::BuildMigrationNoticeSnapshot snapshot) noexcept;
}
