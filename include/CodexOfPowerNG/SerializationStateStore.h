#pragma once

#include "CodexOfPowerNG/BuildTypes.h"
#include "CodexOfPowerNG/RegistrationUndoTypes.h"
#include "CodexOfPowerNG/State.h"

#include <RE/Skyrim.h>

#include <array>
#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace CodexOfPowerNG::SerializationStateStore
{
	struct Snapshot
	{
		std::unordered_map<RE::FormID, std::uint32_t>             registeredItems;
		std::unordered_set<RE::FormID>                            blockedItems;
		std::unordered_set<RE::FormID>                            notifiedItems;
		std::unordered_map<RE::ActorValue, float, ActorValueHash> rewardTotals;
		std::unordered_map<RE::ActorValue, float, ActorValueHash> buildAppliedEffectTotals;
		std::uint32_t                                             attackScore{ 0 };
		std::uint32_t                                             defenseScore{ 0 };
		std::uint32_t                                             utilityScore{ 0 };
		Builds::BuildPointCenti                                  attackBuildPointsCenti{ 0 };
		Builds::BuildPointCenti                                  defenseBuildPointsCenti{ 0 };
		Builds::BuildPointCenti                                  utilityBuildPointsCenti{ 0 };
		std::array<std::string, Builds::kBuildSlotCount>          activeBuildSlots{};
		std::uint32_t                                             buildMigrationVersion{ 0 };
		Builds::BuildMigrationState                               buildMigrationState{ Builds::BuildMigrationState::kNotStarted };
		Builds::BuildMigrationNoticeSnapshot                      buildMigrationNotice{};
		std::deque<Registration::UndoRecord>                      undoHistory;
		std::uint64_t                                             undoNextActionId{ 1 };
	};

	[[nodiscard]] Snapshot SnapshotState() noexcept;
	void                   ReplaceState(Snapshot snapshot) noexcept;
	void                   Clear() noexcept;
}
