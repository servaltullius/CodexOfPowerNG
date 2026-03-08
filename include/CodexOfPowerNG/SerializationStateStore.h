#pragma once

#include "CodexOfPowerNG/RegistrationUndoTypes.h"
#include "CodexOfPowerNG/State.h"

#include <RE/Skyrim.h>

#include <cstdint>
#include <deque>
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
		std::deque<Registration::UndoRecord>                      undoHistory;
		std::uint64_t                                             undoNextActionId{ 1 };
	};

	[[nodiscard]] Snapshot SnapshotState() noexcept;
	void                   ReplaceState(Snapshot snapshot) noexcept;
	void                   Clear() noexcept;
}
