#pragma once

#include <RE/Skyrim.h>

#include <mutex>
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
		bool carryWeightRecoveryUsed{ false };
	};

	[[nodiscard]] RuntimeState& GetState() noexcept;
}
