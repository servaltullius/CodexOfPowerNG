#pragma once

#include "CodexOfPowerNG/BuildTypes.h"
#include "CodexOfPowerNG/RegistrationUndoTypes.h"

#include <RE/Skyrim.h>

#include <array>
#include <deque>
#include <mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace CodexOfPowerNG
{
	struct ActorValueHash
	{
		std::size_t operator()(RE::ActorValue value) const noexcept
		{
			using Underlying = std::underlying_type_t<RE::ActorValue>;
			return std::hash<Underlying>{}(static_cast<Underlying>(value));
		}
	};

	struct RuntimeState
	{
		std::mutex mutex;
		// regKey(FormID) -> discovery group (0..5). Values may be 255 for "unknown" when loaded from older data.
		std::unordered_map<RE::FormID, std::uint32_t> registeredItems;
		std::unordered_set<RE::FormID> blockedItems;
		std::unordered_set<RE::FormID> notifiedItems;
		std::unordered_map<RE::ActorValue, float, ActorValueHash> rewardTotals;
		std::unordered_map<RE::ActorValue, float, ActorValueHash> buildAppliedEffectTotals;
		std::uint32_t                                        attackScore{ 0 };
		std::uint32_t                                        defenseScore{ 0 };
		std::uint32_t                                        utilityScore{ 0 };
		std::array<std::string, Builds::kBuildSlotCount>     activeBuildSlots{};
		std::uint32_t                                        buildMigrationVersion{ 0 };
		Builds::BuildMigrationState                          buildMigrationState{ Builds::BuildMigrationState::kNotStarted };
		Builds::BuildMigrationNoticeSnapshot                 buildMigrationNotice{};
		std::deque<Registration::UndoRecord> undoHistory;
		std::uint64_t                        undoNextActionId{ 1 };
	};

	[[nodiscard]] RuntimeState& GetState() noexcept;
}
